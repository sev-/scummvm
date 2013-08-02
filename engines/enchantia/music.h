/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ENCHANTIA_MUSIC_H
#define ENCHANTIA_MUSIC_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/system.h"

#include "audio/audiostream.h"
#include "audio/fmopl.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "audio/mixer.h"

namespace Enchantia {

struct AdlibTrack {
	byte channel;
	int16 delayIncr;
	int16 delayCount;
	const byte *nextCodeOffset;
	const byte *firstCodeOffset;
	uint16 currInstrument;
	const uint16 *codeOffset;
	int16 loopCounter;
	byte currBaseNote;
	byte hiFreq;
};

struct AdlibInstrument {
	byte unk0;
	byte unk1;
	byte unk2;
	byte unk3;
	byte reg20;
	byte reg40;
	byte reg60;
	byte reg80;
	byte regE0;
	byte reg23;
	byte reg43;
	byte reg63;
	byte reg83;
	byte regE3;
	byte regC0;
};

const uint kMaxTracks = 9;

class AdlibMusicPlayer : public Audio::AudioStream {
public:
	AdlibMusicPlayer(Audio::Mixer *mixer);
	~AdlibMusicPlayer();
	void init();
	void play(const byte *data, int size);
	void stop();
	bool isPlaying() const { return _playing; }
	// AudioStream interface
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }
protected:
	bool _playing;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	const byte *_data;
	OPL::OPL *_opl;
	int _sampleRate;
	int _nextUpdateSamples;
	AdlibTrack _tracks[kMaxTracks];
	void writeReg(byte reg, byte data);
	void update();
	void processTrack(uint trackNum);
	void noteOn(byte channel, byte instrument, byte note);
	void noteOff(byte channel, byte hiFreq);
};

class MidiParser_RS : public MidiParser {
protected:
	byte *_data;
	void parseNextEvent(EventInfo &info);
public:
	MidiParser_RS();
	~MidiParser_RS();
	bool loadMusic(byte *data, uint32 size);
	void unloadMusic();
};

class MidiPlayer : public Audio::MidiPlayer {
public:
	MidiPlayer();
	void playMusic(const char *filename);
	void setGM(bool isGM) { _isGM = isGM; }
	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);
protected:
	bool _isGM;
	byte *_musicData;
	size_t _musicDataSize;
};

} // End of namespace Enchantia

#endif /* ENCHANTIA_MUSIC_H */
