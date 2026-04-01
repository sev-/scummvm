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

#include "engines/freescape/games/eclipse/opl.music.h"

#include "common/textconsole.h"
#include "common/util.h"
#include "freescape/wb.h"
#include "freescape/games/eclipse/eclipse.musicdata.h"

using namespace Freescape::EclipseMusicData;

namespace Freescape {

// ============================================================================
// Embedded music data (shared with AY player, extracted from C64)
// ============================================================================

// OPL2 F-number/block table (95 entries)
// Format: fnum (bits 0-9) | block (bits 10-12)
// Derived from SID frequency table
static const uint16 kOPLFreqs[] = {
	0x0000, 0x0168, 0x017D, 0x0194, 0x01AD, 0x01C5,
	0x01E1, 0x01FD, 0x021B, 0x023B, 0x025E, 0x0282,
	0x02A8, 0x02D0, 0x02FB, 0x0328, 0x0358, 0x038B,
	0x03C1, 0x03FA, 0x061B, 0x063C, 0x065E, 0x0682,
	0x06A7, 0x06D0, 0x06FB, 0x0728, 0x0758, 0x078B,
	0x07C1, 0x07FA, 0x0A1B, 0x0A3B, 0x0A5D, 0x0A81,
	0x0AA8, 0x0AD0, 0x0AFB, 0x0B2B, 0x0B58, 0x0B8B,
	0x0BC1, 0x0BFA, 0x0E1B, 0x0E3B, 0x0E5D, 0x0E81,
	0x0EA8, 0x0ED0, 0x0EFB, 0x0F28, 0x0F58, 0x0F8B,
	0x0FC1, 0x0FFA, 0x121B, 0x123B, 0x125D, 0x1281,
	0x12A8, 0x12D0, 0x12FB, 0x1328, 0x1358, 0x138B,
	0x13C1, 0x13FA, 0x161B, 0x163B, 0x1661, 0x1681,
	0x16A8, 0x16D0, 0x16FB, 0x1729, 0x1758, 0x178B,
	0x17C1, 0x17FA, 0x1A1B, 0x1A3B, 0x1A5D, 0x1A81,
	0x1AA8, 0x1AD0, 0x1AFB, 0x1B28, 0x1B58, 0x1B8B,
	0x1BC1, 0x1BFA, 0x1E1B, 0x1E3B, 0x1E5D
};

// Instruments, order lists, pattern data, and arpeggio intervals
// are in eclipse.musicdata.h (shared with the AY player).

// ============================================================================
// (pattern data removed - now in eclipse.musicdata.h)
//
// ============================================================================
// OPL2 FM instrument patches (our own creation, not from C64)
// ============================================================================

// OPL operator register offsets for channels 0-2
// Each OPL channel has 2 operators (modulator + carrier)
// Modulator offsets: 0x00, 0x01, 0x02 for channels 0-2
// Carrier offsets:   0x03, 0x04, 0x05 for channels 0-2
static const byte kOPLModOffset[] = { 0x00, 0x01, 0x02 };
static const byte kOPLCarOffset[] = { 0x03, 0x04, 0x05 };

// FM patch definitions, 11 bytes each:
//  [0] mod: AM/VIB/EG/KSR/MULT  (reg 0x20+mod)
//  [1] car: AM/VIB/EG/KSR/MULT  (reg 0x20+car)
//  [2] mod: KSL/output level     (reg 0x40+mod)
//  [3] car: KSL/output level     (reg 0x40+car)
//  [4] mod: attack/decay          (reg 0x60+mod)
//  [5] car: attack/decay          (reg 0x60+car)
//  [6] mod: sustain/release       (reg 0x80+mod)
//  [7] car: sustain/release       (reg 0x80+car)
//  [8] mod: wave select           (reg 0xE0+mod)
//  [9] car: wave select           (reg 0xE0+car)
// [10] feedback/connection        (reg 0xC0+ch)
// Carrier ADSR derived from SID AD/SR bytes using timing-matched conversion.
// Modulator settings tuned per SID waveform type:
//   triangle → additive sine (soft, warm)
//   pulse    → FM with slight harmonic content (brighter)
//   sawtooth → FM with feedback (rich harmonics)
//   noise    → inharmonic FM (metallic percussion)
static const byte kOPLPatches[][11] = {
	// 0: rest — silent
	{ 0x00, 0x00, 0x3F, 0x3F, 0xFF, 0xFF, 0x0F, 0x0F, 0x00, 0x00, 0x00 },
	// 1: bass pulse (AD=0x42 SR=0x24) — punchy FM bass
	{ 0x02, 0x01, 0x1E, 0x00, 0xF1, 0x8A, 0x07, 0xD9, 0x00, 0x01, 0x06 },
	// 2: triangle lead (AD=0x8A SR=0xAC) — warm sine lead
	{ 0x01, 0x01, 0x2A, 0x00, 0xF1, 0x75, 0x07, 0x54, 0x00, 0x00, 0x01 },
	// 3: triangle pad (AD=0x6C SR=0x4F) — soft sustained pad
	{ 0x01, 0x01, 0x2A, 0x00, 0xF1, 0x84, 0x07, 0xB1, 0x00, 0x00, 0x01 },
	// 4: pulse arpeggio (AD=0x3A SR=0xAC) — bright arpeggio
	{ 0x02, 0x01, 0x1E, 0x00, 0xF1, 0x95, 0x07, 0x54, 0x00, 0x01, 0x06 },
	// 5: noise percussion (AD=0x42 SR=0x00) — metallic hit
	{ 0x01, 0x0C, 0x00, 0x00, 0xFA, 0x8A, 0x07, 0xFD, 0x00, 0x00, 0x01 },
	// 6: triangle melody (AD=0x3D SR=0x1C) — flute-like
	{ 0x01, 0x01, 0x2A, 0x00, 0xF1, 0x92, 0x07, 0xE4, 0x00, 0x00, 0x01 },
	// 7: pulse melody (AD=0x4C SR=0x2C) — vibrato lead
	{ 0x02, 0x01, 0x1E, 0x00, 0xF1, 0x84, 0x07, 0xD4, 0x00, 0x01, 0x06 },
	// 8: triangle sustain (AD=0x5D SR=0xBC) — organ-like
	{ 0x01, 0x01, 0x2A, 0x00, 0xF1, 0x82, 0x07, 0x44, 0x00, 0x00, 0x01 },
	// 9: pulse sustain (AD=0x4C SR=0xAF) — electric piano
	{ 0x02, 0x01, 0x1E, 0x00, 0xF1, 0x84, 0x07, 0x51, 0x00, 0x01, 0x06 },
	// 10: sawtooth lead (AD=0x4A SR=0x2A) — brass-like
	{ 0x21, 0x21, 0x1A, 0x00, 0xF1, 0x85, 0x07, 0xD5, 0x02, 0x00, 0x0E },
	// 11: pulse w/ arpeggio (AD=0x6A SR=0x6B) — harpsichord-like
	{ 0x02, 0x01, 0x1E, 0x00, 0xF1, 0x85, 0x07, 0x94, 0x00, 0x01, 0x06 },
};

// ============================================================================
// ChannelState
// ============================================================================

void EclipseOPLMusicPlayer::ChannelState::reset() {
	orderList = nullptr;
	orderPos = 0;
	patternDataOffset = 0;
	patternOffset = 0;
	instrumentOffset = 0;
	currentNote = 0;
	transpose = 0;
	durationReload = 0;
	durationCounter = 0;
	effectMode = 0;
	effectParam = 0;
	arpeggioTarget = 0;
	arpeggioParam = 0;
	arpeggioSequencePos = 0;
	memset(arpeggioSequence, 0, sizeof(arpeggioSequence));
	arpeggioSequenceLen = 0;
	noteStepCommand = 0;
	stepDownCounter = 0;
	vibratoPhase = 0;
	vibratoCounter = 0;
	delayValue = 0;
	delayCounter = 0;
	waveform = 0;
	instrumentFlags = 0;
	gateOffDisabled = false;
	keyOn = false;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

EclipseOPLMusicPlayer::EclipseOPLMusicPlayer()
	: _opl(nullptr),
	  _musicActive(false),
	  _speedDivider(1),
	  _speedCounter(0) {
	memcpy(_arpeggioIntervals, kArpeggioIntervals, 8);

	_opl = OPL::Config::create();
	if (!_opl || !_opl->init()) {
		warning("EclipseOPLMusicPlayer: Failed to create OPL emulator");
		delete _opl;
		_opl = nullptr;
	}
}

EclipseOPLMusicPlayer::~EclipseOPLMusicPlayer() {
	stopMusic();
	delete _opl;
}

// ============================================================================
// Public interface
// ============================================================================

void EclipseOPLMusicPlayer::startMusic() {
	if (!_opl)
		return;
	_opl->start(new Common::Functor0Mem<void, EclipseOPLMusicPlayer>(
		this, &EclipseOPLMusicPlayer::onTimer), 50);
	setupSong();
}

void EclipseOPLMusicPlayer::stopMusic() {
	_musicActive = false;
	if (_opl) {
		silenceAll();
		_opl->stop();
	}
}

bool EclipseOPLMusicPlayer::isPlaying() const {
	return _musicActive;
}

// ============================================================================
// OPL register helpers
// ============================================================================

void EclipseOPLMusicPlayer::noteToFnumBlock(byte note, uint16 &fnum, byte &block) const {
	if (note > kMaxNote)
		note = kMaxNote;
	uint16 combined = kOPLFreqs[note];
	fnum = combined & 0x03FF;
	block = (combined >> 10) & 0x07;
}

void EclipseOPLMusicPlayer::setFrequency(int channel, uint16 fnum, byte block) {
	if (!_opl)
		return;
	_opl->writeReg(0xA0 + channel, fnum & 0xFF);
	// Preserve key-on bit in 0xB0
	byte b0 = ((fnum >> 8) & 0x03) | (block << 2);
	if (_channels[channel].keyOn)
		b0 |= 0x20;
	_opl->writeReg(0xB0 + channel, b0);
}

void EclipseOPLMusicPlayer::setOPLInstrument(int channel, byte instrumentOffset) {
	if (!_opl)
		return;
	byte patchIdx = instrumentOffset / kInstrumentSize;
	if (patchIdx >= ARRAYSIZE(kOPLPatches))
		patchIdx = 0;

	const byte *patch = kOPLPatches[patchIdx];
	byte mod = kOPLModOffset[channel];
	byte car = kOPLCarOffset[channel];

	_opl->writeReg(0x20 + mod, patch[0]);
	_opl->writeReg(0x20 + car, patch[1]);
	_opl->writeReg(0x40 + mod, patch[2]);
	_opl->writeReg(0x40 + car, patch[3]);
	_opl->writeReg(0x60 + mod, patch[4]);
	_opl->writeReg(0x60 + car, patch[5]);
	_opl->writeReg(0x80 + mod, patch[6]);
	_opl->writeReg(0x80 + car, patch[7]);
	_opl->writeReg(0xE0 + mod, patch[8]);
	_opl->writeReg(0xE0 + car, patch[9]);
	_opl->writeReg(0xC0 + channel, patch[10]);
}

void EclipseOPLMusicPlayer::noteOn(int channel, byte note) {
	if (!_opl)
		return;
	uint16 fnum;
	byte block;
	noteToFnumBlock(note, fnum, block);

	_channels[channel].keyOn = true;
	_opl->writeReg(0xA0 + channel, fnum & 0xFF);
	_opl->writeReg(0xB0 + channel, 0x20 | (block << 2) | ((fnum >> 8) & 0x03));
}

void EclipseOPLMusicPlayer::noteOff(int channel) {
	if (!_opl)
		return;
	_channels[channel].keyOn = false;
	// Clear key-on bit, preserve frequency
	byte b0 = _opl ? 0x00 : 0x00; // just clear everything
	_opl->writeReg(0xB0 + channel, b0);
}

// ============================================================================
// Timer / sequencer core
// ============================================================================

void EclipseOPLMusicPlayer::onTimer() {
	if (!_musicActive)
		return;

	bool newBeat = (_speedCounter == 0);

	for (int channel = kChannelCount - 1; channel >= 0; channel--)
		processChannel(channel, newBeat);

	if (!_musicActive)
		return;

	if (newBeat)
		_speedCounter = _speedDivider;
	else
		_speedCounter--;
}

void EclipseOPLMusicPlayer::processChannel(int channel, bool newBeat) {
	if (newBeat) {
		_channels[channel].durationCounter--;
		if (_channels[channel].durationCounter == 0xFF) {
			parseCommands(channel);
			if (!_musicActive)
				return;
			finalizeChannel(channel);
			return;
		}

		if (_channels[channel].noteStepCommand != 0) {
			if (_channels[channel].noteStepCommand == 0xDE) {
				if (_channels[channel].currentNote > 0)
					_channels[channel].currentNote--;
			} else if (_channels[channel].currentNote < kMaxNote) {
				_channels[channel].currentNote++;
			}
			loadCurrentFrequency(channel);
			finalizeChannel(channel);
			return;
		}
	} else if (_channels[channel].stepDownCounter != 0) {
		_channels[channel].stepDownCounter--;
		if (_channels[channel].currentNote > 0)
			_channels[channel].currentNote--;
		loadCurrentFrequency(channel);
		finalizeChannel(channel);
		return;
	}

	applyFrameEffects(channel);
	finalizeChannel(channel);
}

void EclipseOPLMusicPlayer::finalizeChannel(int channel) {
	// Gate off at half duration: trigger release via key-off
	if (_channels[channel].durationReload != 0 &&
	    !_channels[channel].gateOffDisabled &&
	    ((_channels[channel].durationReload >> 1) == _channels[channel].durationCounter)) {
		noteOff(channel);
	}
}

// ============================================================================
// Song setup
// ============================================================================

void EclipseOPLMusicPlayer::setupSong() {
	silenceAll();

	// Enable wave select (required for non-sine waveforms)
	if (_opl)
		_opl->writeReg(0x01, 0x20);

	_speedDivider = 1;
	_speedCounter = 0;

	const byte *orderLists[3] = { kOrderList0, kOrderList1, kOrderList2 };

	for (int i = 0; i < kChannelCount; i++) {
		_channels[i].reset();
		_channels[i].orderList = orderLists[i];
		loadNextPattern(i);
	}

	_musicActive = true;
}

void EclipseOPLMusicPlayer::silenceAll() {
	if (!_opl)
		return;
	for (int ch = 0; ch < 9; ch++) {
		_opl->writeReg(0xB0 + ch, 0x00); // key off
		_opl->writeReg(0x40 + kOPLModOffset[ch < 3 ? ch : 0], 0x3F); // silence mod
		_opl->writeReg(0x40 + kOPLCarOffset[ch < 3 ? ch : 0], 0x3F); // silence car
	}
}

// ============================================================================
// Order list / pattern navigation
// ============================================================================

void EclipseOPLMusicPlayer::loadNextPattern(int channel) {
	int safety = 200;
	while (safety-- > 0) {
		byte value = _channels[channel].orderList[_channels[channel].orderPos];
		_channels[channel].orderPos++;

		if (value == 0xFF) {
			_channels[channel].orderPos = 0;
			continue;
		}

		if (value >= 0xC0) {
			_channels[channel].transpose = (byte)WBCommon::decodeOrderTranspose(value);
			continue;
		}

		if (value < ARRAYSIZE(kPatternOffsets)) {
			_channels[channel].patternDataOffset = kPatternOffsets[value];
			_channels[channel].patternOffset = 0;
		}
		break;
	}
}

byte EclipseOPLMusicPlayer::readPatternByte(int channel) {
	byte value = kPatternData[_channels[channel].patternDataOffset + _channels[channel].patternOffset];
	_channels[channel].patternOffset++;
	return value;
}

byte EclipseOPLMusicPlayer::clampNote(byte note) const {
	return note > kMaxNote ? kMaxNote : note;
}

// ============================================================================
// Pattern command parser
// ============================================================================

void EclipseOPLMusicPlayer::parseCommands(int channel) {
	if (_channels[channel].effectMode != 2) {
		_channels[channel].effectParam = 0;
		_channels[channel].effectMode = 0;
		_channels[channel].arpeggioSequenceLen = 0;
		_channels[channel].arpeggioSequencePos = 0;
	}

	_channels[channel].arpeggioTarget = 0;
	_channels[channel].noteStepCommand = 0;

	int safety = 200;
	while (safety-- > 0) {
		byte cmd = readPatternByte(channel);

		if (cmd == 0xFF) {
			loadNextPattern(channel);
			continue;
		}

		if (cmd == 0xFE) {
			stopMusic();
			return;
		}

		if (cmd == 0xFD) {
			readPatternByte(channel);
			cmd = readPatternByte(channel);
			if (cmd == 0xFF) {
				loadNextPattern(channel);
				continue;
			}
		}

		if (cmd >= 0xF0) {
			_speedDivider = cmd & 0x0F;
			continue;
		}

		if (cmd >= 0xC0) {
			byte instrument = cmd & 0x1F;
			if (instrument < kInstrumentCount)
				_channels[channel].instrumentOffset = instrument * kInstrumentSize;
			continue;
		}

		if (cmd >= 0x80) {
			_channels[channel].durationReload = cmd & 0x3F;
			continue;
		}

		if (cmd == 0x7F) {
			_channels[channel].noteStepCommand = 0xDE;
			_channels[channel].effectMode = 0xDE;
			continue;
		}

		if (cmd == 0x7E) {
			_channels[channel].effectMode = 0xFE;
			continue;
		}

		if (cmd == 0x7D) {
			_channels[channel].effectMode = 1;
			_channels[channel].effectParam = readPatternByte(channel);
			buildEffectArpeggio(channel);
			continue;
		}

		if (cmd == 0x7C) {
			_channels[channel].effectMode = 2;
			_channels[channel].effectParam = readPatternByte(channel);
			buildEffectArpeggio(channel);
			continue;
		}

		if (cmd == 0x7B) {
			_channels[channel].effectParam = 0;
			_channels[channel].effectMode = 1;
			_channels[channel].arpeggioTarget = readPatternByte(channel) + _channels[channel].transpose;
			_channels[channel].arpeggioParam = readPatternByte(channel);
			continue;
		}

		if (cmd == 0x7A) {
			_channels[channel].delayValue = readPatternByte(channel);
			cmd = readPatternByte(channel);
		}

		applyNote(channel, cmd);
		return;
	}
}

// ============================================================================
// Note application
// ============================================================================

void EclipseOPLMusicPlayer::applyNote(int channel, byte note) {
	byte instrumentOffset = _channels[channel].instrumentOffset;
	byte ctrl = kInstruments[instrumentOffset + 0];
	byte autoEffect = kInstruments[instrumentOffset + 4];
	byte flags = kInstruments[instrumentOffset + 5];
	byte sustainRelease = kInstruments[instrumentOffset + 2];
	byte actualNote = note;

	if (actualNote != 0)
		actualNote = clampNote(actualNote + _channels[channel].transpose);

	_channels[channel].currentNote = actualNote;
	_channels[channel].waveform = ctrl;
	_channels[channel].instrumentFlags = flags;
	_channels[channel].stepDownCounter = 0;

	if (actualNote != 0 && _channels[channel].effectParam == 0 && autoEffect != 0) {
		_channels[channel].effectParam = autoEffect;
		buildEffectArpeggio(channel);
	}

	if (actualNote != 0 && (flags & 0x02) != 0) {
		_channels[channel].stepDownCounter = 2;
		_channels[channel].currentNote = clampNote(_channels[channel].currentNote + 2);
	}

	// Set the OPL FM patch for this instrument
	setOPLInstrument(channel, instrumentOffset);

	_channels[channel].gateOffDisabled = (sustainRelease & 0x0F) == 0x0F;

	if (actualNote == 0) {
		noteOff(channel);
	} else {
		// Key off then on to retrigger envelope
		noteOff(channel);
		noteOn(channel, actualNote);
	}

	_channels[channel].durationCounter = _channels[channel].durationReload;
	_channels[channel].delayCounter = _channels[channel].delayValue;
	_channels[channel].arpeggioSequencePos = 0;
}

// ============================================================================
// Frequency helpers
// ============================================================================

void EclipseOPLMusicPlayer::loadCurrentFrequency(int channel) {
	byte note = clampNote(_channels[channel].currentNote);
	uint16 fnum;
	byte block;
	noteToFnumBlock(note, fnum, block);
	setFrequency(channel, fnum, block);
}

// ============================================================================
// Effects
// ============================================================================

void EclipseOPLMusicPlayer::buildEffectArpeggio(int channel) {
	_channels[channel].arpeggioSequenceLen = WBCommon::buildArpeggioTable(
		_arpeggioIntervals,
		_channels[channel].effectParam,
		_channels[channel].arpeggioSequence,
		sizeof(_channels[channel].arpeggioSequence),
		true);
	_channels[channel].arpeggioSequencePos = 0;
}

void EclipseOPLMusicPlayer::applyFrameEffects(int channel) {
	if (_channels[channel].currentNote == 0)
		return;

	if (applyInstrumentVibrato(channel))
		return;

	applyEffectArpeggio(channel);
	applyTimedSlide(channel);
}

bool EclipseOPLMusicPlayer::applyInstrumentVibrato(int channel) {
	byte vibrato = kInstruments[_channels[channel].instrumentOffset + 3];
	if (vibrato == 0 || _channels[channel].currentNote >= kMaxNote)
		return false;

	byte shift = vibrato & 0x0F;
	byte span = vibrato >> 4;
	if (span == 0)
		return false;

	uint16 noteFnum, nextFnum;
	byte noteBlock, nextBlock;
	noteToFnumBlock(_channels[channel].currentNote, noteFnum, noteBlock);
	noteToFnumBlock(_channels[channel].currentNote + 1, nextFnum, nextBlock);

	// Normalize to same block for delta computation
	int32 noteFreq = noteFnum << noteBlock;
	int32 nextFreq = nextFnum << nextBlock;
	int32 delta = nextFreq - noteFreq;
	if (delta <= 0)
		return false;

	while (shift-- != 0)
		delta >>= 1;

	if ((_channels[channel].vibratoPhase & 0x80) != 0) {
		if (_channels[channel].vibratoCounter != 0)
			_channels[channel].vibratoCounter--;
		if (_channels[channel].vibratoCounter == 0)
			_channels[channel].vibratoPhase = 0;
	} else {
		_channels[channel].vibratoCounter++;
		if (_channels[channel].vibratoCounter >= span)
			_channels[channel].vibratoPhase = 0xFF;
	}

	if (_channels[channel].delayCounter != 0) {
		_channels[channel].delayCounter--;
		return false;
	}

	int32 freq = noteFreq;
	for (byte i = 0; i < (span >> 1); i++)
		freq -= delta;
	for (byte i = 0; i < _channels[channel].vibratoCounter; i++)
		freq += delta;

	if (freq < 1) freq = 1;

	// Convert back to fnum/block
	byte block = 0;
	while (freq > 1023 && block < 7) {
		freq >>= 1;
		block++;
	}
	setFrequency(channel, freq & 0x3FF, block);
	return true;
}

void EclipseOPLMusicPlayer::applyEffectArpeggio(int channel) {
	if (_channels[channel].effectParam == 0 || _channels[channel].arpeggioSequenceLen == 0)
		return;

	if (_channels[channel].arpeggioSequencePos >= _channels[channel].arpeggioSequenceLen)
		_channels[channel].arpeggioSequencePos = 0;

	byte note = clampNote(_channels[channel].currentNote +
	                      _channels[channel].arpeggioSequence[_channels[channel].arpeggioSequencePos]);
	_channels[channel].arpeggioSequencePos++;

	uint16 fnum;
	byte block;
	noteToFnumBlock(note, fnum, block);
	setFrequency(channel, fnum, block);
}

void EclipseOPLMusicPlayer::applyTimedSlide(int channel) {
	if (_channels[channel].arpeggioTarget == 0)
		return;

	byte total = _channels[channel].durationReload;
	byte remaining = _channels[channel].durationCounter;
	byte start = _channels[channel].arpeggioParam >> 4;
	byte span = _channels[channel].arpeggioParam & 0x0F;
	byte elapsed = total - remaining;

	if (elapsed <= start || elapsed > start + span || span == 0)
		return;

	byte currentNote = clampNote(_channels[channel].currentNote);
	byte targetNote = clampNote(_channels[channel].arpeggioTarget);
	if (currentNote == targetNote)
		return;

	uint16 srcFnum, tgtFnum;
	byte srcBlock, tgtBlock;
	noteToFnumBlock(currentNote, srcFnum, srcBlock);
	noteToFnumBlock(targetNote, tgtFnum, tgtBlock);

	int32 srcFreq = srcFnum << srcBlock;
	int32 tgtFreq = tgtFnum << tgtBlock;
	int32 difference = srcFreq > tgtFreq ? srcFreq - tgtFreq : tgtFreq - srcFreq;
	uint16 divisor = span * (_speedDivider + 1);
	if (divisor == 0)
		return;

	int32 delta = difference / divisor;
	if (delta == 0)
		return;

	// Read current frequency from stored fnum/block
	uint16 curFnum;
	byte curBlock;
	noteToFnumBlock(currentNote, curFnum, curBlock);
	int32 curFreq = curFnum << curBlock;

	if (tgtFreq > srcFreq)
		curFreq += delta;
	else
		curFreq -= delta;

	if (curFreq < 1) curFreq = 1;

	byte block = 0;
	while (curFreq > 1023 && block < 7) {
		curFreq >>= 1;
		block++;
	}
	setFrequency(channel, curFreq & 0x3FF, block);
}

} // namespace Freescape
