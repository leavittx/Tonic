//
//  Wurley.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Wurley.h"

namespace Tonic {
  namespace Tonic_ {

    Wurley_::Wurley_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(wurley_.channelsOut() == 2);
    }

    void  Wurley_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  Wurley_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      wurley_.noteOn(frequency, amplitude);
    }

    void  Wurley_::noteOff(TonicFloat amplitude) {
      wurley_.noteOff(amplitude);
    }

    void  Wurley_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = (unsigned int)outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = wurley_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //wurley_.setFrequency(frequencyValue);
    
      for (unsigned int i = 0; i < bufferFrames; i++) {
        wurley_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
