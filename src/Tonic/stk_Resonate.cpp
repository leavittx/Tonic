//
//  Resonate.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_Resonate.h"

namespace Tonic {
  namespace Tonic_ {

    Resonate_::Resonate_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(resonate_.channelsOut() == 2);
    }

    void  Resonate_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  Resonate_::setResonanceFrequency(ControlGenerator controlGenerator) {
      resonanceFrequency_ = controlGenerator;
    }

    void  Resonate_::setResonanceRadius(ControlGenerator controlGenerator) {
      resonanceRadius_ = controlGenerator;
    }

    void  Resonate_::setNotchFrequency(ControlGenerator controlGenerator) {
      notchFrequency_ = controlGenerator;
    }

    void  Resonate_::setNotchRadius(ControlGenerator controlGenerator) {
      notchRadius_ = controlGenerator;
    }

    void  Resonate_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      resonate_.noteOn(frequency, amplitude);
    }

    void  Resonate_::noteOff(TonicFloat amplitude) {
      resonate_.noteOff(amplitude);
    }

    void  Resonate_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = resonate_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      float resonanceFrequencyValue = resonanceFrequency_.tick(context).value;
      float resonanceRadiusValue = resonanceRadius_.tick(context).value;
      float notchFrequencyValue = notchFrequency_.tick(context).value;
      float notchRadiusValue = notchRadius_.tick(context).value;
      //resonate_.setFrequency(frequencyValue);
      //resonate_.setResonance(resonanceFrequencyValue, resonanceRadiusValue);
      resonate_.setNotch(notchFrequencyValue, notchRadiusValue);
      resonate_.setEqualGainZeroes();
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        resonate_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }
}
