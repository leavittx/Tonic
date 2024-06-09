//
//  Rhodey.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Rhodey__
#define __TonicDemo__Rhodey__

#include "Generator.h"
#include "Rhodey.h"

namespace Tonic {

  namespace Tonic_ {

    class Rhodey_ : public Generator_ {

    protected:
      stk::Rhodey rhodey_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude);
      void noteOff(TonicFloat amplitude);
      Rhodey_();
    };
  }

  class Rhodey : public TemplatedInstrument<Tonic_::Rhodey_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Rhodey, frequency, setFrequency);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__Rhodey__) */

