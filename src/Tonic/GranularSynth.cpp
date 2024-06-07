//
//  GranularSynth.cpp
//  TonicDemo
//

//
// See LICENSE.txt for license and usage information.
//


#include "GranularSynth.h"

namespace Tonic {
  namespace Tonic_ {

    GranularSynth_::GranularSynth_() {
      stk::Stk::setSampleRate(Tonic::sampleRate());
      // TODO: set depending on file
      setIsStereoOutput(true);
    }

    void  GranularSynth_::setRandomFactor(ControlGenerator randomFactor) {
      randomFactor_ = randomFactor;
    }

    void  GranularSynth_::setStretch(ControlGenerator stretch) {
      stretch_ = stretch;
    }

    void  GranularSynth_::setDuration(ControlGenerator duration) {
      duration_ = duration;
    }

    void  GranularSynth_::setRamp(ControlGenerator ramp) {
      ramp_ = ramp;
    }

    void  GranularSynth_::setOffset(ControlGenerator offset) {
      offset_ = offset;
    }

    void  GranularSynth_::setDelay(ControlGenerator delay) {
      delay_ = delay;
    }

    void  GranularSynth_::setVoices(ControlGenerator voices) {
      voices_ = voices;
    }

    void  GranularSynth_::setFilePath(const std::string& path, bool typeRaw) {
      if (filePathCurrent_ != path) {
        try {
          grani_.openFile(path, typeRaw);
        }
        catch (stk::StkError&) {
          cerr << "error" << endl;
        }
        filePathCurrent_ = path;

        if (grani_.channelsOut() != 2) {
          cerr << "channels: " << grani_.channelsOut() << endl;
        }
      }
    }

    void   GranularSynth_::computeSynthesisBlock(const SynthesisContext_& context) {
      unsigned long bufferFrames = (unsigned int)outputFrames_.frames();
      stk::StkFloat* samples = (stk::StkFloat*)&outputFrames_[0];
      const stk::StkFrames& lastframe = grani_.lastFrame();

      float randomFactorValue = clamp(randomFactor_.tick(context).value, 0, 1);
      float stretchValue = clamp(stretch_.tick(context).value, 0, 1000);
      float durationValue = duration_.tick(context).value;
      float rampValue = clamp(ramp_.tick(context).value, 0, 100);
      float offsetValue = offset_.tick(context).value;
      float delayValue = delay_.tick(context).value;
      int voicesValue = static_cast<int>(std::round(clamp(voices_.tick(context).value, 0, 50)));

      grani_.setRandomFactor(randomFactorValue);
      grani_.setStretch(stretchValue);
      grani_.setGrainParameters(durationValue, rampValue, offsetValue, delayValue);
      if (voicesCurrent_ != voicesValue) {
        grani_.setVoices(voicesValue);
        voicesCurrent_ = voicesValue;
      }

      for (unsigned int i = 0; i < bufferFrames; i++) {
        grani_.tick();
        for (int j = 0; j < lastframe.channels(); j++)
          *samples++ = lastframe[j];
      }
    }
  }

}
