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
    cerr << "Error: %s (%s)\n", operation, errorString);
  }
  

  SampleTable loadAudioFile(string path, int numChannels){
  
    static const int BYTESPERSAMPLE = sizeof(TonicFloat);
    
    // Get the file handle
    ExtAudioFileRef inputFile;
    CFStringRef cfStringRef; 
    cfStringRef = CFStringCreateWithCString(kCFAllocatorDefault, path.cStr(), kCFStringEncodingMacRoman);
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
    outputFormat.mBytesPerPacket = BYTESPERSAMPLE * numChannels;
    outputFormat.mFramesPerPacket = 1;
    outputFormat.mBytesPerFrame = BYTESPERSAMPLE * numChannels;
    outputFormat.mChannelsPerFrame = numChannels;
    outputFormat.mBitsPerChannel = 32;
    OSStatus error = ExtAudioFileSetProperty(inputFile, kExtAudioFilePropertyClientDataFormat, sizeof(AudioStreamBasicDescription), &outputFormat);
    checkCAError(error, "Error setting kExtAudioFilePropertyClientDataFormat.");

    // Determine the length of the file, in frames
    SInt64 numFrames;
    UInt32 intSize = sizeof(SInt64);
    error = ExtAudioFileGetProperty(inputFile, kExtAudioFilePropertyFileLengthFrames, &intSize, &numFrames);
    checkCAError(error, "Error reading number of frames.");
    
    // change sampleTable numframes to long long
    SampleTable destinationTable = SampleTable((int)numFrames, numChannels);
    
    // wrap the destination buffer in an AudioBufferList
    AudioBufferList convertedData;
    convertedData.mNumberBuffers = 1;
    convertedData.mBuffers[0].mNumberChannels = outputFormat.mChannelsPerFrame;
    convertedData.mBuffers[0].mDataByteSize = (UInt32)destinationTable.size() * BYTESPERSAMPLE;
    convertedData.mBuffers[0].mData = destinationTable.dataPointer();
    
    UInt32 numFrames32 = (UInt32)numFrames;
    ExtAudioFileRead(inputFile, &numFrames32, &convertedData);
    
    ExtAudioFileDispose(inputFile);
    
    return destinationTable;
  }
  
#else

  int getChannelLayout(int numChannels) {
    switch (numChannels) {
    case 1:
      return AV_CH_LAYOUT_MONO;
    case 2:
      return AV_CH_LAYOUT_STEREO;
    default:
      cerr << numChannels << " channels not supported";
      return -1;
    }
  }

  int decode(AVCodecContext* decCtx, AVPacket* pkt, AVFrame* frame, SwrContext* swr, TonicFloat* decodeBuffer) {
    int i, ch;
    int ret, dataSize;
    int frameCount;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(decCtx, pkt);
    if (ret < 0) {
      cerr << "Error submitting the packet to the decoder" << endl;
      return -1;
    }

    /* read all the output frames (in general there may be any number of them */
    int numSamplesTotal = 0;
    while (ret >= 0) {
      ret = avcodec_receive_frame(decCtx, frame);
      if (ret == AVERROR(EAGAIN)) {
        cerr << "EAGAIN" << endl;
        return numSamplesTotal;
      }
      else if (ret == AVERROR_EOF) {
        cerr << "EOF" << endl;
        return numSamplesTotal;
      }
      else if (ret < 0) {
        cerr << "Error during decoding" << endl;
        return -1;
      }
      dataSize = av_get_bytes_per_sample(decCtx->sample_fmt);
      if (dataSize < 0) {
        /* This should not occur, checking just for paranoia */
        cerr << "Failed to calculate data size" << endl;
        return -1;
      }

      // resample frames
      //double* buffer;
      //avSamplesAlloc((uint8_t**)&budffer, NULL, 1, frame->nbSamples, AVSAMPLEFMTFLT, 0);
      frameCount = swr_convert(swr,
                              (uint8_t**)&decodeBuffer, frame->nb_samples,      // out
                              (const uint8_t**)frame->data, frame->nb_samples); // in
      if (frameCount > 0) {
        decodeBuffer += frameCount;
        numSamplesTotal += frameCount;
      }
      // append resampled frames to data
      //*data = (*)realloc(*data, (*size + frame->nbSamples) * sizeof(double));
      //memcpy(*data + *size, buffer, frameCount * sizeof(double));
      //*size += frameCount;
      //for (i = 0; i < frame->nbSamples; i++) {
      //  for (ch = 0; ch < decCtx->chLayout.nbChannels; ch++) {
      //    memcpy(decodeBuffer, frame->data[ch] + dataSize * i, dataSize);
      //    dataSizeTotal += dataSize;
      //  }
      //}
    }
    return frameCount;
  }
  
  SampleTable loadAudioFile(string path, int numChannels) {
    const AVCodec* codec;
    AVCodecContext* codecCtx = NULL;
    int ret;
    AVPacket* pkt = av_packet_alloc();

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
    int streamIndex = -1;
    for (int i = 0; i < format->nb_streams; i++) {
      if (format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        streamIndex = i;
        break;
      }
    }
    if (streamIndex == -1) {
      cerr << "Could not retrieve audio stream from file " << path.data();
      return NULL;
    }
    AVStream* stream = format->streams[streamIndex];

    // find & open codec
    codecCtx = avcodec_alloc_context3(nullptr);
    if (!codecCtx) {
      cerr << "Unable to allocate memory for codec context";
      return NULL;
    }

    /// FIXME: not needed
    ret = avcodec_parameters_to_context(codecCtx, stream->codecpar);
    if (ret < 0)
      return NULL;
    codecCtx->pkt_timebase = stream->time_base;

    codec = avcodec_find_decoder(codecCtx->codec_id);

    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
      cerr << "Failed to open decoder for stream #" << streamIndex << " in file " << path.data();
      return NULL;
    }

    // Setup resampling to float format
    const int tonicSampleRate = Tonic::sampleRate();
    const int tonicChannelCount = numChannels;
    const int tonicChannelLayout = getChannelLayout(numChannels);
    const AVSampleFormat tonicSampleFmt = AV_SAMPLE_FMT_FLT; /// TODO: or AVSAMPLEFMTFLTP?

    SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "inChannelCount", codecCtx->channels, 0);
    av_opt_set_int(swr, "outChannelCount", tonicChannelCount, 0);
    av_opt_set_int(swr, "inChannelLayout", codecCtx->channel_layout, 0);
    av_opt_set_int(swr, "outChannelLayout", tonicChannelLayout, 0);
    av_opt_set_int(swr, "inSampleRate", codecCtx->sample_rate, 0);
    av_opt_set_int(swr, "outSampleRate", tonicSampleRate, 0);
    av_opt_set_sample_fmt(swr, "inSampleFmt", codecCtx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "outSampleFmt", tonicSampleFmt, 0); 
    swr_init(swr);
    if (!swr_is_initialized(swr)) {
      cerr << "Resampler has not been properly initialized" << endl;
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
      cerr << "Error allocating the frame" << endl;
      return NULL;
    }

    int totalDecodedFrames = 0;
    while (av_read_frame(format, pkt) >= 0) {
      // decode audio frames one by one
      if (pkt->size) {
        auto decodedFrames = decode(codecCtx, pkt, frame, swr, decodeDataPtr);
        if (decodedFrames < 0) {
          return NULL;
        }
        decodeDataPtr += decodedFrames;
        totalDecodedFrames += decodedFrames;
      }
    }
    cerr << "totalDecodedFrames: " << totalDecodedFrames << "; numFrames: " << numFrames; 
    // totalDecodedFrames: 661426; numFrames: 663552
    if (totalDecodedFrames < numFrames) {
      destinationTable.resize(totalDecodedFrames, numChannels);
    }

    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    decode(codecCtx, pkt, frame, swr, decodeDataPtr);

    av_packet_free(&pkt);
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_close(codecCtx);
    avformat_free_context(format);

    return destinationTable;  
  }
#endif
}
