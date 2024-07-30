//
//  PercFlut.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__PercFlut__
#define __TonicDemo__PercFlut__

#include "Generator.h"
#include "PercFlut.h"

namespace Tonic {

  namespace Tonic_ {

    class PercFlut_ : public Instrument_ {

    protected:
      stk::PercFlut percFlut_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &percFlut_; }

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      PercFlut_();
    };
  }

  class PercFlut : public TemplatedInstrument<Tonic_::PercFlut_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(PercFlut, frequency, setFrequency);
  };


}

#endif /* defined(__TonicDemo__PercFlut__) */

