//
//  BandedWG.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "stk_BandedWG.h"

namespace Tonic {
  namespace Tonic_ {

    BandedWG_::BandedWG_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      setIsStereoOutput(bandedWG_.channelsOut() == 2);
    }

    void  BandedWG_::setFrequency(ControlGenerator controlGenerator) {
      frequency_ = controlGenerator;
    }

    void  BandedWG_::noteOn(TonicFloat frequency, TonicFloat amplitude) {
      bandedWG_.noteOn(frequency, amplitude);
    }

    void  BandedWG_::noteOff(TonicFloat amplitude) {
      bandedWG_.noteOff(amplitude);
    }

    void BandedWG_::setPreset(int preset) {
      bandedWG_.setPreset(preset);
    }

    void  BandedWG_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = bandedWG_.lastFrame();
    
      for (unsigned long i = 0; i < bufferFrames; i++) {
        bandedWG_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
