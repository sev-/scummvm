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

#include "graphics/cursorman.h"

#include "comet/comet.h"
#include "comet/comet_gui.h"
#include "comet/animationmgr.h"
#include "comet/input.h"
#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/screen.h"
#include "comet/console.h"
#include "comet/talktext.h"

namespace Comet {

// GuiPage

GuiPage::GuiPage(CometEngine *vm)
	: CometTaskBase(vm) {
}

void GuiPage::enter() {
	_vm->_gui->onEnterPage(this);
}

void GuiPage::leave() {
	_vm->_gui->onLeavePage(this);
}

Gui::Gui(CometEngine *vm) : _vm(vm) {
	_guiInventory = new GuiInventory(_vm);
	_guiCommandBar = new GuiCommandBar(_vm);
	_guiJournal = new GuiJournal(_vm);
	_guiTownMap = new GuiTownMap(_vm);
	_guiMainMenu = new GuiMainMenu(_vm);
	_guiOptionsMenu = new GuiOptionsMenu(_vm);
	_guiPuzzle = new GuiPuzzle(_vm);
//	_guiSaveLoadMenu = new GuiSaveLoadMenu(_vm);
	_gameScreen = new byte[64000];
	_currPage = NULL;
}

Gui::~Gui() {
	delete _guiInventory;
	delete _guiCommandBar;
	delete _guiJournal;
	delete _guiTownMap;
	delete _guiMainMenu;
	delete _guiOptionsMenu;
	delete _guiPuzzle;
//	delete _guiSaveLoadMenu;
	delete[] _gameScreen;
}

void Gui::onEnterPage(GuiPage *page) {
	debug("Gui::onEnterPage()");
	if (_currPage) {
		_stack.push_back(_currPage);
	}
	else {
		_vm->_screen->copyToScreen(_gameScreen);
	}
	_currPage = page;
}

void Gui::onLeavePage(GuiPage *page) {
	debug("Gui::onLeavePage()");
	_lastResult = page->getResult();
	if (_stack.size() > 0) {
		_currPage = _stack.back();
		_stack.pop_back();
		_vm->_screen->copyFromScreen(_gameScreen);
		for (Common::Array<GuiPage*>::iterator it = _stack.begin(); it != _stack.end(); it++)
			(*it)->draw();
	} else {
		_currPage = NULL;
	}
}

GuiPage *Gui::getGuiPage(GuiPageIdent page) {
	switch (page) {
	case kGuiInventory:
		return _guiInventory;
	case kGuiCommandBar:
		return _guiCommandBar;
	case kGuiJournal:
		return _guiJournal;
	case kGuiTownMap:
		return _guiTownMap;
	case kGuiMainMenu:
		return _guiMainMenu;
	case kGuiOptionsMenu:
		return _guiOptionsMenu;
	case kGuiPuzzle:
		return _guiPuzzle;
		break;
#if 0
	case kGuiSaveMenu:
	case kGuiLoadMenu:
		// TODO _guiSaveLoadMenu->setAsSaveMenu(page == kGuiSaveMenu);
		return _guiSaveLoadMenu;
#endif
	default:
		error("Gui::getGuiPage() Page %d not yet refactored", page);
		break;
	}
}

// GuiPage

void GuiPage::drawIcon(int elementIndex, int x, int y) {
	_vm->_screen->drawAnimationElement(_vm->_iconSprite, elementIndex, x, y);
}

void GuiPage::drawAnimatedIcon(uint frameListIndex, int x, int y, uint animFrameCounter) {
	drawAnimatedIconSprite(_vm->_iconSprite, frameListIndex, x, y, animFrameCounter);
}

void GuiPage::drawAnimatedInventoryIcon(uint frameListIndex, int x, int y, uint animFrameCounter) {
	drawAnimatedIconSprite(_vm->_inventoryItemSprites, frameListIndex, x, y, animFrameCounter);
}

void GuiPage::drawAnimatedIconSprite(AnimationResource *animation, uint frameListIndex, int x, int y, uint animFrameCounter) {
	AnimationFrameList *frameList = animation->getFrameList(frameListIndex);
	uint frameIndex = 0;
	if (frameList->getFrameCount() > 1) {
		frameIndex = animFrameCounter % frameList->getFrameCount();
		frameList->accumulateDrawOffset(x, y, frameIndex);
	}
	_vm->_screen->drawAnimationElement(animation, frameList->getFrame(frameIndex)->getElementIndex(), x, y);
}

// GuiInventory

enum GuiInventoryAction {
	kIANone   = -1,
	kIAUp     = -2,
	kIADown   = -3,
	kIAUse    = -4,
	kIASelect = -5,
	kIAExit   = -6
};

static const GuiRectangle kInventorySlotRects[] = {
	{160, 182, 170, 190, kIADown},
	{160,  53, 170,  61, kIAUp},
	{ 74,  62, 253,  73,  0},
	{ 74,  74, 253,  85,  1},
	{ 74,  86, 253,  97,  2},
	{ 74,  98, 253, 109,  3},
	{ 74, 110, 253, 121,  4},
	{ 74, 122, 253, 133,  5},
	{ 74, 134, 253, 145,  6},
	{ 74, 146, 253, 157,  7},
	{ 74, 158, 253, 169,  8},
	{ 74, 170, 253, 181,  9}
};

const uint kMaxInventoryItemsOnScreen = 10;

void GuiInventory::enter() {
	GuiPage::enter();
	_mouseSelectedItem = -1;
	_firstItem = 0;
	_currentItem = 0;
	_animFrameCounter = 0;
	_inventoryStatus = 0;
	_vm->setMouseCursor(-1);
	// Build items array and set up variables
	_items.clear();
	_vm->_inventory.buildItems(_items, _firstItem, _currentItem);
	_inventoryAction = kIANone;
}

void GuiInventory::update() {

	//bool doWarpMouse = false;
#if 0
	_mouseSelectedItem = _vm->findRect(kInventorySlotRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), MIN<int>(_items.size() - _firstItem, 10) + 2, kIANone);
	if (_mouseSelectedItem >= 0)
		_currentItem = _firstItem + _mouseSelectedItem;
#endif
	drawInventory();

	switch (_inventoryAction) {
	case kIANone:
		break;
	case kIADown:
		if ((_currentItem - _firstItem + 1 < kMaxInventoryItemsOnScreen) && _currentItem + 1 < _items.size()) {
			// TODO doWarpMouse = _mouseSelectedItem == (_currentItem - _firstItem);
			++_currentItem;
		} else if (_firstItem + kMaxInventoryItemsOnScreen < _items.size()) {
			++_firstItem;
			++_currentItem;
		}
		break;
	case kIAUp:
		if (_currentItem > _firstItem) {
			// TODO doWarpMouse = _mouseSelectedItem == (_currentItem - _firstItem);
			--_currentItem;
		} else if (_firstItem > 0) {
			--_firstItem;
			--_currentItem;
		}
		break;
	case kIAExit:
		_inventoryStatus = 2;
		break;
	case kIASelect:
	case kIAUse:
		_vm->_inventory.resetStatus();
		_vm->_inventory.selectItem(_items[_currentItem]);
		// Return just selects, U actually uses the item
		if (_inventoryAction == kIAUse)
			_vm->_inventory.requestUseSelectedItem();
		_inventoryStatus = 1;
		break;
	default:
		break;
	}
#if 0	
	if (doWarpMouse) {
		_vm->warpMouseToRect(kInventorySlotRects[_currentItem - _firstItem + 2]);
		doWarpMouse = false;
	}
#endif
	
	_inventoryAction = kIANone;

	_vm->_screen->update();

	// TODO Save the current status etc. return 2 - _inventoryStatus;
}

void GuiInventory::handleEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_DOWN:
			_inventoryAction = kIADown;
			break;
		case Common::KEYCODE_UP:
			_inventoryAction = kIAUp;
			break;
		case Common::KEYCODE_ESCAPE:
			_inventoryAction = kIAExit;
			break;
		case Common::KEYCODE_RETURN:
			_inventoryAction = kIASelect;
			break;
		case Common::KEYCODE_u:
			_inventoryAction = kIAUse;
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (_mouseSelectedItem >= 0)
			_inventoryAction = kIAUse;
		else if (_mouseSelectedItem != kIANone)
			_inventoryAction = _mouseSelectedItem;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_inventoryAction = kIAExit;
		break;
	default:
		break;
	}
}

void GuiInventory::drawInventory() {
	const uint xadd = 74, yadd = 64, itemHeight = 12;

	drawIcon(16);
	// Draw up arrow
	if (_firstItem > 0)
		drawIcon(53);
	// Draw down arrow
	if (_firstItem + kMaxInventoryItemsOnScreen < _items.size())
		drawIcon(52);
	for (uint itemIndex = 0; (itemIndex < kMaxInventoryItemsOnScreen) && (_firstItem + itemIndex < _items.size()); itemIndex++) {
		byte *itemName = _vm->_inventoryItemNames->getString(_items[_firstItem + itemIndex]);
		const int itemX = xadd + 21, itemY = yadd + itemHeight * itemIndex;
		_vm->_screen->setFontColor(120);
		_vm->_screen->drawText(itemX, itemY, itemName);
		_vm->_screen->setFontColor(119);
		_vm->_screen->drawText(itemX + 1, itemY + 1, itemName);
		drawAnimatedInventoryIcon(_items[_firstItem + itemIndex], xadd, yadd + itemHeight * itemIndex - 3, _animFrameCounter);
	}
	if (_items.size() > 0) {
		const int selectionY = yadd + (_currentItem - _firstItem) * itemHeight - 1;
		_vm->_screen->frameRect(xadd + 16, selectionY, 253, selectionY + itemHeight - 1, _selectionColor);
		_selectionColor++;
		if (_selectionColor >= 96)
			_selectionColor = 80;
	}
}

// GuiCommandBar

enum GuiCommandBarAction {
	kCBANone      = -1,
	kCBAExit      = -2,
	kCBAVerbTalk  = 0,
	kCBAVerbGet   = 1,
	kCBAUseItem   = 2,
	kCBAVerbLook  = 3,
	kCBAInventory = 4,
	kCBAMap       = 5,
	kCBAMenu      = 6
};

static const GuiRectangle kCommandBarRects[] = {
	{  6, 4,  41, 34, kCBAVerbTalk}, 
	{ 51, 4,  86, 34, kCBAVerbGet}, 
	{ 96, 4, 131, 34, kCBAUseItem},
	{141, 4, 176, 34, kCBAVerbLook}, 
	{186, 4, 221, 34, kCBAInventory}, 
	{231, 4, 266, 34, kCBAMap},
	{276, 4, 311, 34, kCBAMenu}};
const int kCommandBarItemCount = 6; // Intentionally doesn't match actual count!

void GuiCommandBar::enter() {
	GuiPage::enter();
	_commandBarStatus = 0;
	_animFrameCounter = 0;
	_vm->setMouseCursor(-1);
	// The Lovecraft Museum that comes with the CD version needs some special handling
	if (isMuseum() && (_commandBarSelectedItem != kCBAVerbLook || _commandBarSelectedItem != kCBAMenu))
		_commandBarSelectedItem = kCBAVerbLook;
	_commandBarAction = kCBANone;
}

void GuiCommandBar::update() {
	TASK_BODY_BEGIN

	// TODO int mouseSelectedItem;
	// TODO bool doWarpMouse = false;

#if 0
	mouseSelectedItem = _vm->findRect(kCommandBarRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), kCommandBarItemCount + 1, kCBANone);

	if (isMuseum && (mouseSelectedItem != kCBAVerbLook || mouseSelectedItem != kCBAMenu))
		mouseSelectedItem = kCBANone;

	if (mouseSelectedItem != kCBANone)
		_commandBarSelectedItem = mouseSelectedItem;
#endif

	drawCommandBar(_commandBarSelectedItem);
	++_animFrameCounter;

#if 0
	if (isMuseum && (_vm->_input->getKeyCode() == Common::KEYCODE_RIGHT || _vm->_input->getKeyCode() == Common::KEYCODE_LEFT)) {
		if (_commandBarSelectedItem == kCBAVerbLook)
			_commandBarSelectedItem = kCBAMenu;
		else if (_commandBarSelectedItem == kCBAMenu)
			_commandBarSelectedItem = kCBAVerbLook;
		_vm->_input->clearKeyCode();
	}
#endif

#if 0
	if (doWarpMouse) {
		_vm->warpMouseToRect(kCommandBarRects[_commandBarSelectedItem]);
		doWarpMouse = false;
	}
#endif
	
	if (_commandBarAction >= 0) {
		drawCommandBar(_commandBarAction);
	}
	
	switch (_commandBarAction) {
	case kCBANone:
		break;
	case kCBAExit:
		_commandBarStatus = 2;
		break;
	case kCBAVerbTalk:
		_vm->_verbs.requestTalk();
		_commandBarStatus = 1;
		break;
	case kCBAVerbGet:
		_vm->_verbs.requestGet();
		_commandBarStatus = 1;
		break;
	case kCBAVerbLook:
		_vm->_verbs.requestLook();
		_commandBarStatus = 1;
		break;
	case kCBAUseItem:
		_vm->useCurrentInventoryItem();
		_commandBarStatus = 1;
		break;
	}
	
	_vm->_screen->update();

	// Can't use switch here
	if (_commandBarAction == kCBAInventory) {
		TASK_AWAIT_TASK(_vm->_gui->getGuiPage(kGuiInventory));
		_commandBarStatus = _vm->_gui->getLastResult();
	} else if (_commandBarAction == kCBAMenu) {
		TASK_AWAIT_TASK(_vm->_gui->getGuiPage(kGuiMainMenu));
		_commandBarStatus = _vm->_gui->getLastResult();
	} else if (_commandBarAction == kCBAMap && _vm->canShowMap()) {
		TASK_AWAIT_TASK(_vm->_gui->getGuiPage(kGuiTownMap));
		_commandBarStatus = _vm->_gui->getLastResult();
	}
	
	_commandBarAction = kCBANone;

	TASK_BODY_END
}

void GuiCommandBar::handleEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_RIGHT:
			// TODO doWarpMouse = mouseSelectedItem == _commandBarSelectedItem;
			selectNextItem();
			break;
		case Common::KEYCODE_LEFT:
			// TODO doWarpMouse = mouseSelectedItem == _commandBarSelectedItem;
			selectPrevItem();
			break;
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_TAB:
			_commandBarAction = kCBAExit;
			break;
		case Common::KEYCODE_RETURN:
			_commandBarAction = _commandBarSelectedItem;
			break;
		case Common::KEYCODE_t:
			_commandBarAction = kCBAVerbTalk;
			break;
		case Common::KEYCODE_g:
			_commandBarAction = kCBAVerbGet;
			break;
		case Common::KEYCODE_l:
			_commandBarAction = kCBAVerbLook;
			break;
		case Common::KEYCODE_o:
			_commandBarAction = kCBAInventory;
			break;
		case Common::KEYCODE_u:
			_commandBarAction = kCBAUseItem;
			break;
		case Common::KEYCODE_d:
			_commandBarAction = kCBAMenu;
			break;
		case Common::KEYCODE_m:
			_commandBarAction = kCBAMap;
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (_commandBarSelectedItem != kCBANone)
			_commandBarAction = _commandBarSelectedItem;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_commandBarAction = kCBAExit;
		break;
	default:
		break;
	}
}

void GuiCommandBar::selectNextItem() {
	++_commandBarSelectedItem;
	if (_commandBarSelectedItem > kCommandBarItemCount)
		_commandBarSelectedItem = 0;
}

void GuiCommandBar::selectPrevItem() {
	--_commandBarSelectedItem;
	if (_commandBarSelectedItem < 0)
		_commandBarSelectedItem = kCommandBarItemCount;
}

void GuiCommandBar::draw() {
	drawCommandBar(_commandBarSelectedItem);
}

void GuiCommandBar::drawCommandBar(int selectedItem) {
	const int kIconX = 196;
	const int kIconY = 14;
	
	drawIcon(0);
	drawIcon(selectedItem + 1);
	_vm->_inventory.testSelectFirstItem();
	if (_vm->_inventory.getSelectedItem() >= 0)
		drawAnimatedInventoryIcon(_vm->_inventory.getSelectedItem(), kIconX, kIconY, _animFrameCounter);
}

// GuiMainMenu

enum GuiMainMenuAction {
	kMMANone    = -1,
	kMMAExit    = -2,
	kMMASave    = 0,
	kMMALoad    = 1,
	kMMAOptions = 2,
	kMMAQuit    = 3
};

static const GuiRectangle kMainMenuRects[] = {
	{136,  64, 184,  80, kMMASave},
	{136,  87, 184, 103, kMMALoad},
	{136, 110, 184, 126, kMMAOptions},
	{136, 133, 184, 149, kMMAQuit}};

void GuiMainMenu::enter() {
	GuiPage::enter();
	_mainMenuStatus = 0;
	_mainMenuAction = kMMANone;
	_vm->setMouseCursor(-1);
}

void GuiMainMenu::leave() {
	GuiPage::leave();
}

void GuiMainMenu::update() {

	// TODO bool doWarpMouse = false;
#if 0
	_mouseSelectedItem = _vm->findRect(kMainMenuRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), 4, kMMANone);
	if (_mouseSelectedItem != kMMANone)
		_mainMenuSelectedItem = _mouseSelectedItem;
#endif		
	drawMainMenu(_mainMenuSelectedItem);
#if 0	
	if (doWarpMouse) {
		_vm->warpMouseToRect(kMainMenuRects[_mainMenuSelectedItem]);
		doWarpMouse = false;
	}
#endif
#if 0
	if (_mainMenuAction >= 0) {
		drawMainMenu(_mainMenuSelectedItem);
		_vm->_screen->update();
	}
#endif

	switch (_mainMenuAction) {
	case kMMANone:
		break;
	case kMMAExit:
		_mainMenuStatus = 2;
		break;
	case kMMASave:
		_mainMenuStatus = 0;// TODO _vm->_gui->run(kGuiSaveMenu);
		break;
	case kMMALoad:
		_mainMenuStatus = 0;// TODO _vm->_gui->run(kGuiLoadMenu);
		break;
	case kMMAOptions:
		_mainMenuStatus = 0;// TODO _vm->_gui->run(kGuiOptionsMenu);
		break;
	case kMMAQuit:
		_vm->quitGame();
		_mainMenuStatus = 0;
		break;
	}
	
	_vm->_screen->update();

	// return 0;
}

void GuiMainMenu::handleEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_DOWN:
			// TODO doWarpMouse = _mouseSelectedItem == _mainMenuSelectedItem;
			if (_mainMenuSelectedItem == 3) {
				_mainMenuSelectedItem = 0;
			} else {
				++_mainMenuSelectedItem;
			}
			break;
		case Common::KEYCODE_UP:
			// TODO doWarpMouse = _mouseSelectedItem == _mainMenuSelectedItem;
			if (_mainMenuSelectedItem == 0) {
				_mainMenuSelectedItem = 3;
			} else {
				--_mainMenuSelectedItem;
			}
			break;
		case Common::KEYCODE_ESCAPE:
			_mainMenuAction = kMMAExit;
			break;
		case Common::KEYCODE_RETURN:
			_mainMenuAction = _mainMenuSelectedItem;
			break;
		case Common::KEYCODE_s:
			_mainMenuAction = kMMASave;
			break;
		case Common::KEYCODE_l:
			_mainMenuAction = kMMALoad;
			break;
		case Common::KEYCODE_t:
			_mainMenuAction = kMMAOptions;
			break;
		case Common::KEYCODE_x:
			_mainMenuAction = kMMAQuit;
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (_mainMenuSelectedItem != kMMANone)
			_mainMenuAction = _mainMenuSelectedItem;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_mainMenuAction = kMMAExit;
		break;
	default:
		break;
	}
}

void GuiMainMenu::draw() {
	drawMainMenu(_mainMenuSelectedItem);
}

void GuiMainMenu::drawMainMenu(int selectedItem) {
	const int x = 137;
	const int y = 65;
	const int itemHeight = 23;
	drawIcon(10);
	drawIcon(11, x, y + selectedItem * itemHeight);
}

// GuiOptionsMenu

enum GuiOptionsMenuAction {
	kOMANone         = -1,
	kOMAExit         = -2,
	kOMAMusicVol     = 0,
	kOMASoundVol     = 1,
	kOMATextSpeed    = 2,
	kOMAGameSpeed    = 3,
	kOMADriveLetter  = 4,
	kOMALanguage     = 5,
	kOMAOk           = 6,
	kOMATalkie       = 7,
	kOMADefMusicVol  = 8,
	kOMADefSoundVol  = 9,
	kOMADefGameSpeed = 10
};

static const GuiRectangle kOptionsMenuRects[] = {
	{127,  64, 189,  79, kOMAMusicVol},
	{127,  84, 189,  99, kOMASoundVol},
	{127, 104, 164, 119, kOMATalkie},
	{127, 124, 189, 139, kOMAGameSpeed},
	{127, 144, 142, 159, kOMALanguage},
	{127, 164, 142, 179, kOMAOk},
	{172, 165, 199, 178, kOMAOk},
	{106,  64, 121,  79, kOMADefMusicVol},
	{106,  84, 121,  99, kOMADefSoundVol},
	{106, 104, 121, 119, kOMATalkie},
	{106, 124, 121, 139, kOMADefGameSpeed},
	{106, 144, 121, 159, kOMALanguage},
	{106, 164, 121, 179, 35}};//???

const int kVolumeConversionFactor = 17;

void GuiOptionsMenu::enter() {
	GuiPage::enter();
	_optionsMenuStatus = 0;
	_optionsMenuAction = kOMANone;
	_animFrameCounter = 0;
	_mouseSelectedItem = -1;
	_optionIncr = 0;
	// Load settings
	// NOTE There are no separate volume controls for speech and effects.
	// Because we know if a sample is speech or an effect we use the proper sound types
	// when playing. This means that the speech volume can only be changed in the ScummVM options.
	_musicVolumeDiv = ConfMan.getInt("music_volume") / kVolumeConversionFactor;
	_digiVolumeDiv = ConfMan.getInt("sfx_volume") / kVolumeConversionFactor;
	_currMusicVolumeDiv = _musicVolumeDiv;
	_textSpeed = ConfMan.getInt("text_speed");
	_gameSpeed = ConfMan.getInt("game_speed");
	_language = 1;
}

void GuiOptionsMenu::leave() {
	// Save settings
	if (_optionsMenuStatus == 1) {
		_vm->_talkText->setTextSpeed(_textSpeed);
		_vm->_gameSpeed = _gameSpeed;
		ConfMan.setInt("text_speed", _textSpeed);
		ConfMan.setInt("text_speed", _gameSpeed);
		ConfMan.setInt("music_volume", _musicVolumeDiv * kVolumeConversionFactor);
		ConfMan.setInt("sfx_volume", _digiVolumeDiv * kVolumeConversionFactor);
		ConfMan.flushToDisk();
		_vm->syncSoundSettings();
	}
	GuiPage::leave();
}

void GuiOptionsMenu::update() {
	// bool doWarpMouse = false;

#if 0
	if (_musicVolumeDiv != _currMusicVolumeDiv) {
		_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, _musicVolumeDiv * kVolumeConversionFactor);
		_currMusicVolumeDiv = _musicVolumeDiv;
	}
#endif
	++_animFrameCounter;
	if (_animFrameCounter == 32)
		_animFrameCounter = 0;

	drawOptionsMenu();

#if 0
	if (doWarpMouse) {
		_vm->warpMouseToRect(kOptionsMenuRects[_optionsMenuSelectedItem]);
		doWarpMouse = false;
	}
#endif
	switch (_optionsMenuAction) {
	case kOMANone:
		break;
	case kOMAExit:
		_optionsMenuStatus = 2;
		break;
	case kOMAMusicVol:
		if (_optionIncr != 0)
			_musicVolumeDiv = CLIP(_musicVolumeDiv + _optionIncr, 0, 15);
		else
			_musicVolumeDiv = _mouseSliderPos / 4;
		break;
	case kOMASoundVol:
		if (_optionIncr != 0)
			_digiVolumeDiv = CLIP(_digiVolumeDiv + _optionIncr, 0, 15);
		else
			_digiVolumeDiv = _mouseSliderPos / 4;
		break;
	case kOMATalkie:
		{
			int talkieMode = _vm->_talkText->getTalkieMode();
			if (talkieMode > 2)
				talkieMode = 0;
			_vm->_talkText->setTalkieMode(talkieMode);
		}
		break;
	case kOMAGameSpeed:
		if (_optionIncr != 0)
			_gameSpeed = CLIP(_gameSpeed + _optionIncr, 0, 15);
		else
			_gameSpeed = _mouseSliderPos / 4;
		break;
	case kOMATextSpeed:
		if (_optionIncr != 0)
			_textSpeed = CLIP(_textSpeed + _optionIncr, 0, 2);
		break;
	case kOMADriveLetter:
		// Nothing/not supported
		break;
	case kOMALanguage:
		if (_optionIncr != 0)
			_language = CLIP(_language + _optionIncr, 0, 4);
		else if (_language < 4)
			_language++;
		else
			_language = 0;
		break;
	case kOMAOk:
		_optionsMenuStatus = 1;
		break;
	case kOMADefMusicVol:
		_musicVolumeDiv = 8;
		break;
	case kOMADefSoundVol:
		_digiVolumeDiv = 8;
		break;
	case kOMADefGameSpeed:
		_gameSpeed = 8;
		break;
	}
	
	_optionsMenuAction = kOMANone;
	_optionIncr = 0;

	_vm->_screen->update();

	// TODO return 0;
}

void GuiOptionsMenu::handleEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_DOWN:
			// doWarpMouse = _mouseSelectedItem == _optionsMenuSelectedItem;
			if (_optionsMenuSelectedItem == 5)
				_optionsMenuSelectedItem = 0;
			else
				++_optionsMenuSelectedItem;
			break;
		case Common::KEYCODE_UP:
			// doWarpMouse = _mouseSelectedItem == _optionsMenuSelectedItem;
			if (_optionsMenuSelectedItem == 0)
				_optionsMenuSelectedItem = 5;
			else
				--_optionsMenuSelectedItem;
			break;
		case Common::KEYCODE_LEFT:
			_optionIncr = -1;
			if (_vm->isFloppy())
				_optionsMenuAction = _optionsMenuSelectedItem;
			else if (_optionsMenuSelectedItem != 5)
				_optionsMenuAction = kOptionsMenuRects[_optionsMenuSelectedItem].id;
			break;
		case Common::KEYCODE_RIGHT:
			_optionIncr = +1;
			if (_vm->isFloppy())
				_optionsMenuAction = _optionsMenuSelectedItem;
			else if (_optionsMenuSelectedItem != 5)
				_optionsMenuAction = kOptionsMenuRects[_optionsMenuSelectedItem].id;
			break;
		case Common::KEYCODE_ESCAPE:
			_optionsMenuAction = kOMAExit;
			break;
		case Common::KEYCODE_RETURN:
			if (_vm->isFloppy() || _optionsMenuSelectedItem == 5 || _optionsMenuSelectedItem == 6)
				_optionsMenuAction = kOMAOk;
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (_optionsMenuSelectedItem != kOMANone)
			_optionsMenuAction = _optionsMenuSelectedItem;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_optionsMenuAction = kOMAExit;
		break;
	case Common::EVENT_MOUSEMOVE:
		if (!_vm->isFloppy()) {
			_mouseSliderPos = CLIP<int16>(event.mouse.x, 127, 189) - 127;
			_mouseSelectedItem = _vm->findRect(kOptionsMenuRects, event.mouse.x, event.mouse.y, 13, kOMANone);
			if (_mouseSelectedItem != kOMANone)
				_optionsMenuSelectedItem = _mouseSelectedItem;
		}
		break;
	default:
		break;
	}
}

void GuiOptionsMenu::drawOptionsMenu() {

	const int kItemLeftX = 107;
	const int kItemTopY = 65;
	const int kItemHeight = 20;
	const int kGaugeX = 128;
	const int kGaugeY = 70;

	int selectedItem = _optionsMenuSelectedItem;
	if (!_vm->isFloppy()) {
		if (selectedItem == kOMADefMusicVol)
			selectedItem = kOMAMusicVol;
		else if (selectedItem == kOMADefSoundVol)
			selectedItem = kOMASoundVol;
		else if (selectedItem == kOMADefGameSpeed)
			selectedItem = kOMAGameSpeed;
		else if (selectedItem == kOMATalkie)
			selectedItem = 2;
	}

	drawIcon(25);

	if (!_vm->isFloppy() && selectedItem == 5)
		_vm->_screen->frameRect(kOptionsMenuRects[6].x, kOptionsMenuRects[6].y, kOptionsMenuRects[6].x2, kOptionsMenuRects[6].y2, 119);
	else
		drawIcon(28, kItemLeftX, selectedItem * kItemHeight + kItemTopY);

	drawIcon(27, kGaugeX + _musicVolumeDiv * 4, kGaugeY);
	drawIcon(27, kGaugeX + _digiVolumeDiv * 4, kGaugeY + kItemHeight);
	drawIcon(27, kGaugeX + _gameSpeed * 4, kGaugeY + kItemHeight * 3);
	drawIcon(_language + 32, 129, _vm->isFloppy() ? 177 : 157);

	if (_vm->isFloppy())
		drawIcon(27, kGaugeX + _textSpeed * 30, kGaugeY + kItemHeight * 2);

	if (_musicVolumeDiv == 0) {
		drawIcon(55);
	} else if (_musicVolumeDiv > 0 && _musicVolumeDiv < 8) {
		if (_animFrameCounter < 16) {
			drawIcon(71);
		} else {
			drawIcon(72);
		}
	} else if (_musicVolumeDiv >= 8) {
		if (_animFrameCounter < 16) {
			drawIcon(73);
		} else {
			drawIcon(74);
		}
	}

	if (_digiVolumeDiv == 0)
		drawAnimatedIcon(1, 0, 0, _animFrameCounter);
	else if (_digiVolumeDiv < 7)
		drawAnimatedIcon(2, 0, 0, _animFrameCounter);
	else
		drawAnimatedIcon(3, 0, 0, _animFrameCounter);

	if (_vm->isFloppy()) {
		if ((_textSpeed == 0 && _animFrameCounter > 6) ||
			(_textSpeed == 1 && _animFrameCounter > 16) ||
			(_textSpeed == 2 && _animFrameCounter > 24))
			drawIcon(70);
	} else
		drawIcon(_vm->_talkText->getTalkieMode() + 79);

	if (_gameSpeed < 5)
		drawIcon(75);
	else if (_gameSpeed >= 5 && _gameSpeed < 10)
		drawIcon(76);
	else if (_gameSpeed >= 10)
		drawIcon(77);

}

// GuiTownMap

enum GuiGuiTownMapAction {
	kMANone   = 0,
	kMASelect = 1,
	kMAExit   = 2
};

static const struct MapPoint { int16 x, y; } kMapPoints[] = {
	{248, 126}, {226, 126}, {224, 150}, {204, 156},
	{178, 154}, {176, 138}, {152, 136}, {124, 134},
	{112, 148}, { 96, 132}, { 92, 114}, {146, 116},
	{176, 106}, {138, 100}, {104,  94}, { 82,  96},
	{172,  94}, {116,  80}, {134,  80}, {148,  86},
	{202, 118}, {178, 120}, {190,  92}
};

static const struct MapRect { int16 x1, y1, x2, y2; } kMapRects[] = {
	{240, 116, 264, 134}, {192, 102, 216, 127}, {164,  82, 189,  96},
	{165,  98, 189, 113}, {108, 108, 162, 124}, {140, 128, 166, 144},
	{ 85,  99, 104, 122}, { 84, 124, 106, 142}, {125,  92, 154, 106},
	{104,  70, 128,  87}
};

static const struct MapExit { int16 moduleNumber, sceneNumber; } kMapExits[] = {
	{0,  0}, {0, 20}, {0, 16}, {0, 12}, {0, 11},
	{0,  6}, {0, 10}, {0,  9}, {0, 13}, {0, 17}
};
	
void GuiTownMap::enter() {
	GuiPage::enter();
	
	_vm->_talkText->stopVoice();

	_prevCursorX = -1;
	_prevCursorY = -1;
	_locationNumber = _vm->_sceneNumber % 30;
	_currMapLocation = -1;
	_selectedMapLocation = -1;
	_mapStatus = 0;
	_mapAction = kMANone;

	if (_vm->isFloppy()) {
		_mapRectX1 = 82;
		_mapRectX2 = 268;
		_mapRectY1 = 68;
		_mapRectY2 = 186;
		_sceneBitMaskStatus = _vm->_scriptVars[2];
		_sceneStatus1 = _vm->_scriptVars[89];
		_sceneStatus2 = _vm->_scriptVars[114];
		_sceneStatus3 = _vm->_scriptVars[61];
		_sceneStatus4 = _vm->_scriptVars[92];
	} else {
		_mapRectX1 = 65;
		_mapRectX2 = 268;
		_mapRectY1 = 66;
		_mapRectY2 = 186;
		_sceneBitMaskStatus = _vm->_scriptVars[2];
		_sceneStatus1 = _vm->_scriptVars[3];
		_sceneStatus2 = _vm->_scriptVars[4];
		_sceneStatus3 = _vm->_scriptVars[5];
		_sceneStatus4 = _vm->_scriptVars[6];
	}

	_cursorX = kMapPoints[_locationNumber].x;
	_cursorY = kMapPoints[_locationNumber].y;

	if (!_vm->isFloppy()) {
		CursorMan.showMouse(false);
		_vm->_system->warpMouse(_cursorX, _cursorY);
	}	

}

void GuiTownMap::leave() {
	if (!_vm->isFloppy()) {
		CursorMan.showMouse(true);
	}
	GuiPage::leave();
}

void GuiTownMap::update() {
	bool cursorChanged = false;

	cursorChanged = _prevCursorX != _cursorX || _prevCursorY != _cursorY;
	_prevCursorX = _cursorX;
	_prevCursorY = _cursorY;

#if 0
	if (!_vm->isFloppy() && (_vm->_input->getMouseX() != _cursorX || _vm->_input->getMouseY() != _cursorY))
		_vm->_system->warpMouse(_cursorX, _cursorY);
#endif

	if (cursorChanged) {
		_currMapLocation = -1;
		_selectedMapLocation = -1;
		drawIcon(50);
		for (int16 mapLocation = 0; mapLocation < 10; mapLocation++) {
			const MapRect &mapRect = kMapRects[mapLocation];
			if ((_sceneBitMaskStatus & (1 << mapLocation)) && 
				_cursorX >= mapRect.x1 && _cursorX <= mapRect.x2 && 
				_cursorY >= mapRect.y1 && _cursorY <= mapRect.y2) {
				_currMapLocation = mapLocation;
				break;
			}
		}
		if (_currMapLocation != -1) {
			byte *locationName = _vm->_textReader->getString(2, 40 + _currMapLocation);
			_vm->_screen->drawTextOutlined(MIN(_cursorX - 2, 283 - _vm->_screen->getTextWidth(locationName)), 
				_cursorY - 6, locationName, 119, 120);
		} else {
			drawIcon(51, _cursorX, _cursorY);
		}
	}

	switch (_mapAction) {
	case kMANone:
		break;
	case kMASelect:
		// TODO Extract to method
		if (_currMapLocation != -1) {
			_selectedMapLocation = _currMapLocation;
			const MapExit &mapExit = kMapExits[_selectedMapLocation];
			_vm->_moduleNumber = mapExit.moduleNumber;
			_vm->_sceneNumber = mapExit.sceneNumber;
			if (_sceneStatus1 == 1) {
				_vm->_moduleNumber += 6;
			} else if (_sceneStatus2 >= 2) {
				_vm->_sceneNumber += (_sceneStatus2 - 1) * 30;
			}
			if ((_locationNumber == 7 || _locationNumber == 8) &&
				_sceneStatus3 == 2 && _sceneStatus4 == 0 &&
				_selectedMapLocation != 6 && _selectedMapLocation != 7 && _selectedMapLocation != 4) {
				_vm->_sceneNumber = 36;
			}
			_mapStatus = 2;
		}
		break;
	case kMAExit:
		_mapStatus = 1;
		_currMapLocation = -1;
		break;
	}
	
	_vm->_screen->update();
	
	// TODO return 1;

}

void GuiTownMap::handleEvent(Common::Event &event) {
	const int16 kCursorAddX = 8, kCursorAddY = 8;

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		// TODO Movement should be pulsing not one-shot
		case Common::KEYCODE_UP:
			_cursorY = MAX(_cursorY - kCursorAddY, _mapRectY1);
			break;
		case Common::KEYCODE_DOWN:
			_cursorY = MIN(_cursorY + kCursorAddY, _mapRectY2);
			break;
		case Common::KEYCODE_LEFT:
			_cursorX = MAX(_cursorX - kCursorAddX, _mapRectX1);
			break;
		case Common::KEYCODE_RIGHT:
			_cursorX = MIN(_cursorX + kCursorAddX, _mapRectX2);
			break;
		case Common::KEYCODE_ESCAPE:
			_mapAction = kMAExit;
			break;
		case Common::KEYCODE_RETURN:
			_mapAction = kMASelect;
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_MOUSEMOVE:
		if (!_vm->isFloppy()) {
			_cursorX = CLIP<int16>(event.mouse.x, _mapRectX1, _mapRectX2);
			_cursorY = CLIP<int16>(event.mouse.y, _mapRectY1, _mapRectY2);
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		_mapAction = kMASelect;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_mapAction = kMAExit;
		break;
	default:
		break;
	}
}

// GuiJournal

enum GuiJournalAction {
	kBANone     = -1,
	kBAExit     = -2,
	kBAPrevPage = 0,
	kBANextPage = 1
};

static const GuiRectangle kBookRects[] = {
	{ 54, 46, 166, 189, kBAPrevPage},
	{167, 46, 279, 189, kBANextPage}
};

void GuiJournal::enter() {
	GuiPage::enter();
	_currPageNumber = -1;
	_talkPageNumber = -1;
	// Use values from script; this is the most recent diary entry
	_pageNumber = _vm->_scriptVars[1];
	_pageCount = 5;//_vm->_scriptVars[1];// TODO DEBUG
	_bookStatus = 0;
	_bookAction = kBANone;
	_mouseSelectedRect = -1;
	_vm->setMouseCursor(-1);
	_firstUpdate = true;
	// Set speech file
	_vm->_talkText->setVoiceFileIndex(7); // TODO Make this _talkText->enterJournal() for symmetry
}

void GuiJournal::leave() {
	_vm->_talkText->leaveJournal();
	// TODO return 2 - _bookStatus;
	GuiPage::leave();
}

void GuiJournal::update() {
	TASK_BODY_BEGIN

	if (_firstUpdate) {
		_firstUpdate = false;
		TASK_AWAIT(bookTurnPageTextFadeInEffect);
	}

	if (_currPageNumber != _pageNumber) {
		drawBookPage(64);
		_currPageNumber = _pageNumber;
	}

	// Play page speech
	if (_talkPageNumber != _pageNumber) {
		if (_pageNumber > 0)
			_vm->_talkText->playVoice(_pageNumber);
		else
			_vm->_talkText->stopVoice();
		_talkPageNumber = _pageNumber;
	}

	switch (_bookAction) {
	case kBANone:
		break;
	case kBAExit:
		_bookStatus = 2;
		break;
	}
	
	if (_bookAction == kBAPrevPage && _pageNumber > 0) {
		TASK_AWAIT(bookTurnPageTextFadeOutEffect);
		TASK_AWAIT(bookTurnPagePrev);
		--_pageNumber;
		TASK_AWAIT(bookTurnPageTextFadeInEffect);
	} else if (_bookAction == kBANextPage && _pageNumber < _pageCount) {
		TASK_AWAIT(bookTurnPageTextFadeOutEffect);
		TASK_AWAIT(bookTurnPageNext);
		++_pageNumber;
		TASK_AWAIT(bookTurnPageTextFadeInEffect);
	}

	_bookAction = kBANone;
	
	_vm->_screen->update();

	TASK_BODY_END
}

void GuiJournal::handleEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_RETURN:
			_bookAction = kBAExit;
			break;
		case Common::KEYCODE_LEFT:
			_bookAction = kBAPrevPage;
			break;
		case Common::KEYCODE_RIGHT:
			_bookAction = kBANextPage;
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_MOUSEMOVE:
		if (!_vm->isFloppy()) {
			_mouseSelectedRect = _vm->findRect(kBookRects, event.mouse.x, event.mouse.y, 2, kBANone);
			if (_mouseSelectedRect == -1)
				_vm->setMouseCursor(-1);
			else if (_mouseSelectedRect == 0)
				_vm->setMouseCursor(3);
			else if (_mouseSelectedRect == 1)
				_vm->setMouseCursor(2);
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (_mouseSelectedRect == 0)
			_bookAction = kBAPrevPage;
		else if (_mouseSelectedRect == 1)
			_bookAction = kBANextPage;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_bookAction = kBAExit;
		break;
	default:
		break;
	}
}

void GuiJournal::drawBookPage(byte fontColor) {
	int xadd = 58, yadd = 48, x = 0, lineNumber = 0;
	Common::String pageNumberString;
	int pageNumberStringWidth;

	byte *pageText = _vm->_textReader->getString(2, _pageNumber);

	drawIcon(30);
	if (_pageNumber < _pageCount)
		drawIcon(37);

	_vm->_screen->setFontColor(58);

	pageNumberString = Common::String::format("- %d -", _pageNumber * 2 + 1);
	pageNumberStringWidth = _vm->_screen->getTextWidth((const byte*)pageNumberString.c_str());
	_vm->_screen->drawText(xadd + (106 - pageNumberStringWidth) / 2, 180, (const byte*)pageNumberString.c_str());

	pageNumberString = Common::String::format("- %d -", _pageNumber * 2 + 2);
	pageNumberStringWidth = _vm->_screen->getTextWidth((const byte*)pageNumberString.c_str());
	_vm->_screen->drawText(xadd + 115 + (106 - pageNumberStringWidth) / 2, 180, (const byte*)pageNumberString.c_str());

	_vm->_screen->setFontColor(fontColor);

	while (*pageText != 0 && *pageText != '*') {
		x = MAX(xadd + (106 - _vm->_screen->getTextWidth(pageText)) / 2, 0);
		_vm->_screen->drawText(x, yadd + lineNumber * 10, pageText);
		if (++lineNumber == 13) {
			xadd += 115;
			yadd -= 130;
		}
		while (*pageText != 0 && *pageText != '*')
			pageText++;
		pageText++;
	}
}

void GuiJournal::bookTurnPageNext() {
	const uint first = 38;
	const uint last = 49;
	const int incr = 1;
	static uint i = first;
	FUNC_BODY_BEGIN
	for (i = first; i != last; i += incr) {
		drawIcon(30);
		drawIcon(i);
		_vm->_screen->update();
		FUNC_YIELD
	}
	FUNC_BODY_END
}

void GuiJournal::bookTurnPagePrev() {
	const uint first = 49;
	const uint last = 38;
	const int incr = -1;
	static uint i = first;
	FUNC_BODY_BEGIN
	for (i = first; i != last; i += incr) {
		drawIcon(30);
		drawIcon(i);
		_vm->_screen->update();
		FUNC_YIELD
	}
	FUNC_BODY_END
}

void GuiJournal::bookTurnPageTextFadeInEffect() {
	const byte firstColor = 72;
	const byte lastColor = 64;
	const int incr = -1;
	static byte fontColor = firstColor;
	FUNC_BODY_BEGIN
	for (fontColor = firstColor; fontColor != lastColor; fontColor += incr) {
		drawBookPage(fontColor);
		_vm->_screen->update();
		FUNC_YIELD
	}
	FUNC_BODY_END
}

void GuiJournal::bookTurnPageTextFadeOutEffect() {
	const byte firstColor = 64;
	const byte lastColor = 72;
	const int incr = 1;
	static byte fontColor = firstColor;
	FUNC_BODY_BEGIN
	for (fontColor = firstColor; fontColor != lastColor; fontColor += incr) {
		drawBookPage(fontColor);
		_vm->_screen->update();
		FUNC_YIELD
	}
	FUNC_BODY_END
}

// GuiPuzzle

static const GuiRectangle kPuzzleTileRects[] = {
	{118,  44, 142,  59,  0},
	{143,  44, 167,  59,  1},
	{168,  44, 192,  59,  2},
	{193,  44, 217,  59,  3},
	{217,  59, 231,  83,  4},
	{217,  84, 231, 108,  5},
	{217, 109, 231, 133,  6},
	{217, 134, 231, 158,  7},
	{118, 158, 142, 171,  8},
	{143, 158, 167, 171,  9},
	{168, 158, 192, 171, 10},
	{193, 158, 217, 171, 11},
	{103,  59, 118,  83, 12},
	{103,  84, 118, 108, 13},
	{103, 109, 118, 133, 14},
	{103, 134, 118, 158, 15},
	{103,  44, 118,  59, 16},
	{217,  44, 231,  59, 17},
	{217, 158, 231, 171, 18},
	{103, 158, 118, 171, 19},
	{119,  60, 216, 157, 20}
};

void GuiPuzzle::enter() {
	GuiPage::enter();
	_puzzleStatus = 0;
	_puzzleCursorX = 0;
	_puzzleCursorY = 0;
	_puzzleSprite = _vm->_animationMan->loadAnimationResource("A07.PAK", 24);
	_isTileMoving = false;
	initPuzzle();
	_fingerBackground = NULL;	
	_puzzleTableRow = 0;
	_puzzleTableColumn = 0;
	loadFingerCursor();
}

void GuiPuzzle::leave() {
	if (_fingerBackground)
		_fingerBackground->free();
	delete _fingerBackground;
	delete _puzzleSprite;
	// TODO return _puzzleStatus == 2 ? 2 : 0;
	GuiPage::leave();
}

void GuiPuzzle::update() {
	TASK_BODY_BEGIN

	drawField();

	if (_isTileMoving) {
		if (_puzzleTableColumn == 0 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
			playTileMoveSound();
			TASK_AWAIT(moveTileRowLeft);
		} else if (_puzzleTableColumn == 5 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
			playTileMoveSound();
			TASK_AWAIT(moveTileRowRight);
		} else if (_puzzleTableRow == 0 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
			playTileMoveSound();
			TASK_AWAIT(moveTileColumnUp);
		} else if (_puzzleTableRow == 5 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
			playTileMoveSound();
			TASK_AWAIT(moveTileColumnDown);
		}
		if (isPuzzleSolved())
			_puzzleStatus = 2;
		_isTileMoving = false;
	}
	
	_vm->_screen->update();

	TASK_BODY_END
}

void GuiPuzzle::handleEvent(Common::Event &event) {
	if (_isTileMoving)
		return;
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
			_puzzleStatus = 1;
			break;
		case Common::KEYCODE_RETURN:
			_isTileMoving = true;
			break;
		case Common::KEYCODE_UP:
			if (_puzzleTableRow > 0) {
				--_puzzleTableRow;
				updateCurrentTile();
			}
			break;
		case Common::KEYCODE_DOWN:
			if (_puzzleTableRow < 5) {
				++_puzzleTableRow;
				updateCurrentTile();
			}
			break;
		case Common::KEYCODE_LEFT:
			if (_puzzleTableColumn > 0) {
				--_puzzleTableColumn;
				updateCurrentTile();
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (_puzzleTableColumn < 5) {
				++_puzzleTableColumn;
				updateCurrentTile();
			}
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_MOUSEMOVE:
		if (!_vm->isFloppy()) {
			selectTileByMouse(event.mouse.x, event.mouse.y);
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (!_vm->isFloppy()) {
			selectTileByMouse(event.mouse.x, event.mouse.y);
			_isTileMoving = true;
		}
		break;
	case Common::EVENT_RBUTTONDOWN:
		_puzzleStatus = 1;
		break;
	default:
		break;
	}
}

void GuiPuzzle::initPuzzle() {
	static const uint16 kPuzzleCheatInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4, 8,13, 0},
		{0, 1, 5, 9,14, 0},
		{0, 2, 6,10,15, 0},
		{0, 3, 7,11,12, 0},
		{0, 0, 0, 0, 0, 0}};
	
	static const uint16 kPuzzleInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4,10,15, 0},
		{0, 5,11,13,12, 0},
		{0, 8, 1, 2, 6, 0},
		{0, 3, 7, 9,14, 0},
		{0, 0, 0, 0, 0, 0}};

	// Initialize the puzzle state
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if (debugPuzzleCheat)
				_puzzleTiles[i][j] = kPuzzleCheatInitialTiles[i][j];
			else
				_puzzleTiles[i][j] = kPuzzleInitialTiles[i][j];
		}
	}
}

void GuiPuzzle::loadFingerCursor() {
	if (!_vm->isFloppy()) {
		AnimationCel *cel = _puzzleSprite->getCelByElementCommand(18, 0);
		_vm->setMouseCursorSprite(cel);
	}
}

void GuiPuzzle::drawFinger() {
	if (_vm->isFloppy()) {
		int16 fingerX = 108 + _puzzleTableColumn * 24;
		int16 fingerY = 48 + _puzzleTableRow * 24;
		if (!_fingerBackground) {
			AnimationCel *cel = _puzzleSprite->getCelByElementCommand(18, 0);
			_fingerBackground = new Graphics::Surface();
			_fingerBackground->create(cel->getWidth(), cel->getHeight(), Graphics::PixelFormat::createFormatCLUT8());
			_prevFingerX = 0;
			_prevFingerY = 0;
		} else if ((_prevFingerX == 228 || _prevFingerY == 168) && (fingerX != _prevFingerX || fingerY != _prevFingerY))
			_vm->_screen->putRect(_fingerBackground, _prevFingerX, _prevFingerY);
		if ((fingerX == 228 || fingerY == 168) && (fingerX != _prevFingerX || fingerY != _prevFingerY))
			_vm->_screen->grabRect(_fingerBackground, fingerX, fingerY);
		_vm->_screen->drawAnimationElement(_puzzleSprite, 18, fingerX, fingerY);
		_prevFingerX = fingerX; 
		_prevFingerY = fingerY;
	}
}

void GuiPuzzle::drawField() {
	_vm->_screen->drawAnimationElement(_puzzleSprite, 17, 0, 0);
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++)
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++)
			drawTile(columnIndex, rowIndex, 0, 0);		
	drawFinger();
}

void GuiPuzzle::drawTile(int columnIndex, int rowIndex, int xOffs, int yOffs) {
	_vm->_screen->drawAnimationElement(_puzzleSprite, _puzzleTiles[columnIndex][rowIndex], 
		119 + (columnIndex - 1) * 24 + xOffs, 60 + (rowIndex - 1) * 24 + yOffs);
}

void GuiPuzzle::updateCurrentTile() {
	/* Comet CD: If the tile selection has been changed by cursor keys,
	   position the mouse at the correct tile. */
	if (!_vm->isFloppy()) {
		int mouseNewTile = 20;
		if (_puzzleTableColumn == 0 && _puzzleTableRow == 0)
			mouseNewTile = 16;
		else if (_puzzleTableColumn == 5 && _puzzleTableRow == 0)
			mouseNewTile = 17;
		else if (_puzzleTableColumn == 5 && _puzzleTableRow == 5)
			mouseNewTile = 18;
		else if (_puzzleTableColumn == 0 && _puzzleTableRow == 5)
			mouseNewTile = 19;
		else if (_puzzleTableColumn > 0 && _puzzleTableColumn < 5 && _puzzleTableRow == 0)
			mouseNewTile = _puzzleTableColumn - 1;
		else if (_puzzleTableColumn > 0 && _puzzleTableColumn < 5 && _puzzleTableRow == 5)
			mouseNewTile = _puzzleTableColumn + 7;
		else if (_puzzleTableRow > 0 && _puzzleTableRow < 5 && _puzzleTableColumn == 0)
			mouseNewTile = _puzzleTableRow + 11;
		else if (_puzzleTableRow > 0 && _puzzleTableRow < 5 && _puzzleTableColumn == 5)
			mouseNewTile = _puzzleTableRow + 3;
		if (mouseNewTile != 20) {
			_puzzleCursorX = (kPuzzleTileRects[mouseNewTile].x + kPuzzleTileRects[mouseNewTile].x2) / 2;
			_puzzleCursorY = (kPuzzleTileRects[mouseNewTile].y + kPuzzleTileRects[mouseNewTile].y2) / 2;
		} else {
			_puzzleCursorX = (_puzzleTableColumn - 1) * 24 + 130;
			_puzzleCursorY = (_puzzleTableRow - 1) * 24 + 71;
		}
		_vm->_system->warpMouse(_puzzleCursorX, _puzzleCursorY);
	}
}

void GuiPuzzle::selectTileByMouse(int mouseX, int mouseY) {
	static const struct { int col, row; } kRectToColRow[] = {
		{1, 0}, {2, 0}, {3, 0}, {4, 0}, 
		{5, 1}, {5, 2}, {5, 3}, {5, 4}, 
		{1, 5}, {2, 5}, {3, 5}, {4, 5}, 
		{0, 1}, {0, 2}, {0, 3}, {0, 4}, 
		{0, 0}, {5, 0}, {5, 5}, {0, 5} 
	};

	/* Comet CD: Find out which tile is selected and convert 
	   mouse coords to tile coords.
	*/
	int mouseSelectedTile;
	_puzzleCursorX = CLIP(mouseX, 103, 231);
	_puzzleCursorY = CLIP(mouseY, 44, 171);
	if (mouseX != _puzzleCursorX || mouseY != _puzzleCursorY)
		_vm->_system->warpMouse(_puzzleCursorX, _puzzleCursorY);
	mouseSelectedTile = _vm->findRect(kPuzzleTileRects, mouseX, mouseY, 21, -1);
	if (mouseSelectedTile >= 0) {
		if (mouseSelectedTile >= 0 && mouseSelectedTile < 20) {
			_puzzleTableColumn = kRectToColRow[mouseSelectedTile].col;
			_puzzleTableRow = kRectToColRow[mouseSelectedTile].row;
		} else if (mouseSelectedTile == 20) {
			_puzzleTableColumn = (_puzzleCursorX - 119) / 24 + 1;
			_puzzleTableRow = (_puzzleCursorY - 60) / 24 + 1; 
		} else {
			_puzzleTableColumn = 0;
			_puzzleTableRow = 0;
		}
	}
}

void GuiPuzzle::moveTileColumnUp() {
	static int yOffs = 0;
	FUNC_BODY_BEGIN
	_puzzleTiles[_puzzleTableColumn][5] = _puzzleTiles[_puzzleTableColumn][1];
	for (yOffs = 0; yOffs < 24; yOffs += 2) {
		_vm->_screen->setClipY(60, 156);
		for (int rowIndex = 1; rowIndex <= 5; rowIndex++)
			drawTile(_puzzleTableColumn, rowIndex, 0, -yOffs);
		_vm->_screen->setClipY(0, 199);
		drawFinger();
		_vm->_screen->update();
		FUNC_YIELD
	}
	for (int rowIndex = 0; rowIndex <= 4; rowIndex++)
		_puzzleTiles[_puzzleTableColumn][rowIndex] = _puzzleTiles[_puzzleTableColumn][rowIndex + 1];
	_puzzleTiles[_puzzleTableColumn][5] = _puzzleTiles[_puzzleTableColumn][1];
	FUNC_BODY_END
}

void GuiPuzzle::moveTileColumnDown() {
	static int yOffs = 0;
	FUNC_BODY_BEGIN
	_puzzleTiles[_puzzleTableColumn][0] = _puzzleTiles[_puzzleTableColumn][4];
	for (yOffs = 0; yOffs < 24; yOffs += 2) {
		_vm->_screen->setClipY(60, 156);
		for (int rowIndex = 0; rowIndex <= 4; rowIndex++)
			drawTile(_puzzleTableColumn, rowIndex, 0, yOffs);
		_vm->_screen->setClipY(0, 199);
		drawFinger();
		_vm->_screen->update();
		FUNC_YIELD
	}
	for (int rowIndex = 5; rowIndex >= 1; rowIndex--)
		_puzzleTiles[_puzzleTableColumn][rowIndex] = _puzzleTiles[_puzzleTableColumn][rowIndex - 1];
	_puzzleTiles[_puzzleTableColumn][0] = _puzzleTiles[_puzzleTableColumn][4];
	FUNC_BODY_END
}

void GuiPuzzle::moveTileRowLeft() {
	static int xOffs = 0;
	FUNC_BODY_BEGIN
	_puzzleTiles[5][_puzzleTableRow] = _puzzleTiles[1][_puzzleTableRow];
	for (xOffs = 0; xOffs < 24; xOffs += 2) {
		_vm->_screen->setClipX(120, 215);
		for (int columnIndex = 1; columnIndex <= 5; columnIndex++)
			drawTile(columnIndex, _puzzleTableRow, -xOffs, 0);
		_vm->_screen->setClipX(0, 319);
		drawFinger();
		_vm->_screen->update();
		FUNC_YIELD
	}
	for (int columnIndex = 0; columnIndex <= 4; columnIndex++)
		_puzzleTiles[columnIndex][_puzzleTableRow] = _puzzleTiles[columnIndex + 1][_puzzleTableRow];
	_puzzleTiles[5][_puzzleTableRow] = _puzzleTiles[1][_puzzleTableRow];
	FUNC_BODY_END
}

void GuiPuzzle::moveTileRowRight() {
	static int xOffs = 0;
	FUNC_BODY_BEGIN
	_puzzleTiles[0][_puzzleTableRow] = _puzzleTiles[4][_puzzleTableRow];
	for (xOffs = 0; xOffs < 24; xOffs += 2) {
		_vm->_screen->setClipX(120, 215);
		for (int columnIndex = 0; columnIndex <= 4; columnIndex++)
			drawTile(columnIndex, _puzzleTableRow, xOffs, 0);
		_vm->_screen->setClipX(0, 319);
		drawFinger();
		_vm->_screen->update();
		FUNC_YIELD
	}
	for (int columnIndex = 5; columnIndex >= 1; columnIndex--)
		_puzzleTiles[columnIndex][_puzzleTableRow] = _puzzleTiles[columnIndex - 1][_puzzleTableRow];
	_puzzleTiles[0][_puzzleTableRow] = _puzzleTiles[4][_puzzleTableRow];
	FUNC_BODY_END
}

bool GuiPuzzle::isPuzzleSolved() {
	int matchingTiles = 0;
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++)
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++)
			if (_puzzleTiles[columnIndex][rowIndex] == (rowIndex - 1) * 4 + (columnIndex - 1))
				matchingTiles++;
	return matchingTiles == 16;
}

void GuiPuzzle::playTileMoveSound() {
	if (!_vm->isFloppy()) {
		_vm->playSample(75, 1);
	}
}

#if 0
// GuiSaveLoadMenu

int GuiSaveLoadMenu::run() {

	static const GuiRectangle saveLoadMenuRects[] = {
		{93,  62, 232,  73, 0},
		{93,  74, 232,  85, 1},
		{93,  86, 232,  97, 2},
		{93,  98, 232, 109, 3},
		{93, 110, 232, 121, 4},
		{93, 122, 232, 133, 5},
		{93, 134, 232, 145, 6},
		{93, 146, 232, 157, 7},
		{93, 158, 232, 169, 8},
		{93, 170, 232, 181, 9}};

	int saveLoadMenuStatus = 0, selectedItem = 0;

	loadSavegamesList();

	_vm->_input->waitForKeys();

	while (saveLoadMenuStatus == 0 && !_vm->shouldQuit()) {
		int mouseSelectedItem;

		mouseSelectedItem = _vm->findRect(saveLoadMenuRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), 10, -1);
		if (mouseSelectedItem != -1)
			selectedItem = mouseSelectedItem;

		drawSaveLoadMenu(selectedItem);
		
		_vm->syncUpdate();

		_vm->_input->handleEvents();

		if (mouseSelectedItem != -1 && _vm->_input->leftButton()) {
			saveLoadMenuStatus = 1;
		} else if (_vm->_input->rightButton()) {
			saveLoadMenuStatus = 2;
		}

		switch (_vm->_input->getKeyCode()) {
		case Common::KEYCODE_RETURN:
			saveLoadMenuStatus = 1;
			break;
		case Common::KEYCODE_ESCAPE:
			saveLoadMenuStatus = 2;
			break;
		case Common::KEYCODE_DOWN:
			if (selectedItem < 9)
				selectedItem++;
			else
				selectedItem = 0;
			break;
		case Common::KEYCODE_UP:
			if (selectedItem > 0)
				selectedItem--;
			else
				selectedItem = 9;
			break;
		default:
			if (_asSaveMenu && _vm->_input->getKeyCode() >= Common::KEYCODE_SPACE && _vm->_input->getKeyCode() <= Common::KEYCODE_z) {
				if (handleEditSavegameDescription(selectedItem) == 1)
					saveLoadMenuStatus = 1;
			}
			break;
		}

		// When loading only allow to select entries which are actually occupied...
		if (saveLoadMenuStatus == 1 && !_asSaveMenu && _savegames[selectedItem].description.size() == 0)
			saveLoadMenuStatus = 0;

		_vm->_input->waitForKeys();

	}

	if (saveLoadMenuStatus == 1) {
		if (_asSaveMenu) {
			debug("Save: %d", selectedItem);
			if (_savegames[selectedItem].filename.size() == 0)
				_savegames[selectedItem].filename = Common::String::format("%s.%03d", _vm->getTargetName().c_str(), selectedItem);
			_vm->savegame(_savegames[selectedItem].filename.c_str(), _savegames[selectedItem].description.c_str());
		} else {
			debug("Load: %d", selectedItem);
			_vm->loadgame(_savegames[selectedItem].filename.c_str());
		}
	}

	return 2 - saveLoadMenuStatus;
}

void GuiSaveLoadMenu::draw() {
}

void GuiSaveLoadMenu::drawSaveLoadMenu(int selectedItem) {
	const int x = 95;
	const int y = 64;
	const int kItemHeight = 12;
	drawIcon(_asSaveMenu ? 14 : 15);
	for (int itemIndex = 0; itemIndex < 10; itemIndex++)
		drawSavegameDescription(_savegames[itemIndex].description, itemIndex);
	_vm->_screen->frameRect(x - 2, y + selectedItem * kItemHeight - 2, x + 138, y + selectedItem * kItemHeight + 9, 119);
}
	
void GuiSaveLoadMenu::loadSavegamesList() {
	debug("GuiSaveLoadMenu::loadSavegamesList()");

	for (int i = 0; i < 10; i++) {
		_savegames[i].description.clear();
		_savegames[i].filename.clear();
	}

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Comet::CometEngine::SaveHeader header;
	Common::String pattern = _vm->getTargetName();
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end()); // Sort (hopefully ensuring we are sorted numerically..)

	int savegameCount = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end() && savegameCount < 10; file++) {
		Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
		if (in) {
			if (_vm->readSaveHeader(in, false, header) == Comet::CometEngine::kRSHENoError) {
				_savegames[savegameCount].description = header.description;
				_savegames[savegameCount].filename = *file;
				savegameCount++;
			}
			delete in;
		}
	}
}

int GuiSaveLoadMenu::handleEditSavegameDescription(int savegameIndex) {
	int editSavegameDescriptionStatus = 0;
	Common::String description = _savegames[savegameIndex].description;
	bool redrawSavegameDescription = true;

	while (editSavegameDescriptionStatus == 0 && !_vm->shouldQuit()) {
		Common::Event event;

		if (redrawSavegameDescription) {
			_vm->_screen->fillRect(94, 63 + savegameIndex * 12, 231, 71 + savegameIndex * 12, 19);
			drawSavegameDescription(description, savegameIndex);
			redrawSavegameDescription = false;
		}

		_vm->syncUpdate();

		if (_vm->_input->leftButton())
			editSavegameDescriptionStatus = 1;
		else if (_vm->_input->rightButton())
			editSavegameDescriptionStatus = 2;

		// Local event polling since handleEvents seems to drop characters for some reason...	
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					editSavegameDescriptionStatus = 2;
					break;
				case Common::KEYCODE_RETURN:
					editSavegameDescriptionStatus = 1;
					break;
				case Common::KEYCODE_BACKSPACE:
					description.deleteLastChar();
					redrawSavegameDescription = true;
					break;
				default:
					if (event.kbd.keycode >= Common::KEYCODE_SPACE && event.kbd.keycode <= Common::KEYCODE_z &&
						description.size() < 29 && _vm->_screen->getTextWidth((const byte*)description.c_str()) < 130) {
						description += event.kbd.ascii;
						redrawSavegameDescription = true;
					}
					break;
				}
			default:
				_vm->_input->handleMouseEvent(event);
				break;
			}
		}
	}

	if (editSavegameDescriptionStatus == 1)
		_savegames[savegameIndex].description = description;

	return editSavegameDescriptionStatus;
}

void GuiSaveLoadMenu::drawSavegameDescription(Common::String &description, int savegameIndex) {
	const int x = 95;
	const int y = 64;
	const int kItemHeight = 12;
	int textX = (135 - _vm->_screen->getTextWidth((const byte*)description.c_str())) / 2;
	_vm->_screen->setFontColor(120);
	_vm->_screen->drawText(x + textX + 1, y + savegameIndex * kItemHeight + 1, (const byte*)description.c_str());
	_vm->_screen->setFontColor(119);
	_vm->_screen->drawText(x + textX, y + savegameIndex * kItemHeight, (const byte*)description.c_str());
}
#endif
} // End of namespace Comet
