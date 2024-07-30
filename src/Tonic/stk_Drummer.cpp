//
//  Drummer.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Drummer.h"

namespace Tonic {
  namespace Tonic_ {

    Drummer_::Drummer_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(hevyMetl_.channelsOut() == 2);
    }

    void  Drummer_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  Drummer_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      hevyMetl_.noteOn(frequency, amplitude);
    }

    void  Drummer_::noteOff(TonicFloat amplitude) {
      hevyMetl_.noteOff(amplitude);
    }

    void  Drummer_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = hevyMetl_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //drummer_.setFrequency(frequencyValue);
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        hevyMetl_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
