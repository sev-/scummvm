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
#include "sound/decoders/wave.h"

#include "comet/resource.h"

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

TextResource::TextResource() : _data(NULL), _stringCount(0) {
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

} // End of namespace Prisoner
