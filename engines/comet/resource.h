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
#include "common/memstream.h"
#include "common/system.h"
#include "common/hash-str.h"

#include "common/array.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "comet/resourcemgr.h"
#include "common/rect.h"

namespace Comet {

class CometSurface;

// GenericResource

class GenericResource : public BaseResource {
protected:
	void internalLoad(Common::MemoryReadStream &stream);
};

// TextResource

class TextResource : public BaseResource {
public:
	TextResource();
	~TextResource();

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

// FontResource

class FontResource : public BaseResource {
public:
	FontResource();
	~FontResource();

	void setColor(byte color);
	void drawText(int x, int y, byte *destBuffer, const byte *text, byte color);
	int getTextWidth(const byte *text);
private:
	byte *_fontData;
	byte *_charData;
	byte *_charInfo;
	int _charHeight, _bytesPerLine;
protected:
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
};

enum AnimationCommandType {
	kActElement			= 0,
	kActCelSprite		= 1,
	kActNop0			= 2,
	kActNop1			= 3,
	kActFilledPolygon	= 4,
	kActRectangle		= 5,
	kActPolygon			= 6,
	kActPixels			= 7,
	kActPolygon1		= 8,	// unused in Comet? / Alias for kActPolygon
	kActPolygon2		= 9,	// unused in Comet? / Alias for kActPolygon
	kActCelRle			= 10
};

class AnimationCommand {
public:
	byte _cmd;
	byte _arg1, _arg2;
	Common::Array<Common::Point> _points;
	void loadFromStream(Common::SeekableReadStream &stream, bool ptAsByte);
	void draw(CometSurface *destSurface, AnimationResource *animation, int16 x, int16 y, byte parentFlags);
};

class AnimationElement {
public:
	byte _width, _height, _flags;
	Common::Array<AnimationCommand*> _commands;
	~AnimationElement();
	void loadFromStream(Common::SeekableReadStream &stream);
	void draw(CometSurface *destSurface, AnimationResource *animation, int16 x, int16 y, byte parentFlags = 0);
};

struct AnimationCel {
	uint16 _flags;
	uint16 _width, _height;
	uint16 _dataSize;
	byte *_data;
	AnimationCel(uint32 dataSize) : _flags(0), _width(0), _height(0), _dataSize(dataSize), _data(0) {}
	~AnimationCel() { delete[] _data; }
	void loadFromStream(Common::SeekableReadStream &stream);
};

struct AnimationFrame {
	uint16 _elementIndex;
	uint16 _flags;
	int16 _xOffs, _yOffs;
	void loadFromStream(Common::SeekableReadStream &stream);
};

struct AnimationFrameList {
	byte _priority;
	Common::Array<AnimationFrame*> _frames;
	~AnimationFrameList();
	void loadFromStream(Common::SeekableReadStream &stream);
};

class AnimationResource : public BaseResource {
public:
	AnimationResource();
	~AnimationResource();

	int16 getCelWidth(int16 celIndex) const { return _cels[celIndex]->_width; }
	int16 getCelHeight(int16 celIndex) const { return _cels[celIndex]->_height; }
	AnimationCel *getCelByElementCommand(int elementIndex, int commandIndex);
	AnimationCommand *getElementCommand(int elementIndex, int commandIndex);

//protected://all public while in progress
	typedef Common::Array<uint32> OffsetArray;
	Common::Array<AnimationElement*> _elements;
	Common::Array<AnimationCel*> _cels;
	Common::Array<AnimationFrameList*> _anims;
	// TODO: Section 4 is palette
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
	void loadOffsets(Common::SeekableReadStream &sourceS, OffsetArray &offsets);
};

// ScreenResource

class ScreenResource : public BaseResource {
public:
	ScreenResource();
	~ScreenResource();

	byte *getScreen() const { return _screen; }
protected:
	byte *_screen;
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
};

// PaletteResource

class PaletteResource : public BaseResource {
public:
	PaletteResource();
	~PaletteResource();

	byte *getPalette() const { return _palette; }
protected:
	byte *_palette;
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
};

// SoundResource

class SoundResource : public BaseResource {
public:
	SoundResource();
	~SoundResource();

	Audio::SeekableAudioStream *makeAudioStream();
protected:
	byte *_data;
	int32 _dataSize;
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
};

// ScriptResource

class ScriptResource : public BaseResource {
public:
	ScriptResource();
	~ScriptResource();

	byte *getScript(int index);
	int getCount();
protected:
	byte *_scriptData;
	void free();
	void internalLoad(Common::MemoryReadStream &stream);
};

} // End of namespace Comet

#endif
