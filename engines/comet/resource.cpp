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

#include "common/file.h"

#include "common/stream.h"
#include "common/zlib.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/decoders/raw.h"
#include "sound/decoders/voc.h"

#include "comet/resource.h"
#include "comet/screen.h"

namespace Comet {

/* GenericResource */

void GenericResource::internalLoad(Common::MemoryReadStream &stream) {
	
	debug("GenericResource::internalLoad() stream.size = %d", stream.size());
	
	FILE *x = fopen("dump.0", "wb");
	while (!stream.eos()) {
		byte b = stream.readByte();
		fwrite(&b, 1, 1, x);
	}
	fclose(x);

}

/* TextResource */

TextResource::TextResource() : _data(NULL), _stringCount(0), _stringOffsets(NULL) {
}

void TextResource::free() {
	delete _stringOffsets;
	delete _data;
}

void TextResource::internalLoad(Common::MemoryReadStream &stream) {
	uint32 firstOffs = stream.readUint32LE();
	uint32 size;
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
	/*	We add 1 to the offset since that's where the actual text starts.
		The leftover byte at the beginning is the '*'-character, which
		serves as string terminator for the preceeding string. */
	return _data + _stringOffsets[stringIndex] + 1;
}

void TextResource::loadString(uint stringIndex, byte *buffer) {
	int stringLen = _stringOffsets[stringIndex + 1] - _stringOffsets[stringIndex];
	memcpy(buffer, getString(stringIndex), stringLen);
}

/* FontResource */

FontResource::FontResource() : _fontData(NULL) {
}

void FontResource::free() {
	delete _fontData;
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

void FontResource::setColor(byte color) {
	_color = color;
}

void FontResource::drawText(int x, int y, byte *destBuffer, byte *text) {

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
						destBuffer[(x + w) + (y + h) * 320] = _color;
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

void FontResource::drawTextOutlined(int x, int y, byte *destBuffer, byte *text, byte color2, byte color) {
	setColor(color);
	drawText(x + 1, y + 1, destBuffer, text);
	drawText(x + 1, y - 1, destBuffer, text);
	drawText(x + 1, y, destBuffer, text);
	drawText(x - 1, y, destBuffer, text);
	drawText(x, y + 1, destBuffer, text);
	drawText(x, y - 1, destBuffer, text);
	drawText(x - 1, y + 1, destBuffer, text);
	drawText(x - 1, y - 1, destBuffer, text);
	setColor(color2);
	drawText(x, y, destBuffer, text);
}

int FontResource::getTextWidth(byte *text) {
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

/* AnimationResource */

AnimationElement::~AnimationElement() {
	for (Common::Array<AnimationCommand*>::iterator iter = commands.begin(); iter != commands.end(); ++iter)
		delete (*iter);
}

AnimationFrameList::~AnimationFrameList() {
	for (Common::Array<AnimationFrame*>::iterator iter = frames.begin(); iter != frames.end(); ++iter)
		delete (*iter);
}

AnimationResource::AnimationResource() {
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
		AnimationElement *animationElement = loadAnimationElement(stream);
		_elements.push_back(animationElement);
	}
	
	// Load animation cels
	stream.seek(sectionOffsets[1]);
	loadOffsets(stream, offsets);
	offsets.push_back(sectionOffsets[2] - sectionOffsets[1]);
	for (uint i = 0; i < offsets.size() - 1; i++) {
		stream.seek(sectionOffsets[1] + offsets[i] - 2);
		AnimationCel *animationCel = new AnimationCel();
		animationCel->flags = stream.readUint16LE();
		animationCel->width = stream.readByte() * 16;
		animationCel->height = stream.readByte();
		animationCel->dataSize = offsets[i + 1] - offsets[i] - 2;
		animationCel->data = new byte[animationCel->dataSize];
		stream.read(animationCel->data, animationCel->dataSize);
		debug(8, "Animation::load() cel width = %d; height = %d; dataSize = %d", animationCel->width, animationCel->height, animationCel->dataSize);
		_cels.push_back(animationCel);
	}

	// Load animation frames
	stream.seek(sectionOffsets[2]);
	loadOffsets(stream, offsets);
	for (uint i = 0; i < offsets.size(); i++) {
		stream.seek(sectionOffsets[2] + offsets[i]);
		AnimationFrameList *animationFrameList = loadAnimationFrameList(stream);
		_anims.push_back(animationFrameList);
	}

	// TODO: Load section 4 data

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

AnimationElement *AnimationResource::loadAnimationElement(Common::SeekableReadStream &sourceS) {
	AnimationElement *animationElement = new AnimationElement();
	animationElement->width = sourceS.readByte();
	animationElement->height = sourceS.readByte();
	animationElement->flags = sourceS.readByte();
	byte cmdCount = sourceS.readByte();
	debug(8, "Animation::loadAnimationElement() cmdCount = %d; flags = %02X", cmdCount, animationElement->flags);
	while (cmdCount--) {
		AnimationCommand *animationCommand = loadAnimationCommand(sourceS, animationElement->flags & 0x10);
		animationElement->commands.push_back(animationCommand);
	}
	return animationElement;
}

AnimationCommand *AnimationResource::loadAnimationCommand(Common::SeekableReadStream &sourceS, bool ptAsByte) {
	AnimationCommand *animationCommand = new AnimationCommand();
	animationCommand->cmd = sourceS.readByte();
	byte pointsCount = sourceS.readByte();
	animationCommand->arg1 = sourceS.readByte();
	animationCommand->arg2 = sourceS.readByte();
	debug(8, "Animation::loadAnimationCommand() cmd = %d; pointsCount = %d; arg1 = %d; arg2 = %d",
		animationCommand->cmd, pointsCount, animationCommand->arg1, animationCommand->arg2);
	while (pointsCount--) {
		Point pt;
		if (ptAsByte) {
			pt.x = (int8)sourceS.readByte();
			pt.y = (int8)sourceS.readByte();
		} else {
			pt.x = (int16)sourceS.readUint16LE();
			pt.y = (int16)sourceS.readUint16LE();
		}
		debug(8, "Animation::loadAnimationCommand()	 x = %d; y = %d", pt.x, pt.y);
		animationCommand->points.push_back(pt);
	}
	return animationCommand;
}

AnimationFrameList *AnimationResource::loadAnimationFrameList(Common::SeekableReadStream &sourceS) {
	AnimationFrameList *animationFrameList = new AnimationFrameList();
	animationFrameList->priority = sourceS.readByte();
	byte frameCount = sourceS.readByte();
	debug(8, "Animation::loadAnimationFrameList() frameCount = %d", frameCount);
	while (frameCount--) {
		AnimationFrame *animationFrame = new AnimationFrame();
		animationFrame->elementIndex = sourceS.readUint16LE();
		animationFrame->flags = sourceS.readUint16LE();
		animationFrame->xOffs = (int16)sourceS.readUint16LE();
		animationFrame->yOffs = (int16)sourceS.readUint16LE();
		debug(0, "Animation::loadAnimationFrameList() elementIndex = %d; flags = %04X; xOffs = %d; yOffs = %d",
			animationFrame->elementIndex, animationFrame->flags, animationFrame->xOffs, animationFrame->yOffs);
		animationFrameList->frames.push_back(animationFrame);
	}
	return animationFrameList;
}

/* ScreenResource */

ScreenResource::ScreenResource() : _screen(NULL) {
}

void ScreenResource::free() {
	delete[] _screen;
}

void ScreenResource::internalLoad(Common::MemoryReadStream &stream) {
	_screen = new byte[stream.size()];
	stream.read(_screen, stream.size());	
}

/* SoundResource */

SoundResource::SoundResource() : _data(NULL), _dataSize(0) {
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
	Common::MemoryReadStream vocStream(_data, _dataSize, DisposeAfterUse::NO);
	return Audio::makeVOCStream(&vocStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
}

/* ScriptResource */

ScriptResource::ScriptResource() : _scriptData(NULL) {
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

} // End of namespace Prisoner
