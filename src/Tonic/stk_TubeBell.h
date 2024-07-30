//
//  TubeBell.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__TubeBell__
#define __TonicDemo__TubeBell__

#include "Generator.h"
#include "TubeBell.h"

namespace Tonic {

  namespace Tonic_ {

    class TubeBell_ : public Instrument_ {

    protected:
      stk::TubeBell tubeBell_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &tubeBell_; }

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      TubeBell_();
    };
  }

  class TubeBell : public TemplatedInstrument<Tonic_::TubeBell_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(TubeBell, frequency, setFrequency);
  };


}

#endif /* defined(__TonicDemo__TubeBell__) */

