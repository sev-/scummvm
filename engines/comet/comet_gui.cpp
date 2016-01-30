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

Gui::Gui(CometEngine *vm) : _vm(vm) {
	_guiInventory = new GuiInventory(_vm);
	_guiCommandBar = new GuiCommandBar(_vm);
	_guiJournal = new GuiJournal(_vm);
	_guiTownMap = new GuiTownMap(_vm);
	_guiMainMenu = new GuiMainMenu(_vm);
	_guiOptionsMenu = new GuiOptionsMenu(_vm);
	_guiPuzzle = new GuiPuzzle(_vm);
	_guiSaveLoadMenu = new GuiSaveLoadMenu(_vm);
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
	delete _guiSaveLoadMenu;
	delete[] _gameScreen;
}

int Gui::run(GuiPageIdent page) {
	int result;
	if (_currPage)
		_stack.push_back(_currPage);
	else
		_vm->_screen->copyToScreen(_gameScreen);
	switch (page) {
	case kGuiInventory:
		_currPage = _guiInventory;
		break;
	case kGuiCommandBar:
		_currPage = _guiCommandBar;
		break;
	case kGuiJournal:
		_currPage = _guiJournal;
		break;
	case kGuiTownMap:
		_currPage = _guiTownMap;
		break;
	case kGuiMainMenu:
		_currPage = _guiMainMenu;
		break;
	case kGuiOptionsMenu:
		_currPage = _guiOptionsMenu;
		break;
	case kGuiPuzzle:
		_currPage = _guiPuzzle;
		break;
	case kGuiSaveMenu:
	case kGuiLoadMenu:
		_guiSaveLoadMenu->setAsSaveMenu(page == kGuiSaveMenu);
		_currPage = _guiSaveLoadMenu;
		break;
	}
	result = _currPage->run();
	if (_stack.size() > 0) {
		_currPage = _stack.back();
		_stack.pop_back();
		_vm->_screen->copyFromScreen(_gameScreen);
		for (Common::Array<GuiPage*>::iterator it = _stack.begin(); it != _stack.end(); it++)
			(*it)->draw();
	} else
		_currPage = NULL;
	return result;
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

int GuiInventory::run() {
	const int kIANone		= -1;
	const int kIAUp			= -2;
	const int kIADown		= -3;
	const int kIAUse		= -4;
	const int kIASelect		= -5;
	const int kIAExit		= -6;
	const uint kMaxItemsOnScreen = 10;

	static const GuiRectangle inventorySlotRects[] = {
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

	Common::Array<uint16> items;
	uint firstItem = 0, currentItem = 0, animFrameCounter = 0;
	int inventoryStatus = 0;

	_vm->_input->waitForKeys();
	_vm->setMouseCursor(-1);

	// Build items array and set up variables
	_vm->_inventory.buildItems(items, firstItem, currentItem);

	while (inventoryStatus == 0 && !_vm->shouldQuit()) {
		int inventoryAction = kIANone, mouseSelectedItem;
		bool doWarpMouse = false;

		_vm->_input->handleEvents();

		mouseSelectedItem = _vm->findRect(inventorySlotRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), MIN<int>(items.size() - firstItem, 10) + 2, kIANone);
		if (mouseSelectedItem >= 0)
			currentItem = firstItem + mouseSelectedItem;

		drawInventory(items, firstItem, currentItem, animFrameCounter++);
		_vm->syncUpdate();

		if (_vm->_input->rightButton()) {
			inventoryAction = kIAExit;
		} else if (_vm->_input->leftButton()) {
			if (mouseSelectedItem >= 0)
				inventoryAction = kIAUse;
			else if (mouseSelectedItem != kIANone)
				inventoryAction = mouseSelectedItem;
		}

		switch (_vm->_input->getKeyCode()) {
		case Common::KEYCODE_DOWN:
			inventoryAction = kIADown;
			break;
		case Common::KEYCODE_UP:
			inventoryAction = kIAUp;
			break;
		case Common::KEYCODE_ESCAPE:
			inventoryAction = kIAExit;
			break;
		case Common::KEYCODE_RETURN:
			inventoryAction = kIASelect;
			break;
		case Common::KEYCODE_u:
			inventoryAction = kIAUse;
			break;
		default:
			break;
		}

		switch (inventoryAction) {
		case kIANone:
			break;
		case kIADown:
			if ((currentItem - firstItem + 1 < kMaxItemsOnScreen) && currentItem + 1 < items.size()) {
				doWarpMouse = mouseSelectedItem == (currentItem - firstItem);
				currentItem++;
			} else if (firstItem + kMaxItemsOnScreen < items.size()) {
				firstItem++;
				currentItem++;
			}
			break;
		case kIAUp:
			if (currentItem > firstItem) {
				doWarpMouse = mouseSelectedItem == (currentItem - firstItem);
				currentItem--;
			} else if (firstItem > 0) {
				firstItem--;
				currentItem--;
			}
			break;
		case kIAExit:
			inventoryStatus = 2;
			break;
		case kIASelect:
		case kIAUse:
			_vm->_inventory.resetStatus();
			_vm->_inventory.selectItem(items[currentItem]);
			// Return just selects, U actually uses the item
			if (inventoryAction == kIAUse)
				_vm->_inventory.requestUseSelectedItem();
			inventoryStatus = 1;
			break;
		default:
			break;
		}
		
		if (doWarpMouse) {
			_vm->warpMouseToRect(inventorySlotRects[currentItem - firstItem + 2]);
			doWarpMouse = false;
		}

		_vm->_input->waitForKeys();
	}

	return 2 - inventoryStatus;
}

void GuiInventory::drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter) {
	const uint kMaxItemsOnScreen = 10;
	const uint xadd = 74, yadd = 64, itemHeight = 12;

	drawIcon(16);
	// Draw up arrow
	if (firstItem > 0)
		drawIcon(53);
	// Draw down arrow
	if (firstItem + kMaxItemsOnScreen < items.size())
		drawIcon(52);
	for (uint itemIndex = 0; (itemIndex < kMaxItemsOnScreen) && (firstItem + itemIndex < items.size()); itemIndex++) {
		byte *itemName = _vm->_inventoryItemNames->getString(items[firstItem + itemIndex]);
		const int itemX = xadd + 21, itemY = yadd + itemHeight * itemIndex;
		_vm->_screen->setFontColor(120);
		_vm->_screen->drawText(itemX, itemY, itemName);
		_vm->_screen->setFontColor(119);
		_vm->_screen->drawText(itemX + 1, itemY + 1, itemName);
		drawAnimatedInventoryIcon(items[firstItem + itemIndex], xadd, yadd + itemHeight * itemIndex - 3, animFrameCounter);
	}
	if (items.size() > 0) {
		const int selectionY = yadd + (currentItem - firstItem) * itemHeight - 1;
		_vm->_screen->frameRect(xadd + 16, selectionY, 253, selectionY + itemHeight - 1, _selectionColor);
		_selectionColor++;
		if (_selectionColor >= 96)
			_selectionColor = 80;
	}
}

// GuiCommandBar

int GuiCommandBar::run() {
	return handleCommandBar();
}

void GuiCommandBar::draw() {
	drawCommandBar(_commandBarSelectedItem);
}

void GuiCommandBar::drawCommandBar(int selectedItem) {
	const int x = 196;
	const int y = 14;

	drawIcon(0);
	drawIcon(selectedItem + 1);
	_vm->_inventory.testSelectFirstItem();
	if (_vm->_inventory.getSelectedItem() >= 0)
		drawAnimatedInventoryIcon(_vm->_inventory.getSelectedItem(), x, y, _animFrameCounter);
}

int GuiCommandBar::handleCommandBar() {
	const int kCBANone		= -1;
	const int kCBAExit		= -2;
	const int kCBAVerbTalk	= 0;
	const int kCBAVerbGet	= 1;
	const int kCBAUseItem	= 2;
	const int kCBAVerbLook	= 3;
	const int kCBAInventory	= 4;
	const int kCBAMap		= 5;
	const int kCBAMenu		= 6;

	static const GuiRectangle commandBarRects[] = {
		{  6, 4,  41, 34, kCBAVerbTalk}, 
		{ 51, 4,  86, 34, kCBAVerbGet}, 
		{ 96, 4, 131, 34, kCBAUseItem},
		{141, 4, 176, 34, kCBAVerbLook}, 
		{186, 4, 221, 34, kCBAInventory}, 
		{231, 4, 266, 34, kCBAMap},
		{276, 4, 311, 34, kCBAMenu}};
	const int commandBarItemCount = 6; // Intentionally doesn't match actual count!

    // The Lovecraft Museum that comes with the CD version needs some special handling
	const bool isMuseum = _vm->getGameID() == GID_MUSEUM;

	int commandBarStatus = 0;
	_animFrameCounter = 0;
	
	_vm->_input->waitForKeys();
	_vm->setMouseCursor(-1);

	if (isMuseum && (_commandBarSelectedItem != kCBAVerbLook || _commandBarSelectedItem != kCBAMenu))
		_commandBarSelectedItem = kCBAVerbLook;

	while (commandBarStatus == 0 && !_vm->shouldQuit()) {
		int mouseSelectedItem, commandBarAction = kCBANone;
		bool doWarpMouse = false;

		mouseSelectedItem = _vm->findRect(commandBarRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), commandBarItemCount + 1, kCBANone);

		if (isMuseum && (mouseSelectedItem != kCBAVerbLook || mouseSelectedItem != kCBAMenu))
			mouseSelectedItem = kCBANone;

		if (mouseSelectedItem != kCBANone)
			_commandBarSelectedItem = mouseSelectedItem;
			
		drawCommandBar(_commandBarSelectedItem);
		_animFrameCounter++;
		_vm->syncUpdate();
		_vm->_input->handleEvents();

		if (_vm->_input->getKeyCode() == Common::KEYCODE_INVALID && !_vm->_input->leftButton() && !_vm->_input->rightButton())
			continue;

		if (_vm->_input->rightButton())
			commandBarAction = kCBAExit;
		else if (_vm->_input->leftButton() && _commandBarSelectedItem != kCBANone)
			commandBarAction = _commandBarSelectedItem;

		if (isMuseum && (_vm->_input->getKeyCode() == Common::KEYCODE_RIGHT || _vm->_input->getKeyCode() == Common::KEYCODE_LEFT)) {
			if (_commandBarSelectedItem == kCBAVerbLook)
				_commandBarSelectedItem = kCBAMenu;
			else if (_commandBarSelectedItem == kCBAMenu)
				_commandBarSelectedItem = kCBAVerbLook;
			_vm->_input->clearKeyCode();
		}

		switch (_vm->_input->getKeyCode()) {
		case Common::KEYCODE_RIGHT:
			doWarpMouse = mouseSelectedItem == _commandBarSelectedItem;
			if (_commandBarSelectedItem == commandBarItemCount)
				_commandBarSelectedItem = 0;
			else
				_commandBarSelectedItem++;
			break;
		case Common::KEYCODE_LEFT:
			doWarpMouse = mouseSelectedItem == _commandBarSelectedItem;
			if (_commandBarSelectedItem == 0)
				_commandBarSelectedItem = commandBarItemCount;
			else
				_commandBarSelectedItem--;
			break;
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_TAB:
			commandBarAction = kCBAExit;
			break;
		case Common::KEYCODE_RETURN:
			commandBarAction = _commandBarSelectedItem;
			break;
		case Common::KEYCODE_t:
			commandBarAction = kCBAVerbTalk;
			break;
		case Common::KEYCODE_g:
			commandBarAction = kCBAVerbGet;
			break;
		case Common::KEYCODE_l:
			commandBarAction = kCBAVerbLook;
			break;
		case Common::KEYCODE_o:
			commandBarAction = kCBAInventory;
			break;
		case Common::KEYCODE_u:
			commandBarAction = kCBAUseItem;
			break;
		case Common::KEYCODE_d:
			commandBarAction = kCBAMenu;
			break;
		case Common::KEYCODE_m:
			commandBarAction = kCBAMap;
			break;
		default:
			break;
		}
		
		if (doWarpMouse) {
			_vm->warpMouseToRect(commandBarRects[_commandBarSelectedItem]);
			doWarpMouse = false;
		}
		
		if (commandBarAction >= 0) {
			drawCommandBar(commandBarAction);
			_vm->_screen->update();
		}
		
		switch (commandBarAction) {
		case kCBANone:
			break;
		case kCBAExit:
			commandBarStatus = 2;
			break;
		case kCBAVerbTalk:
			_vm->_verbs.requestTalk();
			commandBarStatus = 1;
			break;
		case kCBAVerbGet:
			_vm->_verbs.requestGet();
			commandBarStatus = 1;
			break;
		case kCBAVerbLook:
			_vm->_verbs.requestLook();
			commandBarStatus = 1;
			break;
		case kCBAUseItem:
			_vm->useCurrentInventoryItem();
			commandBarStatus = 1;
			break;
		case kCBAInventory:
			commandBarStatus = _vm->_gui->run(kGuiInventory);
			break;
		case kCBAMap:
			commandBarStatus = _vm->handleMap();
			break;
		case kCBAMenu:
			commandBarStatus = _vm->_gui->run(kGuiMainMenu);
			break;
		}

		_vm->_input->waitForKeys();
	}

	_vm->_input->waitForKeys();

	return 0;
}

// GuiMainMenu

int GuiMainMenu::run() {
	const int kMMANone		= -1;
	const int kMMAExit		= -2;
	const int kMMASave		= 0;
	const int kMMALoad		= 1;
	const int kMMAOptions	= 2;
	const int kMMAQuit		= 3;

	static const GuiRectangle mainMenuRects[] = {
		{136,  64, 184,  80, kMMASave},
		{136,  87, 184, 103, kMMALoad},
		{136, 110, 184, 126, kMMAOptions},
		{136, 133, 184, 149, kMMAQuit}};

	int mainMenuStatus = 0;

	_vm->_input->waitForKeys();
	_vm->setMouseCursor(-1);

	while (mainMenuStatus == 0 && !_vm->shouldQuit()) {
		int mouseSelectedItem, mainMenuAction = kMMANone;
		bool doWarpMouse = false;
	
		mouseSelectedItem = _vm->findRect(mainMenuRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), 4, kMMANone);
		if (mouseSelectedItem != kMMANone)
			_mainMenuSelectedItem = mouseSelectedItem;
			
		drawMainMenu(_mainMenuSelectedItem);
		
		_vm->syncUpdate();

		_vm->_input->handleEvents();

		if (_vm->_input->getKeyCode() == Common::KEYCODE_INVALID && !_vm->_input->leftButton() && !_vm->_input->rightButton())
			continue;

		if (_vm->_input->rightButton()) {
			mainMenuAction = kMMAExit;
		} else if (_vm->_input->leftButton() && _mainMenuSelectedItem != kMMANone) {
			mainMenuAction = _mainMenuSelectedItem;
		}

		switch (_vm->_input->getKeyCode()) {
		case Common::KEYCODE_DOWN:
			doWarpMouse = mouseSelectedItem == _mainMenuSelectedItem;
			if (_mainMenuSelectedItem == 3) {
				_mainMenuSelectedItem = 0;
			} else {
				_mainMenuSelectedItem++;
			}
			break;
		case Common::KEYCODE_UP:
			doWarpMouse = mouseSelectedItem == _mainMenuSelectedItem;
			if (_mainMenuSelectedItem == 0) {
				_mainMenuSelectedItem = 3;
			} else {
				_mainMenuSelectedItem--;
			}
			break;
		case Common::KEYCODE_ESCAPE:
			mainMenuAction = kMMAExit;
			break;
		case Common::KEYCODE_RETURN:
			mainMenuAction = _mainMenuSelectedItem;
			break;
		case Common::KEYCODE_s:
			mainMenuAction = kMMASave;
			break;
		case Common::KEYCODE_l:
			mainMenuAction = kMMALoad;
			break;
		case Common::KEYCODE_t:
			mainMenuAction = kMMAOptions;
			break;
		case Common::KEYCODE_x:
			mainMenuAction = kMMAQuit;
			break;
		default:
			break;
		}
		
		if (doWarpMouse) {
			_vm->warpMouseToRect(mainMenuRects[_mainMenuSelectedItem]);
			doWarpMouse = false;
		}

		if (mainMenuAction >= 0) {
			drawMainMenu(_mainMenuSelectedItem);
			_vm->_screen->update();
		}

		switch (mainMenuAction) {
		case kMMANone:
			break;
		case kMMAExit:
			mainMenuStatus = 2;
			break;
		case kMMASave:
			mainMenuStatus = _vm->_gui->run(kGuiSaveMenu);
			break;
		case kMMALoad:
			mainMenuStatus = _vm->_gui->run(kGuiLoadMenu);
			break;
		case kMMAOptions:
			mainMenuStatus = _vm->_gui->run(kGuiOptionsMenu);
			break;
		case kMMAQuit:
			_vm->quitGame();
			mainMenuStatus = 0;
			break;
		}

		_vm->_input->waitForKeys();
	
	}

	_vm->_input->waitForKeys();

	return 0;
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

int GuiOptionsMenu::run() {
	const int kOMANone			= -1;
	const int kOMAExit			= -2;
	const int kOMAMusicVol		= 0;
	const int kOMASoundVol		= 1;
	const int kOMATextSpeed		= 2;
	const int kOMAGameSpeed		= 3;
	const int kOMADriveLetter	= 4;
	const int kOMALanguage		= 5;
	const int kOMAOk			= 6;
	const int kOMATalkie		= 7;
	const int kOMADefMusicVol	= 8;
	const int kOMADefSoundVol	= 9;
	const int kOMADefGameSpeed	= 10;
	
	const int kVolumeConversionFactor = 17;

	static const GuiRectangle optionsMenuRects[] = {
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

	int optionsMenuStatus = 0;
	int musicVolumeDiv, currMusicVolumeDiv, digiVolumeDiv, textSpeed, gameSpeed, language;
	uint animFrameCounter = 0;

	// NOTE There are no separate volume controls for speech and effects.
	// Because we know if a sample is speech or an effect we use the proper sound types
	// when playing. This means that the speech volume can only be changed in the ScummVM options.

	musicVolumeDiv = ConfMan.getInt("music_volume") / kVolumeConversionFactor;
	digiVolumeDiv = ConfMan.getInt("sfx_volume") / kVolumeConversionFactor;
	currMusicVolumeDiv = musicVolumeDiv;
	textSpeed = ConfMan.getInt("text_speed");
	gameSpeed = ConfMan.getInt("game_speed");
	language = 1;
	
	_vm->_input->waitForKeys();

	while (optionsMenuStatus == 0 && !_vm->shouldQuit()) {
		int mouseSelectedItem = -1, optionsMenuAction = kOMANone, selectedItemToDraw;
		int optionIncr = 0;
		bool doWaitForKeys = true;
		bool doWarpMouse = false;

		int16 mouseX = CLIP(_vm->_input->getMouseX(), 127, 189);

		if (!_vm->isFloppy()) {
			mouseSelectedItem = _vm->findRect(optionsMenuRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), 13, kOMANone);
			if (mouseSelectedItem != kOMANone)
				_optionsMenuSelectedItem = mouseSelectedItem;
		}

		if (musicVolumeDiv != currMusicVolumeDiv) {
			_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, musicVolumeDiv * kVolumeConversionFactor);
			currMusicVolumeDiv = musicVolumeDiv;
		}

		animFrameCounter++;
		if (animFrameCounter == 32)
			animFrameCounter = 0;

		selectedItemToDraw = _optionsMenuSelectedItem;
		if (!_vm->isFloppy()) {
			if (selectedItemToDraw == kOMADefMusicVol)
				selectedItemToDraw = kOMAMusicVol;
			else if (selectedItemToDraw == kOMADefSoundVol)
				selectedItemToDraw = kOMASoundVol;
			else if (selectedItemToDraw == kOMADefGameSpeed)
				selectedItemToDraw = kOMAGameSpeed;
			else if (selectedItemToDraw == kOMATalkie)
				selectedItemToDraw = 2;
		}

		drawOptionsMenu(selectedItemToDraw, musicVolumeDiv, digiVolumeDiv, textSpeed,
			gameSpeed, language, animFrameCounter, optionsMenuRects);

		_vm->syncUpdate();
		_vm->_input->handleEvents();

		if (_vm->_input->getKeyCode() == Common::KEYCODE_INVALID && !_vm->_input->leftButton() && !_vm->_input->rightButton())
			continue;

		if (_vm->_input->rightButton())
			optionsMenuAction = kOMAExit;
		else if (_vm->_input->leftButton() && _optionsMenuSelectedItem != kOMANone)
			optionsMenuAction = _optionsMenuSelectedItem;

		switch (_vm->_input->getKeyCode()) {
		case Common::KEYCODE_DOWN:
			doWarpMouse = mouseSelectedItem == _optionsMenuSelectedItem;
			if (_optionsMenuSelectedItem == 5)
				_optionsMenuSelectedItem = 0;
			else
				_optionsMenuSelectedItem++;
			break;
		case Common::KEYCODE_UP:
			doWarpMouse = mouseSelectedItem == _optionsMenuSelectedItem;
			if (_optionsMenuSelectedItem == 0)
				_optionsMenuSelectedItem = 5;
			else
				_optionsMenuSelectedItem--;
			break;
		case Common::KEYCODE_LEFT:
			optionIncr = -1;
			if (_vm->isFloppy())
				optionsMenuAction = _optionsMenuSelectedItem;
			else if (_optionsMenuSelectedItem != 5)
				optionsMenuAction = optionsMenuRects[_optionsMenuSelectedItem].id;
			break;
		case Common::KEYCODE_RIGHT:
			optionIncr = +1;
			if (_vm->isFloppy())
				optionsMenuAction = _optionsMenuSelectedItem;
			else if (_optionsMenuSelectedItem != 5)
				optionsMenuAction = optionsMenuRects[_optionsMenuSelectedItem].id;
			break;
		case Common::KEYCODE_ESCAPE:
			optionsMenuAction = kOMAExit;
			break;
		case Common::KEYCODE_RETURN:
			if (_vm->isFloppy() || _optionsMenuSelectedItem == 5 || _optionsMenuSelectedItem == 6)
				optionsMenuAction = kOMAOk;
			break;
		default:
			break;
		}

		if (doWarpMouse) {
			_vm->warpMouseToRect(optionsMenuRects[_optionsMenuSelectedItem]);
			doWarpMouse = false;
		}

		switch (optionsMenuAction) {
		case kOMANone:
			break;
		case kOMAExit:
			optionsMenuStatus = 2;
			break;
		case kOMAMusicVol:
			if (optionIncr != 0)
				musicVolumeDiv = CLIP(musicVolumeDiv + optionIncr, 0, 15);
			else
				musicVolumeDiv = (mouseX - 127) / 4;
			doWaitForKeys = false;
			break;
		case kOMASoundVol:
			if (optionIncr != 0)
				digiVolumeDiv = CLIP(digiVolumeDiv + optionIncr, 0, 15);
			else
				digiVolumeDiv = (mouseX - 127) / 4;
			doWaitForKeys = false;
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
			if (optionIncr != 0)
				gameSpeed = CLIP(gameSpeed + optionIncr, 0, 15);
			else
				gameSpeed = (mouseX - 127) / 4;
			doWaitForKeys = false;
			break;
		case kOMATextSpeed:
			if (optionIncr != 0)
				textSpeed = CLIP(textSpeed + optionIncr, 0, 2);
			doWaitForKeys = false;
			break;
		case kOMADriveLetter:
			// Nothing/not supported
			break;
		case kOMALanguage:
			if (optionIncr != 0)
				language = CLIP(language + optionIncr, 0, 4);
			else if (language < 4)
				language++;
			else
				language = 0;
			break;
		case kOMAOk:
			optionsMenuStatus = 1;
			break;
		case kOMADefMusicVol:
			musicVolumeDiv = 8;
			break;
		case kOMADefSoundVol:
			digiVolumeDiv = 8;
			break;
		case kOMADefGameSpeed:
			gameSpeed = 8;
			break;
		}

		if (doWaitForKeys)
			_vm->_input->waitForKeys();
	}

	_vm->_input->waitForKeys();

	if (optionsMenuStatus == 1) {
		// TODO Also save game speed, text speed
		_vm->_talkText->setTextSpeed(textSpeed);
		_vm->_gameSpeed = gameSpeed;
		ConfMan.setInt("text_speed", textSpeed);
		ConfMan.setInt("text_speed", gameSpeed);
		ConfMan.setInt("music_volume", musicVolumeDiv * kVolumeConversionFactor);
		ConfMan.setInt("sfx_volume", digiVolumeDiv * kVolumeConversionFactor);
		ConfMan.flushToDisk();
		_vm->syncSoundSettings();
	}

	return 0;
}

void GuiOptionsMenu::draw() {
}

void GuiOptionsMenu::drawOptionsMenu(int selectedItem, int musicVolumeDiv, int digiVolumeDiv, int textSpeed,
	int gameSpeed, int language, uint animFrameCounter, const GuiRectangle *guiRectangles) {

	const int itemLeftX = 107;
	const int itemTopY = 65;
	const int itemHeight = 20;
	const int gaugeX = 128;
	const int gaugeY = 70;

	drawIcon(25);

	if (!_vm->isFloppy() && selectedItem == 5)
		_vm->_screen->frameRect(guiRectangles[6].x, guiRectangles[6].y, guiRectangles[6].x2, guiRectangles[6].y2, 119);
	else
		drawIcon(28, itemLeftX, selectedItem * itemHeight + itemTopY);

	drawIcon(27, gaugeX + musicVolumeDiv * 4, gaugeY);
	drawIcon(27, gaugeX + digiVolumeDiv * 4, gaugeY + itemHeight);
	drawIcon(27, gaugeX + gameSpeed * 4, gaugeY + itemHeight * 3);
	drawIcon(language + 32, 129, _vm->isFloppy() ? 177 : 157);

	if (_vm->isFloppy())
		drawIcon(27, gaugeX + textSpeed * 30, gaugeY + itemHeight * 2);

	if (musicVolumeDiv == 0) {
		drawIcon(55);
	} else if (musicVolumeDiv > 0 && musicVolumeDiv < 8) {
		if (animFrameCounter < 16) {
			drawIcon(71);
		} else {
			drawIcon(72);
		}
	} else if (musicVolumeDiv >= 8) {
		if (animFrameCounter < 16) {
			drawIcon(73);
		} else {
			drawIcon(74);
		}
	}

	if (digiVolumeDiv == 0)
		drawAnimatedIcon(1, 0, 0, animFrameCounter);
	else if (digiVolumeDiv < 7)
		drawAnimatedIcon(2, 0, 0, animFrameCounter);
	else
		drawAnimatedIcon(3, 0, 0, animFrameCounter);

	if (_vm->isFloppy()) {
		if ((textSpeed == 0 && animFrameCounter > 6) ||
			(textSpeed == 1 && animFrameCounter > 16) ||
			(textSpeed == 2 && animFrameCounter > 24))
			drawIcon(70);
	} else
		drawIcon(_vm->_talkText->getTalkieMode() + 79);

	if (gameSpeed < 5)
		drawIcon(75);
	else if (gameSpeed >= 5 && gameSpeed < 10)
		drawIcon(76);
	else if (gameSpeed >= 10)
		drawIcon(77);

}

// GuiTownMap

int GuiTownMap::run() {

	static const struct MapPoint { int16 x, y; } mapPoints[] = {
		{248, 126}, {226, 126}, {224, 150}, {204, 156},
		{178, 154}, {176, 138}, {152, 136}, {124, 134},
		{112, 148}, { 96, 132}, { 92, 114}, {146, 116},
		{176, 106}, {138, 100}, {104,  94}, { 82,  96},
		{172,  94}, {116,  80}, {134,  80}, {148,  86},
		{202, 118}, {178, 120}, {190,  92}
	};

	static const struct MapRect { int16 x1, y1, x2, y2; } mapRects[] = {
		{240, 116, 264, 134}, {192, 102, 216, 127}, {164,  82, 189,  96},
		{165,  98, 189, 113}, {108, 108, 162, 124}, {140, 128, 166, 144},
		{ 85,  99, 104, 122}, { 84, 124, 106, 142}, {125,  92, 154, 106},
		{104,  70, 128,  87}
	};

	static const struct MapExit { int16 moduleNumber, sceneNumber; } mapExits[] = {
		{0,  0}, {0, 20}, {0, 16}, {0, 12}, {0, 11},
		{0,  6}, {0, 10}, {0,  9}, {0, 13}, {0, 17}
	};
	
	const int16 kCursorAddX = 8, kCursorAddY = 8;

	int mapRectX1, mapRectX2, mapRectY1, mapRectY2;
	int mapStatus = 0;
	uint16 sceneBitMaskStatus;
	uint16 sceneStatus1, sceneStatus2, sceneStatus3, sceneStatus4;
	int16 cursorX, cursorY;
	int16 prevCursorX = -1, prevCursorY = -1;
	int16 locationNumber = _vm->_sceneNumber % 30;

	if (_vm->isFloppy()) {
		mapRectX1 = 82;
		mapRectX2 = 268;
		mapRectY1 = 68;
		mapRectY2 = 186;
		sceneBitMaskStatus = _vm->_scriptVars[2];
		sceneStatus1 = _vm->_scriptVars[89];
		sceneStatus2 = _vm->_scriptVars[114];
		sceneStatus3 = _vm->_scriptVars[61];
		sceneStatus4 = _vm->_scriptVars[92];
	} else {
		mapRectX1 = 65;
		mapRectX2 = 268;
		mapRectY1 = 66;
		mapRectY2 = 186;
		sceneBitMaskStatus = _vm->_scriptVars[2];
		sceneStatus1 = _vm->_scriptVars[3];
		sceneStatus2 = _vm->_scriptVars[4];
		sceneStatus3 = _vm->_scriptVars[5];
		sceneStatus4 = _vm->_scriptVars[6];
	}

	cursorX = mapPoints[locationNumber].x;
	cursorY = mapPoints[locationNumber].y;

	_vm->_input->waitForKeys();

	if (!_vm->isFloppy()) {
		CursorMan.showMouse(false);
		_vm->_system->warpMouse(cursorX, cursorY);
	}	

	while (mapStatus == 0 && !_vm->shouldQuit()) {

		int16 currMapLocation = 0, selectedMapLocation = 0;
		bool cursorChanged = false;

		_vm->_input->handleEvents();

		if (!_vm->isFloppy()) {
			cursorX = CLIP(_vm->_input->getMouseX(), mapRectX1, mapRectX2);
			cursorY = CLIP(_vm->_input->getMouseY(), mapRectY1, mapRectY2);
		}

		switch (_vm->_input->getKeyCode()) {
		case Common::KEYCODE_UP:
			cursorY = MAX(cursorY - kCursorAddY, mapRectY1);
			break;
		case Common::KEYCODE_DOWN:
			cursorY = MIN(cursorY + kCursorAddY, mapRectY2);
			break;
		case Common::KEYCODE_LEFT:
			cursorX = MAX(cursorX - kCursorAddX, mapRectX1);
			break;
		case Common::KEYCODE_RIGHT:
			cursorX = MIN(cursorX + kCursorAddX, mapRectX2);
			break;
		default:
			break;
		}

		cursorChanged = prevCursorX != cursorX || prevCursorY != cursorY;

		prevCursorX = cursorX;
		prevCursorY = cursorY;

		if (!_vm->isFloppy() && (_vm->_input->getMouseX() != cursorX || _vm->_input->getMouseY() != cursorY))
			_vm->_system->warpMouse(cursorX, cursorY);

		if (_vm->_input->getKeyCode() == Common::KEYCODE_ESCAPE || _vm->_input->rightButton()) {
			mapStatus = 1;
			cursorChanged = false;
			currMapLocation = -1;
		}

		if (cursorChanged) {
			currMapLocation = -1;
			selectedMapLocation = -1;
			drawIcon(50);
			for (int16 mapLocation = 0; mapLocation < 10; mapLocation++) {
				const MapRect &mapRect = mapRects[mapLocation];
				if ((sceneBitMaskStatus & (1 << mapLocation)) && 
					cursorX >= mapRect.x1 && cursorX <= mapRect.x2 && 
					cursorY >= mapRect.y1 && cursorY <= mapRect.y2) {
					currMapLocation = mapLocation;
					break;
				}
			}
			if (currMapLocation != -1) {
				byte *locationName = _vm->_textReader->getString(2, 40 + currMapLocation);
				_vm->_screen->drawTextOutlined(MIN(cursorX - 2, 283 - _vm->_screen->getTextWidth(locationName)), 
					cursorY - 6, locationName, 119, 120);
			} else {
				drawIcon(51, cursorX, cursorY);
			}
		}

		if (currMapLocation != -1 && (_vm->_input->getKeyCode() == Common::KEYCODE_RETURN || _vm->_input->leftButton())) {
			selectedMapLocation = currMapLocation;
			const MapExit &mapExit = mapExits[selectedMapLocation];
			_vm->_moduleNumber = mapExit.moduleNumber;
			_vm->_sceneNumber = mapExit.sceneNumber;
			if (sceneStatus1 == 1) {
				_vm->_moduleNumber += 6;
			} else if (sceneStatus2 >= 2) {
				_vm->_sceneNumber += (sceneStatus2 - 1) * 30;
			}
			if ((locationNumber == 7 || locationNumber == 8) &&
				sceneStatus3 == 2 && sceneStatus4 == 0 &&
				selectedMapLocation != 6 && selectedMapLocation != 7 && selectedMapLocation != 4) {
				_vm->_sceneNumber = 36;
			}
			mapStatus = 2;
			debug("moduleNumber: %d; sceneNumber: %d", _vm->_moduleNumber, _vm->_sceneNumber);
		}

		_vm->syncUpdate();

	}

	_vm->_input->waitForKeys();

	CursorMan.showMouse(!_vm->isFloppy());

	return 1;
}

void GuiTownMap::draw() {
}

// GuiJournal

int GuiJournal::run() {
	return handleReadBook();
}

void GuiJournal::draw() {
}

int GuiJournal::handleReadBook() {

	const int kBANone			= -1;
	const int kBAExit			= -2;
	const int kBAPrevPage		= 0;
	const int kBANextPage		= 1;

	static const GuiRectangle bookRects[] = {
		{ 54, 46, 166, 189, kBAPrevPage},
		{167, 46, 279, 189, kBANextPage}
	};

	int currPageNumber = -1, pageNumber, pageCount, talkPageNumber = -1;
	int bookStatus = 0;

	// Use values from script; this is the most recent diary entry
	pageNumber = _vm->_scriptVars[1];
	pageCount = _vm->_scriptVars[1];

	_vm->setMouseCursor(-1);

	bookTurnPageTextEffect(false, pageNumber, pageCount);

	// Set speech file
	_vm->_talkText->setVoiceFileIndex(7);

	while (bookStatus == 0) {
	
		int bookAction = kBANone, mouseSelectedRect;

		if (currPageNumber != pageNumber) {
			drawBookPage(pageNumber, pageCount, 64);
			currPageNumber = pageNumber;
		}

		do {
			// Play page speech
			if (talkPageNumber != pageNumber) {
				if (pageNumber > 0)
					_vm->_talkText->playVoice(pageNumber);
				else
					_vm->_talkText->stopVoice();
				talkPageNumber = pageNumber;
			}
			_vm->_input->handleEvents();
			if (!_vm->isFloppy()) {
				mouseSelectedRect = _vm->findRect(bookRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), 2, kBANone);
				if (mouseSelectedRect == -1)
					_vm->setMouseCursor(-1);
				else if (mouseSelectedRect == 0)
					_vm->setMouseCursor(3);
				else if (mouseSelectedRect == 1)
					_vm->setMouseCursor(2);
			}
			_vm->syncUpdate();
			if (_vm->shouldQuit() || _vm->_input->rightButton() || _vm->_input->getKeyCode() == Common::KEYCODE_RETURN ||
				_vm->_input->getKeyCode() == Common::KEYCODE_ESCAPE)
				bookAction = kBAExit;
			else if (_vm->_input->getKeyCode() == Common::KEYCODE_LEFT || (_vm->_input->leftButton() && mouseSelectedRect == 0))
				bookAction = kBAPrevPage;
			else if (_vm->_input->getKeyCode() == Common::KEYCODE_RIGHT || (_vm->_input->leftButton() && mouseSelectedRect == 1))
				bookAction = kBANextPage;
		} while (bookAction == kBANone);

		switch (bookAction) {
		case kBAExit:
			bookStatus = 2;
			break;
		case kBAPrevPage:
			if (pageNumber > 0) {
				bookTurnPageTextEffect(true, pageNumber, pageCount);
				bookTurnPage(false);
				pageNumber--;
				bookTurnPageTextEffect(false, pageNumber, pageCount);
			}
			break;
		case kBANextPage:
			if (pageNumber < pageCount) {
				bookTurnPageTextEffect(true, pageNumber, pageCount);
				bookTurnPage(true);
				pageNumber++;
				bookTurnPageTextEffect(false, pageNumber, pageCount);
			}
			break;
		}

		_vm->_input->waitForKeys();

	}

	_vm->_input->waitForKeys();
	_vm->_talkText->leaveJournal();

	return 2 - bookStatus;
}

void GuiJournal::drawBookPage(int pageTextIndex, int pageTextMaxIndex, byte fontColor) {
	int xadd = 58, yadd = 48, x = 0, lineNumber = 0;
	Common::String pageNumberString;
	int pageNumberStringWidth;

	byte *pageText = _vm->_textReader->getString(2, pageTextIndex);

	drawIcon(30);
	if (pageTextIndex < pageTextMaxIndex)
		drawIcon(37);

	_vm->_screen->setFontColor(58);

	pageNumberString = Common::String::format("- %d -", pageTextIndex * 2 + 1);
	pageNumberStringWidth = _vm->_screen->getTextWidth((const byte*)pageNumberString.c_str());
	_vm->_screen->drawText(xadd + (106 - pageNumberStringWidth) / 2, 180, (const byte*)pageNumberString.c_str());

	pageNumberString = Common::String::format("- %d -", pageTextIndex * 2 + 2);
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

void GuiJournal::bookTurnPage(bool turnDirection) {
	const uint first = turnDirection ? 38 : 49;
	const uint last = turnDirection ? 49 : 38;
	const int incr = turnDirection ? +1 : -1;
	for (uint i = first; i != last; i += incr) {
		drawIcon(30);
		drawIcon(i);
		_vm->syncUpdate();
	}
}

void GuiJournal::bookTurnPageTextEffect(bool turnDirection, int pageTextIndex, int pageTextMaxIndex) {
	const byte firstColor = turnDirection ? 64 : 72;
	const byte lastColor = turnDirection ? 72 : 64;
	const int incr = turnDirection ? +1 : -1;
	for (byte fontColor = firstColor; fontColor != lastColor; fontColor += incr) {
		drawBookPage(pageTextIndex, pageTextMaxIndex, fontColor);
		_vm->syncUpdate();
	}
}

// GuiPuzzle

int GuiPuzzle::run() {
	return runPuzzle();
}

void GuiPuzzle::draw() {
}

int GuiPuzzle::runPuzzle() {
	static const uint16 puzzleCheatInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4, 8,13, 0},
		{0, 1, 5, 9,14, 0},
		{0, 2, 6,10,15, 0},
		{0, 3, 7,11,12, 0},
		{0, 0, 0, 0, 0, 0}};

	static const uint16 puzzleInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4,10,15, 0},
		{0, 5,11,13,12, 0},
		{0, 8, 1, 2, 6, 0},
		{0, 3, 7, 9,14, 0},
		{0, 0, 0, 0, 0, 0}};

	static const GuiRectangle puzzleTileRects[] = {
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

	static const struct { int col, row; } rectToColRow[] = {
		{1, 0}, {2, 0}, {3, 0}, {4, 0}, 
		{5, 1}, {5, 2}, {5, 3}, {5, 4}, 
		{1, 5}, {2, 5}, {3, 5}, {4, 5}, 
		{0, 1}, {0, 2}, {0, 3}, {0, 4}, 
		{0, 0}, {5, 0}, {5, 5}, {0, 5} 
	};

	int puzzleStatus = 0;
	int puzzleCursorX = 0, puzzleCursorY = 0;

	_puzzleSprite = _vm->_animationMan->loadAnimationResource("A07.PAK", 24);

	// Initialize the puzzle state
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if (debugPuzzleCheat)
				_puzzleTiles[i][j] = puzzleCheatInitialTiles[i][j];
			else
				_puzzleTiles[i][j] = puzzleInitialTiles[i][j];
		}
	}

	_fingerBackground = NULL;	
	_puzzleTableRow = 0;
	_puzzleTableColumn = 0;

	loadFingerCursor();

	while (puzzleStatus == 0 && !_vm->shouldQuit()) {

		_vm->_input->handleEvents();

		if (!_vm->isFloppy()) {
			/* Comet CD: Find out which tile is selected and convert 
			   mouse coords to tile coords.
			*/
			int mouseSelectedTile;
			puzzleCursorX = CLIP(_vm->_input->getMouseX(), 103, 231);
			puzzleCursorY = CLIP(_vm->_input->getMouseY(), 44, 171);
			if (_vm->_input->getMouseX() != puzzleCursorX || _vm->_input->getMouseY() != puzzleCursorY)
				_vm->_system->warpMouse(puzzleCursorX, puzzleCursorY);
			mouseSelectedTile = _vm->findRect(puzzleTileRects, _vm->_input->getMouseX(), _vm->_input->getMouseY(), 21, -1);
			if (mouseSelectedTile >= 0) {
				if (mouseSelectedTile >= 0 && mouseSelectedTile < 20) {
					_puzzleTableColumn = rectToColRow[mouseSelectedTile].col;
					_puzzleTableRow = rectToColRow[mouseSelectedTile].row;
				} else if (mouseSelectedTile == 20) {
					_puzzleTableColumn = (puzzleCursorX - 119) / 24 + 1;
					_puzzleTableRow = (puzzleCursorY - 60) / 24 + 1; 
				} else {
					_puzzleTableColumn = 0;
					_puzzleTableRow = 0;
				}
			}
		}

		drawField();
		_vm->syncUpdate();

		if (_vm->_input->getKeyCode() != Common::KEYCODE_INVALID) {
			bool selectionChanged = false;

			switch (_vm->_input->getKeyCode()) {
			case Common::KEYCODE_UP:
				if (_puzzleTableRow > 0) {
					_puzzleTableRow--;
					selectionChanged = true;
				}
				break;
			case Common::KEYCODE_DOWN:
				if (_puzzleTableRow < 5) {
					_puzzleTableRow++;
					selectionChanged = true;
				}
				break;
			case Common::KEYCODE_LEFT:
				if (_puzzleTableColumn > 0) {
					_puzzleTableColumn--;
					selectionChanged = true;
				}
				break;
			case Common::KEYCODE_RIGHT:
				if (_puzzleTableColumn < 5) {
					_puzzleTableColumn++;
					selectionChanged = true;
				}
				break;
			default:
				break;
			}

			if (selectionChanged && !_vm->isFloppy()) {
				/* Comet CD: If the tile selection has been changed by cursor keys,
				   position the mouse at the correct tile. */
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
					puzzleCursorX = (puzzleTileRects[mouseNewTile].x + puzzleTileRects[mouseNewTile].x2) / 2;
					puzzleCursorY = (puzzleTileRects[mouseNewTile].y + puzzleTileRects[mouseNewTile].y2) / 2;
				} else {
					puzzleCursorX = (_puzzleTableColumn - 1) * 24 + 130;
					puzzleCursorY = (_puzzleTableRow - 1) * 24 + 71;
				}
				_vm->_system->warpMouse(puzzleCursorX, puzzleCursorY);
			}
			
		}

		if (_vm->_input->getKeyCode() == Common::KEYCODE_ESCAPE || _vm->_input->rightButton()) {
			puzzleStatus = 1;
		} else if (_vm->_input->getKeyCode() == Common::KEYCODE_RETURN || _vm->_input->leftButton()) {
			if (_puzzleTableColumn == 0 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				playTileSound();
				moveTileRow(_puzzleTableRow, -1);
			} else if (_puzzleTableColumn == 5 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				playTileSound();
				moveTileRow(_puzzleTableRow, 1);
			} else if (_puzzleTableRow == 0 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				playTileSound();
				moveTileColumn(_puzzleTableColumn, -1);
			} else if (_puzzleTableRow == 5 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				playTileSound();
				moveTileColumn(_puzzleTableColumn, 1);
			}
			if (testIsSolved())
				puzzleStatus = 2;
		} else {
			_vm->_input->waitForKeys();
		}
	}

	if (_fingerBackground)
		_fingerBackground->free();
	delete _fingerBackground;
	delete _puzzleSprite;

	return puzzleStatus == 2 ? 2 : 0;
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

void GuiPuzzle::moveTileColumn(int columnIndex, int direction) {
	if (direction < 0) {
		_puzzleTiles[columnIndex][5] = _puzzleTiles[columnIndex][1];
		for (int yOffs = 0; yOffs < 24; yOffs += 2) {
			_vm->_screen->setClipY(60, 156);
			for (int rowIndex = 1; rowIndex <= 5; rowIndex++)
				drawTile(columnIndex, rowIndex, 0, -yOffs);
			_vm->_screen->setClipY(0, 199);
			drawFinger();
			_vm->syncUpdate();
		}
		for (int rowIndex = 0; rowIndex <= 4; rowIndex++)
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex + 1];
		_puzzleTiles[columnIndex][5] = _puzzleTiles[columnIndex][1];
	} else {
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
		for (int yOffs = 0; yOffs < 24; yOffs += 2) {
			_vm->_screen->setClipY(60, 156);
			for (int rowIndex = 0; rowIndex <= 4; rowIndex++)
				drawTile(columnIndex, rowIndex, 0, yOffs);
			_vm->_screen->setClipY(0, 199);
			drawFinger();
			_vm->syncUpdate();
		}
		for (int rowIndex = 5; rowIndex >= 1; rowIndex--)
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex - 1];
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
	}
}

void GuiPuzzle::moveTileRow(int rowIndex, int direction) {
	if (direction < 0) {
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_vm->_screen->setClipX(120, 215);
			for (int columnIndex = 1; columnIndex <= 5; columnIndex++)
				drawTile(columnIndex, rowIndex, -xOffs, 0);
			_vm->_screen->setClipX(0, 319);
			drawFinger();
			_vm->syncUpdate();
		}
		for (int columnIndex = 0; columnIndex <= 4; columnIndex++)
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex + 1][rowIndex];
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
	} else {
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_vm->_screen->setClipX(120, 215);
			for (int columnIndex = 0; columnIndex <= 4; columnIndex++)
				drawTile(columnIndex, rowIndex, xOffs, 0);
			_vm->_screen->setClipX(0, 319);
			drawFinger();
			_vm->syncUpdate();
		}
		for (int columnIndex = 5; columnIndex >= 1; columnIndex--)
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex - 1][rowIndex];
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
	}
}

bool GuiPuzzle::testIsSolved() {
	int matchingTiles = 0;
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++)
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++)
			if (_puzzleTiles[columnIndex][rowIndex] == (rowIndex - 1) * 4 + (columnIndex - 1))
				matchingTiles++;
	return matchingTiles == 16;
}

void GuiPuzzle::playTileSound() {
	if (!_vm->isFloppy())
		_vm->playSample(75, 1);
}

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
	const int itemHeight = 12;
	drawIcon(_asSaveMenu ? 14 : 15);
	for (int itemIndex = 0; itemIndex < 10; itemIndex++)
		drawSavegameDescription(_savegames[itemIndex].description, itemIndex);
	_vm->_screen->frameRect(x - 2, y + selectedItem * itemHeight - 2, x + 138, y + selectedItem * itemHeight + 9, 119);
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
	const int itemHeight = 12;
	int textX = (135 - _vm->_screen->getTextWidth((const byte*)description.c_str())) / 2;
	_vm->_screen->setFontColor(120);
	_vm->_screen->drawText(x + textX + 1, y + savegameIndex * itemHeight + 1, (const byte*)description.c_str());
	_vm->_screen->setFontColor(119);
	_vm->_screen->drawText(x + textX, y + savegameIndex * itemHeight, (const byte*)description.c_str());
}

} // End of namespace Comet
