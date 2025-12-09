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
             TonicFloat* decodeBuffer, int maxOutputFrames) {
    int i, ch;
    int ret, dataSize;
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
      dataSize = av_get_bytes_per_sample(decCtx->sample_fmt);

      // Resample frames
      framesCount = swr_convert(swr,
                               (uint8_t**)&decodeBuffer, std::min(frame->nb_samples, maxOutputFrames), // out
                               (const uint8_t**)frame->data, frame->nb_samples); // in
      if (framesCount > 0) {
        int samplesCount = framesCount * channels;
        decodeBuffer += samplesCount;
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
    int numFrames = static_cast<int>(codecCtx->sample_rate * duration);
    std::unique_ptr<SampleTable> destinationTable = std::make_unique<SampleTable>(numFrames, numChannels);
    TonicFloat* decodeDataPtr = destinationTable->dataPointer();
    if (decodeDataPtr == nullptr) {
      std::cerr << "decodeDataPtr is nullptr" << std::endl;
      return nullptr;
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
      std::cerr << "Error allocating the frame" << std::endl;
      return nullptr;
    }

    int totalDecodedFrames = 0;
    while (av_read_frame(format, pkt) >= 0) {
      // Decode audio frames one by one
      if (pkt->size) {
        auto decodedFrames = decode(codecCtx, pkt, frame, swr, numChannels, decodeDataPtr, numFrames);
        if (decodedFrames < 0) {
          std::cerr << "Error decoding audio frames" << std::endl;
          return nullptr;
        }
        int decodedSamples = decodedFrames * numChannels;
        decodeDataPtr += decodedSamples;
        numFrames -= decodedFrames;
        totalDecodedFrames += decodedFrames;
        //std::cerr << "decoded frames: " << decodedFrames << "; decodedSamples: " << decodedSamples << std::endl;
      }
    }
    std::cerr << "loadAudioFile | totalDecodedFrames: " << totalDecodedFrames << "; numFrames: " << numFrames << std::endl; 
    // totalDecodedFrames: 661426; numFrames: 663552
    // Shrink the sample table to actual size
    if (totalDecodedFrames < numFrames) {
      destinationTable->resize(totalDecodedFrames, numChannels);
    }

    // Flush the decoder
    pkt->data = nullptr;
    pkt->size = 0;
    decode(codecCtx, pkt, frame, swr, numChannels, decodeDataPtr, numFrames);

    // Cleanup
    av_packet_free(&pkt);
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_free_context(&codecCtx);
    avformat_free_context(format);

    return destinationTable;  
  }
}
