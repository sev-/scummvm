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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef COMET_MUSIC_H
#define COMET_MUSIC_H

#include "common/mutex.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/fmopl.h"

#include "comet/comet.h"

// FIXME
#define musicSync 1500

namespace Comet {

struct ChannelTable2Element {
	uint16 index;
	ChannelTable2Element *var2;
	uint16 var4;
	uint8* dataPtr;
	uint8* commandPtr;
	unsigned short varE;
	uint16 var10;
	uint8 var12;
	uint16 var13;
	uint16 var15;
	uint8 var17;
	uint16 var18;
	uint8 var1A;
	uint8 var1B;
	uint8 var1C;
	uint8 var1D;
	uint8 var1E;
};

struct ChannelTableElement {
	uint16 var0;
	uint16 var2;
	uint8 var4;
	uint8 var5;
	uint8 var6;
	uint8 var7;
	uint16 var8;
};

// FIXEM: Remove globals
extern unsigned char channelTableMelodic[];
extern unsigned char channelTableRythme[];
extern unsigned char* channelTable;
extern uint16 globTableEntry[300];
extern uint16* globTable[13];
extern unsigned char smallData2[];
extern uint8 smallTable[];

class MusicPlayer : public Audio::AudioStream {
public:
	MusicPlayer(CometEngine *vm);
	~MusicPlayer();

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }
	bool playing() const { return _playing; }

	void playMusic(int musicNumber);
	void stopMusic();

// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return !_playing; }
	bool endOfStream() const { return false; }
	int getRate() const { return _rate; }
	
protected:
	Audio::SoundHandle _handle;
	FM_OPL *_opl;
	bool _playing;
	bool _first;
	bool _ended;
	Common::Mutex _mutex;
	CometEngine *_vm;
 	uint32 _rate;

	int _musicVolume;
	int _currentMusic;
	unsigned int _musicChrono;
	unsigned char _musicParam1;
	uint8 *_currentMusicPtr;
	uint8 *_currentMusicPtr2;
	uint8 *_currentMusicPtr3;
	uint8 _generalVolume;
	int _fadeParam[3];
	int _musicTimer;
	int _nextUpdateTimer;
	unsigned char _regBDConf;
	
	ChannelTableElement _channelDataTable[11];
	ChannelTable2Element _channelTable2[11], _channelTable3[11];

	void initTables();
	void writeOPL(byte reg, byte val);
	void createDefaultChannel(int index);
	void resetChannelFrequency(int channelIdx);
	void setupChannelFrequency(int channelIdx, int cl, int dx,int bp);
	int musicStart();
	int musicLoad(void *ptr);
	void executeMusicCommand(ChannelTable2Element *entry);
	void applyDirectFrequency(int index, int param1, int param2, int param3);
	void configChannel(uint8 value, uint8* data);
	void changeOuputLevel(uint8 value, uint8 *data, int bp);
	void applyMusicCommandToOPL(ChannelTable2Element *element2, ChannelTableElement *element);
	int update();
	int fadeMusic(int param1, int param2, int param3);
	void loadMusic(char *musicPtr);

};

} // End of namespace Gob

#endif // COMET_MUSIC_H
