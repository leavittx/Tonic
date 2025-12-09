//
//  PolySynth.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//

#include "PolySynth.h"

#include <array>

void BasicPolyphonicAllocator::addVoice(Synth synth)
{
  PolyVoice v;
  v.synth = synth;
  v.currentNote = 0;

  auto params = synth.getParameters();
  for (auto& param : params)
  {
    size_t paramIdx = 0;
    if (param.getName() == "freq")
      paramIdx = 0;
    else if (param.getName() == "polyGate")
      paramIdx = 1;
    else if (param.getName() == "polyVelocity")
      paramIdx = 2;
    else if (param.getName() == "polyVoiceNumber")
      paramIdx = 3;

    voiceIdxToParams[voiceData.size()][paramIdx] = param;
  }

  inactiveVoiceQueue.push_back(voiceData.size());
  voiceData.push_back(v);
}

void BasicPolyphonicAllocator::noteOn(int note, int velocity)
{
  int voiceNumber = getNextVoice(note);

  if (voiceNumber < 0)
    return; // no voice available

  std::cerr << ">> " << "Starting note " << note << " on voice " << voiceNumber << std::endl;

  PolyVoice& voice = voiceData[voiceNumber];

  //// TODO: set frequency
  //voice.synth.setParameter("freq", mtof(note));
  //voice.synth.setParameter("polyGate", 1.0);
  //voice.synth.setParameter("polyVelocity", velocity);
  //voice.synth.setParameter("polyVoiceNumber", voiceNumber);
  voiceIdxToParams[voiceNumber][0].value(mtof(note));
  voiceIdxToParams[voiceNumber][1].value(1.0);
  voiceIdxToParams[voiceNumber][2].value(velocity);
  voiceIdxToParams[voiceNumber][3].value(voiceNumber);

  voice.currentNote = note;

  activeVoiceQueue.remove(voiceNumber);
  activeVoiceQueue.push_back(voiceNumber);
  inactiveVoiceQueue.remove(voiceNumber);

  std::cerr << "Active voices: " << activeVoiceQueue.size() << std::endl;
}

void BasicPolyphonicAllocator::noteOff(int note)
{
  // clear the oldest active voice with this note number
  for (int voiceNumber : activeVoiceQueue)
  {
    PolyVoice& voice = voiceData[voiceNumber];
    if (voice.currentNote == note)
    {
      std::cout << ">> " << "Stopping note " << note << " on voice " << voiceNumber << std::endl;

      voice.synth.setParameter("polyGate", 0.0);

      activeVoiceQueue.remove(voiceNumber);
      inactiveVoiceQueue.remove(voiceNumber);
      inactiveVoiceQueue.push_back(voiceNumber);

      break;
    }
  }
}

int BasicPolyphonicAllocator::getNextVoice(int note)
{
  // Find a voice not playing any note
  if (inactiveVoiceQueue.size())
  {
    return inactiveVoiceQueue.front();
  }

  return -1;
}

int OldestNoteStealingPolyphonicAllocator::getNextVoice(int note)
{
  int voice = BasicPolyphonicAllocator::getNextVoice(note);
  if (voice >= 0)
    return voice;

  if (activeVoiceQueue.size())
  {
    return activeVoiceQueue.front();
  }

  return -1;
}

int SequentialPolyphonicAllocator::getNextVoice(int note)
{
  lastVoiceIdx = (lastVoiceIdx + 1) % voiceData.size();
  return lastVoiceIdx;
}

int LowestNoteStealingPolyphonicAllocator::getNextVoice(int note)
{
  int voice = BasicPolyphonicAllocator::getNextVoice(note);
  if (voice >= 0)
    return voice;

  // Find the playing voice with the lowest note that's lower than the requested note
  int lowestNote = note;
  int lowestVoice = -1;
  for (int voiceNumber : activeVoiceQueue)
  {
    PolyVoice& voice = voiceData[voiceNumber];
    if (voice.currentNote < lowestNote)
    {
      lowestNote = voice.currentNote;
      lowestVoice = voiceNumber;
    }
  }

  return lowestVoice;
}