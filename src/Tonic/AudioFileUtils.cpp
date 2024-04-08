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
}

#define MAX_AUDIO_FRAME_SIZE 192000 

#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

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

  static int decode(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, uint8_t* decodeBuffer) {
    int i, ch;
    int ret, data_size;

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
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        break;
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
      for (i = 0; i < frame->nb_samples; i++) {
        for (ch = 0; ch < dec_ctx->ch_layout.nb_channels; ch++) {
          memcpy(decodeBuffer, frame->data[ch] + data_size * i, data_size);
          data_size_total += data_size;
        }
      }
    }
    return data_size_total;
  }
  
  SampleTable loadAudioFile(string path, int numChannels) {
#if 1
    const AVCodec* codec;
    AVCodecContext* codecCtx = NULL;
    AVCodecParserContext* parser = NULL;
    int len, ret;
    uint8_t* data;
    size_t   data_size;
    AVPacket* pkt;
    enum AVSampleFormat sfmt;
    int n_channels = 0;
    const char* fmt;

    pkt = av_packet_alloc();

    /// TODO: guess the format
    //codec = avcodec_find_decoder(AV_CODEC_ID_MP3);
    //if (!codec) {
    //  fprintf(stderr, "Codec not found\n");
    //  return NULL;
    //}

     // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path.data(), NULL, NULL) != 0) {
      fprintf(stderr, "Could not open file '%s'\n", path.data());
      return NULL;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
      fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path.data());
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

    ret = avcodec_parameters_to_context(codecCtx, stream->codecpar);
    if (ret < 0)
      return NULL;
    codecCtx->pkt_timebase = stream->time_base;

    codec = avcodec_find_decoder(codecCtx->codec_id);


    //if (avcodec_open2(codecCtx, avcodec_find_decoder(codecCtx->codec_id), NULL) < 0) {
    //  fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n", stream_index, path);
    //  return NULL;
    //}

    parser = av_parser_init(codec->id);
    if (!parser) {
      fprintf(stderr, "Parser not found\n");
      return NULL;
    }

    //codecCtx = avcodec_alloc_context3(codec);
    //if (!codecCtx) {
    //  fprintf(stderr, "Could not allocate audio codec context\n");
    //  return NULL;
    //}

    /* open it */
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
      fprintf(stderr, "Could not open codec\n");
      return NULL;
    }

    /* decode until eof */
    data = inbuf;

    float duration = static_cast<float>(format->duration) / AV_TIME_BASE;
    int numFrames = static_cast<int>(codecCtx->sample_rate * duration);
    SampleTable destinationTable = SampleTable(numFrames, numChannels);
    // TODO: force decoding to float
    uint8_t* decodeDataPtr = reinterpret_cast<uint8_t*>(destinationTable.dataPointer());
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

    // iterate through frames
    *data = NULL;
    //*size = 0;
    while (av_read_frame(format, pkt) >= 0) {
      // decode one frame
#if 1
      if (pkt->size) {
        auto decodedSize = decode(codecCtx, pkt, frame, decodeDataPtr);
        if (decodedSize < 0) {
          return NULL;
        }
        decodeDataPtr += decodedSize;
      }
#else
      int gotFrame;
      if (avcodec_decode_audio4(codec, frame, &gotFrame, &pkt) < 0) {
        break;
      }
      if (!gotFrame) {
        continue;
      }

      // resample frames
      double* buffer;
      av_samples_alloc((uint8_t**)&buffer, NULL, 1, frame->nb_samples, AV_SAMPLE_FMT_DBL, 0);
      int frame_count = swr_convert(swr, (uint8_t**)&buffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
      // append resampled frames to data
      *data = (double*)realloc(*data, (*size + frame->nb_samples) * sizeof(double));
      memcpy(*data + *size, buffer, frame_count * sizeof(double));
      *size += frame_count;
#endif
    }

    //while (data_size > 0) {
    //  if (!decoded_frame) {
    //    if (!(decoded_frame = av_frame_alloc())) {
    //      fprintf(stderr, "Could not allocate audio frame\n");
    //      return NULL;
    //    }
    //  }

    //  ret = av_parser_parse2(parser, codecCtx, &pkt->data, &pkt->size,
    //                         data, data_size,
    //                         AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    //  if (ret < 0) {
    //    fprintf(stderr, "Error while parsing\n");
    //    return NULL;
    //  }
    //  data += ret;
    //  data_size -= ret;

    //  if (pkt->size) {
    //    auto decodedSize = decode(codecCtx, pkt, decoded_frame, decodeDataPtr);
    //    if (decodedSize < 0) {
    //      return NULL;
    //    }
    //    decodeDataPtr += decodedSize;
    //  }

    //  if (data_size < AUDIO_REFILL_THRESH) {
    //    memmove(inbuf, data, data_size);
    //    data = inbuf;
    //    len = fread(data + data_size, 1, AUDIO_INBUF_SIZE - data_size, f);
    //    if (len > 0)
    //      data_size += len;
    //  }
    //}

    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    decode(codecCtx, pkt, frame, decodeDataPtr);

#else
    AVCodec* codec;
    AVCodecContext* c = NULL;
    int len;
    FILE* f;
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    AVPacket avpkt;
    AVFrame* decoded_frame = NULL;

    bool firstFrame = true;

    av_init_packet(&avpkt);

    /* find the mpeg audio decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_MP3);
    if (!codec) {
      fprintf(stderr, "Codec not found\n");
      return NULL;
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
      fprintf(stderr, "Could not allocate audio codec context\n");
      return NULL;
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
      fprintf(stderr, "Could not open codec\n");
      return NULL;
    }

    f = fopen(path.data(), "rb");
    if (!f) {
      cerr << "Could not open " << path;
      return NULL;
    }

    /* decode until eof */
    avpkt.data = inbuf;
    avpkt.size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);

    while (avpkt.size > 0) {
      int got_frame = 0;

      if (!decoded_frame) {
        if (!(decoded_frame = av_frame_alloc())) {
          fprintf(stderr, "Could not allocate audio frame\n");
          return NULL;
        }
      }
      else
        av_frame_unref(decoded_frame);

      len = avcodec_decode_audio4(c, decoded_frame, &got_frame, &avpkt);
      if (len < 0) {
        fprintf(stderr, "Error while decoding\n");
        return NULL;
      }
      if (got_frame) {
        if (firstFrame) {
          ao_sample_format sample_format;

          if (c->sample_fmt == AV_SAMPLE_FMT_U8) {
            sample_format.bits = 8;
          }
          else if (c->sample_fmt == AV_SAMPLE_FMT_S16) {
            sample_format.bits = 16;
          }
          else if (c->sample_fmt == AV_SAMPLE_FMT_S16P) {
            sample_format.bits = 16;
          }
          else if (c->sample_fmt == AV_SAMPLE_FMT_S32) {
            sample_format.bits = 32;
          }

          sample_format.channels = c->channels;
          sample_format.rate = c->sample_rate;
          sample_format.byte_format = AO_FMT_NATIVE;
          sample_format.matrix = 0;

          firstFrame = false;
        }

        /* if a frame has been decoded, output it */
        int data_size = av_samples_get_buffer_size(NULL, c->channels,
          decoded_frame->nb_samples,
          c->sample_fmt, 1);

        // Send the buffer contents to the audio device
        //ao_play(device, (char*)decoded_frame->data[0], data_size);
      }
      avpkt.size -= len;
      avpkt.data += len;
      avpkt.dts =
        avpkt.pts = AV_NOPTS_VALUE;
      if (avpkt.size < AUDIO_REFILL_THRESH) {
        /* Refill the input buffer, to avoid trying to decode
         * incomplete frames. Instead of this, one could also use
         * a parser, or use a proper container format through
         * libavformat. */
        memmove(inbuf, avpkt.data, avpkt.size);
        avpkt.data = inbuf;
        len = fread(avpkt.data + avpkt.size, 1,
          AUDIO_INBUF_SIZE - avpkt.size, f);
        if (len > 0)
          avpkt.size += len;
      }
    }

    fclose(f);

    avcodec_close(c);
    av_free(c);
    av_frame_free(&decoded_frame);
#endif
  }
  #endif
}
