//
//  BeeThree.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__BeeThree__
#define __TonicDemo__BeeThree__

#include "Generator.h"
#include "BeeThree.h"

namespace Tonic {

  namespace Tonic_ {

    class BeeThree_ : public Instrument_ {

    protected:
      stk::BeeThree beeThree_;

      void computeSynthesisBlock(const SynthesisContext_& context);
      stk::Instrmnt* getStkInstrument() override { return &beeThree_; }

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude) override;
      void noteOff(TonicFloat amplitude) override;
      BeeThree_();
    };
  }

  class BeeThree : public TemplatedInstrument<Tonic_::BeeThree_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(BeeThree, frequency, setFrequency);
  };


}

#endif /* defined(__TonicDemo__BeeThree__) */

