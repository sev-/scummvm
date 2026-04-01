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

#ifndef FREESCAPE_ECLIPSE_OPL_MUSIC_H
#define FREESCAPE_ECLIPSE_OPL_MUSIC_H

#include "audio/fmopl.h"

namespace Freescape {

/**
 * OPL2/AdLib music player for Total Eclipse DOS.
 *
 * Ports the Wally Beben C64 SID music to the OPL2 FM chip by:
 * - Reusing the same sequencer (order lists, patterns, instruments)
 * - Converting SID note numbers to OPL F-number/block pairs
 * - Mapping SID waveforms to OPL FM instrument patches
 * - Using OPL's built-in ADSR envelopes
 */
class EclipseOPLMusicPlayer {
public:
	EclipseOPLMusicPlayer();
	~EclipseOPLMusicPlayer();

	void startMusic();
	void stopMusic();
	bool isPlaying() const;

private:
	static const byte kChannelCount = 3;
	static const byte kMaxNote = 94;

	struct ChannelState {
		const byte *orderList;
		byte orderPos;

		uint16 patternDataOffset;
		uint16 patternOffset;

		byte instrumentOffset;
		byte currentNote;
		byte transpose;

		byte durationReload;
		byte durationCounter;

		byte effectMode;
		byte effectParam;
		byte arpeggioTarget;
		byte arpeggioParam;
		byte arpeggioSequencePos;
		byte arpeggioSequence[9];
		byte arpeggioSequenceLen;

		byte noteStepCommand;
		byte stepDownCounter;

		byte vibratoPhase;
		byte vibratoCounter;

		byte delayValue;
		byte delayCounter;

		byte waveform;
		byte instrumentFlags;
		bool gateOffDisabled;
		bool keyOn;

		void reset();
	};

	OPL::OPL *_opl;
	bool _musicActive;
	byte _speedDivider;
	byte _speedCounter;
	ChannelState _channels[kChannelCount];
	byte _arpeggioIntervals[8];

	void onTimer();
	void setupSong();
	void silenceAll();
	void loadNextPattern(int channel);
	void buildEffectArpeggio(int channel);
	void loadCurrentFrequency(int channel);
	void finalizeChannel(int channel);
	void processChannel(int channel, bool newBeat);
	void parseCommands(int channel);
	void applyNote(int channel, byte note);
	void applyFrameEffects(int channel);
	bool applyInstrumentVibrato(int channel);
	void applyEffectArpeggio(int channel);
	void applyTimedSlide(int channel);

	void setOPLInstrument(int channel, byte instrumentOffset);
	void noteOn(int channel, byte note);
	void noteOff(int channel);
	void setFrequency(int channel, uint16 fnum, byte block);
	void noteToFnumBlock(byte note, uint16 &fnum, byte &block) const;

	byte readPatternByte(int channel);
	byte clampNote(byte note) const;
};

} // namespace Freescape

#endif
