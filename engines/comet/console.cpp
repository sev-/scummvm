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

#include "comet/resourcemgr.h"

#include "comet/console.h"
#include "comet/comet.h"

#include "gui/debugger.h"

namespace Comet {

bool debugShowActorNum;

CometConsole::CometConsole(CometEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("showActorNum", WRAP_METHOD(CometConsole, Cmd_ShowActorNum));
	DCmd_Register("dumpResource", WRAP_METHOD(CometConsole, Cmd_DumpResource));

	debugShowActorNum = false;
}

CometConsole::~CometConsole() {
}

bool CometConsole::Cmd_ShowActorNum(int argc, const char **argv) {
	DebugPrintf("Enabling Display of Actor Number...\n");
	debugShowActorNum = true;
	return true;
}

bool CometConsole::Cmd_DumpResource(int argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Usage: dumpResource <Pak Name> <Resource Number>\n");
		return true;
	}

	Common::String pakName = Common::String(argv[1]);
	pakName += ".pak";

	uint32 resourceNum = atoi(argv[2]);

	uint32 size = 0;
	byte *buf = _vm->_res->loadRawFromPak(pakName.c_str(), resourceNum, &size);

	Common::String outFileName;
	outFileName = outFileName.format("%s-%03d.dump", argv[1], resourceNum);
	Common::DumpFile *outFile = new Common::DumpFile();
	outFile->open(outFileName);
	outFile->write(buf, size);
	outFile->flush();
	outFile->close();
	delete outFile;

	free(buf);
	return true;
}

} // End of namespace Comet
