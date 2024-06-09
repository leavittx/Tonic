//
//  BlowBotl.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__BlowBotl__
#define __TonicDemo__BlowBotl__

#include "Generator.h"
#include "BlowBotl.h"

namespace Tonic {

  namespace Tonic_ {

    class BlowBotl_ : public Generator_ {

    protected:
      stk::BlowBotl blowBotl_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     frequency_;

    public:
      void setFrequency(ControlGenerator controlGenerator);
      void noteOn(TonicFloat frequency, TonicFloat amplitude);
      void noteOff(TonicFloat amplitude);
      BlowBotl_();
    };
  }

  class BlowBotl : public TemplatedInstrument<Tonic_::BlowBotl_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(BlowBotl, frequency, setFrequency);
    void noteOn(TonicFloat frequency, TonicFloat amplitude) override {
      this->gen()->noteOn(frequency, amplitude);
    }
    void noteOff(TonicFloat amplitude) override {
      this->gen()->noteOff(amplitude);
    }
  };


}

#endif /* defined(__TonicDemo__BlowBotl__) */

