//
//  Resonate.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Resonate__
#define __TonicDemo__Resonate__

#include "Generator.h"
#include "Resonate.h"

namespace Tonic {

  namespace Tonic_ {

    class Resonate_ : public Instrument_ {

    protected:
      stk::Resonate resonate_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     frequency_;
      ControlGenerator     resonanceFrequency_;
      ControlGenerator     resonanceRadius_;
      ControlGenerator     notchFrequency_;
      ControlGenerator     notchRadius_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void setResonanceFrequency(ControlGenerator controlGenerator);
      void setResonanceRadius(ControlGenerator controlGenerator);
      void setNotchFrequency(ControlGenerator controlGenerator);
      void setNotchRadius(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      Resonate_();
    };
  }

  class Resonate : public TemplatedInstrument<Tonic_::Resonate_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Resonate, frequency, setFrequency);
    TONIC_MAKE_CTRL_GEN_SETTERS(Resonate, resonanceFrequency, setResonanceFrequency);
    TONIC_MAKE_CTRL_GEN_SETTERS(Resonate, resonanceRadius, setResonanceRadius);
    TONIC_MAKE_CTRL_GEN_SETTERS(Resonate, notchFrequency, setNotchFrequency);
    TONIC_MAKE_CTRL_GEN_SETTERS(Resonate, notchRadius, setNotchRadius);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__Resonate__) */

