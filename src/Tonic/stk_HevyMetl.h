//
//  HevyMetl.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__HevyMetl__
#define __TonicDemo__HevyMetl__

#include "Generator.h"
#include "HevyMetl.h"

namespace Tonic {

  namespace Tonic_ {

    class HevyMetl_ : public Instrument_ {

    protected:
      stk::HevyMetl hevyMetl_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &hevyMetl_; }

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      HevyMetl_();
    };
  }

  class HevyMetl : public TemplatedInstrument<Tonic_::HevyMetl_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(HevyMetl, frequency, setFrequency);
  };


}

#endif /* defined(__TonicDemo__HevyMetl__) */

