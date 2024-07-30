//
//  stk_Chorus.h
//  Tonic 
//


//
// See LICENSE.txt for license and usage information.
//

#ifndef TONIC_STK_CHORUS_H
#define TONIC_STK_CHORUS_H

#include "Effect.h"
#include "Chorus.h"


namespace Tonic {
  
  namespace Tonic_ {
    
    class Chorus_ : public WetDryEffect_
    {
      protected:
        stk::Chorus chorus_;

        // Input generators
        ControlGenerator  modDepthCtrlGen_;
        ControlGenerator  modFrequencyCtrlGen_;
      
        void computeSynthesisBlock( const SynthesisContext_ &context );

      public:
      
        Chorus_() {

          setIsStereoOutput(true);

          // Default to 50% wet
          setDryLevelGen(FixedValue(1.0f));
          setWetLevelGen(FixedValue(0.5f));
        }
      
        void setModDepthCtrlGen( ControlGenerator gen ) { modDepthCtrlGen_ = gen; }
        void setModFrequencyCtrlGen( ControlGenerator gen ) { modFrequencyCtrlGen_ = gen; }
    };
    
    inline void Chorus_::computeSynthesisBlock(const SynthesisContext_& context) {

      unsigned long bufferFrames = (unsigned int)dryFrames_.frames();
      stk::StkFloat* inptr = (stk::StkFloat*)&dryFrames_[0];
      stk::StkFloat* outptr = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = chorus_.lastFrame();

      TonicFloat modDepthValue = modDepthCtrlGen_.tick(context).value;
      TonicFloat modFrequencyValue = modFrequencyCtrlGen_.tick(context).value;

      // We should always output fully wet signal here
      chorus_.setEffectMix(1.0f);
      chorus_.setModDepth(modDepthValue);
      chorus_.setModFrequency(modFrequencyValue);

      // In Reverb, kSynthesisBlockSize is used
      for (unsigned long i = 0; i < bufferFrames; i++) {
        chorus_.tick(*inptr++);
        for (int j = 0; j < lastframe.channels(); j++)
          *outptr++ = lastframe[j];
      }
    }
  }
  
  class Chorus : public TemplatedWetDryEffect<Chorus, Tonic_::Chorus_>
  {

    public:
        
      //! Set modulation depth in range 0.0 - 1.0.
      TONIC_MAKE_CTRL_GEN_SETTERS(Chorus, modDepth, setModDepthCtrlGen);
    
      //! Set modulation frequency.
      TONIC_MAKE_CTRL_GEN_SETTERS(Chorus, modFrequency, setModFrequencyCtrlGen);
  };
}

#endif


