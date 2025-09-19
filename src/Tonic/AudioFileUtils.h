//
//  AudioFileUtils.h
//  TonicLib
//

#ifndef __TonicLib__AudioFileUtils__
#define __TonicLib__AudioFileUtils__

#include <memory>

#include "SampleTable.h"

namespace Tonic {
  
  std::unique_ptr<SampleTable> loadAudioFile(string path, int numChannels = 2);
  
}

#endif /* defined(__TonicLib__AudioFileUtils__) */
