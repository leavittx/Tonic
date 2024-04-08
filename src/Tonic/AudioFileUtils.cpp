//
//  AudioFileUtils.cpp
//  TonicLib
//
//  Created by Morgan Packard on 10/26/13.
//  Copyright (c) 2013 Nick Donaldson. All rights reserved.
//

#include "AudioFileUtils.h"

#ifdef __APPLE__
  #include <AudioToolbox/AudioToolbox.h>
#endif

extern "C"
{
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libavutil/avutil.h>
  #include <libavutil/opt.h>
  #include <libswresample/swresample.h>
}


namespace Tonic {


#ifdef __APPLE__

  void checkCAError(OSStatus error, const char *operation){
    if (error == noErr) return;
    char errorString[20];
    // See if it appears to be a 4-char-code
    *(UInt32 *)(errorString + 1) = CFSwapInt32HostToBig(error);
    if (isprint(errorString[1]) && isprint(errorString[2]) &&
        isprint(errorString[3]) && isprint(errorString[4])) {
        errorString[0] = errorString[5] = '\'';
        errorString[6] = '\0';
    } else {
        // No, format it as an integer
        sprintf(errorString, "%d", (int)error);
    }
    fprintf(stderr, "Error: %s (%s)\n", operation, errorString);
  }
  

  SampleTable loadAudioFile(string path, int numChannels){
  
    static const int BYTES_PER_SAMPLE = sizeof(TonicFloat);
    
    // Get the file handle
    ExtAudioFileRef inputFile;
    CFStringRef cfStringRef; 
    cfStringRef = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingMacRoman);
    CFURLRef inputFileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfStringRef, kCFURLPOSIXPathStyle, false);
    CFRelease(cfStringRef);
    
    checkCAError(ExtAudioFileOpenURL(inputFileURL,  &inputFile), "ExtAudioFileOpenURL failed");
    CFRelease(inputFileURL);
    
    // Define the format for the data we want to extract from the audio file
    AudioStreamBasicDescription outputFormat;
    memset(&outputFormat, 0, sizeof(outputFormat));
    outputFormat.mSampleRate = 44100.0;
    outputFormat.mFormatID = kAudioFormatLinearPCM;
    outputFormat.mFormatFlags = kAudioFormatFlagIsFloat;
    outputFormat.mBytesPerPacket = BYTES_PER_SAMPLE * numChannels;
    outputFormat.mFramesPerPacket = 1;
    outputFormat.mBytesPerFrame = BYTES_PER_SAMPLE * numChannels;
    outputFormat.mChannelsPerFrame = numChannels;
    outputFormat.mBitsPerChannel = 32;
    OSStatus error = ExtAudioFileSetProperty(inputFile, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription), &outputFormat);
    checkCAError(error, "Error setting kExtAudioFileProperty_ClientDataFormat.");

    // Determine the length of the file, in frames
    SInt64 numFrames;
    UInt32 intSize = sizeof(SInt64);
    error = ExtAudioFileGetProperty(inputFile, kExtAudioFileProperty_FileLengthFrames, &intSize, &numFrames);
    checkCAError(error, "Error reading number of frames.");
    
    // change sampleTable numframes to long long
    SampleTable destinationTable = SampleTable((int)numFrames, numChannels);
    
    // wrap the destination buffer in an AudioBufferList
    AudioBufferList convertedData;
    convertedData.mNumberBuffers = 1;
    convertedData.mBuffers[0].mNumberChannels = outputFormat.mChannelsPerFrame;
    convertedData.mBuffers[0].mDataByteSize = (UInt32)destinationTable.size() * BYTES_PER_SAMPLE;
    convertedData.mBuffers[0].mData = destinationTable.dataPointer();
    
    UInt32 numFrames32 = (UInt32)numFrames;
    ExtAudioFileRead(inputFile, &numFrames32, &convertedData);
    
    ExtAudioFileDispose(inputFile);
    
    return destinationTable;
  }
  
#else

  static int decode(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, SwrContext* swr, TonicFloat* decodeBuffer) {
    //static std::vector<float> data;
    int i, ch;
    int ret, data_size;
    int frame_count;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
      fprintf(stderr, "Error submitting the packet to the decoder\n");
      return -1;
    }

    /* read all the output frames (in general there may be any number of them */
    int data_size_total = 0;
    while (ret >= 0) {
      ret = avcodec_receive_frame(dec_ctx, frame);
      if (ret == AVERROR(EAGAIN)) {
        fprintf(stderr, "EAGAIN\n");
        return data_size_total;
      }
      else if (ret == AVERROR_EOF) {
        fprintf(stderr, "EOF\n");
        return data_size_total;
      }
      else if (ret < 0) {
        fprintf(stderr, "Error during decoding\n");
        return -1;
      }
      data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
      if (data_size < 0) {
        /* This should not occur, checking just for paranoia */
        fprintf(stderr, "Failed to calculate data size\n");
        return -1;
      }

      // resample frames
      double* buffer;
      //av_samples_alloc((uint8_t**)&budffer, NULL, 1, frame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
      frame_count = swr_convert(swr, 
                                    (uint8_t**)&decodeBuffer, frame->nb_samples, // out
                                    (const uint8_t**)frame->data, frame->nb_samples); // in
      if (frame_count > 0) {
        decodeBuffer += frame_count;
        data_size_total += frame_count;
      }
      // append resampled frames to data
      //*data = (*)realloc(*data, (*size + frame->nb_samples) * sizeof(double));
      //memcpy(*data + *size, buffer, frame_count * sizeof(double));
      //*size += frame_count;
      //for (i = 0; i < frame->nb_samples; i++) {
      //  for (ch = 0; ch < dec_ctx->ch_layout.nb_channels; ch++) {
      //    memcpy(decodeBuffer, frame->data[ch] + data_size * i, data_size);
      //    data_size_total += data_size;
      //  }
      //}
    }
    return frame_count;
  }
  
  SampleTable loadAudioFile(string path, int numChannels) {
    const AVCodec* codec;
    AVCodecContext* codecCtx = NULL;
    int len, ret;
    AVPacket* pkt;
    enum AVSampleFormat sfmt;
    int n_channels = 0;
    const char* fmt;

    pkt = av_packet_alloc();

     // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path.data(), NULL, NULL) != 0) {
      cerr << "Could not open file " << path.data();
      return NULL;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
      cerr << "Could not retrieve stream info from file " << path.data();
      return NULL;
    }

    // Find the index of the first audio stream
    int stream_index = -1;
    for (int i = 0; i < format->nb_streams; i++) {
      if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        stream_index = i;
        break;
      }
    }
    if (stream_index == -1) {
      fprintf(stderr, "Could not retrieve audio stream from file '%s'\n", path.data());
      return NULL;
    }
    AVStream* stream = format->streams[stream_index];

    // find & open codec
    codecCtx = avcodec_alloc_context3(nullptr);
    if (!codecCtx)
      return AVERROR(ENOMEM);

    /// FIXME: not needed
    ret = avcodec_parameters_to_context(codecCtx, stream->codecpar);
    if (ret < 0)
      return NULL;
    codecCtx->pkt_timebase = stream->time_base;

    codec = avcodec_find_decoder(codecCtx->codec_id);

    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
      fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n", stream_index, path.data());
      return NULL;
    }

    // Force decoding to float
    const int tonic_sample_rate = 44100;
    const int tonic_channel_count = 2;
    const int tonic_channel_layout = AV_CH_LAYOUT_STEREO;
    const AVSampleFormat tonic_sample_fmt = AV_SAMPLE_FMT_FLT; /// TODO: or AV_SAMPLE_FMT_FLTP?
    SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count", codecCtx->channels, 0);
    av_opt_set_int(swr, "out_channel_count", tonic_channel_count, 0);
    av_opt_set_int(swr, "in_channel_layout", codecCtx->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", tonic_channel_layout, 0);
    av_opt_set_int(swr, "in_sample_rate", codecCtx->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", tonic_sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", codecCtx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", tonic_sample_fmt, 0); 
    swr_init(swr);
    if (!swr_is_initialized(swr)) {
      fprintf(stderr, "Resampler has not been properly initialized\n");
      return NULL;
    }

    float duration = static_cast<float>(format->duration) / AV_TIME_BASE;
    int numFrames = static_cast<int>(codecCtx->sample_rate * duration);
    SampleTable destinationTable = SampleTable(numFrames, numChannels);
    TonicFloat* decodeDataPtr = destinationTable.dataPointer();
    if (decodeDataPtr == nullptr) {
      cerr << "decodeDataPtr is nullptr" << endl;
      return NULL;
    }
    //destinationTable.resize();

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
      fprintf(stderr, "Error allocating the frame\n");
      return NULL;
    }

    while (av_read_frame(format, pkt) >= 0) {
      // decode one frame
      if (pkt->size) {
        auto decodedSize = decode(codecCtx, pkt, frame, swr, decodeDataPtr);
        if (decodedSize < 0) {
          return NULL;
        }
        decodeDataPtr += decodedSize;
      }
    }

    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    decode(codecCtx, pkt, frame, swr, decodeDataPtr);

    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_close(codecCtx);
    avformat_free_context(format);

    return destinationTable;
  }
#endif
}
