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

// Not nice, decompression code should probably go to common/zlib.cpp one day...
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/memstream.h"

#if defined(USE_ZLIB)
  #ifdef __SYMBIAN32__
	#include <zlib\zlib.h>
  #else
	#include <zlib.h>
  #endif
#else
  #error Sorry, zlib is required for the Prisoner engine
#endif

#include "prisoner/kroarchive.h"

namespace Prisoner {

int uncompress (Bytef *dest, uint32 *destLen, Bytef *source, uint32 sourceLen) {
	z_stream stream;
	int err;
	stream.next_in = (Bytef*)source;
	stream.avail_in = (uInt)sourceLen;
	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	err = inflateInit2_(&stream, -MAX_WBITS, ZLIB_VERSION, sizeof(z_stream));
	if (err != Z_OK) return err;
	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
			return Z_DATA_ERROR;
		return err;
	}
	*destLen = stream.total_out;
	err = inflateEnd(&stream);
	return err;
}

void decryptBuffer(byte *buf, uint32 size) {
	byte value = 0x5A;
	while (size > 0) {
		*buf ^= value;
		value = *buf;
		buf++;
		size--;
	}
}

/* KroArchive */

KroArchive::KroArchive() : _fd(NULL) {
}

KroArchive::~KroArchive() {
	close();
}

void KroArchive::open(const char *filename) {

	close();
	_entries.clear();

	_fd = new Common::File();
	if (!_fd->open(filename))
		error("KroArchive::open() Could not open %s", filename);

	_fd->readUint32LE(); // skip Burp
	uint32 count = _fd->readUint32LE();
	_entries.reserve(count);

	for (uint32 i = 0; i < count; i++) {
		KroArchiveEntry entry;
		entry.size = _fd->readUint32LE();
		entry.compressedSize = _fd->readUint32LE();
		_fd->readUint32LE(); // skip unknown/unused value
		entry.offset = _fd->readUint32LE();
		entry.compressionType = _fd->readUint32LE();
		_entries.push_back(entry);
	}

}

void KroArchive::close() {
	delete _fd;
}

byte *KroArchive::load(uint index) {
	byte *data = NULL;
	KroArchiveEntry *entry = &_entries[index];

	debug(1, "KroArchive::load(%08X)", index);

	_fd->seek(entry->offset);

	data = new byte[entry->size];

	switch (entry->compressionType) {
	case 0: // uncompressed
	{
		_fd->read(data, entry->size);
		break;
	}
	case 4: // zlib compressed
	{
		uint32 dstLen = entry->size;
		byte *compressedData = new byte[entry->compressedSize];
		_fd->read(compressedData, entry->compressedSize);
		if (uncompress(data, &dstLen, compressedData, entry->compressedSize) != Z_OK)
			error("KroArchive::load(%d) Error decompressing type %d", index, entry->compressionType);
		delete[] compressedData;
		break;
	}
	default:
		error("KroArchive::load(%d) Unknown compression type %d", index, entry->compressionType);
	}

	return data;
}

uint32 KroArchive::getSize(uint index) {
	return _entries[index].size;
}

void KroArchive::loadDirectory(const char *filename, uint32 offset, bool isEncrypted) {
	Common::ReadStream *stream = NULL;

	debug(0, "%s", filename);

	Common::File *fd = new Common::File();
	if (!fd->open(filename))
		error("PakDirectory::load() Could not open %s", filename);

	if (isEncrypted) {
		// Load file and decrypt
		debug("Directory from encrypted data");
		uint32 dirDataSize;
		byte *dirData;
		fd->seek(offset);
		dirDataSize = fd->readUint32LE();
		debug("dirDataSize = %d", dirDataSize);
		dirData = (byte*)malloc(dirDataSize);
		fd->read(dirData, dirDataSize);
		decryptBuffer(dirData, dirDataSize);
		delete fd;
		stream = new Common::MemoryReadStream(dirData, dirDataSize, DisposeAfterUse::YES);
	} else {
		debug("Directory from non-encrypted data");
		stream = fd;
	}

	uint32 count = stream->readUint32LE();
	_pakDirectory.reserve(count);
	for (uint32 i = 0; i < count; i++) {
		PakDirectoryEntry entry;
		char pakName[9];
		stream->read(pakName, 8);
		pakName[8] = 0;
		entry.pakName = pakName;
		entry.baseIndex = stream->readUint32LE();
		_pakDirectory.push_back(entry);
		debug("{\"%s\", %d}, ", entry.pakName.c_str(), entry.baseIndex);
		//debug(0, "pakName = %s; baseIndex = %d", entry.pakName.c_str(), entry.baseIndex);
	}

	delete stream;

}

void KroArchive::loadDirectory(const _PakDirectoryEntry directory[]) {
	const _PakDirectoryEntry *xentry = directory;
	while (xentry->pakName) {
		PakDirectoryEntry entry;
		entry.pakName = xentry->pakName;
		entry.baseIndex = xentry->baseIndex;
		_pakDirectory.push_back(entry);
		xentry++;
	}
}

uint32 KroArchive::getPakBaseIndex(Common::String &pakName) {
	for (uint i = 0; i < _pakDirectory.size(); i++) {
		if (_pakDirectory[i].pakName == pakName)
			return _pakDirectory[i].baseIndex;
	}
	error("KroArchive::getPakBaseIndex() PakName %s not found", pakName.c_str());
}

} // End of namespace Prisoner
