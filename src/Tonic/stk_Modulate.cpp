//
//  Modulate.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Modulate.h"

namespace Tonic {
  namespace Tonic_ {

    Modulate_::Modulate_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      // TODO: set depending on file
      setIsStereoOutput(true);
    }

    void  Modulate_::setVibratoRate(ControlGenerator controlGenerator) {
      vibratoRate_ = controlGenerator;
    }

    void  Modulate_::setVibratoGain(ControlGenerator controlGenerator) {
      vibratoGain_ = controlGenerator;
    }

    void  Modulate_::setRandomRate(ControlGenerator controlGenerator) {
      randomRate_ = controlGenerator;
    }

    void  Modulate_::setRandomGain(ControlGenerator controlGenerator) {
      randomGain_ = controlGenerator;
    }

    void   Modulate_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = (unsigned int)outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = modulate_.lastFrame();

      float vibratoRateValue = vibratoRate_.tick(context).value;
      float vibratoGainValue = vibratoGain_.tick(context).value;
      float randomRateValue = randomRate_.tick(context).value;
      float randomGainValue = randomGain_.tick(context).value;
     
      modulate_.setVibratoRate(vibratoRateValue);
      modulate_.setVibratoGain(vibratoGainValue);
      modulate_.setRandomRate(randomRateValue);
      modulate_.setRandomGain(randomGainValue);

      for (unsigned int i = 0; i < bufferFrames; i++) {
        modulate_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
