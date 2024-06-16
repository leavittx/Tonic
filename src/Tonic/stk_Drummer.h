//
//  Drummer.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Drummer__
#define __TonicDemo__Drummer__

#include "Generator.h"
#include "Drummer.h"

namespace Tonic {

  namespace Tonic_ {

    class Drummer_ : public Generator_ {

    protected:
      stk::Drummer drummer_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude);
      void noteOff(TonicFloat amplitude);
      Drummer_();
    };
  }

  class Drummer : public TemplatedInstrument<Tonic_::Drummer_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Drummer, frequency, setFrequency);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__Drummer__) */

