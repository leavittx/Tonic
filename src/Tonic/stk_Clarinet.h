//
//  Clarinet.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Clarinet__
#define __TonicDemo__Clarinet__

#include "Generator.h"
#include "Clarinet.h"

namespace Tonic {

  namespace Tonic_ {

    class Clarinet_ : public Instrument_ {

    protected:
      stk::Clarinet clarinet_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      Clarinet_();
    };
  }

  class Clarinet : public TemplatedInstrument<Tonic_::Clarinet_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Clarinet, frequency, setFrequency);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__Clarinet__) */

