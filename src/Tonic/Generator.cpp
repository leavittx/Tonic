//
//  Generator.cpp
//  Tonic
//
//  Created by Morgan Packard on 1/23/13.
//
//
// See LICENSE.txt for license and usage information.
//

// Use assertion instead of throwing an exception in destructor
#include <assert.h>

#include "Generator.h"
#include "Instrmnt.h"

namespace Tonic{ namespace Tonic_{
  
  Generator_::Generator_() : lastFrameIndex_(0), isStereoOutput_(false), owningThreadId_(std::this_thread::get_id()) {
    outputFrames_.resize(kSynthesisBlockSize, 1, 0);
  }
  
  Generator_::~Generator_() {
    if (std::this_thread::get_id() != owningThreadId_) {
      std::cerr << "Warning: Tonic::Generator_ being destructed in a different thread it was created in" << std::endl;
      assert( !"Tonic::Generator_ being destructed in a different thread it was created in" );
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
