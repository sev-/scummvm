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

#include "comet/resource.h"
#include "comet/graphics.h"
#include "comet/screen.h"
#include "common/file.h"
#include "common/stream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"

namespace Comet {

// GenericResource

void GenericResource::internalLoad(Common::MemoryReadStream &stream) {
	debug("GenericResource::internalLoad() stream.size = %d", stream.size());
}

// TextResource

TextResource::TextResource() : _data(NULL), _stringCount(0), _stringOffsets(NULL) {
}

TextResource::~TextResource() {
	free();
}

void TextResource::free() {
	delete[] _stringOffsets;
	delete[] _data;
}

void TextResource::internalLoad(Common::MemoryReadStream &stream) {
	uint32 firstOffs = stream.readUint32LE();
	uint32 size = stream.size();
	_stringCount = firstOffs / 4;
	_stringOffsets = new uint32[_stringCount + 1];
	_stringOffsets[0] = 0;
	for (uint i = 1; i < _stringCount; i++)
		_stringOffsets[i] = stream.readUint32LE() - firstOffs;
	_stringOffsets[_stringCount] = size - firstOffs;
	size -= firstOffs; // size of text data
	_data = new byte[size];
	stream.read(_data, size);
	// decrypt the text data
	for (uint i = 0; i < size; i++)
		_data[i] = _data[i] - 0x54 * (i + 1);
}

byte *TextResource::getString(uint stringIndex) {
	//	We add 1 to the offset since that's where the actual text starts.
	//	The leftover byte at the beginning is the '*'-character, which
	//	serves as string terminator for the preceeding string.
	return _data + _stringOffsets[stringIndex] + 1;
}

void TextResource::loadString(uint stringIndex, byte *buffer) {
	int stringLen = _stringOffsets[stringIndex + 1] - _stringOffsets[stringIndex];
	memcpy(buffer, getString(stringIndex), stringLen);
}

// FontResource

FontResource::FontResource() : _fontData(NULL) {
}

FontResource::~FontResource() {
	free();
}

void FontResource::free() {
	delete[] _fontData;
}

void FontResource::internalLoad(Common::MemoryReadStream &stream) {
	_fontData = new byte[stream.size()];
	stream.read(_fontData, stream.size());
	uint16 skipChars = _fontData[0] * 2;
	_charHeight = _fontData[2];
	_bytesPerLine = _fontData[3];
	if (_bytesPerLine == 0)
		_bytesPerLine = READ_LE_UINT16(_fontData + 4);
	_charData = _fontData + 8;
	_charInfo = _fontData + READ_BE_UINT16(_fontData + 6) - skipChars;
}

void FontResource::drawText(int x, int y, byte *destBuffer, const byte *text, byte color) {
	static const byte startFlags[] = {
		0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
	};

	while (*text) {
		uint16 charOfs = text[0] * 2;
		uint16 charInfo = READ_BE_UINT16(_charInfo + charOfs);
		byte charWidth = (charInfo & 0xF000) >> 12;

		if (charWidth > 0) {
			charInfo = charInfo & 0x0FFF;
			byte *charData = _charData + (charInfo >> 3);
			for (int h = 0; h < _charHeight; h++) {
				byte charMask = startFlags[charInfo & 7];
				byte charByte = charData[0];
				byte dataOfs = 0;
				for (int w = 0; w < charWidth; w++) {
					if (charMask & charByte)
						destBuffer[(x + w) + (y + h) * 320] = color;
					if (charMask & 1) {
						dataOfs++;
						charByte = charData[dataOfs];
						charMask = 0x80;
					} else {
						charMask >>= 1;
					}
				}
				charData += _bytesPerLine;
			}
			x += charWidth + 1;
		} else {
			x += 3;
		}
		text++;
	}
}

int FontResource::getTextWidth(const byte *text) {
	int textWidth = 0;
	while (*text) {
		uint16 charOfs = text[0] * 2;
		uint16 charInfo = READ_BE_UINT16(_charInfo + charOfs);
		byte charWidth = (charInfo & 0xF000) >> 12;
		if (charWidth > 0)
			textWidth += charWidth + 1;
		else
			textWidth += 3;
		text++;
	}
	return textWidth;
}

// AnimationCommand

void AnimationCommand::loadFromStream(Common::SeekableReadStream &stream, bool ptAsByte) {
	_cmd = stream.readByte();
	byte pointsCount = stream.readByte();
	_arg1 = stream.readByte();
	_arg2 = stream.readByte();
	debug(8, "AnimationCommand::loadFromStream() cmd = %d; pointsCount = %d; arg1 = %d; arg2 = %d",
		_cmd, pointsCount, _arg1, _arg2);
	while (pointsCount--) {
		Common::Point pt;
		if (ptAsByte) {
			pt.x = (int8)stream.readByte();
			pt.y = (int8)stream.readByte();
		} else {
			pt.x = (int16)stream.readUint16LE();
			pt.y = (int16)stream.readUint16LE();
		}
		debug(8, "AnimationCommand::loadFromStream()	 x = %d; y = %d", pt.x, pt.y);
		_points.push_back(pt);
	}
}

void AnimationCommand::draw(CometSurface *destSurface, AnimationResource *animation, int16 x, int16 y, byte parentFlags) {

	debug(8, "Screen::drawAnimationCommand() cmd = %d; points = %d", _cmd, _points.size());

	Common::Array<Common::Point> points;

	// The commands' points need to be adjusted according to the parentFlags and the x/y position
	points.reserve(_points.size() + 1);
	for (uint i = 0; i < _points.size(); i++) {
		int16 ax = _points[i].x;
		int16 ay = _points[i].y;
		if (parentFlags & 0x80)
			ax = -ax;
		if (parentFlags & 0x20)
			ay = -ay;
		points.push_back(Common::Point(x + ax, y + ay));
	}

	switch (_cmd) {

	case kActElement:
	{
		int16 subElementIndex = ((_arg2 << 8) | _arg1) & 0x0FFF;
		AnimationElement *subElement = animation->getElement(subElementIndex);
		subElement->draw(destSurface, animation, points[0].x, points[0].y, parentFlags | (_arg2 & 0xA0));
		break;
	}

	case kActCelSprite:
	{
		int16 celIndex = ((_arg2 << 8) | _arg1) & 0x0FFF;
		int16 celX = points[0].x, celY = points[0].y;
		if (parentFlags & 0x80)
			celX -= animation->getCelWidth(celIndex);
		if (parentFlags & 0x20)
			celY += animation->getCelHeight(celIndex);
		destSurface->drawAnimationCelSprite(*animation->getCel(celIndex), celX, celY, parentFlags | (_arg2 & 0xA0));
		break;
	}

	case kActFilledPolygon:
		destSurface->drawFilledPolygon(points, _arg2, _arg1);
		break;

	case kActRectangle:
		destSurface->drawRectangle(points, _arg2, _arg1);
		break;

	case kActPolygon:
		destSurface->drawPolygon(points, _arg2);
		break;

	case kActPixels:
		destSurface->drawPixels(points, _arg2); 
		break;

	case kActCelRle:
	{
		AnimationCel *cel = animation->getCel((_arg2 << 8) | _arg1);
		destSurface->drawAnimationCelRle(*cel, points[0].x, points[0].y - cel->getHeight() + 1);
		break;
	}

	default:
		warning("Screen::drawAnimationCommand() Unknown command %d", _cmd);

	}

}

// AnimationElement

AnimationElement::~AnimationElement() {
	for (Common::Array<AnimationCommand*>::iterator iter = _commands.begin(); iter != _commands.end(); ++iter)
		delete (*iter);
}

void AnimationElement::loadFromStream(Common::SeekableReadStream &stream) {
	_width = stream.readByte();
	_height = stream.readByte();
	_flags = stream.readByte();
	byte cmdCount = stream.readByte();
	debug(8, "AnimationElement::loadFromStream() cmdCount = %d; flags = %02X", cmdCount, _flags);
	while (cmdCount--) {
		AnimationCommand *animationCommand = new AnimationCommand();
		animationCommand->loadFromStream(stream, _flags & 0x10);
		_commands.push_back(animationCommand);
	}
}

void AnimationElement::draw(CometSurface *destSurface, AnimationResource *animation, int16 x, int16 y, byte parentFlags) {
	byte flags = _flags | (parentFlags & 0xA0);
	for (Common::Array<AnimationCommand*>::iterator iter = _commands.begin(); iter != _commands.end(); ++iter)
		(*iter)->draw(destSurface, animation, x, y, flags);
}

// AnimationCel

AnimationCel::~AnimationCel() {
	if (_dataSize > 0)
		delete[] _data;
}

void AnimationCel::loadFromStream(Common::SeekableReadStream &stream) {
	_flags = stream.readUint16LE();
	_width = stream.readByte() * 16;
	_height = stream.readByte();
	byte *data = new byte[_dataSize];
	stream.read(data, _dataSize);
	_data = data;
	debug(8, "AnimationCel::loadFromStream() cel width = %d; height = %d; dataSize = %d", _width, _height, _dataSize);
}

// AnimationFrame

void AnimationFrame::loadFromStream(Common::SeekableReadStream &stream) {
	_elementIndex = stream.readUint16LE();
	_flags = stream.readUint16LE();
	_xOffs = (int16)stream.readUint16LE();
	_yOffs = (int16)stream.readUint16LE();
	debug(0, "AnimationFrame::loadFromStream() elementIndex = %d; flags = %04X; xOffs = %d; yOffs = %d",
		_elementIndex, _flags, _xOffs, _yOffs);
}

void AnimationFrame::accumulateDrawOffset(int &x, int &y) {
	x += _xOffs;
	y += _yOffs;
}

// AnimationFrameList

AnimationFrameList::~AnimationFrameList() {
	for (Common::Array<AnimationFrame*>::iterator iter = _frames.begin(); iter != _frames.end(); ++iter)
		delete (*iter);
}

void AnimationFrameList::loadFromStream(Common::SeekableReadStream &stream) {
	_priority = stream.readByte();
	byte frameCount = stream.readByte();
	debug(8, "AnimationFrameList::loadFromStream() frameCount = %d", frameCount);
	while (frameCount--) {
		AnimationFrame *animationFrame = new AnimationFrame();
		animationFrame->loadFromStream(stream);
		_frames.push_back(animationFrame);
	}
}

void AnimationFrameList::accumulateDrawOffset(int &x, int &y, int targetFrameIndex) {
	for (int frameIndex = 0; frameIndex <= targetFrameIndex; ++frameIndex)
		_frames[frameIndex]->accumulateDrawOffset(x, y);
}

// AnimationResource

AnimationResource::AnimationResource() {
}

AnimationResource::~AnimationResource() {
	free();
}

void AnimationResource::drawElement(CometSurface *destSurface, int elementIndex, int16 x, int16 y, byte parentFlags) {
	_elements[elementIndex]->draw(destSurface, this, x, y, parentFlags);
}

AnimationCel *AnimationResource::getCelByElementCommand(int elementIndex, int commandIndex) {
	AnimationCommand *cmd = _elements[elementIndex]->getCommand(commandIndex);
	return _cels[((cmd->_arg2 << 8) | cmd->_arg1) & 0x0FFF];
}

AnimationCommand *AnimationResource::getElementCommand(int elementIndex, int commandIndex) {
	return _elements[elementIndex]->_commands[commandIndex];
}

void AnimationResource::free() {
	for (Common::Array<AnimationElement*>::iterator iter = _elements.begin(); iter != _elements.end(); ++iter)
		delete (*iter);
	for (Common::Array<AnimationCel*>::iterator iter = _cels.begin(); iter != _cels.end(); ++iter)
		delete (*iter);
	for (Common::Array<AnimationFrameList*>::iterator iter = _anims.begin(); iter != _anims.end(); ++iter)
		delete (*iter);
}

void AnimationResource::internalLoad(Common::MemoryReadStream &stream) {
	OffsetArray sectionOffsets, offsets;

	loadOffsets(stream, sectionOffsets);

	if (sectionOffsets.size() < 4)
		error("Animation::load() Unexpected section count");

	// Load animation elements
	stream.seek(sectionOffsets[0]);
	loadOffsets(stream, offsets);
	for (uint i = 0; i < offsets.size(); i++) {
		stream.seek(sectionOffsets[0] + offsets[i] - 2);
		AnimationElement *animationElement = new AnimationElement();
		animationElement->loadFromStream(stream);
		_elements.push_back(animationElement);
	}
	
	// Load animation cels
	stream.seek(sectionOffsets[1]);
	loadOffsets(stream, offsets);
	offsets.push_back(sectionOffsets[2] - sectionOffsets[1]);
	for (uint i = 0; i < offsets.size() - 1; i++) {
		uint32 celDataSize = offsets[i + 1] - offsets[i] - 2;
		stream.seek(sectionOffsets[1] + offsets[i] - 2);
		AnimationCel *animationCel = new AnimationCel(celDataSize);
		animationCel->loadFromStream(stream);
		_cels.push_back(animationCel);
	}

	// Load animation frames
	stream.seek(sectionOffsets[2]);
	loadOffsets(stream, offsets);
	for (uint i = 0; i < offsets.size(); i++) {
		stream.seek(sectionOffsets[2] + offsets[i]);
		AnimationFrameList *animationFrameList = new AnimationFrameList();
		animationFrameList->loadFromStream(stream);
		_anims.push_back(animationFrameList);
	}

	// NOTE Load section 4 data (never used in Comet, maybe in Eternam)
}

void AnimationResource::loadOffsets(Common::SeekableReadStream &sourceS, OffsetArray &offsets) {
	offsets.clear();
	uint32 offset = sourceS.readUint32LE();
	uint count = offset / 4;
	debug(8, "Animation::loadOffsets() count = %d", count);
	while (count--) {
		offsets.push_back(offset);
		offset = sourceS.readUint32LE();
	}
}

// ScreenResource

ScreenResource::ScreenResource() : _screen(NULL) {
}

ScreenResource::~ScreenResource() {
	free();
}

void ScreenResource::free() {
	delete[] _screen;
}

void ScreenResource::internalLoad(Common::MemoryReadStream &stream) {
	if (stream.size() != 64000)
		error("ScreenResource::internalLoad() Unexpected data size (%d)", stream.size());
	_screen = new byte[stream.size()];
	stream.read(_screen, stream.size());
}

// PaletteResource

PaletteResource::PaletteResource() : _palette(NULL) {
}

PaletteResource::~PaletteResource() {
	free();
}

void PaletteResource::free() {
	delete[] _palette;
}

void PaletteResource::internalLoad(Common::MemoryReadStream &stream) {
	if (stream.size() != 768)
		error("PaletteResource::internalLoad() Unexpected data size (%d)", stream.size());
	_palette = new byte[stream.size()];
	stream.read(_palette, stream.size());	
}

// SoundResource

SoundResource::SoundResource() : _data(NULL), _dataSize(0) {
}

SoundResource::~SoundResource() {
	free();
}

void SoundResource::free() {
	delete[] _data;
}
	
void SoundResource::internalLoad(Common::MemoryReadStream &stream) {
	_dataSize = stream.size();
	_data = new byte[_dataSize];
	stream.read(_data, _dataSize);
	// For speech sounds, the VOC header's first byte is '\0' instead of 'C' so we have to work around it
	_data[0] = 'C';
}

Audio::SeekableAudioStream *SoundResource::makeAudioStream() {
	Common::MemoryReadStream *vocStream = new Common::MemoryReadStream(_data, _dataSize, DisposeAfterUse::NO);
	return Audio::makeVOCStream(vocStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
}

// ScriptResource

ScriptResource::ScriptResource() : _scriptData(NULL) {
}

ScriptResource::~ScriptResource() {
	free();
}

byte *ScriptResource::getScript(int index) {
	return _scriptData + READ_LE_UINT16(_scriptData + index * 2);
}

int ScriptResource::getCount() {
	return READ_LE_UINT16(_scriptData) / 2;
}

void ScriptResource::free() {
	delete[] _scriptData;
}

void ScriptResource::internalLoad(Common::MemoryReadStream &stream) {
	_scriptData = new byte[stream.size()];
	stream.read(_scriptData, stream.size());
}

} // End of namespace Comet
