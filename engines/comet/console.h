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

#ifndef COMET_CONSOLE_H
#define COMET_CONSOLE_H

#include "gui/debugger.h"

namespace Comet {

class CometEngine;

extern bool debugRectangles;
extern bool debugShowActorNum;
extern bool debugTestPuzzle;
extern bool debugPuzzleCheat;

class CometConsole : public GUI::Debugger {
public:
	CometConsole(CometEngine *vm);
	virtual ~CometConsole(void);

private:
	CometEngine *_vm;

	bool Cmd_ToggleDebugRectangles(int argc, const char **argv);
	bool Cmd_ShowActorNum(int argc, const char **argv);
	bool Cmd_DumpResource(int argc, const char **argv);
	bool Cmd_TestBeamRoom(int argc, const char **argv);
	bool Cmd_TestPuzzle(int argc, const char **argv);
	bool Cmd_PuzzleCheat(int argc, const char **argv);
};

} // End of namespace Comet

#endif
