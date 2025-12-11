//
//  AudioFileUtils.cpp
//  TonicLib
//

#include "AudioFileUtils.h"

extern "C"
{
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libavutil/avutil.h>
  #include <libavutil/opt.h>
  #include <libswresample/swresample.h>
}

namespace Tonic {
  void ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vargs)
  {
    if (level <= av_log_get_level())
    {
      const int buffer_size = 4096;
      static char buffer[buffer_size];
#ifdef _WIN32
      vsprintf_s(buffer, buffer_size, fmt, vargs);
#else
      vsprintf(buffer, fmt, vargs);
#endif
      // Erase \n on the end of the ffmpeg's error string
      auto len = strlen(buffer);
      buffer[len - 1] = '\0';
      std::cerr << "FFmpeg | " << buffer << std::endl;
    }
  }

  const AVChannelLayout* getChannelLayout2(unsigned numChannels)
  {
    static const AVChannelLayout layouts[] = {
      AV_CHANNEL_LAYOUT_MONO,
      AV_CHANNEL_LAYOUT_STEREO
    };
    if (numChannels > sizeof(layouts) / sizeof(AVChannelLayout))
    {
      std::cerr << numChannels << " output channels not supported" << std::endl;
      return nullptr;
    }
    return &layouts[numChannels - 1];
  }

  int decode(AVCodecContext* decCtx, AVPacket* pkt, AVFrame* frame, SwrContext* swr, int channels,
             std::vector<TonicFloat>& samplesData, int decodedSamples) {
    int i, ch;
    int ret;
    int framesCount;

    // Send the packet with the compressed data to the decoder
    ret = avcodec_send_packet(decCtx, pkt);
    if (ret < 0) {
      // Just wait for the next valid packet. https://github.com/bytedeco/javacv/issues/1679#issuecomment-892606462
      std::cerr << "Error submitting the packet to the decoder" << std::endl;
      return 0;
    }

    // Read all the output frames - in general there may be more than one
    int numFramesTotal = 0;
    while (true) {
      ret = avcodec_receive_frame(decCtx, frame);
      if (ret == AVERROR(EAGAIN)) {
        //std::cerr << "EAGAIN" << std::endl;
        return numFramesTotal;
      }
      else if (ret == AVERROR_EOF) {
        //std::cerr << "EOF" << std::endl;
        return numFramesTotal;
      }
      else if (ret < 0) {
        std::cerr << "Error during decoding" << std::endl;
        return -1;
      }
      
      auto samplesRemaining = samplesData.size() - decodedSamples;
      auto samplesRequired = frame->nb_samples * channels;
      if (samplesRequired > samplesRemaining) {
          samplesData.resize(samplesData.size() - samplesRemaining + samplesRequired);
      }

      // Resample frames
      float* dataPtr = samplesData.data() + decodedSamples;
      framesCount = swr_convert(swr,
                               (uint8_t**)&dataPtr, frame->nb_samples,           // out
                               (const uint8_t**)frame->data, frame->nb_samples); // in
      //std::cerr << "Frames count: " << framesCount << std::endl; // 4096 for wav, ~300-1000 for mp3
      if (framesCount > 0) {
        decodedSamples += framesCount * channels;
        numFramesTotal += framesCount;
      }
    }
  }
  
  std::unique_ptr<SampleTable> loadAudioFile(std::string path, int numChannels) {
    const AVCodec* codec;
    AVCodecContext* codecCtx = nullptr;
    int ret;
    
    av_log_set_callback(ffmpeg_log_callback);
    av_log_set_level(AV_LOG_TRACE);
    
    AVPacket* pkt = av_packet_alloc();

    // Get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path.data(), nullptr, nullptr) != 0) {
      std::cerr << "Could not open file " << path.data() << std::endl;
      return nullptr;
    }
    if (avformat_find_stream_info(format, nullptr) < 0) {
      std::cerr << "Could not retrieve stream info from file " << path.data() << std::endl;
      return nullptr;
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
      std::cerr << "Could not retrieve audio stream from file " << path.data() << std::endl;
      return nullptr;
    }
    AVStream* stream = format->streams[streamIndex];

    // find & open codec
    codecCtx = avcodec_alloc_context3(nullptr);
    if (!codecCtx) {
      std::cerr << "Unable to allocate memory for codec context" << std::endl;
      return nullptr;
    }

    /// FIXME: not needed
    ret = avcodec_parameters_to_context(codecCtx, stream->codecpar);
    if (ret < 0)
      return nullptr;
    codecCtx->pkt_timebase = stream->time_base;

    codec = avcodec_find_decoder(codecCtx->codec_id);

    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
      std::cerr << "Failed to open decoder for stream #" << streamIndex << " in file " << path.data() << std::endl;
      return nullptr;
    }

    // Setup resampling to the FP32 format
    const int resampleSampleRate = Tonic::sampleRate();
    const AVSampleFormat resampleSampleFmt = AV_SAMPLE_FMT_FLT;
    const AVChannelLayout* outputChannelLayout = getChannelLayout2(numChannels);

    if (!outputChannelLayout) {
      return nullptr;
    }

    SwrContext* swr = nullptr;
    if (swr_alloc_set_opts2(&swr,
                            outputChannelLayout, resampleSampleFmt, resampleSampleRate, 
                            &codecCtx->ch_layout, codecCtx->sample_fmt, codecCtx->sample_rate, 
                            0, nullptr) < 0)
    {
      std::cerr << "Failed to alloc and setup resampler" << std::endl;
      return nullptr;
    }

    if (swr_init(swr) < 0) {
      std::cerr << "Could not open resample context" << std::endl;
      swr_free(&swr);
      return nullptr;
    }

    if (!swr_is_initialized(swr)) {
      std::cerr << "Resampler has not been properly initialized" << std::endl;
      return nullptr;
    }

    float duration = static_cast<float>(format->duration) / AV_TIME_BASE;
    int numFramesEstimation = static_cast<int>(codecCtx->sample_rate * duration);
    
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
      std::cerr << "Error allocating the frame" << std::endl;
      return nullptr;
    }

    int totalDecodedFrames = 0;
    std::vector<TonicFloat> samplesData(numFramesEstimation * numChannels);
    while (av_read_frame(format, pkt) >= 0) {
      // Decode audio frames one by one
      if (pkt->size) {
        auto totalDecodedSamples = totalDecodedFrames * numChannels;
        auto decodedFrames = decode(codecCtx, pkt, frame, swr, numChannels, samplesData, totalDecodedSamples);
        if (decodedFrames < 0) {
          std::cerr << "Error decoding audio frames" << std::endl;
          return nullptr;
        }
        totalDecodedFrames += decodedFrames;
        //std::cerr << "decoded frames: " << decodedFrames << "; decodedSamples: " << decodedSamples << std::endl;
      }
    }
    std::cerr << "loadAudioFile | totalDecodedFrames: " << totalDecodedFrames << "; numFramesEstimation: " << numFramesEstimation << std::endl; 

    // Shrink the samples data to actual size
    samplesData.resize(totalDecodedFrames * numChannels);

    std::unique_ptr<SampleTable> destinationTable = std::make_unique<SampleTable>(totalDecodedFrames, numChannels);
    TonicFloat* destinationTableDataPtr = destinationTable->dataPointer();
    if (destinationTableDataPtr == nullptr) {
        std::cerr << "decodeDataPtr is nullptr" << std::endl;
        return nullptr;
    }
    memcpy(destinationTableDataPtr, samplesData.data(), samplesData.size() * sizeof(TonicFloat));

    // Flush the decoder
    pkt->data = nullptr;
    pkt->size = 0;
    decode(codecCtx, pkt, frame, swr, numChannels, samplesData, 0);

    // Cleanup
    av_packet_free(&pkt);
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_free_context(&codecCtx);
    avformat_free_context(format);

    return destinationTable;  
  }
}
