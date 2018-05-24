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

#include "illusions/menusystem.h"
#include "illusions/illusions.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "gui/options.h"
#include "gui/saveload.h"
#include "gui/widget.h"

namespace Illusions {

// BaseMenuItem

BaseMenuItem::BaseMenuItem(BaseMenuSystem *menuSystem)
	: _menuSystem(menuSystem) {
}

// MenuActionItem

MenuActionItem::MenuActionItem(BaseMenuSystem *menuSystem, const Common::String text)
	: BaseMenuItem(menuSystem), _text(text) {
}

// MenuSliderItem

MenuSliderItem::MenuSliderItem(BaseMenuSystem *menuSystem, const Common::String text, int sliderId, int maxValue, int defValue,
	int x1, int y1, int x2, int y2)
	: BaseMenuItem(menuSystem), _text(text), _sliderId(sliderId), _sliderText(text), _maxValue(maxValue), _defValue(defValue),
	_x1(x1), _y1(y1), _x2(x2), _y2(y2), _currValue(0) {
}

void MenuSliderItem::refresh() {
	_currValue = _menuSystem->getSliderValue(_sliderId);
	buildSliderText();
}

void MenuSliderItem::handleSlider(int delta) {
	int newSliderValue = _currValue + delta;
	if (newSliderValue >= 0 && newSliderValue < _maxValue) {
		_currValue = newSliderValue;
		_menuSystem->setSliderValue(_sliderId, newSliderValue);
	}
}

void MenuSliderItem::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	WRect menuItemRect;
	_menuSystem->calcMenuItemRect(menuItemIndex, menuItemRect);
	int x1 = menuItemRect._topLeft.x + _x1;
	int x2 = menuItemRect._topLeft.x + _x2;
	int y1 = menuItemRect._topLeft.y + _y1;
	int y2 = menuItemRect._topLeft.y + _y2;
	bool canUpdateSlider = false;
	int newSliderValue = 0;

	if (mousePos.y >= y1 && mousePos.y <= y2) {
		if (mousePos.x >= x1 && mousePos.x <= x2) {
			newSliderValue = _maxValue * (mousePos.x + (((x2 - x1) / _maxValue) / 2) - x1) / (x2 - x1);
			if (newSliderValue >= _maxValue)
				newSliderValue = _maxValue - 1;
			canUpdateSlider = true;
		} else if (mousePos.x >= x1 - _defValue && mousePos.x < x1) {
			newSliderValue = 0;
			canUpdateSlider = true;
		} else if (mousePos.x > x2 && mousePos.x <= x2 + _defValue) {
			newSliderValue = _maxValue - 1;
			canUpdateSlider = true;
		}
	}

	if (!canUpdateSlider) {
		_menuSystem->playSoundEffect14();
	} else if (_currValue != newSliderValue) {
		_currValue = newSliderValue;
		_menuSystem->setSliderValue(_sliderId, newSliderValue);
	}

}

void MenuSliderItem::buildSliderText() {
	_sliderText = _text;
	_sliderText += '{';
	for (int i = 0; i < _maxValue; ++i)
		_sliderText += i == _currValue ? '|' : '~';
	_sliderText += '}';
}

// BaseMenu

BaseMenu::BaseMenu(BaseMenuSystem *menuSystem, uint32 fontId, byte field8, byte fieldA, byte fieldC, byte fieldE,
	uint defaultMenuItemIndex)
	: _menuSystem(menuSystem), _fontId(fontId), _field8(field8), _fieldA(fieldA), _fieldC(fieldC), _fieldE(fieldE),
	_defaultMenuItemIndex(defaultMenuItemIndex)
{
}

BaseMenu::~BaseMenu() {
	for (MenuItems::iterator it = _menuItems.begin(); it != _menuItems.end(); ++it)
		delete *it;
}

void BaseMenu::addText(const Common::String text) {
	_text.push_back(text);
}

void BaseMenu::addMenuItem(BaseMenuItem *menuItem) {
	_menuItems.push_back(menuItem);
}

uint BaseMenu::getHeaderLinesCount() {
	return _text.size();
}

const Common::String& BaseMenu::getHeaderLine(uint index) {
	return _text[index];
}

uint BaseMenu::getMenuItemsCount() {
	return _menuItems.size();
}

BaseMenuItem *BaseMenu::getMenuItem(uint index) {
	return _menuItems[index];
}

void BaseMenu::refresh() {
	for (MenuItems::iterator it = _menuItems.begin(); it != _menuItems.end(); ++it)
		(*it)->refresh();
}

// BaseMenuSystem

BaseMenuSystem::BaseMenuSystem(IllusionsEngine *vm)
	: _vm(vm), _isTimeOutEnabled(false), _menuChoiceOffset(0) {
}

BaseMenuSystem::~BaseMenuSystem() {
}

void BaseMenuSystem::playSoundEffect12() {
	// TODO
}

void BaseMenuSystem::playSoundEffect13() {
	// TODO
}

void BaseMenuSystem::playSoundEffect14() {
	// TODO
}

void BaseMenuSystem::selectMenuChoiceIndex(uint choiceIndex) {
	debug(0, "choiceIndex: %d", choiceIndex);
	debug(0, "_menuChoiceOffset: %p", (void*)_menuChoiceOffset);
	if (choiceIndex > 0 && _menuChoiceOffset) {
		*_menuChoiceOffset = _menuChoiceOffsets[choiceIndex - 1];
		debug(0, "*_menuChoiceOffset: %04X", *_menuChoiceOffset);
	}
	_vm->_threads->notifyId(_menuCallerThreadId);
	_menuCallerThreadId = 0;
	closeMenu();
}

void BaseMenuSystem::leaveMenu() {
	playSoundEffect13();
	if (!_menuStack.empty())
		leaveSubMenu();
	else
		closeMenu();
}

void BaseMenuSystem::enterSubMenu(BaseMenu *menu) {
	_menuStack.push(_activeMenu);
	activateMenu(menu);
	_hoveredMenuItemIndex = _hoveredMenuItemIndex3;
	_hoveredMenuItemIndex2 = _hoveredMenuItemIndex3;
	setMouseCursorToMenuItem(_hoveredMenuItemIndex);
	placeActor318();
	placeActor323();
}

void BaseMenuSystem::leaveSubMenu() {
	_activeMenu = _menuStack.pop();
	_field54 = _activeMenu->_field2C18;
	_menuLinesCount = _activeMenu->getHeaderLinesCount();
	_hoveredMenuItemIndex = 1;
	_vm->_screenText->removeText();
	_vm->_screenText->removeText();
	activateMenu(_activeMenu);
	_hoveredMenuItemIndex = _hoveredMenuItemIndex3;
	_hoveredMenuItemIndex2 = _hoveredMenuItemIndex3;
	setMouseCursorToMenuItem(_hoveredMenuItemIndex);
	initActor318();
	placeActor323();
}

void BaseMenuSystem::enterSubMenuById(int menuId) {
	BaseMenu *menu = getMenuById(menuId);
	enterSubMenu(menu);
}

uint BaseMenuSystem::getQueryConfirmationChoiceIndex() const {
	return _queryConfirmationChoiceIndex;
}

void BaseMenuSystem::setQueryConfirmationChoiceIndex(uint queryConfirmationChoiceIndex) {
	_queryConfirmationChoiceIndex = queryConfirmationChoiceIndex;
}

void BaseMenuSystem::setMouseCursorToMenuItem(int menuItemIndex) {
	Common::Point mousePos;
	if (calcMenuItemMousePos(menuItemIndex, mousePos))
		setMousePos(mousePos);
}

void BaseMenuSystem::calcMenuItemRect(uint menuItemIndex, WRect &rect) {
	FontResource *font = _vm->_dict->findFont(_activeMenu->_fontId);
	int charHeight = font->getCharHeight() + font->getLineIncr();
	
	_vm->_screenText->getTextInfoPosition(rect._topLeft);
	if (_activeMenu->_field8) {
		rect._topLeft.y += 4;
		rect._topLeft.x += 4;
	}
	rect._topLeft.y += charHeight * (menuItemIndex + _menuLinesCount - 1);

	WidthHeight textInfoDimensions;
	_vm->_screenText->getTextInfoDimensions(textInfoDimensions);
	rect._bottomRight.x = rect._topLeft.x + textInfoDimensions._width;
	rect._bottomRight.y = rect._topLeft.y + charHeight;
}

bool BaseMenuSystem::calcMenuItemMousePos(uint menuItemIndex, Common::Point &pt) {
	if (menuItemIndex < _hoveredMenuItemIndex3 || menuItemIndex >= _hoveredMenuItemIndex3 + _menuItemCount)
		return false;

	WRect rect;
	calcMenuItemRect(menuItemIndex - _hoveredMenuItemIndex3 + 1, rect);
	pt.x = rect._topLeft.x;
	pt.y = rect._topLeft.y + (rect._bottomRight.y - rect._topLeft.y) / 2;
	return true;
}

bool BaseMenuSystem::calcMenuItemIndexAtPoint(Common::Point pt, uint &menuItemIndex) {
	WRect rect;
	calcMenuItemRect(1, rect);
	
	uint index = _hoveredMenuItemIndex3 + (pt.y - rect._topLeft.y) / (rect._bottomRight.y - rect._topLeft.y);

	if (pt.y < rect._topLeft.y || pt.x < rect._topLeft.x || pt.x > rect._bottomRight.x ||
		index > _field54 || index > _hoveredMenuItemIndex3 + _menuItemCount - 1)
		return false;

	menuItemIndex = index;
	return true;
}

void BaseMenuSystem::setMousePos(Common::Point &mousePos) {
	_vm->_input->setCursorPosition(mousePos);
	Control *mouseCursor = _vm->getObjectControl(0x40004);
	mouseCursor->_actor->_position = mousePos;
}

Common::Point BaseMenuSystem::getMousePos() {
	Control *mouseCursor = _vm->getObjectControl(0x40004);
	return mouseCursor->_actor->_position;
}

void BaseMenuSystem::activateMenu(BaseMenu *menu) {
	_activeMenu = menu;
	// TODO Run menu enter callback if neccessary
	menu->refresh();
	_menuLinesCount = menu->getHeaderLinesCount();
	menu->_field2C18 = menu->getMenuItemsCount();
	_hoveredMenuItemIndex3 = 1;
	_field54 = menu->_field2C18;

	uint v2 = drawMenuText(menu);
	if (menu->_field2C18 <= v2)
		_menuItemCount = menu->_field2C18;
	else
		_menuItemCount = v2;

}

void BaseMenuSystem::updateMenu(BaseMenu *menu) {
	_activeMenu = menu;
	_vm->_screenText->removeText();
	menu->refresh();
	_menuLinesCount = menu->getHeaderLinesCount();
	menu->_field2C18 = menu->getMenuItemsCount();
	_field54 = menu->_field2C18;

	if (_hoveredMenuItemIndex > menu->_field2C18)
		_hoveredMenuItemIndex = menu->_field2C18;
	if (_hoveredMenuItemIndex2 > menu->_field2C18)
		_hoveredMenuItemIndex2 = menu->_field2C18;

	uint v2 = drawMenuText(menu);
	if (menu->_field2C18 <= v2)
		_menuItemCount = menu->_field2C18;
	else if (_hoveredMenuItemIndex3 == 1)
		_menuItemCount = v2;

	placeActor318();
	placeActor323();
}

void BaseMenuSystem::initActor318() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	if (!v0) {
		WidthHeight dimensions;
		dimensions._width = 300;
		dimensions._height = 15;
		if (_vm->getGameId() == kGameIdBBDOU) {
			_vm->_controls->placeSequenceLessActor(0x4013E, Common::Point(0, 0), dimensions, 91);
		} else {
			_vm->_controls->placeSequenceLessActor(0x4013E, Common::Point(0, 0), dimensions, 18);
		}
		v0 = _vm->getObjectControl(0x4013E);
		v0->_flags |= 8;
	}
	placeActor318();
	v0->appearActor();
}	

void BaseMenuSystem::placeActor318() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	v0->fillActor(0);
	
	WidthHeight textInfoDimensions;
	_vm->_screenText->getTextInfoDimensions(textInfoDimensions);

	if ( _activeMenu->_field8 && _activeMenu->_fieldA != _activeMenu->_field8)
		textInfoDimensions._width -= 6;
		
	WidthHeight frameDimensions;
	v0->getActorFrameDimensions(frameDimensions);
	
	FontResource *font = _vm->_dict->findFont(_activeMenu->_fontId);
	int charHeight = font->getCharHeight() + font->getLineIncr();
	if (frameDimensions._height < charHeight)
		charHeight = frameDimensions._height;
		
	v0->drawActorRect(Common::Rect(textInfoDimensions._width - 1, charHeight - 1), _activeMenu->_fieldE);
	
	updateActor318();
}

void BaseMenuSystem::updateActor318() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	WRect rect;
	calcMenuItemRect(_hoveredMenuItemIndex2 - _hoveredMenuItemIndex3 + 1, rect);
  	v0->setActorPosition(rect._topLeft);
}

void BaseMenuSystem::hideActor318() {
	Control *v0 = _vm->getObjectControl(0x4013E);
	if (v0)
		v0->disappearActor();
}

void BaseMenuSystem::initActor323() {
	Control *v0 = _vm->getObjectControl(0x40143);
	if (!v0) {
		WidthHeight dimensions;
		if (_vm->getGameId() == kGameIdBBDOU) {
			dimensions._width = 420;
			dimensions._height = 180;
			_vm->_controls->placeSequenceLessActor(0x40143, Common::Point(0, 0), dimensions, 90);
		} else {
			dimensions._width = 300;
			dimensions._height = 180;
			_vm->_controls->placeSequenceLessActor(0x40143, Common::Point(0, 0), dimensions, 17);
		}
		v0 = _vm->getObjectControl(0x40143);
		v0->_flags |= 8;
	}
	placeActor323();
	v0->appearActor();
}

void BaseMenuSystem::placeActor323() {
	Control *v0 = _vm->getObjectControl(0x40143);
	v0->fillActor(0);
	
	Common::Point textInfoPosition;
	WidthHeight textInfoDimensions;
	_vm->_screenText->getTextInfoPosition(textInfoPosition);
	_vm->_screenText->getTextInfoDimensions(textInfoDimensions);
	
	if (_activeMenu->_field8 && _activeMenu->_fieldA != _activeMenu->_field8) {
		textInfoDimensions._width -= 2;
		textInfoDimensions._height -= 6;
	}

	v0->setActorPosition(textInfoPosition);
	v0->drawActorRect(Common::Rect(textInfoDimensions._width - 1, textInfoDimensions._height - 1), _activeMenu->_fieldC);

}

void BaseMenuSystem::hideActor323() {
	Control *v0 = _vm->getObjectControl(0x40143);
	if (v0)
		v0->disappearActor();
}

void BaseMenuSystem::openMenu(BaseMenu *menu) {
	loadSettings();
	
	_isActive = true;
	_menuStack.clear();
	
	_cursorInitialVisibleFlag = initMenuCursor();
	_savedCursorPos = _vm->_input->getCursorPosition();
	_savedGameState = getGameState();
	Control *cursorControl = _vm->getObjectControl(0x40004);
	_savedCursorActorIndex = cursorControl->_actor->_actorIndex;
	_savedCursorSequenceId = cursorControl->_actor->_sequenceId;
	
	setMenuCursorNum(1);

	if (_vm->getGameId() == kGameIdDuckman) {
		setGameState(4);
	} else if (_vm->getGameId() == kGameIdBBDOU) {
		setGameState(3);
	}
	
	activateMenu(menu);
	
	_hoveredMenuItemIndex = _hoveredMenuItemIndex3;
	_hoveredMenuItemIndex2 = _hoveredMenuItemIndex3;
	setMouseCursorToMenuItem(_hoveredMenuItemIndex);
	initActor318();
	initActor323();
	_vm->_input->discardAllEvents();
}

void BaseMenuSystem::closeMenu() {
	while (!_menuStack.empty()) {
		_vm->_screenText->removeText();
		_menuStack.pop();
	}
	_vm->_screenText->removeText();
	hideActor318();
	hideActor323();
	Control *mouseCursor = _vm->getObjectControl(0x40004);
	setGameState(_savedGameState);
	mouseCursor->_actor->_actorIndex = _savedCursorActorIndex;
	mouseCursor->_actor->_position = _savedCursorPos;
	setMousePos(_savedCursorPos);
	mouseCursor->startSequenceActor(_savedCursorSequenceId, 2, 0);
	if (_cursorInitialVisibleFlag)
		mouseCursor->disappearActor();
	_vm->_input->discardAllEvents();
	_isActive = false;
	saveSettings();
	syncSoundSettings();
}

void BaseMenuSystem::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	debug(0, "BaseMenuSystem::handleClick() menuItemIndex: %d", menuItemIndex);

	if (menuItemIndex == 0) {
	    playSoundEffect14();
	    return;
	}

	BaseMenuItem *menuItem = _activeMenu->getMenuItem(menuItemIndex - 1);
	menuItem->handleClick(menuItemIndex, mousePos);
	
}

void BaseMenuSystem::handleUpKey() {
	// NOTE Up/down arrows seem to be used for switching pages in the current menu.
	// Since the maximum number of items to be displayed at the same time
	// is always set to the total number of items in activateMenu,
	// these two functions have no effect.
	// Still left in to avoid future confusion.
}

void BaseMenuSystem::handleDownKey() {
	// NOTE See handleUpKey
}

uint BaseMenuSystem::drawMenuText(BaseMenu *menu) {
	MenuTextBuilder *menuTextBuilder = new MenuTextBuilder();
	uint lineCount = 0;
	
	for (uint i = 0; i < menu->getHeaderLinesCount(); ++i) {
		menuTextBuilder->appendString(menu->getHeaderLine(i));
		menuTextBuilder->appendNewLine();
	}

	for (uint i = _hoveredMenuItemIndex3; i <= _field54; ++i) {
		menuTextBuilder->appendString(menu->getMenuItem(i - 1)->getText());
		if (i + 1 <= menu->getMenuItemsCount())
			menuTextBuilder->appendNewLine();
		++lineCount;
	}
	
	menuTextBuilder->finalize();

	uint16 *text = menuTextBuilder->getText();

	Common::Point textPt;
	int16 v9 = 0;
	if (menu->_field8)
		v9 = 4;
	textPt.x = v9;
	textPt.y = v9;

	uint flags = 0x01;
	if (menu->_field8 != menu->_fieldA)
		flags = 0x10 | 0x08 | 0x01;
		
	WidthHeight dimensions;

	if (_vm->getGameId() == kGameIdDuckman) {
		dimensions._width = 300;
		dimensions._height = 180;
	} else if (_vm->getGameId() == kGameIdBBDOU) {
		dimensions._width = 580;
		dimensions._height = 420;
	}

	uint16 *outTextPtr;
	if (!_vm->_screenText->insertText(text, menu->_fontId, dimensions, textPt, flags, menu->_field8, menu->_fieldA, 0xFF, 0xFF, 0xFF, outTextPtr)) {
		--lineCount;
		for ( ; *outTextPtr; ++outTextPtr) {
			if (*outTextPtr == 13)
				--lineCount;
		}
	}

	delete menuTextBuilder;

	return lineCount;
}

void BaseMenuSystem::update(Control *cursorControl) {
	bool resetTimeOut = false;
    Common::Point mousePos;

	if (_vm->_input->isCursorMovedByKeyboard()) {
		Common::Point mousePosDelta = _vm->_input->getCursorDelta();
		if (mousePosDelta.x != 0) {
			// Move volume slider left/right
			if (_hoveredMenuItemIndex > 0 && _activeMenu) {
				_activeMenu->getMenuItem(_hoveredMenuItemIndex - 1)->handleSlider(mousePosDelta.x < 0 ? 1 : -1);
				resetTimeOut = true;
			}
			mousePos = getMousePos();
		} else if (mousePosDelta.y != 0) {
			uint newHoveredMenuItemIndex = _hoveredMenuItemIndex;
			uint visibleItemCount = _menuItemCount;
			if (_hoveredMenuItemIndex3 + visibleItemCount - 1 > _field54)
				visibleItemCount = _field54 - _hoveredMenuItemIndex3 + 1;
			if (_hoveredMenuItemIndex) {
				if (mousePosDelta.y < 0) {
					if (newHoveredMenuItemIndex < _hoveredMenuItemIndex3 + visibleItemCount - 1) {
						++newHoveredMenuItemIndex;
					} else {
						newHoveredMenuItemIndex = _hoveredMenuItemIndex3;
					}
				} else {
					if (newHoveredMenuItemIndex > _hoveredMenuItemIndex3) {
						--newHoveredMenuItemIndex;
					} else {
						newHoveredMenuItemIndex = _hoveredMenuItemIndex3 + visibleItemCount - 1;
					}
				}
			} else {
				newHoveredMenuItemIndex = _hoveredMenuItemIndex2;
			}
			calcMenuItemMousePos(newHoveredMenuItemIndex, mousePos);
			if (_hoveredMenuItemIndex != newHoveredMenuItemIndex)
				playSoundEffect12();
		} else {
			mousePos = getMousePos();
		}
		// g_system->delayMillis(100); // TODO Slow down keyboard input, no idea how
	} else {
		mousePos = _vm->_input->getCursorPosition();
	}

	setMousePos(mousePos);

	uint newHoveredMenuItemIndex;
	
	if (calcMenuItemIndexAtPoint(mousePos, newHoveredMenuItemIndex)) {
		if (newHoveredMenuItemIndex != _hoveredMenuItemIndex) {
			if (_hoveredMenuItemIndex == 0)
				initActor318();
			_hoveredMenuItemIndex = newHoveredMenuItemIndex;
			_hoveredMenuItemIndex2 = newHoveredMenuItemIndex;
			setMenuCursorNum(2);
			updateActor318();
			resetTimeOut = true;
		}
	} else if (_hoveredMenuItemIndex != 0) {
		setMenuCursorNum(1);
		hideActor318();
		_hoveredMenuItemIndex = 0;
		resetTimeOut = true;
	}

	if (_vm->_input->hasNewEvents())
		resetTimeOut = true;

	if (_vm->_input->pollEvent(kEventLeftClick)) {
		handleClick(_hoveredMenuItemIndex, mousePos);
	} else if (_vm->_input->pollEvent(kEventAbort) && _activeMenu->_defaultMenuItemIndex) {
		handleClick(_activeMenu->_defaultMenuItemIndex, mousePos);
	} else if (_vm->_input->pollEvent(kEventUp)) {
		handleUpKey();
	} else if (_vm->_input->pollEvent(kEventDown)) {
		handleDownKey();
	}
	
	updateTimeOut(resetTimeOut);
}

void BaseMenuSystem::setTimeOutDuration(uint32 duration, uint timeOutMenuChoiceIndex) {
	if (duration > 0) {
		_isTimeOutEnabled = true;
		_isTimeOutReached = false;
		_timeOutDuration = duration;
		_timeOutMenuChoiceIndex = timeOutMenuChoiceIndex;
		_timeOutStartTime = getCurrentTime();
		_timeOutEndTime = duration + _timeOutStartTime;
	} else {
		_isTimeOutEnabled = false;
	}
}

void BaseMenuSystem::setMenuCallerThreadId(uint32 menuCallerThreadId) {
	_menuCallerThreadId = menuCallerThreadId;
}

void BaseMenuSystem::setMenuChoiceOffsets(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset) {
	_menuChoiceOffsets = menuChoiceOffsets;
	_menuChoiceOffset = menuChoiceOffset;
}

void BaseMenuSystem::setSavegameSlotNum(int slotNum) {
	_vm->_savegameSlotNum = slotNum;
}

void BaseMenuSystem::loadSettings() {
	_musicVolume = ConfMan.getInt("music_volume");
	_sfxVolume = ConfMan.getInt("sfx_volume");
	_speechVolume = ConfMan.getInt("speech_volume");
	_textDuration = 255 - ConfMan.getInt("talkspeed");
}

void BaseMenuSystem::saveSettings() {
	ConfMan.setInt("music_volume", _musicVolume);
	ConfMan.setInt("sfx_volume", _sfxVolume);
	ConfMan.setInt("speech_volume", _speechVolume);
	ConfMan.setInt("talkspeed", 255 - _textDuration);
}

void BaseMenuSystem::syncSoundSettings() {
	_vm->syncSoundSettings();
}

void BaseMenuSystem::playTestSound(int volume) {
	// TODO
}

int BaseMenuSystem::getDefaultSfxVolume() {
	return 11 * 16;
}

int BaseMenuSystem::getDefaultMusicVolume() {
	return 11 * 16;
}

int BaseMenuSystem::getDefaultSpeechVolume() {
	return 15 * 16;
}

int BaseMenuSystem::getDefaultTextDuration() {
	return 240;
}

void BaseMenuSystem::restoreDefaultSettings() {
	_sfxVolume = getDefaultSfxVolume();
	_musicVolume = getDefaultMusicVolume();
	_speechVolume = getDefaultSpeechVolume();
	_textDuration = getDefaultTextDuration();
	updateMenu(_activeMenu);
}

int BaseMenuSystem::getSliderValue(int sliderId) {
	switch (sliderId) {
	case kSliderSFXVolume:
		return _sfxVolume / 16;
	case kSliderMusicVolume:
		return _musicVolume / 16;
	case kSliderSpeechVolume:
		return _speechVolume / 16;
	case kSliderTextDuration:
		return _textDuration / 60;
	default:
		return 0;
	}
}

void BaseMenuSystem::setSliderValue(int sliderId, int value) {
	switch (sliderId) {
	case kSliderSFXVolume:
		_sfxVolume = value * 16;
		playTestSound(_sfxVolume);
		break;
	case kSliderMusicVolume:
		_musicVolume = value * 16;
		playTestSound(_sfxVolume);
		break;
	case kSliderSpeechVolume:
		_speechVolume = value * 16;
		playTestSound(_speechVolume);
		if (_speechVolume == 0 && _textDuration == 0) {
			// Enable text if speech is silent
			_textDuration = getDefaultTextDuration();
		}
		break;
	case kSliderTextDuration:
		_textDuration = value * 60;
		playTestSound(_sfxVolume);
		if (_textDuration == 0 && _speechVolume == 0) {
			// Enable speech if text is disabled
			_speechVolume = getDefaultSpeechVolume();
		}
		break;
	default:
		break;
	}
	updateMenu(_activeMenu);
}

void BaseMenuSystem::updateTimeOut(bool resetTimeOut) {

	if (!_isTimeOutEnabled)
		return;

	if (_menuStack.empty()) {
		if (_isTimeOutReached) {
			resetTimeOut = true;
			_isTimeOutReached = false;
		}
	} else if (!_isTimeOutReached) {
		_isTimeOutReached = true;
	}
	
	if (!_isTimeOutReached) {
		if (resetTimeOut) {
			_timeOutStartTime = getCurrentTime();
			_timeOutEndTime = _timeOutDuration + _timeOutStartTime;
		} else if (isTimerExpired(_timeOutStartTime, _timeOutEndTime)) {
			_isTimeOutEnabled = false;
			selectMenuChoiceIndex(_timeOutMenuChoiceIndex);
		}
	}

}

// MenuTextBuilder

MenuTextBuilder::MenuTextBuilder() : _pos(0) {
}

void MenuTextBuilder::appendString(const Common::String &value) {
	for (uint i = 0; i < value.size(); ++i)
		_text[_pos++] = value[i];
}

void MenuTextBuilder::appendNewLine() {
	_text[_pos++] = '\r';
}

void MenuTextBuilder::finalize() {
	_text[_pos] = '\0';
}

// MenuActionEnterMenu

MenuActionEnterMenu::MenuActionEnterMenu(BaseMenuSystem *menuSystem, const Common::String text, int menuId)
	: MenuActionItem(menuSystem, text), _menuId(menuId) {
}

void MenuActionEnterMenu::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	_menuSystem->enterSubMenuById(_menuId);
}

// MenuActionLeaveMenu

MenuActionLeaveMenu::MenuActionLeaveMenu(BaseMenuSystem *menuSystem, const Common::String text)
	: MenuActionItem(menuSystem, text) {
}

void MenuActionLeaveMenu::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	_menuSystem->leaveMenu();
}

// MenuActionReturnChoice

MenuActionReturnChoice::MenuActionReturnChoice(BaseMenuSystem *menuSystem, const Common::String text, uint choiceIndex)
	: MenuActionItem(menuSystem, text), _choiceIndex(choiceIndex) {
}

void MenuActionReturnChoice::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	_menuSystem->playSoundEffect13();
	_menuSystem->selectMenuChoiceIndex(_choiceIndex);
}

// MenuActionEnterQueryMenu

MenuActionEnterQueryMenu::MenuActionEnterQueryMenu(BaseMenuSystem *menuSystem, const Common::String text, int menuId, uint confirmationChoiceIndex)
	: MenuActionItem(menuSystem, text), _menuId(menuId), _confirmationChoiceIndex(confirmationChoiceIndex) {
}

void MenuActionEnterQueryMenu::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	_menuSystem->setQueryConfirmationChoiceIndex(_confirmationChoiceIndex);
	_menuSystem->enterSubMenuById(_menuId);
}

// MenuActionLoadGame

MenuActionLoadGame::MenuActionLoadGame(BaseMenuSystem *menuSystem, const Common::String text, uint choiceIndex)
	: MenuActionItem(menuSystem, text), _choiceIndex(choiceIndex) {
}

void MenuActionLoadGame::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	const EnginePlugin *plugin = NULL;
	EngineMan.findGame(ConfMan.get("gameid"), &plugin);
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	slot = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());

	delete dialog;

	if (slot >= 0) {
		_menuSystem->setSavegameSlotNum(slot);
		_menuSystem->selectMenuChoiceIndex(_choiceIndex);
	}

}

// MenuActionRestoreDefaultSettings

MenuActionRestoreDefaultSettings::MenuActionRestoreDefaultSettings(BaseMenuSystem *menuSystem, const Common::String text)
	: MenuActionItem(menuSystem, text) {
}

void MenuActionRestoreDefaultSettings::handleClick(uint menuItemIndex, const Common::Point &mousePos) {
	_menuSystem->restoreDefaultSettings();
}

} // End of namespace Illusions
