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
 * $URL$
 * $Id$
 *
 */

#ifndef COMET_TEXT_H
#define COMET_TEXT_H

#include "common/file.h"

#include "comet/comet.h"
#include "comet/resource.h"

namespace Comet {

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

#endif
