//
//  BeeThree.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_BeeThree.h"

namespace Tonic {
  namespace Tonic_ {

    BeeThree_::BeeThree_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(beeThree_.channelsOut() == 2);
    }

    void  BeeThree_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  BeeThree_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      beeThree_.noteOn(frequency, amplitude);
    }

    void  BeeThree_::noteOff(TonicFloat amplitude) {
      beeThree_.noteOff(amplitude);
    }

    void  BeeThree_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = beeThree_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //drummer_.setFrequency(frequencyValue);
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        beeThree_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
