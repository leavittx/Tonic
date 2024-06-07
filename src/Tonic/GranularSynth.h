//
//  GranularSynth.h
//  TonicDemo
//
//  Created by Morgan Packard on 1/29/13.

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__GranularSynth__
#define __TonicDemo__GranularSynth__

#include "Generator.h"
#include "Granulate.h"

namespace Tonic {

  namespace Tonic_ {

    class GranularSynth_ : public Generator_ {

    protected:
      stk::Granulate grani_;

      void computeSynthesisBlock(const SynthesisContext_& context);

    protected:
      ControlGenerator     randomFactor_;
      ControlGenerator     stretch_;
      ControlGenerator     duration_;
      ControlGenerator     ramp_;
      ControlGenerator     offset_;
      ControlGenerator     delay_;

    public:
      void setRandomFactor(ControlGenerator randomFactor);
      void setStretch(ControlGenerator stretch);
      void setDuration(ControlGenerator duration);
      void setRamp(ControlGenerator ramp);
      void setOffset(ControlGenerator offset);
      void setDelay(ControlGenerator delay);
      GranularSynth_();
    };
  }

  class GranularSynth : public TemplatedGenerator<Tonic_::GranularSynth_> {
  public:
    TONIC_MAKE_CTRL_GEN_SETTERS(GranularSynth, randomFactor, setRandomFactor);
    TONIC_MAKE_CTRL_GEN_SETTERS(GranularSynth, stretch, setStretch);
    TONIC_MAKE_CTRL_GEN_SETTERS(GranularSynth, duration, setDuration);
    TONIC_MAKE_CTRL_GEN_SETTERS(GranularSynth, ramp, setRamp);
    TONIC_MAKE_CTRL_GEN_SETTERS(GranularSynth, offset, setOffset);
    TONIC_MAKE_CTRL_GEN_SETTERS(GranularSynth, delay, setDelay);
  };


}

#endif /* defined(__TonicDemo__GranularSynth__) */

