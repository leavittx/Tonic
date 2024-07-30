//
//  PercFlut.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_PercFlut.h"

namespace Tonic {
  namespace Tonic_ {

    PercFlut_::PercFlut_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(percFlut_.channelsOut() == 2);
    }

    void  PercFlut_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  PercFlut_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      percFlut_.noteOn(frequency, amplitude);
    }

    void  PercFlut_::noteOff(TonicFloat amplitude) {
      percFlut_.noteOff(amplitude);
    }

    void  PercFlut_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = percFlut_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //percFlut_.setFrequency(frequencyValue);
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        percFlut_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
