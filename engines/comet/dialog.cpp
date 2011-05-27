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

#include "comet/comet.h"
#include "comet/dialog.h"
#include "comet/screen.h"

namespace Comet {

Dialog::Dialog(CometEngine *vm) : _vm(vm) {
	_introTextIndex = 0;
	_textX = 0;
	_textY = 0;
	_textColor = 79;
	_textColorInc = -1;
	_frameColor = 82;
	_frameColorInc = 1;
	_isRunning = false;
}

Dialog::~Dialog() {
}

void Dialog::start(Script *script) {
	int dialogItemCount, y;

	_vm->resetTextValues();

	_introTextIndex = script->readInt16();

	debug("_introTextIndex = %d", _introTextIndex);

	if (_introTextIndex != -1) {
		//textOfs += _vm->loadString(_vm->_narFileIndex + 3, _introTextIndex, _vm->_tempTextBuffer + textOfs);
	}

	_textX = script->readByte() * 2;
	_textY = script->readByte();
	
	if (_vm->_talkieMode == 2) {
		_textX = 8;
		_textY = 16;
	}

	y = _textY;
	// Y adjustment from drawText3
	if (y < 3)
		y = 3;

	dialogItemCount = script->readByte();

	_items.clear();
	_items.reserve(dialogItemCount);

	for (int index = 0; index < dialogItemCount; index++) {
		DialogItem dialogItem;
		dialogItem.index = script->readInt16();
		dialogItem.text = _vm->_textReader->getString(_vm->_narFileIndex + 3, dialogItem.index);
		dialogItem.scriptIp = script->ip;
		script->ip += 2;
		if (_vm->_talkieMode == 0 || _vm->_talkieMode == 1) {
			// Dialog bubbles containing the whole text
			_vm->setText(dialogItem.text);
			y += _vm->_screen->getTextHeight(dialogItem.text);
			dialogItem.rect.x = _textX - 4;
			dialogItem.rect.y = y - 4 - _vm->_textMaxTextHeight;
			dialogItem.rect.x2 = _textX + _vm->_textMaxTextWidth * 2 + 4;
			dialogItem.rect.y2 = y;
			y += 8;
		} else if (_vm->_talkieMode == 2) {
			// Dialog bubbles containing letters only
			dialogItem.rect.x = _textX - 2;
			dialogItem.rect.y = _textY + index * 16 - 10;
			dialogItem.rect.x2 = _textX + 10;
			dialogItem.rect.y2 = _textY + index * 16;
		}
		dialogItem.rect.id = index;
		_items.push_back(dialogItem);
	}

	if (_items[0].index == _introTextIndex)
		_introTextIndex = -1;

	_selectedItemIndex2 = -1;
	_selectedItemIndex = 0;
	_isRunning = true;

	drawTextBubbles();
}

void Dialog::stop() {
	_isRunning = false;
}

void Dialog::update() {
	int oldDialogSelectedItemIndex = _selectedItemIndex;

	if ((_vm->_cursorDirection & 1) && _selectedItemIndex > 0) {
		_selectedItemIndex--;
		_vm->waitForKeys();
	} else if ((_vm->_cursorDirection & 2) && _selectedItemIndex < (int)_items.size() - 1) {
		_selectedItemIndex++;
		_vm->waitForKeys();
	}

	if (!_vm->isFloppy()) {
		if (oldDialogSelectedItemIndex == _selectedItemIndex) {
			// Handle selection by mouse
			int mouseSelectedItemIndex = -1;
			for (uint i = 0; i < _items.size(); i++) {
				if (_vm->_mouseX > _items[i].rect.x && _vm->_mouseX < _items[i].rect.x2 &&
					_vm->_mouseY > _items[i].rect.y && _vm->_mouseY < _items[i].rect.y2)
					mouseSelectedItemIndex = _items[i].rect.id;
			}
			if (mouseSelectedItemIndex != -1) {
				_selectedItemIndex = mouseSelectedItemIndex;
			}
		} else {
			// TODO: Move mouse cursor to center of selected dialog item
		}
	}

	if (oldDialogSelectedItemIndex != _selectedItemIndex) {
		// Reset the text color
		_textColor = 79;
	}

	drawTextBubbles();

	if (_selectedItemIndex != -1 && (_vm->leftButton() || _vm->_keyScancode == Common::KEYCODE_RETURN)) {
		if (_vm->_talkieMode == 1) {
			_vm->actorTalkWithAnim(0, _items[_selectedItemIndex].index, 0);
			while (_vm->_mixer->isSoundHandleActive(_vm->_sampleHandle)) {
				_vm->updateTalkAnims();
				_vm->handleEvents();
				_vm->_system->updateScreen();
			}
			_vm->waitForKeys();
		}
		_isRunning = false;
	}
	
}

void Dialog::drawTextBubbles() {
	if (_vm->isFloppy() || _vm->_talkieMode == 0 || _vm->_talkieMode == 1) {
		int x = _textX;
		int y = _textY;
		byte color1 = _vm->_actors[0].textColor == 25 ? 22 : _vm->_actors[0].textColor;
		byte color2;
		/* TODO: Draw intro text bubble in floppy version
		   NOTE: Never used in Comet Floppy since _items[0].index == _introTextIndex is always true
		   and so _introTextIndex is set to -1.
		*/
		for (uint i = 0; i < _items.size(); i++) {
			color2 = color1;
			if (i == (uint)_selectedItemIndex) {
				if (_vm->_actors[0].textColor == 25) {
					color2 = _textColor;
					_textColor += _textColorInc;
					if (_textColor > 25) {
						_textColor = 25;
						_textColorInc = -1;
					} else if (_textColor < 22) {
						_textColor = 22;
						_textColorInc = 1;
					}
				} else if (_vm->_textColorFlag & 1) {
					color2 = _vm->_actors[0].textColor;
				} else {
					color2 = 159;
				}
			}
			_vm->setText(_items[i].text);
			_vm->drawBubble(x - 4, y - 4, x + _vm->_textMaxTextWidth * 2 + 4, y + _vm->_textMaxTextHeight);
			y = _vm->_screen->drawText3(x, y, _items[i].text, color2, 1);
			y += 8;
		}
	} else if (_vm->_talkieMode == 2) {
		byte letter[2];
		_frameColor += _frameColorInc;
		if (_frameColor == 94)
			_frameColorInc = -1;
		else if (_frameColor == 82)
			_frameColorInc = 1;
		if (_introTextIndex >= 0) {
			// Play the intro speech
			_vm->playVoice(_introTextIndex);
		}
		_introTextIndex = -1;
		letter[0] = 'A';
		letter[1] = 0;
		for (uint index = 0; index < _items.size(); index++) {
			_vm->_screen->fillRect(_textX - 2, _textY + index * 16 - 10, _textX + 10, _textY + index * 16 + 2, 84);
			if (index == (uint)_selectedItemIndex) {
				_vm->_screen->frameRect(_textX - 3, _textY + index * 16 - 11, _textX + 11, _textY + index * 16 + 3, _frameColor);
			} else {
				_vm->_screen->frameRect(_textX - 3, _textY + index * 16 - 11, _textX + 11, _textY + index * 16 + 3, 88);
			}
			_vm->_screen->setFontColor(94);
			_vm->_screen->drawText(_textX, _textY + index * 16 - 8, letter);
			letter[0]++;
		}
		if (_selectedItemIndex != _selectedItemIndex2) {
			if (_selectedItemIndex >= 0) {
				// Play the speech for the selected item
				_vm->playVoice(_items[_selectedItemIndex].index);
			}
			_selectedItemIndex2 = _selectedItemIndex;
		}
	}
}

uint16 Dialog::getChoiceScriptIp() {
	return _items[_selectedItemIndex].scriptIp;
}

} // End of namespace Comet
