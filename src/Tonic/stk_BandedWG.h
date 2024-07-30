//
//  BandedWG.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__BandedWG__
#define __TonicDemo__BandedWG__

#include "Generator.h"
#include "BandedWG.h"

namespace Tonic {

  namespace Tonic_ {

    class BandedWG_ : public Instrument_ {

    protected:
      stk::BandedWG bandedWG_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &bandedWG_; }

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      void setPreset(int preset);
      BandedWG_();
    };
  }

  class BandedWG : public TemplatedInstrument<Tonic_::BandedWG_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(BandedWG, frequency, setFrequency);
    void setPreset(int preset) {
      this->gen()->setPreset(preset);
    }
  };


}

#endif /* defined(__TonicDemo__BandedWG__) */

