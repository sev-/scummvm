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
 */

#ifndef COMET_UNPACK_H
#define COMET_UNPACK_H

#include "common/util.h"
#include "common/stream.h"
#include "common/endian.h"

struct HufNode {
	unsigned short b0;
	unsigned short b1;
	HufNode *jump;
};

class DecompressImplode {
public:
	int decompress(Common::ReadStream *source, int flags, int size, byte *dest);
protected:
	Common::ReadStream *_source;
	byte *_dest;
	byte _bitBuf;
	int _bitBufLeft;
	byte _buffer[32768];
	int _bufferPos;
	int _bytesWritten;
	HufNode _literalTree[256];
	HufNode _distanceTree[64];
	HufNode _lengthTree[64];
	byte readByte();
	int readBit();
	int readBits(int count);
	void putByte(byte value);
	void flush();
	void recreateTree(HufNode *&currentTree, byte &len, int16 *fpos, int *flens, int16 fmax);
	int decodeSFValue(HufNode *currentTree);
	int createTree(HufNode *currentTree);
};

#endif
