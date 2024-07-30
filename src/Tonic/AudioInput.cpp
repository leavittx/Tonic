//
//  AudioInput.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "AudioInput.h"
#include "RtWvIn.h"

namespace Tonic {
  namespace Tonic_ {

    AudioInput_::AudioInput_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
    }

    void   AudioInput_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];

      //auto& inputFrames = context.inputFrames;
      int numChannelsOut = isStereoOutput() ? 2 : 1;
      int numChannelsIn = context.nChannelsIn;//inputFrames.channels();
      if (firstChannel_ < 0 || firstChannel_ + numChannelsOut > numChannelsIn/* || outputFrames_.frames() != inputFrames.frames()*/) {
        return;
      }

      //const stk::StkFloat* samplesIn = (const stk::StkFloat*)&inputFrames.constData();
      const stk::StkFloat* samplesIn = context.inputFramesRaw;

      for (unsigned long i = 0; i < bufferFrames; i++) {
        for (int j = 0; j < numChannelsOut; j++) {
          *samples++ = samplesIn[i * numChannelsIn + firstChannel_ + j];
        }
      }
    }
  }

}
