//
//  AudioInput.h
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#ifndef __TonicDemo__AudioInput__
#define __TonicDemo__AudioInput__

#include "Generator.h"

namespace Tonic {

  namespace Tonic_ {

    class AudioInput_ : public Generator_ {

    protected:
      int firstChannel_ = 0;

      void computeSynthesisBlock(const SynthesisContext_& context);

    public:
      void setFirstChannel(int idx) {
        firstChannel_ = idx;
      }
      int getFirstChannel() const {
        return firstChannel_;
      }
      AudioInput_();
    };
  }

  class AudioInput : public TemplatedGenerator<Tonic_::AudioInput_> {
  public:
    void setFirstChannel(int idx) {
      this->gen()->setFirstChannel(idx);
    }
    void setIsStereo(bool isStereo) {
      this->gen()->setIsStereoOutput(isStereo);
    }
    int getFirstChannel() {
      return this->gen()->getFirstChannel();
    }
  };


}

#endif /* defined(__TonicDemo__AudioInput__) */

