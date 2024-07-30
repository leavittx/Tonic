//
//  Clarinet.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Clarinet.h"

namespace Tonic {
  namespace Tonic_ {

    Clarinet_::Clarinet_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(clarinet_.channelsOut() == 2);
    }

    void  Clarinet_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  Clarinet_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      clarinet_.noteOn(frequency, amplitude);
    }

    void  Clarinet_::noteOff(TonicFloat amplitude) {
      clarinet_.noteOff(amplitude);
    }

    void  Clarinet_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = clarinet_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //clarinet_.setFrequency(frequencyValue);
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        clarinet_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
