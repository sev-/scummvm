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
 * $URL: https://svn.scummvm.org:4444/svn/kult/kult.cpp $
 * $Id: kult.cpp 127 2011-05-04 04:52:04Z digitall $
 *
 */

#include "common/error.h"
#include "graphics/surface.h"
#include "engines/util.h"
#include "kult/kult.h"
#include "kult/resource.h"

namespace Kult {

// ResourceData

ResourceData::ResourceData(uint sizeWidth) : _sizeWidth(sizeWidth) {
}

ResourceData::~ResourceData() {
}

void ResourceData::loadFromFile(const char *filename) {
	Common::File fd;
	if (!fd.open(filename))
		error("ResourceData::loadFromFile() Could not open %s", filename);
	loadFromStream(fd);
}

void ResourceData::loadFromStream(Common::SeekableReadStream &stream) {
	_dataSize = stream.size();
	_data = new byte[_dataSize];
	stream.read(_data, _dataSize);
	for (byte *dataPtr = _data; dataPtr + _sizeWidth < _data + _dataSize; ) {		
		uint16 size = readSize(dataPtr);
		_items.push_back(dataPtr);
		dataPtr += _sizeWidth + size;
	}
}

byte *ResourceData::get(uint index, uint &size) const {
	size = readSize(_items[index]);
	return _items[index] + _sizeWidth; 
}

Common::MemoryReadStream *ResourceData::getAsStream(uint index) const {
	uint size;
	byte *data = get(index, size);
	return new Common::MemoryReadStream(data, size);
}

// SpriteResource

SpriteResource::SpriteResource() {
}

SpriteResource::~SpriteResource() {
}

void SpriteResource::appendFromFile(const char *filename) {
	Common::File fd;
	if (!fd.open(filename))
		error("SpriteResource::appendFromFile() Could not open %s", filename);
	appendFromStream(fd);
}

void SpriteResource::appendFromStream(Common::SeekableReadStream &stream) {
	stream.skip(4); // Skip junk
	while (1) {		
		uint16 size = stream.readUint16LE();
		byte width = stream.readByte();
		byte height = stream.readByte();
		debug("size = %d; width = %d; height = %d", size, width, height);
		if (stream.eos())
			break;
		size -= 4;
		// Decompress the sprite to 8bpp for easier handling
		Graphics::Surface *sprite = new Graphics::Surface();
    	sprite->create(width * 4, height, Graphics::PixelFormat::createFormatCLUT8());
		byte *dstPixels = (byte*)sprite->pixels;
        for (uint i = 0; i < size; i++) {
        	byte p = stream.readByte();
            *dstPixels++ = (p >> 4) & 0x0F;
            *dstPixels++ = p & 0x0F;
        }
        _sprites.push_back(sprite);
	}
}

} // End of namespace Kult
