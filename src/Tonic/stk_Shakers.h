//
//  Shakers.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Shakers__
#define __TonicDemo__Shakers__

#include "Generator.h"
#include "Shakers.h"

namespace Tonic {

  namespace Tonic_ {

    class Shakers_ : public Instrument_ {

    protected:
      stk::Shakers shakers_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      Shakers_();
    };
  }

  class Shakers : public TemplatedInstrument<Tonic_::Shakers_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Shakers, frequency, setFrequency);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__Shakers__) */

