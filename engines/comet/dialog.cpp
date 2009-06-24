#include "comet/comet.h"
#include "comet/font.h"
#include "comet/pak.h"

#include "comet/screen.h"
#include "comet/dialog.h"

namespace Comet {

Dialog::Dialog(CometEngine *vm) : _vm(vm) {

	_dialogTextSubIndex = 0;
	_dialogTextX = 0;
	_dialogTextY = 0;
	_dialogTextColor = 79;
	_dialogTextColorInc = -1;
	_dialogRunning = false;

}

Dialog::~Dialog() {
}

void Dialog::run(Script *script) {

	int dialogItemCount;

	_vm->resetTextValues();

	_dialogTextSubIndex = script->loadInt16();

	debug("_dialogTextSubIndex = %d", _dialogTextSubIndex);

	if (_dialogTextSubIndex != -1) {
		//textOfs += _vm->loadString(_vm->_narFileIndex + 3, _dialogTextSubIndex, _vm->_tempTextBuffer + textOfs);
	}

	_dialogTextX = script->loadByte() * 2;
	_dialogTextY = script->loadByte();

	dialogItemCount = script->loadByte();

	_dialogItems.clear();
	_dialogItems.reserve(dialogItemCount);

	for (int index = 0; index < dialogItemCount; index++) {
		DialogItem dialogItem;
		dialogItem.index = script->loadInt16();
  		dialogItem.text = _vm->_textReader->getString(_vm->_narFileIndex + 3, dialogItem.index);
  		dialogItem.scriptIp = script->ip;
  		script->ip += 2;
		_dialogItems.push_back(dialogItem);
	}

	if (_dialogItems[0].index == _dialogTextSubIndex)
		_dialogTextSubIndex = -1;

	_dialogSelectedItemIndex2 = -1;
	_dialogSelectedItemIndex = 0;
	_dialogRunning = true;

	drawTextBubbles();

}

void Dialog::stop() {
	_dialogRunning = false;
}

void Dialog::update() {

	int oldDialogSelectedItemIndex;

	oldDialogSelectedItemIndex = _dialogSelectedItemIndex;

	if (_vm->_mouseButtons4 & 1) {
		_vm->waitForKeys();
		if (_dialogSelectedItemIndex > 0)
			_dialogSelectedItemIndex--;
	} else if (_vm->_mouseButtons4 & 2) {
		_vm->waitForKeys();
		if (_dialogSelectedItemIndex < (int)_dialogItems.size() - 1)
			_dialogSelectedItemIndex++;
	}

	if (oldDialogSelectedItemIndex == _dialogSelectedItemIndex) {
		// TODO: Handle selection by mouse
	} else {
		// TODO: Move mouse cursor to center of selected dialog item
	}

	if (oldDialogSelectedItemIndex != _dialogSelectedItemIndex) {
		// Reset the text color
		_dialogTextColor = 79;
	}

	drawTextBubbles();

	// The following was in dialogHandleInput() in the original code, we do it right here

	//handleEvents();

	//printf("Dialog(2)::_keyScancode = %d\n", _keyScancode);

	// TODO: Check for mouseclick later, too
	if (_vm->_keyScancode == Common::KEYCODE_RETURN && _dialogSelectedItemIndex != -1) {
		//DEBUG: if (_talkieMode == 1)
		{
			_vm->actorSayWithAnim(0, _dialogItems[_dialogSelectedItemIndex].index, 0);
			//TODO: loop with updateTalkAnims()
		}
		_dialogRunning = false;
	}

}

void Dialog::drawTextBubbles() {

	int x, y;
	byte color1, color2;

	//TODO: Before...

	_dialogRects.clear();

	x = _dialogTextX;
	y = _dialogTextY;

	if (_vm->_sceneObjects[0].textColor == 25)
		color1 = 22;
 	else
 		color1 = _vm->_sceneObjects[0].textColor;

	for (uint i = 0; i < _dialogItems.size(); i++) {

		color2 = color1;

		if (i == (uint)_dialogSelectedItemIndex) {
			if (_vm->_sceneObjects[0].textColor == 25) {
				color2 = _dialogTextColor;
				_dialogTextColor += _dialogTextColorInc;
				if (_dialogTextColor > 25) {
					_dialogTextColor = 25;
					_dialogTextColorInc = -1;
				} else if (_dialogTextColor < 22) {
					_dialogTextColor = 22;
					_dialogTextColorInc = 1;
				}
			} else if (_vm->_textColorFlag & 1) {
				color2 = _vm->_sceneObjects[0].textColor;
			} else {
				color2 = 159;
			}
		}

		_vm->setText(_dialogItems[i].text);

		_vm->drawBubble(x - 4, y - 4, x + _vm->_textMaxTextWidth * 2 + 4, y + _vm->_textMaxTextHeight);

		y = _vm->_screen->drawText3(x, y, _dialogItems[i].text, color2, 1);

		RectItem rect;
		rect.x = x - 4;
		rect.y = y - 4 - _vm->_textMaxTextHeight;
		rect.x2 = x + _vm->_textMaxTextWidth * 2 + 4;
		rect.y2 = y;
		rect.id = i;
		_dialogRects.push_back(rect);

		y += 8;

	}

	//TODO: After...

}

byte *Dialog::getChoiceScriptIp() {
	return _dialogItems[_dialogSelectedItemIndex].scriptIp;
}

} // End of namespace Comet
