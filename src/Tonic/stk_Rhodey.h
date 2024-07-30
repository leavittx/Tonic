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

    class Rhodey_ : public Instrument_ {

    protected:
      stk::Rhodey rhodey_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &rhodey_; }

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      Rhodey_();
    };
  }

  class Rhodey : public TemplatedInstrument<Tonic_::Rhodey_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Rhodey, frequency, setFrequency);
  };


}

#endif /* defined(__TonicDemo__Rhodey__) */

