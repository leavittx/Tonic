//
//  Generator.cpp
//  Tonic
//
//  Created by Morgan Packard on 1/23/13.
//
//
// See LICENSE.txt for license and usage information.
//


#include "Generator.h"
#include "Instrmnt.h"

#include <thread>

namespace Tonic{ namespace Tonic_{

  static std::thread::id mainThreadId = std::this_thread::get_id();
  
  Generator_::Generator_() : lastFrameIndex_(0), isStereoOutput_(false){
    outputFrames_.resize(kSynthesisBlockSize, 1, 0);
  }
  
  Generator_::~Generator_() {
    if (std::this_thread::get_id() != mainThreadId) {
      std::cerr << "Warning: Tonic::Generator_ being destructed on a non-main thread" << std::endl;
    }
  }
  
  void Generator_::setIsStereoOutput(bool stereo){
    if (stereo != isStereoOutput_){
      outputFrames_.resize(kSynthesisBlockSize, stereo ? 2 : 1, 0);
    }
    isStereoOutput_ = stereo;
  }

  void Generator_::controlChange(int number, TonicFloat value){
    auto* stkInstrument = getStkInstrument();
    if (stkInstrument){
      stkInstrument->controlChange(number, value);
    }
  }
}}
