//
//  Rhodey.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Rhodey.h"

namespace Tonic {
  namespace Tonic_ {

    Rhodey_::Rhodey_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(rhodey_.channelsOut() == 2);
    }

    void  Rhodey_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  Rhodey_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      rhodey_.noteOn(frequency, amplitude);
    }

    void  Rhodey_::noteOff(TonicFloat amplitude) {
      rhodey_.noteOff(amplitude);
    }

    void  Rhodey_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = (unsigned int)outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = rhodey_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      rhodey_.setFrequency(frequencyValue);
    
      for (unsigned int i = 0; i < bufferFrames; i++) {
        rhodey_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
