//
//  FMVoices.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__FMVoices__
#define __TonicDemo__FMVoices__

#include "Generator.h"
#include "FMVoices.h"

namespace Tonic {

  namespace Tonic_ {

    class FMVoices_ : public Instrument_ {

    protected:
      stk::FMVoices fmVoices_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &fmVoices_; }

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      FMVoices_();
    };
  }

  class FMVoices : public TemplatedInstrument<Tonic_::FMVoices_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(FMVoices, frequency, setFrequency);
  };


}

#endif /* defined(__TonicDemo__FMVoices__) */

