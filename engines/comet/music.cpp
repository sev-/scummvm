/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"

#include "common/endian.h"
#include "comet/music.h"
#include "comet/resourcemgr.h"

namespace Comet {

// FIXEM: Remove globals
unsigned char channelTableMelodic[] = {
  	0x00, 0x03, 0x01, 0x04, 0x02, 0x05, 0x08, 0x0B, 0x09, 0x0C, 0x0A, 0x0D,
	0x10, 0x13, 0x11, 0x14, 0x12, 0x15, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

unsigned char channelTableRythme[] = {
	0x00, 0x03, 0x01, 0x04, 0x02, 0x05, 0x08, 0x0B, 0x09, 0x0C, 0x0A,
	0x0D, 0x10, 0x13, 0x14, 0xFF, 0x12, 0xFF, 0x15, 0xFF, 0x11, 0xFF
};

unsigned char* channelTable;

// global table is 300 entry long
uint16 globTableEntry[300] = {
	0x157, 0x158, 0x159, 0x15A, 0x15A, 0x15B, 0x15C, 0x15D, 0x15E, 0x15F, 0x15F, 0x160, 0x161, 0x162, 0x163,
	0x164, 0x164, 0x165, 0x166, 0x167, 0x168, 0x168, 0x169, 0x16A, 0x16B, 0x16C, 0x16D, 0x16D, 0x16E, 0x16F,
	0x170, 0x171, 0x172, 0x173, 0x174, 0x174, 0x175, 0x176, 0x177, 0x178, 0x179, 0x17A, 0x17B, 0x17B, 0x17C,
	0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x181, 0x183, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188, 0x189, 0x18A,
	0x18B, 0x18C, 0x18D, 0x18E, 0x18E, 0x18F, 0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198,
	0x198, 0x19A, 0x19A, 0x19B, 0x19C, 0x19D, 0x19F, 0x19F, 0x1A0, 0x1A1, 0x1A2, 0x1A3, 0x1A4, 0x1A5, 0x1A6,
	0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1AE, 0x1AF, 0x1B0, 0x1B1, 0x1B2, 0x1B3, 0x1B4, 0x1B5,
	0x1B6, 0x1B7, 0x1B8, 0x1B9, 0x1BA, 0x1BB, 0x1BC, 0x1BD, 0x1BF, 0x1C0, 0x1C1, 0x1C2, 0x1C3, 0x1C4, 0x1C5,
	0x1C6, 0x1C7, 0x1C8, 0x1C9, 0x1CA, 0x1CB, 0x1CC, 0x1CD, 0x1CE, 0x1CF, 0x1D0, 0x1D2, 0x1D3, 0x1D4, 0x1D5,
	0x1D6, 0x1D7, 0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DD, 0x1DE, 0x1DF, 0x1E0, 0x1E1, 0x1E2, 0x1E3, 0x1E4, 0x1E5,
	0x1E6, 0x1E8, 0x1E9, 0x1EA, 0x1EB, 0x1EC, 0x1ED, 0x1EF, 0x1F0, 0x1F1, 0x1F2, 0x1F3, 0x1F4, 0x1F6, 0x1F7,
	0x1F8, 0x1F9, 0x1FA, 0x1FB, 0x1FD, 0x1FE, 0x1FF, 0x200, 0x201, 0x202, 0x203, 0x205, 0x206, 0x207, 0x208,
	0x20A, 0x20B, 0x20C, 0x20D, 0x20F, 0x210, 0x211, 0x212, 0x214, 0x215, 0x216, 0x217, 0x219, 0x21A, 0x21B,
	0x21C, 0x21D, 0x21F, 0x220, 0x221, 0x222, 0x224, 0x225, 0x226, 0x227, 0x229, 0x22A, 0x22C, 0x22D, 0x22E,
	0x22F, 0x231, 0x232, 0x234, 0x235, 0x236, 0x237, 0x239, 0x23A, 0x23B, 0x23C, 0x23E, 0x23F, 0x241, 0x242,
	0x243, 0x245, 0x246, 0x247, 0x248, 0x24A, 0x24B, 0x24D, 0x24E, 0x250, 0x251, 0x252, 0x254, 0x255, 0x257,
	0x258, 0x259, 0x25B, 0x25C, 0x25E, 0x25F, 0x260, 0x262, 0x263, 0x264, 0x266, 0x267, 0x269, 0x26A, 0x26B,
	0x26D, 0x26E, 0x270, 0x271, 0x273, 0x274, 0x276, 0x277, 0x279, 0x27A, 0x27C, 0x27D, 0x27F, 0x280, 0x282,
	0x283, 0x285, 0x286, 0x288, 0x289, 0x28A, 0x28C, 0x28D, 0x28F, 0x291, 0x292, 0x294, 0x295, 0x297, 0x299,
	0x29A, 0x29C, 0x29D, 0x29F, 0x2A0, 0x2A2, 0x2A3, 0x2A5, 0x2A7, 0x2A8, 0x2AA, 0x2AB, 0x2AD, 0x2AF, 0x2B0,
};

uint16* globTable[13] =  {
	&globTableEntry[0],
	&globTableEntry[25],
	&globTableEntry[50],
	&globTableEntry[75],
	&globTableEntry[100],
	&globTableEntry[125],
	&globTableEntry[150],
	&globTableEntry[175],
	&globTableEntry[200],
	&globTableEntry[225],
	&globTableEntry[250],
	&globTableEntry[275],
};

unsigned char smallData2[] = {
  0, 1, 2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  3, 4, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  6, 7, 8, 0xFF, 0xFF, 0xFF,
};

uint8 smallTable[] = { 0x10, 8, 4, 2, 1 };


MusicPlayer::MusicPlayer(CometEngine *vm) : _vm(vm) {

	_rate = _vm->_mixer->getOutputRate();
	_opl = OPL::MAME::makeAdLibOPL(_rate);
	_ended = false;
	_playing = false;

	_currentMusic = -1;

	_musicVolume = 0x7F;
	_musicParam1 = 0;
 	_currentMusicPtr = NULL;
	_currentMusicPtr2 = NULL;
	_currentMusicPtr3 = NULL;
	_generalVolume = 0;
	_musicTimer = 0;
	_nextUpdateTimer = musicSync;
	_regBDConf = 0xC0;
	
	initTables();

	for (int i = 0; i < 11; i++) {
		_channelTable2[i].var4 |= 0x20;
		_channelTable2[i].var2->var4 |= 0x20;
		createDefaultChannel(i);
  	}

	_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle,
			this, -1, 255, 0, DisposeAfterUse::NO, true);

  	musicStart();

}

MusicPlayer::~MusicPlayer() {
	Common::StackLock slock(_mutex);

	_vm->_mixer->stopHandle(_handle);
	OPLDestroy(_opl);
}

int MusicPlayer::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);
	int samples;
	int render;

	samples = numSamples;
	while (samples && _playing) {
		if (_nextUpdateTimer > 0) {
			render = (samples > _nextUpdateTimer) ?  (_nextUpdateTimer) : (samples);
			samples -= render;
			_nextUpdateTimer -= render;
			OPL::MAME::YM3812UpdateOne(_opl, buffer, render);
			buffer += render;
		} else {
			update();
			_nextUpdateTimer = musicSync / 4;
		}
	}

	return numSamples;
}

void MusicPlayer::initTables() {

	for (int i = 0; i < 11; i++) {
		_channelDataTable[i].var0 = 0xFFFF;
		_channelDataTable[i].var2 = 0x40;
		_channelDataTable[i].var4 = 0xFF;
		_channelDataTable[i].var5 = 0xFF;
		_channelDataTable[i].var6 = 0xFF;
		_channelDataTable[i].var7 = 0x9C;
		_channelDataTable[i].var8 = 0xFFFF;
		_channelTable2[i].index = i;
		_channelTable2[i].var2 = &_channelTable3[i];
		_channelTable2[i].var4 = 0x40;
		_channelTable2[i].dataPtr = NULL;
		_channelTable2[i].commandPtr = NULL;
		_channelTable2[i].varE = 0;
		_channelTable2[i].var10 = 0;
		_channelTable2[i].var12 = 0;
		_channelTable2[i].var13 = 0;
		_channelTable2[i].var15 = 0;
		_channelTable2[i].var17 = 0;
		_channelTable2[i].var18 = 0;
		_channelTable2[i].var1A = 0x7F;
		_channelTable2[i].var1B = 1;
		_channelTable2[i].var1C = 1;
		_channelTable2[i].var1D = 0x7F;
		_channelTable2[i].var1E = 0;
		_channelTable3[i].index = i;
		_channelTable3[i].var2 = &_channelTable2[i];
		_channelTable3[i].var4 = 0x8040;
		_channelTable3[i].dataPtr = NULL;
		_channelTable3[i].commandPtr = NULL;
		_channelTable3[i].varE = 0;
		_channelTable3[i].var10 = 0;
		_channelTable3[i].var12 = 0;
		_channelTable3[i].var13 = 0;
		_channelTable3[i].var15 = 0;
		_channelTable3[i].var17 = 0;
		_channelTable3[i].var18 = 0;
		_channelTable3[i].var1A = 0x7F;
		_channelTable3[i].var1B = 1;
		_channelTable3[i].var1C = 1;
		_channelTable3[i].var1D = 0x7F;
		_channelTable3[i].var1E = 0;
	}

}

void MusicPlayer::writeOPL(byte reg, byte val) {
	//debugC(6, kDebugMusic, "writeOPL(%02X, %02X)", reg, val);
	OPLWriteReg(_opl, reg, val);
}

void MusicPlayer::createDefaultChannel(int index) {
	_channelDataTable[index].var5 = 0xFF;
	_channelDataTable[index].var4 = 0xFF;
	_channelDataTable[index].var2 = 0x40;
	_channelDataTable[index].var0 = 0xFFFF;
	_channelDataTable[index].var7 = 0x9C;
	_channelDataTable[index].var8 = 0xFFFF;
}

void MusicPlayer::resetChannelFrequency(int channelIdx) {
  	writeOPL(0xA0 + channelIdx, 0);
  	writeOPL(0xB0 + channelIdx, 0);
}

void MusicPlayer::setupChannelFrequency(int channelIdx, int cl, int dx,int bp) {
	uint16 *di;
  	uint16 frequency;
  	uint8 frequencyLow;
  	uint8 frequencyHigh;
  	uint8 blockNumber;
  	if (!(bp & 0x8000))
		writeOPL(0xB0 + channelIdx, 0);
  	di = globTable[cl & 0xF];
  	if(bp & 0x80) {
   		// exit(1);
  	}
  	if(cl & 0x80) {
		dx = 0x40;
  	}
  	frequency = di[bp & 0xFF];
  	frequencyLow = frequency & 0xFF;
  	writeOPL(0xA0 + channelIdx, frequencyLow);
  	blockNumber = (cl & 0x70) >> 2;
  	frequencyHigh = ((frequency >> 8) & 0x3) | blockNumber;
  	if (!(dx & 0x40))
		frequencyHigh |= 0x20; // set key on
  	writeOPL(0xB0 + channelIdx, frequencyHigh);
}

int MusicPlayer::musicStart() {
  	writeOPL(1, 0x20);
  	writeOPL(8, 0);
  	writeOPL(0xBD, _regBDConf);
  	for (int i = 0; i < 18; i++) {
		writeOPL(0x60 + channelTableMelodic[i], 0xFF);
		writeOPL(0x80 + channelTableMelodic[i], 0xFF);
  	}
  	for (int i = 0; i < 9; i++) {
		resetChannelFrequency(i);
  	}
  	for (int i = 0; i < 11; i++) {
		createDefaultChannel(i);
  	}
  	if (!_musicParam1) {
		resetChannelFrequency(6);
		setupChannelFrequency(6, 0, 0x40, 0);
		resetChannelFrequency(7);
		setupChannelFrequency(7, 7, 0x40, 0);
		resetChannelFrequency(8);
		setupChannelFrequency(8, 0, 0x40, 0);
  	}
  	return 0;
}

int MusicPlayer::musicLoad(void *ptr) {
  	uint8 flag1;
  	uint8* musicPtr = (uint8*)ptr;
  	channelTable = channelTableMelodic;
  	flag1 = musicPtr[0x3C] & 0xC0;
  	_musicParam1 = musicPtr[0x3D];
  	if (!_musicParam1) {
		flag1 |= 0x20;
		channelTable = channelTableRythme;
  	}
  	_regBDConf = flag1;
  	for (int i = 0; i < 11; i++) {
		unsigned long int offset;
		offset = *((uint32*)(musicPtr + i*4 + 8));
		if (offset) {
	  		_channelTable2[i].dataPtr = musicPtr + offset;
		} else {
	  		_channelTable2[i].dataPtr = NULL;
		}
		_channelTable2[i].var4 |= 0x40;
  	}
  	_currentMusicPtr = musicPtr + *((uint16*)(musicPtr + 0x34));
  	return 0;
}

void MusicPlayer::executeMusicCommand(ChannelTable2Element *entry) {
  	uint16 opcode;

  	if (entry->var4 & 0x40)
		return;

	// start channel
  	if (entry->var4 & 0x02) {
		entry->commandPtr = entry->dataPtr;
		entry->var4 &= 0xFFFD;
		entry->var18 = 0;
  	} else {
		if(entry->var1A != entry->var1D) {
	  		//exit(1);
		}
		entry->varE--; // voice delay
		if (entry->varE <= 0) {
	  		entry->varE = entry->var10;
		} else {
	  		return;
		}
  	}

  	do {
		opcode = *(uint16*)(entry->commandPtr);
		entry->commandPtr += 2;

		int param = opcode >> 8;

		switch (opcode & 0x7F) {
		case 0:
  			entry->var4 |= 2;
  			if (entry->var4 & 0x20)
				return;
  			entry->var4 |= 0x40;
  			if (!(entry->var4 & 0x8000))
				return;
  			entry->var2->var4 &= 0xFFFB;
			break;
		case 1:
  			entry->var10 = entry->varE = (*(uint16*)((entry->commandPtr) - 1)) + entry->var13;
  			entry->commandPtr++;
  			break;
		case 2:
  			entry->var18++;
  			entry->var15 = param;
			break;
		case 3:
		  	entry->var12 = param;
			break;
		case 4:
		  	entry->var1E = param;
		  	break;
		case 5:
		  	entry->var17 = param;
		  	break;
		case 6:
			break;
		case 7:
		case 8:
		case 9:
			break;
		default:
			break;
		}

  	} while (!(opcode & 0x80));
}

void MusicPlayer::applyDirectFrequency(int index, int param1, int param2, int param3) {
  	if (_musicParam1) {
		setupChannelFrequency(index,param1,param2,param3);
		return;
  	} else {
		int ah;
		if (index < 6) {
	  		setupChannelFrequency(index,param1,param2,param3);
	  		return;
		}
		if (index == 6) {
	  		setupChannelFrequency(index,param1,0x40,param3);
		} else if (index == 8 && !(param1 & 0x80)) {
	  		int indexBackup = index;
	  		int param1Backup = param1;
			int al = param1 & 0x70;
	  		setupChannelFrequency(8, param1, 0x40, param3);
			index = 7;
			param1 &= 0xF;
			param1 += 7;
			if (param1 >= 0xC) {
	  			param1 -= 0xC;
	  			if (al != 0x70)
					al += 0x10;
				}
			setupChannelFrequency( index, param1, 0x40,param3);
	  		param1 = param1Backup;
	  		index = indexBackup;
		}
		ah = (~(smallTable[index - 6])) & _regBDConf;
		writeOPL(0xBD,ah);
		if (!(param2 & 0x40) && !(param1 & 0x80)) {
	  		ah |= smallTable[index - 6];
	  		writeOPL(0xBD, ah);
		}
		_regBDConf = ah;
  	}
}

void MusicPlayer::configChannel(uint8 value, uint8* data) {
  	if (smallData2[value] != 0xFF) {
		writeOPL(0xC0 + smallData2[value], data[2]);
  	}
  	writeOPL(0x60 + value, data[4]); // Attack Rate  Decay Rate
  	writeOPL(0x80 + value, data[5]); // Sustain Level  Release Rate
  	writeOPL(0x20 + value, data[1]); // Tremolo  Vibrato   Sustain   KSR   Frequency Multiplication Factor
  	writeOPL(0xE0 + value, data[3]); //  Waveform Select
}

void MusicPlayer::changeOuputLevel(uint8 value, uint8 *data, int bp) {
  	int keyScaleLevel;
  	int outputLevel;
  	if (value == 0xFF)
		return;
  	data++;
  	outputLevel = (*data) & 0x3F;
  	outputLevel = 0x3F - ((((outputLevel * bp)*2) + 0x7F) / 0xFE);
  	keyScaleLevel = data[0] & 0xC0;
	debug(5, "MusicPlayer::changeOutputLevel() unused keyScaleLevel: %d", keyScaleLevel);
  	writeOPL(0x40 + value, (data[0] & 0xC0) | (outputLevel & 0x3F));
}

void MusicPlayer::applyMusicCommandToOPL(ChannelTable2Element *element2, ChannelTableElement *element) {
  	char al;
  	uint16 dx, bp;
  	uint8 operator1, operator2;

  	if ((element2->var4 & 0x40) != element->var2) {
		element->var2 = element2->var4 & 0x40;
		if(element2->var4 & 0x40) {
	  		applyDirectFrequency(element2->index,element2->var15 | 0x80, 0x40, element2->var17);
	  		createDefaultChannel(element2->index);
	  		return;
		}
  	}

  	if (element2->var4 & 0x40)
		return;

  	if ((element->var8 & 1) || (element->var8 != (element2->var4 & 0x8000))) {
		element->var8 = element2->var4&0x8000;
		element->var5 = 0xFF;
		element->var4 = 0xFF;
		element->var0 = 0xFFFF;
		element->var7 = 0x9C;
  	}

  	operator1 = channelTable[element2->index * 2];
  	operator2 = channelTable[(element2->index * 2) + 1];

  	if (operator1 == 0xFF && operator2 == 0xFF) // do we have an operator ?
		return;

	// change channel main config
  	if (element2->var12 != element->var4) {
		element->var4 = element2->var12;
		configChannel(operator1,(_currentMusicPtr2 + 0xD * element2->var12) + 1);
		if (operator2 != 0xFF) {
	  		configChannel(operator2,(_currentMusicPtr2+0xD*element2->var12)+7);
		}
		element->var5 = 0xFF;
  	}

  	// Ouput level handling

  	al = element2->var1D - element2->var1E;

  	if (al < 0)
		al = 0;

  	if (element->var5 != al) {
		int dx2;
		element->var5 = al;
		dx2 = element2->var1D;
		if(operator2==0xFF) {
	  		dx2 = element->var5;
		}
		changeOuputLevel(operator1,_currentMusicPtr2+0xD*element2->var12,dx2);
		if(operator2 != 0xFF) {
	  		changeOuputLevel(operator2,(_currentMusicPtr2+0xD*element2->var12)+6,element->var5);
		}
  	}

  	bp = dx = element2->var17;

  	if (element2->var17 != element->var7) {
		element->var7 = element2->var17;
		if (element2->var15 == element->var0) {
	  		bp |= 0x8000;
		}
  	} else {
		if (element2->var15 == element->var0)
	  	return;
  	}

  	element->var0 = element2->var15 = element2->var15 | 0x8000;

  	applyDirectFrequency(element2->index, element->var0 & 0xFF, element2->var4, bp);
}

int MusicPlayer::update() {

  	ChannelTable2Element *si;
  	if (_generalVolume & 0xFF)
		return 0;
  	for (int i = 0; i < 11; i++) {
		_currentMusicPtr2 = _currentMusicPtr;
		executeMusicCommand(&_channelTable2[i]);
		si = &_channelTable2[i];
		if (_channelTable2[i].var4 & 4) {
	  		_currentMusicPtr2 = _currentMusicPtr3;
	  		si = _channelTable2[i].var2;
	  		executeMusicCommand(_channelTable2[i].var2);
		}
		applyMusicCommandToOPL(si, &_channelDataTable[i]);
  	}
  	return 0;
}

int MusicPlayer::fadeMusic(int param1, int param2, int param3) {
  	int cx;
  	int si;
  	int dx;
  	int bp;
  	//int di = 1;

  	cx = param1;
  	si = param2;
  	dx = param3;

  	bp = si;

  	si = -1;

  	if (!bp)
		bp = 0x7FF;

  	for (int i = 0; i < 11; i++) {
	  	if (_channelTable2[i].dataPtr) {
			if (dx & 0x100) {
		  		//exit(1);
			}
			if (dx & 0x40) {
		  		if (!(_channelTable2[i].var4 & 0x40))
					_channelTable2[i].var4 |= 0x40;
			}
			// start all
			if (dx & 0x80) {
		  		_channelTable2[i].var4 = 0x40;
		  		cx &= 0x7F;
		  		_channelTable2[i].var1D = cx;
		  		_channelTable2[i].var1A = cx;
		  		_channelTable2[i].var1E = 0;
		  		createDefaultChannel(_channelTable2[i].index);
		  		_channelTable2[i].var4 = 2;
			}
			if (dx & 0x20) {
		  		//exit(1);
			}
			if (dx & 0x2000) {
		  		//exit(1);
			}
			if (dx & 0x8000) {
		  		_channelTable2[i].var1A = cx;
			}
			if (dx & 0x1000) {
		  		//exit(1);
			}
			// still running?
			if (dx & 0x10) {
		  		if (!(dx & 0x2000)) {
					if (!(_channelTable2[i].var4 & 0x40)) {
			  			if (si < _channelTable2[i].var18)
							si = _channelTable2[i].var18;
					}
		  		} else {
					if (_channelTable2[i].var1D != cx) {
			  			si = 0;
					}
		  		}
			}
	  	}
  	}

  	return si;
}

void MusicPlayer::loadMusic(char *musicPtr) {
	musicLoad(musicPtr);
	musicStart();
}

void MusicPlayer::playMusic(int musicNumber) {
	if (musicNumber >= 0) {
		if (fadeMusic(0,0,0x10) == -1) {
		  	byte *musicPtr = _vm->_res->loadRawFromPak("MUS.PAK", musicNumber);
		  	_currentMusic = musicNumber;
			fadeMusic(0,0,0x40);
			loadMusic((char*)musicPtr);
			fadeMusic(_musicVolume,0,0x80);
			_playing = true;
		}
  	}
}

void MusicPlayer::stopMusic() {
	_playing = false;
}

} // End of namespace Comet
