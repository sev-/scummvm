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

#ifndef COMET_RESOURCEMANAGER_H
#define COMET_RESOURCEMANAGER_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/util.h"
#include "common/debug.h"

#include "comet/comet.h"

namespace Comet {

class BaseResource {
public:
	virtual ~BaseResource() {}
	void load(Common::MemoryReadStream &stream);
protected:
	virtual void free() {}
	virtual void internalLoad(Common::MemoryReadStream &stream) = 0;
};

class ResourceLoader {
public:
	virtual ~ResourceLoader() {}
	virtual byte *load(const char *filename, int index, uint32 &dataSize) = 0;
};

struct PakEntry {
	uint32 discSize;
	uint32 uncompressedSize;
	byte compressionType;
	byte flags;
	uint16 nameLen;
};

class PakResourceLoader : public ResourceLoader {
public:
	PakResourceLoader();
	~PakResourceLoader();
	byte *load(const char *filename, int index, uint32 &dataSize);
};

class CC4ResourceLoader : public ResourceLoader {
public:
	CC4ResourceLoader();
	~CC4ResourceLoader();
	byte *load(const char *filename, int index, uint32 &dataSize);
};

class NarResourceLoader : public ResourceLoader {
public:
	NarResourceLoader();
	~NarResourceLoader();
	byte *load(const char *filename, int index, uint32 &dataSize);
};

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();
	void loadFromPak(BaseResource *resource, const char *filename, int index) {
		loadFrom<PakResourceLoader>(resource, filename, index);
	}
	byte *loadRawFromPak(const char *filename, int index, uint32 *outDataSize = NULL) {
		return loadRawFrom<PakResourceLoader>(filename, index, outDataSize);
	}
	void loadFromCC4(BaseResource *resource, const char *filename, int index) {
		loadFrom<CC4ResourceLoader>(resource, filename, index);
	}
	void loadFromNar(BaseResource *resource, const char *filename, int index) {
		loadFrom<NarResourceLoader>(resource, filename, index);
	}
protected:
	template<typename T>
	void loadFrom(BaseResource *resource, const char *filename, int index) {
		uint32 dataSize;
		byte *data = loadRawFrom<T>(filename, index, &dataSize);
		Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::YES);
		resource->load(stream);
	}
	template<typename T>
	byte *loadRawFrom(const char *filename, int index, uint32 *outDataSize = NULL) {
		T loader;
		uint32 dataSize;
		byte *data = loader.load(filename, index, dataSize);
		if (outDataSize)
			*outDataSize = dataSize;
		return data;
	}
};

class TextReader {
public:
	TextReader(CometEngine *vm);
	~TextReader();

	void setTextFilename(const char *filename);
	TextResource *loadTextResource(uint tableIndex);
	byte *getString(uint tableIndex, uint stringIndex);
	void loadString(uint tableIndex, uint stringIndex, byte *buffer);

protected:
	CometEngine *_vm;
	Common::String _textFilename;
	TextResource *_cachedTextResource;
	int _cachedTextResourceTableIndex;

	TextResource *getCachedTextResource(uint tableIndex);
};

} // End of namespace Comet

#endif /* COMET_RESOURCEMANAGER_H */
