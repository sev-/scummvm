/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "enchantia/enchantia.h"
#include "enchantia/resource.h"

namespace Enchantia {

SpriteResource::SpriteResource() {
}

SpriteResource::~SpriteResource() {
	free();
}

void SpriteResource::load(const char *filename) {
	free();

	Common::File fd;
	if (!fd.open(filename))
		error("SpriteResource::load() Could not open %s", filename);

	uint16 firstOffs = fd.readUint16LE();
	int32 *offsets = new int32[(firstOffs * 8) + 1];
	uint offsetsCount = 1;
	offsets[0] = firstOffs * 16;
	while (fd.pos() != offsets[0]) {
		offsets[offsetsCount] = fd.readUint16LE() * 16;
		if (!offsets[offsetsCount] || offsets[offsetsCount] >= fd.size())
			break;
		offsetsCount++;
	}
	offsets[offsetsCount] = fd.size();

	for (uint i = 0; i < offsetsCount; i++) {
		int32 size = offsets[i + 1] - offsets[i];
		byte *data = new byte[size];
		fd.seek(offsets[i]);
		fd.read(data, size);
		_frames.push_back(unpackFrame(data));
		delete[] data;
	}

	delete[] offsets;

	fd.close();

}

void SpriteResource::loadSingle(byte *src) {
	free();

	_frames.push_back(unpackFrame(src));
}

void SpriteResource::free() {
	for (Common::Array<Graphics::Surface*>::iterator it = _frames.begin(); it != _frames.end(); it++) {
		(*it)->free();
		delete *it;
	}
	_frames.clear();
}

Graphics::Surface *SpriteResource::unpackFrame(byte *data) {
	Graphics::Surface *frame = new Graphics::Surface();
	uint16 width = READ_LE_UINT16(data + 0);
	uint16 height = READ_LE_UINT16(data + 2);
	frame->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
    data += 10;
    byte *dest = (byte*)frame->getPixels();
    memset(dest, 0, width * height);
    for (uint p = 0; p < 4; p++) {
        uint y = 0, yc = height;
        byte *row = dest + p;
        while (yc) {
            int8 cmd = *data++;
            if (cmd == 0) {
                --yc;
                y++;
                row = dest + y * width + p;
            } else if (cmd < 0) {
                row += -cmd * 4;
            } else {
                while (cmd--) {
                    *row = *data++;
                    row += 4;
                }
            }
        }
    }
	return frame;
}

} // End of namespace Enchantia
