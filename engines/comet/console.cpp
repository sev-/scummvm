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

#include "comet/console.h"
#include "comet/animationmgr.h"
#include "comet/comet.h"
#include "comet/comet_gui.h"
#include "comet/input.h"
#include "comet/resourcemgr.h"
#include "comet/screen.h"

#include "gui/debugger.h"

namespace Comet {

bool debugRectangles;
bool debugShowActorNum;
bool debugTestPuzzle;
bool debugPuzzleCheat;

CometConsole::CometConsole(CometEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("toggleRects", WRAP_METHOD(CometConsole, Cmd_ToggleDebugRectangles));
	DCmd_Register("showActorNum", WRAP_METHOD(CometConsole, Cmd_ShowActorNum));
	DCmd_Register("dumpResource", WRAP_METHOD(CometConsole, Cmd_DumpResource));
	DCmd_Register("module", WRAP_METHOD(CometConsole, Cmd_Module));
	DCmd_Register("scene", WRAP_METHOD(CometConsole, Cmd_Scene));
	DCmd_Register("testBeamRoom", WRAP_METHOD(CometConsole, Cmd_TestBeamRoom));
	DCmd_Register("testPuzzle", WRAP_METHOD(CometConsole, Cmd_TestPuzzle));
	DCmd_Register("puzzleCheat", WRAP_METHOD(CometConsole, Cmd_PuzzleCheat));
	DCmd_Register("viewCursor", WRAP_METHOD(CometConsole, Cmd_ViewCursor));

	debugRectangles = false;
	debugShowActorNum = false;
	debugTestPuzzle = false;
	debugPuzzleCheat = false;
}

CometConsole::~CometConsole() {
}

bool CometConsole::Cmd_ToggleDebugRectangles(int argc, const char **argv) {
	debugRectangles = !debugRectangles;
	DebugPrintf("Debug Rectangles: %s\n", debugRectangles ? "Enabled" : "Disabled");
	return true;
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

bool CometConsole::Cmd_Module(int argc, const char **argv) {
	int16 moduleNum;
	switch (argc) {
	case 1:
		DebugPrintf("Current Module: %d\n", _vm->_moduleNumber);
		break;
	case 2:
		moduleNum = atoi(argv[1]);
		DebugPrintf("Changing to Module: %d\n", moduleNum);
		_vm->_moduleNumber = moduleNum;
		break;
	default:
		DebugPrintf("Usage: module <Module Number>\n");
		break;
	}
	return true;
}

bool CometConsole::Cmd_Scene(int argc, const char **argv) {
	int16 sceneNum;
	switch (argc) {
	case 1:
		DebugPrintf("Current Scene: %d\n", _vm->_sceneNumber);
		break;
	case 2:
		sceneNum = atoi(argv[1]);
		DebugPrintf("Changing to Scene: %d\n", sceneNum);
		_vm->_sceneNumber = sceneNum;
		break;
	default:
		DebugPrintf("Usage: scene <Scene Number>\n");
		break;
	}
	return true;
}

bool CometConsole::Cmd_TestBeamRoom(int argc, const char **argv) {
	DebugPrintf("Jump To Beam Room For Test...\n");
	_vm->_scriptVars[116] = 1;
	_vm->_scriptVars[139] = 1;
	_vm->_moduleNumber = 7;
	_vm->_sceneNumber = 4;
	return true;
}

bool CometConsole::Cmd_TestPuzzle(int argc, const char **argv) {
	DebugPrintf("Testing Block Puzzle...\n");
	debugTestPuzzle = true;
	return true;
}

bool CometConsole::Cmd_PuzzleCheat(int argc, const char **argv) {
	DebugPrintf("Enabling Block Puzzle Cheating...\n");
	debugPuzzleCheat = true;
	return true;
}

bool CometConsole::Cmd_ViewCursor(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: viewCursor <Resource Number>\n");
		return true;
	}

	uint32 resourceNum = atoi(argv[1]);

	_vm->_screen->setFullPalette(_vm->_gamePalette);
	AnimationResource *anim;
	AnimationCel *currCel;
	bool done = false;
	uint celIndex = 0;
	anim = _vm->_animationMan->loadAnimationResource("RES.PAK", resourceNum);
	while (!done && !_vm->shouldQuit()) {
		int16 x, y;
		_vm->_input->handleEvents();
		switch (_vm->_input->getKeyCode()) {
		case Common::KEYCODE_UP:
			if (celIndex >= anim->getCelCount())
				celIndex = 0;
			else
				celIndex++;
			break;
		case Common::KEYCODE_DOWN:
			if (celIndex == 0)
				celIndex = anim->getCelCount() - 1;
			else
				celIndex--;
			break;
		case Common::KEYCODE_ESCAPE:
			done = true;
			break;
		default:
			break;
		}
		debug(0, "celIndex = %d", celIndex);
		currCel = anim->getCel(celIndex);
		x = _vm->_input->getMouseX() + 20;
		y = _vm->_input->getMouseY() + 20;
		_vm->_screen->clear();
		_vm->_screen->drawAnimationCelSprite(*currCel, x, y, 0);
		_vm->_screen->update();
	}
	delete anim;

	return true;
}

} // End of namespace Comet
