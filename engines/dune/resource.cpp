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
 * $URL: 
 * $Id: resource.cpp
 *
 */

#include "common/file.h"

#include "dune/resource.h"

namespace Dune {

#define PACKED_CHECKSUM 171

class BitReader {
public:
	BitReader(byte *data) : _data(data), _curBit(0), _queue(0) { }

	byte getBit() {
		if (!_curBit)
			refillQueue();

		byte result = _queue & 0x1;
		_queue >>= 1;
		_curBit--;

		return result;
	}

	byte getByte() {
		return *_data++;
	}

private:
	void refillQueue() {
		_curBit = 16;
		_queue = READ_LE_UINT16(_data);
		_data += 2;
	}

	byte *_data;
	byte _curBit;
	uint16 _queue;
};

Resource::Resource(Common::String filename) {
	Common::File f;
	
	if (f.open(filename)) {
		uint16 origSize = f.size();
		byte *origData = new byte[_size];

		f.read(origData, origSize);
		f.close();

		// Check if the file is packed
		if (origSize < 6) {
			// Can't be packed, stop here
			_size = origSize;
			_data = origData;
		} else {
			// Get a checksum of the first 6 bytes
			byte sum = 0;	// sum must be a byte, so that the salt value can overflow it to 0xAB
			for (int i = 0; i < 6; i++)
				sum += origData[i];

			if (sum == PACKED_CHECKSUM) {
				// File is packed, unpack it
				// Details taken from http://wiki.multimedia.cx/index.php?title=HNM_(1)

				// Header
				// word unpacked_len - size of the unpacked data
				// byte zero         - should be zero
				// word packed_len   - size of the packed data, including this header
				// byte salt         - adjust checksum to 171 (0xAB)
				_size = READ_LE_UINT16(origData);
				assert (*(origData + 2) == 0);
				uint16 packedSize = READ_LE_UINT16(origData + 3);
				if (packedSize != origSize)
					error("File %s is corrupt - size is %d, it should be %d", filename.c_str(), origSize, packedSize);

				_data = new byte[_size];
				memset(_data, 0, _size);
				byte *dst = _data;

				byte count;
				int16 offset;
				BitReader br(origData + 6);

				while (true) {
					if (br.getBit()) {
						*dst++ = br.getByte();
					} else {
						if (br.getBit()) {
							byte b1 = br.getByte();
							byte b2 = br.getByte();

							count = b1 & 0x7;
							offset = ((b1 >> 3) | (b2 << 5)) - 0x2000;

							if (!count)
								count = br.getByte();
							
							if (!count)
								break;	// finish the unpacking
						} else {
							count = br.getBit() * 2;
							count += br.getBit();
							offset = br.getByte() - 256;
						}

						count += 2;

						byte *src = dst + offset;
						while (count--)
							*dst++ = *src++;
					}	// if (!b)
				}	// while (true)

				// Delete the original packed data
				delete[] origData;
			} else {
				// File isn't packed
				_size = origSize;
				_data = origData;
			}	// if (sum == PACKED_CHECKSUM)
		}	// if (origSize >= 6)
	} else {
		error("File %s not found", filename.c_str());
	}
}

Resource::~Resource() {
	delete[] _data;
}

void Resource::dump(Common::String outFilename) {
	Common::DumpFile f;
	if (f.open(outFilename)) {
		f.write(_data, _size);
		f.flush();
		f.close();
	} else {
		error("Error opening %s for output", outFilename.c_str());
	}
}

} // End of namespace Dune
