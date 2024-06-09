//
//  Modulate.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__Modulate__
#define __TonicDemo__Modulate__

#include "Generator.h"
#include "Modulate.h"

namespace Tonic {

  namespace Tonic_ {

    class Modulate_ : public Generator_ {

    protected:
      stk::Modulate modulate_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     vibratoRate_;
      ControlGenerator     vibratoGain_;
      ControlGenerator     randomRate_;
      ControlGenerator     randomGain_;

    public:
      void setVibratoRate(ControlGenerator controlGenerator);
      void setVibratoGain(ControlGenerator controlGenerator);
      void setRandomRate(ControlGenerator controlGenerator);
      void setRandomGain(ControlGenerator controlGenerator);
      Modulate_();
    };
  }

  class Modulate : public TemplatedGenerator<Tonic_::Modulate_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(Modulate, vibratoRate, setVibratoRate);
    TONIC_MAKE_CTRL_GEN_SETTERS(Modulate, vibratoGain, setVibratoGain);
    TONIC_MAKE_CTRL_GEN_SETTERS(Modulate, randomRate, setRandomRate);
    TONIC_MAKE_CTRL_GEN_SETTERS(Modulate, randomGain, setRandomGain);
  };


}

#endif /* defined(__TonicDemo__Modulate__) */

