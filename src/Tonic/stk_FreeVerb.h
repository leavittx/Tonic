//
//  stk_FreeVerb.h
//  Tonic 
//


//
// See LICENSE.txt for license and usage information.
//

#ifndef TONIC_STK_FREE_VERB_H
#define TONIC_STK_FREE_VERB_H

#include "Effect.h"
#include "FreeVerb.h"


namespace Tonic {
  
  namespace Tonic_ {
    
    class FreeVerb_ : public WetDryEffect_
    {
      protected:
        stk::FreeVerb freeVerb_;

        // Input generators
        ControlGenerator  widthCtrlGen_;
        ControlGenerator  dampingCtrlGen_;
        ControlGenerator  roomSizeCtrlGen_;
        ControlGenerator  freezeCtrlGen_;
      
        void computeSynthesisBlock( const SynthesisContext_ &context );

      public:
      
        FreeVerb_();
      
        void setWidthCtrlGen( ControlGenerator gen ) { widthCtrlGen_ = gen; }
        void setDampingCtrlGen( ControlGenerator gen ) { dampingCtrlGen_ = gen; }
        void setRoomSizeCtrlGen( ControlGenerator gen ) { roomSizeCtrlGen_ = gen; }
        void setFreezeCtrlGen( ControlGenerator gen ) { freezeCtrlGen_ = gen; }
    };
    
    inline void FreeVerb_::computeSynthesisBlock(const SynthesisContext_& context) {

      unsigned long bufferFrames = (unsigned int)dryFrames_.frames();
      stk::StkFloat* inptr = (stk::StkFloat*)&dryFrames_[0];
      stk::StkFloat* outptr = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = freeVerb_.lastFrame();

      TonicFloat roomSizeValue = roomSizeCtrlGen_.tick(context).value;
      TonicFloat dampingValue = dampingCtrlGen_.tick(context).value;
      TonicFloat widthValue = widthCtrlGen_.tick(context).value;
      bool freezeValue = freezeCtrlGen_.tick(context).value > 0.5f;

      // We should always output fully wet signal here
      freeVerb_.setEffectMix(1.0f);
      freeVerb_.setRoomSize(roomSizeValue);
      freeVerb_.setDamping(dampingValue);
      freeVerb_.setWidth(widthValue);
      bool freezeValueCurrent = freeVerb_.getMode() > 0.5f;
      if (freezeValue != freezeValueCurrent) {
        freeVerb_.setMode(freezeValue);
      }

      // In Reverb, kSynthesisBlockSize is used
      for (unsigned long i = 0; i < bufferFrames; i++) {
        freeVerb_.tick(*inptr++);
        for (int j = 0; j < lastframe.channels(); j++)
          *outptr++ = lastframe[j];
      }
    }
  }
  
  class FreeVerb : public TemplatedWetDryEffect<FreeVerb, Tonic_::FreeVerb_>
  {

    public:
        
      //! Initial delay before passing through reverb
      TONIC_MAKE_CTRL_GEN_SETTERS(FreeVerb, width, setWidthCtrlGen);
    
      //! Non-zero value will disable input filtering
      TONIC_MAKE_CTRL_GEN_SETTERS(FreeVerb, damping, setDampingCtrlGen);
    
      //! Value 0-1, affects spacing of early reflections
      TONIC_MAKE_CTRL_GEN_SETTERS(FreeVerb, roomSize, setRoomSizeCtrlGen);
    
      //! Value in seconds of overall decay time
      TONIC_MAKE_CTRL_GEN_SETTERS(FreeVerb, freeze, setFreezeCtrlGen);
  };
}

#endif


