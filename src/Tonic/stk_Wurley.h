//
//  Wurley.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Wurley__
#define __TonicDemo__Wurley__

#include "Generator.h"
#include "Wurley.h"

namespace Tonic {

  namespace Tonic_ {

    class Wurley_ : public Generator_ {

    protected:
      stk::Wurley wurley_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude);
      void noteOff(TonicFloat amplitude);
      Wurley_();
    };
  }

  class Wurley : public TemplatedInstrument<Tonic_::Wurley_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Wurley, frequency, setFrequency);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__Wurley__) */
