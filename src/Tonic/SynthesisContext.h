//
//  TonicSynthesisContext.h
//  Tonic
//
//  Created by Morgan Packard on 1/23/13.
//
//
// See LICENSE.txt for license and usage information.
//


#ifndef TONIC_TONICSYNTHESISCONTEXT_H
#define TONIC_TONICSYNTHESISCONTEXT_H

#include "TonicFrames.h"
//! Top-level namespace.
/*! Objects under the Tonic namespace are used to bulid synths and generator chains */
namespace Tonic {

namespace Tonic_ {

  //! Context which defines a particular synthesis graph

  /*!
      Context passed down from root BufferFiller graph object to all sub-generators.
      synchronizes signal flow in cases when generator output is shared between multiple inputs
  */
  struct SynthesisContext_ {

    //! Number of frames elapsed since context start
    // unsigned long will last 38+ days before overflow at 44.1 kHz
    unsigned long elapsedFrames;

    //! Elapsed time since context start
    double elapsedTime;

    //! If true, generators will be forced to compute fresh output
    // TODO: Not fully implmenented yet -- ND 2013/05/20
    bool forceNewOutput;

    //! Input frames for AudioInput node
    //Tonic::TonicFrames inputFrames;

    TonicFloat* inputFramesRaw;
    int nChannelsIn = 0;

    SynthesisContext_() : elapsedFrames(0), elapsedTime(0), forceNewOutput(true), inputFramesRaw(nullptr), nChannelsIn(0) {}

    void tick() {
      elapsedFrames += kSynthesisBlockSize;
      elapsedTime = (double)elapsedFrames / sampleRate();
      forceNewOutput = false;
    };

  };

} // namespace Tonic_


//! Dummy context for ticking things in-place.
// Will always be at time 0, forceNewOutput == true
static const Tonic_::SynthesisContext_ DummyContext;

} // namespace Tonic

#endif
