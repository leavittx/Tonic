//
//  Moog.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Moog.h"

namespace Tonic {
  namespace Tonic_ {

    Moog_::Moog_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(false);
    }

    void  Moog_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  Moog_::setModulationSpeed(ControlGenerator controlGenerator) {
      modulationSpeed_ = controlGenerator;
    }

    void  Moog_::setModulationDepth(ControlGenerator controlGenerator) {
      modulationDepth_ = controlGenerator;
    }

    void  Moog_::setRandomGain(ControlGenerator controlGenerator) {
      randomGain_ = controlGenerator;
    }

    void  Moog_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      rhodey_.noteOn(frequency, amplitude);
    }

    void  Moog_::noteOff(TonicFloat amplitude) {
      rhodey_.noteOff(amplitude);
    }

    void  Moog_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = (unsigned int)outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = rhodey_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      float modulationSpeedValue = modulationSpeed_.tick(context).value;
      float modulationDepthValue = modulationDepth_.tick(context).value;
     
      rhodey_.setFrequency(frequencyValue);
      rhodey_.setModulationSpeed(modulationSpeedValue);
      rhodey_.setModulationDepth(modulationDepthValue);

      for (unsigned int i = 0; i < bufferFrames; i++) {
        rhodey_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
