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
 * $URL: https://svn.scummvm.org:4444/svn/kult/kult.h $
 * $Id: kult.h 126 2011-04-30 00:09:02Z digitall $
 *
 */

#ifndef KULT_RESOURCE_H
#define KULT_RESOURCE_H

#include "common/array.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "kult/kult.h"

namespace Kult {

class ResourceData {
public:
	ResourceData(uint sizeWidth);
	~ResourceData();
	void loadFromFile(const char *filename);
	void loadFromStream(Common::SeekableReadStream &stream);
	byte *get(uint index, uint &size) const;
	Common::MemoryReadStream *getAsStream(uint index) const;
	uint getCount() const { return _items.size(); }
	byte *getData() const { return _data; }
	uint getDataSize() const { return _dataSize; }
protected:
	Common::Array<byte*> _items;
	uint _sizeWidth;
	byte *_data;
	uint _dataSize;
	uint16 readSize(byte *dataPtr) const { return (_sizeWidth == 1 ? *dataPtr : READ_LE_UINT16(dataPtr)) - _sizeWidth; }	
};

class SpriteResource {
public:
	SpriteResource();
	~SpriteResource();
	void appendFromFile(const char *filename);
	void appendFromStream(Common::SeekableReadStream &stream);
	Graphics::Surface *getSprite(uint index) const { return _sprites[index]; }
	uint getCount() const { return _sprites.size(); }
protected:
	Common::Array<Graphics::Surface*> _sprites;
};

} // End of namespace Kult

#endif /* KULT_RESOURCE_H */
