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


#if 0

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
    // TODO: optimize errorstring if it is too slow
    cerr << "Error: " << operation << " (" << errorString << ")" << endl;
  }
  

  SampleTable loadAudioFile(string path, int numChannels){
  
    static const int BYTESPERSAMPLE = sizeof(TonicFloat);
    
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
    outputFormat.mBytesPerPacket = BYTESPERSAMPLE * numChannels;
    outputFormat.mFramesPerPacket = 1;
    outputFormat.mBytesPerFrame = BYTESPERSAMPLE * numChannels;
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
    convertedData.mBuffers[0].mDataByteSize = (UInt32)destinationTable.size() * BYTESPERSAMPLE;
    convertedData.mBuffers[0].mData = destinationTable.dataPointer();
    
    UInt32 numFrames32 = (UInt32)numFrames;
    ExtAudioFileRead(inputFile, &numFrames32, &convertedData);
    
    ExtAudioFileDispose(inputFile);
    
    return destinationTable;
  }
  
#else

  void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vargs)
  {
    if (level <= av_log_get_level())
    {
      const int buffer_size = 300;
      static char buffer[buffer_size];
#ifdef _WIN32
      vsprintf_s(buffer, buffer_size, fmt, vargs);
#else
      vsprintf(buffer, fmt, vargs);
#endif
      // Erase \n on the end of the ffmpeg's error string
      auto len = strlen(buffer);
      buffer[len - 1] = '\0';
      cerr << "FFmpeg | " << buffer << endl;
    }
  }

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

#ifndef FF_API_OLD_CHANNEL_LAYOUT
  const AVChannelLayout* getChannelLayout2(unsigned numChannels)
  {
    constexpr AVChannelLayout layouts[] = {
      AV_CHANNEL_LAYOUT_MONO,
      AV_CHANNEL_LAYOUT_STEREO
    };
    if (numChannels > sizeof(layouts) / sizeof(AVChannelLayout))
    {
      cerr << numChannels << " not supported" << endl;
      numChannels = 1;
    }
    return &layouts[numChannels - 1];
  }
#endif

  int decode(AVCodecContext* decCtx, AVPacket* pkt, AVFrame* frame, SwrContext* swr, int channels, TonicFloat* decodeBuffer) {
    int i, ch;
    int ret, dataSize;
    int framesCount;

    // Send the packet with the compressed data to the decoder
    ret = avcodec_send_packet(decCtx, pkt);
    if (ret < 0) {
      // Just wait for the next valid packet. https://github.com/bytedeco/javacv/issues/1679#issuecomment-892606462
      cerr << "Error submitting the packet to the decoder" << endl;
      return 0;
    }

    // Read all the output frames - in general there may be more than one
    int numFramesTotal = 0;
    while (true) {
      ret = avcodec_receive_frame(decCtx, frame);
      if (ret == AVERROR(EAGAIN)) {
        //cerr << "EAGAIN" << endl;
        return numFramesTotal;
      }
      else if (ret == AVERROR_EOF) {
        //cerr << "EOF" << endl;
        return numFramesTotal;
      }
      else if (ret < 0) {
        cerr << "Error during decoding" << endl;
        return -1;
      }
      dataSize = av_get_bytes_per_sample(decCtx->sample_fmt);

      // Resample frames
      framesCount = swr_convert(swr,
                              (uint8_t**)&decodeBuffer, frame->nb_samples,      // out
                              (const uint8_t**)frame->data, frame->nb_samples); // in
      if (framesCount > 0) {
        int samplesCount = framesCount * channels;
        decodeBuffer += samplesCount;
        numFramesTotal += framesCount;
      }
    }
  }
  
  SampleTable loadAudioFile(string path, int numChannels) {
    const AVCodec* codec;
    AVCodecContext* codecCtx = NULL;
    int ret;
    
    av_log_set_level(AV_LOG_ERROR);
    av_log_set_callback(ffmpeg_log_callback);
    
    AVPacket* pkt = av_packet_alloc();

    // Get format from audio file
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

    // Setup resampling to the FP32 format
    const int resampleSampleRate = Tonic::sampleRate();
    const int resampleChannelCount = numChannels;
    const int resampleChannelLayout = getChannelLayout(numChannels);
    const AVSampleFormat resampleSampleFmt = AV_SAMPLE_FMT_FLT;

#if FF_API_OLD_CHANNEL_LAYOUT
    SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count", codecCtx->channels, 0);
    av_opt_set_int(swr, "out_channel_count", resampleChannelCount, 0);
    av_opt_set_int(swr, "in_channel_layout", codecCtx->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", resampleChannelLayout, 0);
    av_opt_set_int(swr, "in_sample_rate", codecCtx->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", resampleSampleRate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt", codecCtx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", resampleSampleFmt, 0);
#else
    SwrContext* swr = nullptr;
    if (swr_alloc_set_opts2(&swr, getChannelLayout2(numChannels), resampleSampleFmt, 
                        resampleSampleRate, &codecCtx->ch_layout,
                        codecCtx->sample_fmt, codecCtx->sample_rate, 0, nullptr))
    {
        cerr << "Failed to alloc and setup resampler" << endl;
        return NULL;
    }
#endif                   
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

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
      cerr << "Error allocating the frame" << endl;
      return NULL;
    }

    int totalDecodedFrames = 0;
    while (av_read_frame(format, pkt) >= 0) {
      // Decode audio frames one by one
      if (pkt->size) {
        auto decodedFrames = decode(codecCtx, pkt, frame, swr, numChannels, decodeDataPtr);
        if (decodedFrames < 0) {
          cerr << "Error decoding audio frames" << endl;
          return NULL;
        }
        int decodedSamples = decodedFrames * numChannels;
        decodeDataPtr += decodedSamples;
        totalDecodedFrames += decodedFrames;
        //cerr << "decoded frames: " << decodedFrames << "; decodedSamples: " << decodedSamples << endl;
      }
    }
    cerr << "loadAudioFile | totalDecodedFrames: " << totalDecodedFrames << "; numFrames: " << numFrames << endl; 
    // totalDecodedFrames: 661426; numFrames: 663552
    // Shrink the sample table to actual size
    if (totalDecodedFrames < numFrames) {
      destinationTable.resize(totalDecodedFrames, numChannels);
    }

    // Flush the decoder
    pkt->data = NULL;
    pkt->size = 0;
    decode(codecCtx, pkt, frame, swr, numChannels, decodeDataPtr);

    // Cleanup
    av_packet_free(&pkt);
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_close(codecCtx);
    avformat_free_context(format);

    return destinationTable;  
  }
#endif
}
