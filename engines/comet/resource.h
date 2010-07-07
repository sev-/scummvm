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

#ifndef COMET_RESOURCE_H
#define COMET_RESOURCE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/hash-str.h"

#include "common/array.h"
#include "common/str-array.h"

#include "graphics/surface.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "comet/resourcemgr.h"

namespace Comet {

/* GenericResource */

class GenericResource : public BaseResource {
protected:	
	void internalLoad(Common::MemoryReadStream &stream);
};

/* TextResource */

class TextResource : public BaseResource {
public:
	TextResource();
	byte *getString(uint stringIndex);
	void loadString(uint stringIndex, byte *buffer);
	uint count() const { return _stringCount; }
protected:
	byte *_data;
	uint _stringCount;
	uint32 *_stringOffsets;
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
};

/* FontResource */

class FontResource : public BaseResource {
public:
	FontResource();
	void setColor(byte color);
	void drawText(int x, int y, byte *destBuffer, byte *text);
	void drawTextOutlined(int x, int y, byte *destBuffer, byte *text, byte color2, byte color);
	int getTextWidth(byte *text);
private:
	byte *_fontData;
	byte *_charData;
	byte *_charInfo;
	byte _color;
	int _charHeight, _bytesPerLine;
protected:	
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
};

struct Point;

struct AnimationCommand {
	byte cmd;
	byte arg1, arg2;
	Common::Array<Point> points;
};

struct AnimationElement {
	byte width, height, flags;
	Common::Array<AnimationCommand*> commands;
	~AnimationElement();
};

struct AnimationCel {
	uint16 flags;
	uint16 width, height;
	uint16 dataSize;
	byte *data;
};

struct AnimationFrame {
	uint16 elementIndex;
	uint16 flags;
	int16 xOffs, yOffs;
};

struct AnimationFrameList {
	byte priority;
	Common::Array<AnimationFrame*> frames;
	~AnimationFrameList();
};

class AnimationResource : public BaseResource {
public:
	AnimationResource();
	int16 getCelWidth(int16 celIndex) const { return _cels[celIndex]->width; }
	int16 getCelHeight(int16 celIndex) const { return _cels[celIndex]->height; }
//protected://all public while in progress
	typedef Common::Array<uint32> OffsetArray;
	Common::Array<AnimationElement*> _elements;
	Common::Array<AnimationCel*> _cels;
	Common::Array<AnimationFrameList*> _anims;
	// TODO: Section 4 is palette
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
	void loadOffsets(Common::SeekableReadStream &sourceS, OffsetArray &offsets);
	AnimationElement *loadAnimationElement(Common::SeekableReadStream &sourceS);
	AnimationCommand *loadAnimationCommand(Common::SeekableReadStream &sourceS, bool ptAsByte);
	AnimationFrameList *loadAnimationFrameList(Common::SeekableReadStream &sourceS);
};

/* ScreenResource */

class ScreenResource : public BaseResource {
public:
	ScreenResource();
protected:
	byte *_screen;
	void free();	
	void internalLoad(Common::MemoryReadStream &stream);
};

/* SoundResource */

class SoundResource : public BaseResource {
public:
	SoundResource();
	Audio::SeekableAudioStream *makeAudioStream();
protected:
	byte *_data;
	int32 _dataSize;
	void free();	
	void internalLoad(Common::MemoryReadStream &stream);
};

/* ScriptResource */

class ScriptResource : public BaseResource {
public:
	ScriptResource();
	byte *getScript(int index);
	int getCount();
protected:
	byte *_scriptData;
	void free();	
	void internalLoad(Common::MemoryReadStream &stream);
};

} // End of namespace Comet

#endif /* COMET_RESOURCE_H */
