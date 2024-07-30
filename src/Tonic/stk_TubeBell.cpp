//
//  TubeBell.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_TubeBell.h"

namespace Tonic {
  namespace Tonic_ {

    TubeBell_::TubeBell_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(tubeBell_.channelsOut() == 2);
    }

    void  TubeBell_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  TubeBell_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      tubeBell_.noteOn(frequency, amplitude);
    }

    void  TubeBell_::noteOff(TonicFloat amplitude) {
      tubeBell_.noteOff(amplitude);
    }

    void  TubeBell_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = tubeBell_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //tubeBell_.setFrequency(frequencyValue);
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        tubeBell_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
