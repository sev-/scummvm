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

#include "common/debug.h"
#include "common/stream.h"

#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/unpack.h"

namespace Comet {

// BaseResource

void BaseResource::load(Common::MemoryReadStream &stream) {
	free();
	internalLoad(stream);
}

// PakResourceLoader

PakResourceLoader::PakResourceLoader() {
}

PakResourceLoader::~PakResourceLoader() {
}

byte *PakResourceLoader::load(const char *filename, int index, uint32 &dataSize) {

	Common::File fd;
	PakEntry pakEntry;
	uint32 offset;
	byte *data = NULL;
	
	debug(1, "load('%s', %d)", filename, index);
	
	if (!fd.open(filename))
		error("PakResourceLoader::load() Could not open %s", filename);

	fd.seek((index + 1) * 4);
	offset = fd.readUint32LE();
	fd.seek(offset);

	fd.readUint32LE();
	pakEntry.discSize = fd.readUint32LE();
	pakEntry.uncompressedSize = fd.readUint32LE();
	pakEntry.compressionType = fd.readByte();
	pakEntry.flags = fd.readByte();
	pakEntry.nameLen = fd.readUint16LE();
	// Skip filename which may or may not be present
	fd.seek(pakEntry.nameLen, SEEK_CUR);

	switch (pakEntry.compressionType) {
	case 0:
	{
		dataSize = pakEntry.discSize;
		data = (byte*)malloc(dataSize);
		fd.read(data, dataSize);
		break;
	}
	case 1:
	{
		DecompressImplode dec;
		dataSize = pakEntry.uncompressedSize;
		data = (byte*)malloc(dataSize);
		dec.decompress(&fd, pakEntry.flags, pakEntry.uncompressedSize, data);
		break;
	}
	default:
		error("PakResourceLoader::load('%s', %d) Compression method %d not supported", filename, index, pakEntry.compressionType);
	}

	fd.close();
	
	return data;
}

// CC4ResourceLoader

CC4ResourceLoader::CC4ResourceLoader() {
}

CC4ResourceLoader::~CC4ResourceLoader() {
}

byte *CC4ResourceLoader::load(const char *filename, int index, uint32 &dataSize) {

	Common::File fd;
	uint32 offset, nextOffset, firstOffset;
	byte *data = NULL;
	
	if (!fd.open(filename))
		error("CC4ResourceLoader::load() Could not open %s", filename);

	firstOffset = fd.readUint32LE();
	fd.seek(index * 4);
	offset = fd.readUint32LE();
	
	if ((uint32)fd.pos() < firstOffset)
		nextOffset = fd.readUint32LE();
	else		
		nextOffset = fd.size();

	dataSize = nextOffset - offset;
	data = (byte*)malloc(dataSize);

	fd.seek(offset);
	fd.read(data, dataSize);
	
	return data;
}

// NarResourceLoader

NarResourceLoader::NarResourceLoader() {
}

NarResourceLoader::~NarResourceLoader() {
}

byte *NarResourceLoader::load(const char *filename, int index, uint32 &dataSize) {

	Common::File fd;
	uint32 offset, nextOffset = 0, firstOffset;
	byte *data = NULL;
	
	if (!fd.open(filename))
		error("NarResourceLoader::load() Could not open %s", filename);

	do {
		firstOffset = fd.readUint32LE();
	} while (firstOffset == 0 && !fd.eos());

	fd.seek(index * 4);
	offset = fd.readUint32LE();
	
	if (offset > 0) {
		
		if ((uint32)fd.pos() < firstOffset) {
			do {
				nextOffset = fd.readUint32LE();
			} while (nextOffset == 0 && (uint32)fd.pos() < firstOffset);
		}
		
		if (nextOffset == 0)
			nextOffset = fd.size();
	
		dataSize = nextOffset - offset - 1;
		data = (byte*)malloc(dataSize);
	
		fd.seek(offset);
		fd.read(data, dataSize);

	}
		
	return data;

}

// ResourceManager

ResourceManager::ResourceManager() {
}

ResourceManager::~ResourceManager() {
}

// TextReader

TextReader::TextReader(CometEngine *vm)
	: _vm(vm), _cachedTextResource(NULL), _cachedTextResourceTableIndex(-1) {
}

TextReader::~TextReader() {
	delete _cachedTextResource;
}

void TextReader::setTextFilename(const char *filename) {
	debugC(kDebugText, "TextReader::setTextFilename(filename: \"%s\")", filename);
	_textFilename = filename;
	_cachedTextResourceTableIndex = -1;
	delete _cachedTextResource;
}

TextResource *TextReader::loadTextResource(uint tableIndex) {
	debugC(kDebugText, "TextReader::loadTextResource(tableIndex: %d)", tableIndex);
	TextResource *textResource = new TextResource();
	_vm->_res->loadFromCC4(textResource, _textFilename.c_str(), tableIndex);
	return textResource;
}

byte *TextReader::getString(uint tableIndex, uint stringIndex) {
	debugC(kDebugText, "TextReader::getString(tableIndex: %d, stringIndex: %d)", tableIndex, stringIndex);
	return getCachedTextResource(tableIndex)->getString(stringIndex);
}

void TextReader::loadString(uint tableIndex, uint stringIndex, byte *buffer) {
	debugC(kDebugText, "TextReader::loadString(tableIndex: %d, stringIndex: %d, buffer)", tableIndex, stringIndex);
	getCachedTextResource(tableIndex)->loadString(stringIndex, buffer);
}

TextResource *TextReader::getCachedTextResource(uint tableIndex) {
	debugC(kDebugText, "TextReader::getCachedTextResource(tableIndex: %d)", tableIndex);
	if ((uint)_cachedTextResourceTableIndex != tableIndex || !_cachedTextResource) {
		debugC(kDebugText, "\tloading table %d", tableIndex);
		delete _cachedTextResource;
		_cachedTextResource = loadTextResource(tableIndex);
		_cachedTextResourceTableIndex = tableIndex;
	}
	return _cachedTextResource;
}

} // End of namespace Prisoner
