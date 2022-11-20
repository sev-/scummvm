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

#ifndef ENCHANTIA_RESOURCE_H
#define ENCHANTIA_RESOURCE_H

#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/surface.h"

namespace Enchantia {

class SpriteResource {
public:
	SpriteResource();
	~SpriteResource();
	void load(const char *filename);
	void loadSingle(byte *src);
	uint getCount() const { return _frames.size(); }
	Graphics::Surface *getFrame(uint index) { return _frames[index]; }
protected:
	Common::Array<Graphics::Surface*> _frames;
	void free();
	Graphics::Surface *unpackFrame(byte *data);
};

} // End of namespace Enchantia

#endif
