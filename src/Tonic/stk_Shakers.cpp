//
//  Shakers.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Shakers.h"

namespace Tonic {
  namespace Tonic_ {

    Shakers_::Shakers_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(shakers_.channelsOut() == 2);
    }

    void  Shakers_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  Shakers_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      shakers_.noteOn(frequency, amplitude);
    }

    void  Shakers_::noteOff(TonicFloat amplitude) {
      shakers_.noteOff(amplitude);
    }

    void  Shakers_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = shakers_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //shakers_.setFrequency(frequencyValue);
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        shakers_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }
}
