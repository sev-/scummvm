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

#include "enchantia/music.h"
#include "common/debug.h"

namespace Enchantia {

/* AdlibMusicPlayer */

static const struct { byte reg, data; } kAdlibInitData[] = {
	{0x01, 0x20}, {0x40, 0x3F}, {0x41, 0x3F}, {0x42, 0x3F},
	{0x43, 0x3F}, {0x44, 0x3F}, {0x45, 0x3F}, {0x48, 0x3F},
	{0x49, 0x3F}, {0x4A, 0x3F}, {0x4B, 0x3F}, {0x4C, 0x3F},
	{0x4D, 0x3F}, {0x50, 0x3F}, {0x51, 0x3F}, {0x52, 0x3F},
	{0x53, 0x3F}, {0x54, 0x3F}, {0x55, 0x3F}, {0x60, 0x88},
	{0x61, 0x88}, {0x62, 0x88}, {0x63, 0x88}, {0x64, 0x88},
	{0x65, 0x88}, {0x68, 0x88}, {0x69, 0x88}, {0x6A, 0x88},
	{0x6B, 0x88}, {0x6C, 0x88}, {0x6D, 0x88}, {0x80, 0x88},
	{0x81, 0x88}, {0x82, 0x88}, {0x83, 0x88}, {0x84, 0x88},
	{0x85, 0x88}, {0x88, 0x88}, {0x89, 0x88}, {0x8A, 0x88},
	{0x8B, 0x88}, {0x8C, 0x88}, {0x8D, 0x88}, {0xB0, 0x00},
	{0xB1, 0x00}, {0xB2, 0x00}, {0xB3, 0x00}, {0xB4, 0x00},
	{0xB5, 0x00}, {0xB6, 0x00}, {0xB7, 0x00}, {0xB8, 0x00},
	{0xC0, 0x00}, {0xC1, 0x00}, {0xC2, 0x00}, {0xC3, 0x00},
	{0xC4, 0x00}, {0xC5, 0x00}, {0xC6, 0x00}, {0xC7, 0x00},
	{0xC8, 0x00}, {0xBD, 0x00}
};

static const byte kAdlibChannelMap[] =
	{0, 1, 2, 8, 9, 10, 16, 17, 18};

static const uint16 kAdlibNoteToFreq[] = {
	0x0900, 0x019F, 0x1200, 0x0207, 0x05C1, 0x0D7B, 0x016B, 0x0181,
	0x0198, 0x01B0, 0x01CA, 0x01E5, 0x0202, 0x0220, 0x0241, 0x0263,
	0x0287, 0x02AE, 0x056B, 0x0581, 0x0598, 0x05B0, 0x05CA, 0x05E5,
	0x0602, 0x0620, 0x0641, 0x0663, 0x0687, 0x06AE, 0x096B, 0x0981,
	0x0998, 0x09B0, 0x09CA, 0x09E5, 0x0A02, 0x0A20, 0x0A41, 0x0A63,
	0x0A87, 0x0AAE, 0x0D6B, 0x0D81, 0x0D98, 0x0DB0, 0x0DCA, 0x0DE5,
	0x0E02, 0x0E20, 0x0E41, 0x0E63, 0x0E87, 0x0EAE, 0x116B, 0x1181,
	0x1198, 0x11B0, 0x11CA, 0x11E5, 0x1202, 0x1220, 0x1241, 0x1263,
	0x1287, 0x12AE, 0x156B, 0x1581, 0x1598, 0x15B0, 0x15CA, 0x15E5,
	0x1602, 0x1620, 0x1641, 0x1663, 0x1687, 0x16AE, 0x196B, 0x1981,
	0x1998, 0x19B0, 0x19CA, 0x19E5, 0x1A02, 0x1A20, 0x1A41, 0x1A63,
	0x1A87, 0x1AAE, 0x1D6B, 0x1D81, 0x1D98, 0x1DB0, 0x1DCA, 0x1DE5,
	0x1E02, 0x1E20, 0x1E41, 0x1E63, 0x1E87, 0x1EAE
};

static const AdlibInstrument kAdlibInstruments[] = {
	{0x00, 0x0A, 0, 0, 0xCF, 0x00, 0x84, 0xA5, 0, 0x1D, 0x15, 0xE4, 0x85, 0, 0x04},
	{0xB0, 0x07, 0, 0, 0x20, 0x11, 0xC3, 0x51, 0, 0x20, 0x0F, 0xD0, 0x2B, 0, 0x04},
	{0xA3, 0x17, 0, 0, 0xC1, 0x1F, 0x72, 0x22, 0, 0xC0, 0x0F, 0x74, 0x22, 0, 0x0C},
	{0xA3, 0x17, 0, 0, 0xC1, 0x1F, 0x72, 0x22, 0, 0xC0, 0x19, 0x74, 0x22, 0, 0x0C},
	{0x10, 0x17, 0, 0, 0x01, 0x20, 0x99, 0x40, 0, 0xA0, 0x0F, 0xD5, 0x54, 0, 0x02},
	{0xE6, 0x11, 0, 0, 0xE2, 0x21, 0x31, 0x20, 0, 0xE0, 0x12, 0x33, 0x33, 0, 0x00},
	{0xB3, 0x0B, 0, 0, 0x80, 0x00, 0xE7, 0xF7, 0, 0x80, 0x3F, 0xFF, 0x05, 0, 0x0F},
	{0x22, 0x11, 0, 0, 0xC2, 0x19, 0x63, 0x50, 0, 0xC0, 0x0F, 0xB2, 0xF2, 0, 0x00},
	{0x22, 0x11, 0, 0, 0xC2, 0x19, 0x63, 0x50, 0, 0xC0, 0x18, 0xB2, 0xF2, 0, 0x00},
	{0x00, 0x09, 0, 0, 0xE9, 0x00, 0xF0, 0x00, 0, 0x00, 0x08, 0xE9, 0xF9, 3, 0x0E},
	{0x9F, 0x01, 0, 0, 0xC0, 0x00, 0xF0, 0x01, 2, 0x90, 0x00, 0xF6, 0x06, 0, 0x0C},
	{0x00, 0x12, 0, 0, 0xC0, 0x00, 0xCA, 0x4A, 2, 0x00, 0x00, 0xF8, 0x88, 0, 0x0E},
	{0x07, 0x02, 0, 0, 0xC0, 0x00, 0xE5, 0xC2, 1, 0x01, 0x4F, 0xF7, 0x57, 0, 0x04},
	{0xC1, 0x05, 0, 0, 0x00, 0x0C, 0xB8, 0x88, 0, 0x00, 0x00, 0xD8, 0xFB, 0, 0x00},
	{0x7B, 0x0D, 0, 0, 0x2C, 0x01, 0xF0, 0x01, 2, 0x29, 0x00, 0x99, 0xF9, 1, 0x0C},
	{0x43, 0x0A, 0, 0, 0x01, 0x06, 0xF0, 0x00, 1, 0x01, 0x07, 0xE7, 0x33, 2, 0x06},
	{0x43, 0x0A, 0, 0, 0x01, 0x06, 0xF0, 0x00, 1, 0x01, 0x13, 0xE7, 0x33, 2, 0x06},
	{0xE9, 0x10, 0, 0, 0xE0, 0x1C, 0x83, 0x34, 0, 0x20, 0x0B, 0x84, 0x25, 0, 0x0E},
	{0xE9, 0x10, 0, 0, 0xE0, 0x1C, 0x83, 0x34, 0, 0x20, 0x13, 0x84, 0x25, 0, 0x0E},
	{0x8F, 0x11, 0, 0, 0x02, 0x30, 0xB8, 0x88, 0, 0x02, 0x07, 0x95, 0x35, 0, 0x00},
	{0x8F, 0x11, 0, 0, 0x02, 0x30, 0xB8, 0x88, 0, 0x02, 0x11, 0x95, 0x35, 0, 0x00},
	{0x49, 0x15, 0, 0, 0xC2, 0x3F, 0x88, 0x88, 0, 0xC2, 0x07, 0x73, 0x5B, 0, 0x01},
	{0x04, 0x14, 0, 0, 0x00, 0x3F, 0x88, 0x88, 0, 0xC2, 0x11, 0x73, 0x5B, 0, 0x01},
	{0x00, 0x08, 0, 0, 0x00, 0x00, 0x88, 0x88, 0, 0x00, 0x00, 0x88, 0x88, 0, 0x00},
	{0x00, 0x08, 0, 0, 0x00, 0x00, 0x88, 0x88, 0, 0x00, 0x00, 0x88, 0x88, 0, 0x00}
};


static const byte kAdlibDefNotes[] = {0xFF};

AdlibMusicPlayer::AdlibMusicPlayer(Audio::Mixer *mixer)
	: _mixer(mixer), _playing(false), _data(NULL), _nextUpdateSamples(0)  {

	_sampleRate = _mixer->getOutputRate();
	_opl = OPL::Config::create();
	_opl->init();
}

AdlibMusicPlayer::~AdlibMusicPlayer() {
	stop();
	delete _opl;
	_opl = NULL;
}

void AdlibMusicPlayer::play(const byte *data, int size) {
	stop();
	_data = data;

	for (uint trackNum = 0; trackNum < kMaxTracks; trackNum++) {
		AdlibTrack &track = _tracks[trackNum];
		uint16 trackOffs = READ_LE_UINT16(_data + trackNum * 2);
		track.channel = trackNum;
		track.delayIncr = 0;
		track.delayCount = 0;
		track.nextCodeOffset = kAdlibDefNotes;
		track.firstCodeOffset = kAdlibDefNotes;
		track.currInstrument = 0;
		track.loopCounter = 0;
		track.currBaseNote = 0;
		if (trackOffs != 0xFFFF)
			track.codeOffset = (uint16*)(_data + trackOffs);
		else
			track.codeOffset = NULL;
	}

	_playing = true;
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

void AdlibMusicPlayer::stop() {
	//	_mixer->stopHandle(_soundHandle);
	_data = NULL;
	_playing = false;
}

void AdlibMusicPlayer::init() {
	for (uint i = 0; i < ARRAYSIZE(kAdlibInitData); i++)
		writeReg(kAdlibInitData[i].reg, kAdlibInitData[i].data);
}

int AdlibMusicPlayer::readBuffer(int16 *buffer, const int numSamples) {
	if (!_playing) {
		memset(buffer, 0, sizeof(int16) * numSamples);
	} else if (_nextUpdateSamples == 0) {
		memset(buffer, 0, sizeof(int16) * numSamples);
		// Poll
		update();
		_nextUpdateSamples = _sampleRate / 10;
	} else {
		int samples = numSamples;
		int render;
		while (samples && _playing) {
			if (_nextUpdateSamples) {
				render = (samples > _nextUpdateSamples) ?  (_nextUpdateSamples) : (samples);
				samples -= render;
				_nextUpdateSamples -= render;
				//_opl->readBuffer(buffer, render);	// FIXME: deprecated
				buffer += render;
			} else {
				// Poll
				update();
				_nextUpdateSamples = _sampleRate / 10;
			}
		}
	}
	return numSamples;
}

void AdlibMusicPlayer::writeReg(byte reg, byte data) {
	_opl->writeReg(reg, data);
}

void AdlibMusicPlayer::update() {
	for (uint trackNum = 0; trackNum < kMaxTracks; trackNum++)
		processTrack(trackNum);
}

void AdlibMusicPlayer::processTrack(uint trackNum) {
	AdlibTrack &track = _tracks[trackNum];

	if (!track.codeOffset)
		return;

	if (++track.delayIncr < track.delayCount)
		return;

	track.delayIncr = 0;

	while (track.codeOffset) {
		byte note = *track.nextCodeOffset++;

		if (!(note & 0x80)) {
			noteOff(track.channel, track.hiFreq);
			if (note > 0)
				noteOn(track.channel, track.currInstrument, track.currBaseNote + note);
			break;
		} else if (note == 0xFF) {
			// Notes ended
			debug(2, "[Track %02d] Notes ended", trackNum);
			if (--track.loopCounter >= 0) {
				// Loop the notes
				track.nextCodeOffset = track.firstCodeOffset;
			} else {
				track.loopCounter = 0;
				while (track.codeOffset) {
					uint16 trackCmd = READ_LE_UINT16(track.codeOffset++);
					if (trackCmd == 0xFFFE) {
						// Set loop counter
						track.loopCounter = READ_LE_UINT16(track.codeOffset++);
						debug(2, "[Track %02d] Set loop counter to %d", trackNum, track.loopCounter);
					} else if (trackCmd == 0xFFFF) {
						// End the track for good
						track.codeOffset = NULL;
						debug(2, "[Track %02d] End the track for good", trackNum);
					} else if (trackCmd == 0xFFFD) {
						// Jump
						uint16 jumpOffs = READ_LE_UINT16(track.codeOffset);
						debug(2, "[Track %02d] Jump to %04X", trackNum, jumpOffs);
						track.codeOffset = (uint16*)(_data + jumpOffs);
					} else {
						// Play notes
						debug(2, "[Track %02d] Play notes from %04X", trackNum, trackCmd);
						track.firstCodeOffset = _data + trackCmd;
						track.nextCodeOffset = track.firstCodeOffset;
						track.delayIncr = 0;
						break;
					}
				}
			}
		} else if (note == 0xFE) {
			// Set instrument
			track.currInstrument = *track.nextCodeOffset++;
			debug(2, "[Track %02d] Set instrument to %d", trackNum, track.currInstrument);
		} else if (note == 0xFD) {
			// Set base note
			track.currBaseNote = *track.nextCodeOffset++;
			debug(2, "[Track %02d] Set base note to %d", trackNum, track.currBaseNote);
		} else {
			// Set delay
			track.delayCount = note & 0x7F;
			debug(2, "[Track %02d] Set delay to %d", trackNum, track.delayCount);
		}

	}

}

void AdlibMusicPlayer::noteOn(byte channel, byte instrument, byte note) {
	AdlibInstrument instr = kAdlibInstruments[instrument];

	byte mchannel = kAdlibChannelMap[channel];
	uint16 freq = kAdlibNoteToFreq[note];

	writeReg(0x20 | mchannel, instr.reg20);
	writeReg(0x23 | mchannel, instr.reg23);
	writeReg(0x40 | mchannel, instr.reg40);
	writeReg(0x43 | mchannel, instr.reg43);
	writeReg(0x60 | mchannel, instr.reg60);
	writeReg(0x80 | mchannel, instr.reg80);
	writeReg(0x63 | mchannel, instr.reg63);
	writeReg(0x83 | mchannel, instr.reg83);
	writeReg(0xE0 | mchannel, instr.regE0);
	writeReg(0xE3 | mchannel, instr.regE3);
	writeReg(0x40 | mchannel, instr.reg40);
	writeReg(0xC0 | channel, instr.regC0);
	writeReg(0xA0 | channel, freq & 0xFF);
	writeReg(0xB0 | channel, ((freq >> 8) & 0xFF) | 0x20);

}

void AdlibMusicPlayer::noteOff(byte channel, byte hiFreq) {
	writeReg(0xB0 | channel, hiFreq & 0xDF);
}

// MidiParser_RS

MidiParser_RS::MidiParser_RS() : _data(NULL) {
}

MidiParser_RS::~MidiParser_RS() {
	delete[] _data;
	_data = NULL;
}

bool MidiParser_RS::loadMusic(byte *data, uint32 size) {
	unloadMusic();
	_data = new byte[size];
	memcpy(_data, data, size);

	// CHECKME Check the values...
	//_num_tracks = 1;
	_tracks[0] = _data;
	_ppqn = 300;
	resetTracking();
	setTempo(500000);
	setTrack(0);

	return true;
}

void MidiParser_RS::unloadMusic() {
	MidiParser::unloadMusic();
	delete[] _data;
	_data = NULL;
}

void MidiParser_RS::parseNextEvent(EventInfo &info) {

	byte delta, value;

	info.start = _position._playPos;
	info.delta = 0;

	if (*(_position._playPos) == 0xFF) {
		// End of track reached, loop
		// HOW? _currData = _data + READ_LE_UINT16(_currData + 1);
	}

	delta = *(_position._playPos++);

	if (delta < 0xF0) {
		value = *(_position._playPos++);
		if (value < 0xF0) {
			info.delta = delta * 4; // ??? TODO FIXME etc.
			if (value >= 0x80) {
				info.basic.param1 = *(_position._playPos++);
				info.event = value;
			} else {
				// Reuse the last event
				info.basic.param1 = value;
			}
			// Read the second parameter if the event has one
			if (info.event < 0xC0 || info.event >= 0xE0)
				info.basic.param2 = *(_position._playPos++);
		}
	}

}

// MidiPlayer

MidiPlayer::MidiPlayer() : _isGM(false) {
	MidiPlayer::createDriver();
	int ret = _driver->open();
	if (ret == 0)
		_driver->setTimerCallback(this, &timerCallback);
}

void MidiPlayer::send(uint32 b) {
	/* CHECKME if we need special handling*//*
	if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}*/
	Audio::MidiPlayer::send(b);
}

void MidiPlayer::playMusic(const char *filename) {
	Common::StackLock lock(_mutex);

	stop();
	Common::File fd;
	if (!fd.open(filename))
		error("MidiPlayer::playMusic() Could not open %s", filename);
	byte *data = new byte[fd.size()];
	fd.read(data, fd.size());
	_parser = new MidiParser_RS();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->loadMusic(data, fd.size());
	//_parser->property(MidiParser::mpAutoLoop, loop);
	setVolume(255);
	_isPlaying = true;
	// Free the data, the parser keeps its own copy	of the data
	delete[] data;
	fd.close();

}

} // End of namespace Enchantia
