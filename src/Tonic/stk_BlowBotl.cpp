//
//  BlowBotl.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_BlowBotl.h"

namespace Tonic {
  namespace Tonic_ {

    BlowBotl_::BlowBotl_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(blowBotl_.channelsOut() == 2);
    }

    void  BlowBotl_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  BlowBotl_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      blowBotl_.noteOn(frequency, amplitude);
    }

    void  BlowBotl_::noteOff(TonicFloat amplitude) {
      blowBotl_.noteOff(amplitude);
    }

    void  BlowBotl_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = (unsigned int)outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = blowBotl_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //blowBotl_.setFrequency(frequencyValue);
    
      for (unsigned int i = 0; i < bufferFrames; i++) {
        blowBotl_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
