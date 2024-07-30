//
//  FMVoices.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_FMVoices.h"

namespace Tonic {
  namespace Tonic_ {

    FMVoices_::FMVoices_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(fmVoices_.channelsOut() == 2);
    }

    void  FMVoices_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  FMVoices_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      fmVoices_.noteOn(frequency, amplitude);
    }

    void  FMVoices_::noteOff(TonicFloat amplitude) {
      fmVoices_.noteOff(amplitude);
    }

    void  FMVoices_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = fmVoices_.lastFrame();

      float frequencyValue = frequency_.tick(context).value;
      //fmVoices_.setFrequency(frequencyValue);
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        fmVoices_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
