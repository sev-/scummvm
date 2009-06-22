/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * $URL: https://www.switchlink.se/svn/comet/scene.h $
 * $Id: comet.h 46 2009-06-07 14:17:12Z johndoe $
 *
 */

#ifndef COMET_TEXT_H
#define COMET_TEXT_H

#include "common/file.h"

namespace Comet {

class TextStrings {
public:
    TextStrings(Common::File *fd, uint32 size);
    ~TextStrings();
	byte *getString(uint stringIndex);
	void loadString(uint stringIndex, byte *buffer);
protected:
	byte *_data;
	uint _stringCount;
	uint32 *_stringOffsets;
};

class TextReader {
public:
	TextReader();
	~TextReader();
	void open(const char *filename);
	void close();
	TextStrings *loadTextStrings(uint tableIndex);
	byte *getString(uint tableIndex, uint stringIndex);
	void loadString(uint tableIndex, uint stringIndex, byte *buffer);
protected:
	Common::File *_fd;
	uint _tableCount;
	uint32 *_tableOffsets;
	TextStrings *_cachedTextStrings;
	uint _cachedTextStringsTableIndex;
	TextStrings *getCachedTextStrings(uint tableIndex);
};

} // End of namespace Comet

#endif
