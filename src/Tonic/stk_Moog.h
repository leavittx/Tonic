//
//  Moog.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Moog__
#define __TonicDemo__Moog__

#include "Generator.h"
#include "Moog.h"

namespace Tonic {

  namespace Tonic_ {

    class Moog_ : public Instrument_ {

    protected:
      stk::Moog moog_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &moog_; }

    protected:
      ControlGenerator     frequency_;
      ControlGenerator     modulationSpeed_;
      ControlGenerator     modulationDepth_;
      ControlGenerator     randomGain_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void setModulationSpeed(ControlGenerator controlGenerator);
      void setModulationDepth(ControlGenerator controlGenerator);
      void setRandomGain(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      /// TODO: controlChange
      Moog_();
    };
  }

  class Moog : public TemplatedInstrument<Tonic_::Moog_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Moog, frequency, setFrequency);
    TONIC_MAKE_CTRL_GEN_SETTERS(Moog, modulationSpeed, setModulationSpeed);
    TONIC_MAKE_CTRL_GEN_SETTERS(Moog, modulationDepth, setModulationDepth);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__Moog__) */

