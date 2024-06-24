//
//  PolySynth.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//

#include "PolySynth2.h"

void BasicPolyphonicAllocator2::addVoice(Generator gen, Synth synth, int instanceIdx)
{
  PolyVoice v;
  v.instanceIdx = instanceIdx;
  v.gen = gen;
  v.synth = synth;
  v.currentNote = 0;

  auto paramPrefix = std::to_string(instanceIdx) + "_";
  auto params = synth.getParameters();
  for (auto& param : params)
  {
    size_t paramIdx = 0;
    if (param.getName().find(paramPrefix + "freq") != string::npos)
      paramIdx = 0;
    else if (param.getName().find(paramPrefix + "polyGate") != string::npos)
      paramIdx = 1;
    else if (param.getName().find(paramPrefix + "polyVelocity") != string::npos)
      paramIdx = 2;
    else if (param.getName().find(paramPrefix + "polyVoiceNumber") != string::npos)
      paramIdx = 3;
    else
      continue;

    voiceIdxToParams[voiceData.size()][paramIdx] = param;
  }

  inactiveVoiceQueue.push_back(voiceData.size());
  voiceData.push_back(v);
}

void BasicPolyphonicAllocator2::noteOn(int note, int velocity)
{
  int voiceNumber = getNextVoice(note);

  if (voiceNumber < 0)
    return; // no voice available

  cerr << ">> " << "Starting note " << note << " on voice " << voiceNumber << "\n";

  PolyVoice& voice = voiceData[voiceNumber];

  if (voice.gen.isInstrument())
  {
    voice.gen.noteOn(mtof(note), velocity);
  }
  else
  {
    //voice.synth.setParameter("freq", mtof(note));
    //voice.synth.setParameter("polyGate", 1.0);
    //voice.synth.setParameter("polyVelocity", velocity);
    //voice.synth.setParameter("polyVoiceNumber", voiceNumber);

    cerr << "Param name: " << voiceIdxToParams[voiceNumber][0].getName() << endl;

    voiceIdxToParams[voiceNumber][0].value(mtof(note));
    voiceIdxToParams[voiceNumber][1].value(1.0);
    voiceIdxToParams[voiceNumber][2].value(velocity);
    voiceIdxToParams[voiceNumber][3].value(voiceNumber);
  }

  voice.currentNote = note;

  activeVoiceQueue.remove(voiceNumber);
  activeVoiceQueue.push_back(voiceNumber);
  inactiveVoiceQueue.remove(voiceNumber);

  cerr << "Active voices: " << activeVoiceQueue.size() << endl;
}

void BasicPolyphonicAllocator2::noteOff(int note)
{
  // clear the oldest active voice with this note number
  for (int voiceNumber : activeVoiceQueue)
  {
    PolyVoice& voice = voiceData[voiceNumber];
    if (voice.currentNote == note)
    {
      cout << ">> " << "Stopping note " << note << " on voice " << voiceNumber << "\n";

      if (voice.gen.isInstrument())
      {
        voice.gen.noteOff(1.0);
      }
      else
      {
        //voice.synth.setParameter("polyGate", 0.0);
        voiceIdxToParams[voiceNumber][1].value(0.0);
      }

      activeVoiceQueue.remove(voiceNumber);
      inactiveVoiceQueue.remove(voiceNumber);
      inactiveVoiceQueue.push_back(voiceNumber);

      break;
    }
  }
}

int BasicPolyphonicAllocator2::getNextVoice(int note)
{
  // Find a voice not playing any note
  if (inactiveVoiceQueue.size())
  {
    return inactiveVoiceQueue.front();
  }

  return -1;
}

int OldestNoteStealingPolyphonicAllocator2::getNextVoice(int note)
{
  int voice = BasicPolyphonicAllocator2::getNextVoice(note);
  if (voice >= 0)
    return voice;

  if (activeVoiceQueue.size())
  {
    return activeVoiceQueue.front();
  }

  return -1;
}

int LowestNoteStealingPolyphonicAllocator2::getNextVoice(int note)
{
  int voice = BasicPolyphonicAllocator2::getNextVoice(note);
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