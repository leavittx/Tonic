//
//  PolySynth.cpp
//  PolyMIDIDemo
//
//  Created by Michael Dewberry on 6/7/13.
//
//

#include <list>
#include <array>

#include "TonicCore.h"
#include "Generator.h"
#include "Synth.h"
#include "Arithmetic.h"


using namespace Tonic;

template<typename VoiceAllocator>
class PolySynthWithAllocator2 : public Adder
{
public:
	PolySynthWithAllocator2() : Adder()
	{
	}

	void addVoice(Generator gen, Synth synth, int instanceIdx)
	{
		allocator.addVoice(gen, synth, instanceIdx);
		Adder::input(gen);
	}

	typedef Generator(VoiceCreateFn)();
	void addVoices(VoiceCreateFn createFn, int count)
	{
		for (int i = 0; i < count; i++)
			addVoice(createFn());
	}

	void noteOn(int note, float velocity)
	{
		allocator.noteOn(note, velocity);
	}

	void noteOff(int note)
	{
		allocator.noteOff(note);
	}

protected:
	VoiceAllocator allocator;
};

class BasicPolyphonicAllocator2
{
public:
	class PolyVoice
	{
	public:
    int instanceIdx;
		int currentNote;
		Generator gen;
    Synth synth;
	};

	void addVoice(Generator gen, Synth synth, int instanceIdx);
	void noteOn(int noteNumber, float velocity);
	void noteOff(int noteNumber);

protected:
	virtual int getNextVoice(int note);
	vector<PolyVoice> voiceData;
	list<int> inactiveVoiceQueue;
	list<int> activeVoiceQueue;
  std::map<int, std::array<ControlParameter, 4>> voiceIdxToParams;
};

class OldestNoteStealingPolyphonicAllocator2 : public BasicPolyphonicAllocator2
{
protected:
	virtual int getNextVoice(int note);
};

class LowestNoteStealingPolyphonicAllocator2 : public BasicPolyphonicAllocator2
{
protected:
	virtual int getNextVoice(int note);
};

typedef PolySynthWithAllocator2<LowestNoteStealingPolyphonicAllocator2> PolySynth2;

