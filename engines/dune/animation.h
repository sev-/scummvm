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
 * $Id: sentences.h
 *
 */

#ifndef ANIMATION_H
#define ANIMATION_H

namespace Common {

class MemoryReadStream;

}

namespace Dune {

struct FrameInfo {
	uint16 offset;
	bool isCompressed;
	uint16 width;
	uint16 height;
	byte palOffset;
};

class Animation {
public:
	Animation(Common::MemoryReadStream *res, OSystem *system);
	~Animation();

	void setPalette();
	uint16 getFrameCount();
	FrameInfo getFrameInfo(uint16 frameIndex);
	void drawFrame(uint16 frameIndex);

private:
	Common::MemoryReadStream *_res;
	byte _pal[256 * 3];

	OSystem *_system;
};

} // End of namespace Dune
 
#endif
