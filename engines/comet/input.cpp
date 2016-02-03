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
 */

#include "comet/input.h"
#include "comet/comet.h"
#include "comet/actor.h"
#include "comet/animationmgr.h"
#include "comet/comet_gui.h"
#include "comet/dialog.h"
#include "comet/music.h"
#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/talktext.h"

#include "common/stream.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"
#include "graphics/surface.h"

namespace Comet {

Input::Input(CometEngine *vm)
	: _vm(vm), _mouseX(0), _mouseY(0), _keyCode(Common::KEYCODE_INVALID),
	_keyDirection(0), _arrowDirection(0), _scriptKeybFlag(0),
	_mouseWalking(false), _mouseCursorDirection(0), _leftButton(false), _rightButton(false) {
}

Input::~Input() {
}

bool Input::leftButton() const {
	return _leftButton && !_vm->isFloppy();
}

bool Input::rightButton() const {
	return _rightButton && !_vm->isFloppy();
}

void Input::sync(Common::Serializer &s) {
	s.syncAsByte(_blockedInput);
}

void Input::blockInput(int flagIndex) {
	if (flagIndex == 0) {
		_blockedInput = 15;
		_vm->_actors->getActor(0)->stopWalking();
	} else {
		static const int kFlagsMap[5] = {0, 1, 8, 2, 4};
		_blockedInput |= kFlagsMap[flagIndex];
	}
}

void Input::unblockInput() {
	_blockedInput = 0;
	if (_vm->_actors->getActor(0)->_status == 2)
		_vm->_actors->getActor(0)->_status = 0;
}

int Input::mouseCalcCursorDirection(int fromX, int fromY, int toX, int toY) {
	int deltaX, deltaY;
	int deltaXAbs, deltaYAbs;
	int direction;
	deltaX = toX - fromX;
	deltaY = toY - fromY;
	deltaXAbs = ABS(toX - fromX);
	deltaYAbs = ABS(toY - fromY);
	if (deltaX == 0 && deltaY == 0)
		direction = 0;
	else if (deltaY > 0 && deltaY > deltaXAbs && deltaYAbs > 2)
		direction = 3;
	else if (deltaY < 0 && deltaYAbs > deltaXAbs && deltaYAbs > 2)
		direction = 1;
	else if (deltaX > 0 && deltaX > deltaYAbs && deltaXAbs > 2)
		direction = 2;
	else if (deltaX < 0 && deltaXAbs > deltaYAbs && deltaXAbs > 2)
		direction = 4;
	else
		direction = 0;
	return direction;
}

int Input::arrowDirectionToWalkDirection(int arrowDirection) {
	static const byte kWalkDirections[] = {
		0, 1, 3, 0, 4, 4, 4, 0, 2, 2, 2, 0, 0, 0, 0, 0
	};
	return kWalkDirections[arrowDirection & 0x0F];
}

void Input::handleEvents() {
	Common::Event event;
	while (_vm->_system->getEventManager()->pollEvent(event)) {
		handleEvent(event);
	}
}

void Input::handleEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
			_keyDirection = 1;
			break;
		case Common::KEYCODE_DOWN:
			_keyDirection = 2;
			break;
		case Common::KEYCODE_LEFT:
			_keyDirection = 4;
			break;
		case Common::KEYCODE_RIGHT:
			_keyDirection = 8;
			break;
		case Common::KEYCODE_d:
			if (event.kbd.flags & Common::KBD_CTRL) {
				_vm->openConsole();
				event.kbd.keycode = Common::KEYCODE_INVALID;
			}
			break;
		default:
			break;
		}
		_keyCode = event.kbd.keycode;
		_keyAscii = event.kbd.ascii;
		break;
	case Common::EVENT_KEYUP:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_UP:
			_keyDirection &= ~1;
			break;
		case Common::KEYCODE_DOWN:
			_keyDirection &= ~2;
			break;
		case Common::KEYCODE_LEFT:
			_keyDirection &= ~4;
			break;
		case Common::KEYCODE_RIGHT:
			_keyDirection &= ~8;
			break;
		default:
			break;
		}
		_keyCode = Common::KEYCODE_INVALID;
		break;
	default:
		handleMouseEvent(event);
		break;
	}
}

void Input::handleMouseEvent(Common::Event &event) {
	if (!_vm->isFloppy()) {
		// Handle mouse-related events only in the CD version
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_leftButton = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_leftButton = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_rightButton = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_rightButton = false;
			break;
		default:
			break;
		}
	}
}

void Input::waitForKeys() {
	while ((_keyCode != Common::KEYCODE_INVALID || _keyDirection != 0 || _leftButton || _rightButton) && !_vm->shouldQuit()) {
		handleEvents();
		_vm->_system->delayMillis(20);
	}
}

void Input::waitForKeyPress() {
	waitForKeys();
	while (_keyCode == Common::KEYCODE_INVALID && _keyDirection == 0 && !_leftButton && !_rightButton && !_vm->shouldQuit()) {
		handleEvents();
		_vm->_system->delayMillis(20);
	}
}

bool Input::isButtonPressed() {
	return
		(_keyCode == Common::KEYCODE_RETURN) ||
		(!isCurrentArrowKeyDirectionBlocked() && !_vm->_dialog->isRunning()) ||
		(!_vm->isFloppy() && (_leftButton || _rightButton));
}

int Input::getWalkDirection() {
	if (isCurrentArrowKeyDirectionBlocked() || _vm->_dialog->isRunning())
		return 0;
	return arrowDirectionToWalkDirection(_arrowDirection);
}

void Input::updateArrowDirection() {
	static const byte kCursorDirectionToKeyDirections[] = {
		0, 1, 8, 2, 4
	};
	_arrowDirection = _keyDirection;
	if (!_vm->isFloppy() && !_vm->_dialog->isRunning() && !_vm->_talkText->isActive() && _blockedInput != 0x0F) {
		if (!_mouseWalking && _arrowDirection == 0) {
			Actor *mainActor = _vm->_actors->getActor(0);
			_mouseCursorDirection = mouseCalcCursorDirection(mainActor->_x, mainActor->_y, _mouseX, _mouseY);
		} else if (_arrowDirection != 0) {
			_mouseCursorDirection = arrowDirectionToWalkDirection(_arrowDirection);
		}
		_mouseWalking = _leftButton;
		if (_mouseWalking && _mouseCursorDirection != 0) {
			_arrowDirection = kCursorDirectionToKeyDirections[_mouseCursorDirection];
		}
	}
}

} // End of namespace Comet
