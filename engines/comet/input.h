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

#ifndef COMET_INPUT_H
#define COMET_INPUT_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/array.h"
#include "common/list.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/config-manager.h"
#include "audio/mixer.h"

#include "engines/engine.h"

#include "comet/console.h"

namespace Comet {

class CometEngine;

class Input {
public:
	Input(CometEngine *vm);
	~Input();
	void handleEvents();
	void handleEvent(Common::Event &event);
	void handleMouseEvent(Common::Event &event);
	void waitForKeys();
	void waitForKeyPress();
	void handleInput();
	void blockInput(int flagIndex);
	void unblockInput();
	bool leftButton() const;
	bool rightButton() const;
	Common::KeyCode getKeyCode() const { return _keyCode; }
	bool isCursorDirection(int flag) const { return (_cursorDirection & flag) != 0;}
	int getBlockedInput() const { return _blockedInput; }
	void setBlockedInput(int value) { _blockedInput = value; }
	int getMouseX() const { return _mouseX; }
	int getMouseY() const { return _mouseY; }
	void clearKeyDirection() { _keyDirection = 0; }
	void clearKeyCode() { _keyCode = Common::KEYCODE_INVALID; }
public:
	int16 _cursorDirection;
	int16 _scriptKeybFlag;
protected:
	CometEngine *_vm;
	Common::KeyCode _keyCode;
	char _keyAscii;
	int _keyDirection;
	int _mouseX, _mouseY;
	bool _leftButton, _rightButton;
	byte _blockedInput;
	int16 _mouseClick;
	int _walkDirection;
	bool _mouseWalking;
	int _mouseCursorDirection;
	int mouseCalcCursorDirection(int fromX, int fromY, int toX, int toY);
};

} // End of namespace Comet

#endif
