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

// Console module

#include "dune/console.h"
#include "dune/dune.h"
#include "dune/resource.h"

namespace Dune {

DuneConsole::DuneConsole(DuneEngine *engine) : GUI::Debugger(),
	_engine(engine) {

	DCmd_Register("dump",				WRAP_METHOD(DuneConsole, cmdDump));
}

DuneConsole::~DuneConsole() {
}

bool DuneConsole::cmdDump(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Decompresses the given HSQ file (without an extension) into a raw uncompressed file\n");
		DebugPrintf("  Usage: %s <file name>\n\n", argv[0]);
		DebugPrintf("  Example: %s phrase11\n", argv[0]);
		DebugPrintf("  The above will uncompress phrase11.hsq into phrase11.raw\n");
		return true;
	}

	Common::String fileName(argv[1]);
	if (fileName.contains('.')) {
		DebugPrintf("Please supply the file name without the extension\n");
		return true;
	}

	Resource *hsqResource = new Resource(fileName + ".hsq");
	hsqResource->dump(fileName + ".raw");
	delete hsqResource;

	DebugPrintf("%s has been dumped to %s\n", (fileName + ".hsq").c_str(), (fileName + ".raw").c_str());
	return true;
}

} // End of namespace Dune
