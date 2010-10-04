#include "comet/comet.h"
#include "comet/comet_gui.h"
#include "comet/animationmgr.h"
#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/screen.h"

namespace Comet {

Gui::Gui(CometEngine *vm) : _vm(vm) {
	_guiInventory = new GuiInventory(_vm);
	_guiCommandBar = new GuiCommandBar(_vm);
	_guiDiary = new GuiDiary(_vm);
	_guiTownMap = new GuiTownMap(_vm);
	_guiMainMenu = new GuiMainMenu(_vm);
	_guiOptionsMenu = new GuiOptionsMenu(_vm);
	_guiPuzzle = new GuiPuzzle(_vm);
}

Gui::~Gui() {
	delete _guiInventory;
	delete _guiCommandBar;
	delete _guiDiary;
	delete _guiTownMap;
	delete _guiMainMenu;
	delete _guiOptionsMenu;
	delete _guiPuzzle;
}

int Gui::runInventory() {
	return _guiInventory->run();
}

int Gui::runCommandBar() {
	return _guiCommandBar->run();
}

int Gui::runDiary() {
	return _guiDiary->run();
}

int Gui::runTownMap() {
	return _guiTownMap->run();
}

int Gui::runMainMenu() {
	return _guiMainMenu->run();
}

int Gui::runOptionsMenu() {
	return _guiOptionsMenu->run();
}

int Gui::runPuzzle() {
	return _guiPuzzle->run();
}

/* GuiInventory */

GuiInventory::GuiInventory(CometEngine *vm) : _vm(vm) {
}

GuiInventory::~GuiInventory() {
}

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

	_vm->waitForKeys();
		
	// Build items array and set up variables
	for (int i = 0; i < 256; i++) {
		if (_vm->_inventoryItemStatus[i] >= 1) {
			items.push_back(i);
			if (i == _vm->_currentInventoryItem) {
				firstItem = items.size() < 5 ? 0 : items.size() - 5;
				currentItem = items.size() - 1;
			}
		}
	}

	while (inventoryStatus == 0) {
		int inventoryAction = kIANone, mouseSelectedItem;
			
		_vm->handleEvents();

		mouseSelectedItem = _vm->findRect(inventorySlotRects, _vm->_mouseX, _vm->_mouseY, MIN<int>(items.size() - firstItem, 10) + 2, kIANone);
			
		if (mouseSelectedItem >= 0) {
			currentItem = firstItem + mouseSelectedItem;
		}			
	
		drawInventory(items, firstItem, currentItem, animFrameCounter++);

		_vm->_screen->update();
		_vm->_system->delayMillis(40); // TODO: Adjust or use fps counter

		if (_vm->_rightButton) {
			inventoryAction = kIAExit;
		} else if (_vm->_leftButton) {
			if (mouseSelectedItem >= 0)
				inventoryAction = kIAUse;
			else if (mouseSelectedItem != kIANone)
				inventoryAction = mouseSelectedItem;				
		}

		switch (_vm->_keyScancode) {
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
			if ((currentItem - firstItem + 1 < kMaxItemsOnScreen) && (currentItem + 1 < items.size())) {
				// TODO: Check mouse rectangle
				currentItem++;
			} else if (firstItem + kMaxItemsOnScreen < items.size()) {
				firstItem++;
				currentItem++;
			}
			break;
		case kIAUp:
			if (currentItem > firstItem) {
				// TODO: Check mouse rectangle
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
			// TODO: Move elsewhere
			for (uint i = 0; i < 255; i++) {
				if (_vm->_inventoryItemStatus[i] == 2)
					_vm->_inventoryItemStatus[i] = 1;
			}
			_vm->_currentInventoryItem = items[currentItem];
			// Return just selects, U actually uses the item
			if (inventoryAction == kIAUse) {
				//debug("Use item #%d", _currentInventoryItem);
				_vm->_inventoryItemStatus[_vm->_currentInventoryItem] = 2;
			}
			inventoryStatus = 1;
			break;
		default:
			break;
		}
		
  		_vm->waitForKeys();
	}
	
	// TODO...

	return 2 - inventoryStatus;;
}

void GuiInventory::drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter) {

	const uint kMaxItemsOnScreen = 10;

	uint xadd = 74, yadd = 64, itemHeight = 12;

	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 16, 0, 0);

	// Draw up arrow
	if (firstItem > 0)
		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 53, 0, 0);

	// Draw down arrow
	if (firstItem + kMaxItemsOnScreen < items.size())
		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 52, 0, 0);

	for (uint itemIndex = 0; (itemIndex < kMaxItemsOnScreen) && (firstItem + itemIndex < items.size()); itemIndex++) {
		byte *itemName = _vm->_inventoryItemNames->getString(items[firstItem + itemIndex]);
		int itemX = xadd + 21, itemY = yadd + itemHeight * itemIndex;
		_vm->_screen->setFontColor(120);
		_vm->_screen->drawText(itemX, itemY, itemName);
		_vm->_screen->setFontColor(119);
		_vm->_screen->drawText(itemX + 1, itemY + 1, itemName);
		_vm->drawAnimatedIcon(_vm->_inventoryItemSprites, items[firstItem + itemIndex], xadd, yadd + itemHeight * itemIndex - 3, animFrameCounter);
	}
	
	if (items.size() > 0) {
		int selectionY = yadd + (currentItem - firstItem) * itemHeight - 1;
		_vm->_screen->frameRect(xadd + 16, selectionY, 253, selectionY + itemHeight - 1, _selectionColor);
		_selectionColor++;
		if (_selectionColor >= 96)
			_selectionColor = 80;
	}

}

/* GuiCommandBar */

GuiCommandBar::GuiCommandBar(CometEngine *vm) : _vm(vm), _commandBarSelectedItem(-1) {
}

GuiCommandBar::~GuiCommandBar() {
}

int GuiCommandBar::run() {
	handleCommandBar();
	return 0;
}

void GuiCommandBar::drawCommandBar(int selectedItem, int animFrameCounter) {

	const int x = 196;
	const int y = 14;

	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 0, 0, 0);
	_vm->_screen->drawAnimationElement(_vm->_iconSprite, selectedItem + 1, 0, 0);

	if (_vm->_currentInventoryItem >= 0 && _vm->_inventoryItemStatus[_vm->_currentInventoryItem] == 0) {
		_vm->_currentInventoryItem = -1;
		for (int inventoryItem = 0; inventoryItem <= 255 && _vm->_currentInventoryItem == -1; inventoryItem++) {
			if (_vm->_inventoryItemStatus[inventoryItem] > 0)
				_vm->_currentInventoryItem = inventoryItem;
		}
	}	

	if (_vm->_currentInventoryItem >= 0)
		_vm->drawAnimatedIcon(_vm->_inventoryItemSprites, _vm->_currentInventoryItem, x, y, animFrameCounter);
	
}

void GuiCommandBar::handleCommandBar() {

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

	int commandBarStatus = 0;
	int animFrameCounter = 0;
	
	//_menuStatus++;
	
	_vm->waitForKeys();

	// TODO: copyScreens(vgaScreen, _sceneBackground);
	// TODO: copyScreens(vgaScreen, _workScreen);
	// TODO: setMouseCursor(1, 0);

	while (commandBarStatus == 0) {
		int mouseSelectedItem, commandBarAction = kCBANone;
	
		mouseSelectedItem = _vm->findRect(commandBarRects, _vm->_mouseX, _vm->_mouseY, commandBarItemCount + 1, kCBANone);
		if (mouseSelectedItem != kCBANone)
			_commandBarSelectedItem = mouseSelectedItem;
			
		drawCommandBar(_commandBarSelectedItem,	animFrameCounter++);		
		_vm->_screen->update();
		_vm->_system->delayMillis(40); // TODO
		
		_vm->handleEvents();

		if (_vm->_keyScancode == Common::KEYCODE_INVALID && !_vm->_leftButton && !_vm->_rightButton)
			continue;

		if (_vm->_rightButton) {
			commandBarAction = kCBAExit;
		} else if (_vm->_leftButton && _commandBarSelectedItem != kCBANone) {
			commandBarAction = _commandBarSelectedItem;
		}
		
		switch (_vm->_keyScancode) {
		case Common::KEYCODE_RIGHT:
			if (_commandBarSelectedItem == commandBarItemCount) {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem = 0;
			} else {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem++;
			}
			break;
		case Common::KEYCODE_LEFT:
			if (_commandBarSelectedItem == 0) {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem = commandBarItemCount;
			} else {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem--;
			}
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
		
		if (commandBarAction >= 0) {
			drawCommandBar(commandBarAction, animFrameCounter);		
			_vm->_screen->update();
		}
		
		switch (commandBarAction) {
		case kCBANone:
			break;
		case kCBAExit:
			commandBarStatus = 2;
			break;
		case kCBAVerbTalk:
			_vm->_cmdTalk = true;
			commandBarStatus = 1;
			break;
		case kCBAVerbGet:
			_vm->_cmdGet = true;
			commandBarStatus = 1;
			break;
		case kCBAVerbLook:
			_vm->_cmdLook = true;
			commandBarStatus = 1;
			break;
		case kCBAUseItem:
			_vm->useCurrentInventoryItem();
			commandBarStatus = 1;
			break;
		case kCBAInventory:
			commandBarStatus = _vm->_gui->runInventory();
			break;
		case kCBAMap:
			commandBarStatus = _vm->handleMap();
			break;
		case kCBAMenu:
			commandBarStatus = _vm->_gui->runMainMenu();//CHECKME
			break;
		}								

		_vm->waitForKeys();
	
	}

	_vm->waitForKeys();

	/* TODO ??
	_menuStatus--;
	loadSceneBackground();
	*/

}
	
/* GuiMainMenu */

GuiMainMenu::GuiMainMenu(CometEngine *vm) : _vm(vm), _mainMenuSelectedItem(0) {
}

GuiMainMenu::~GuiMainMenu() {
}

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
	
	//_menuStatus++;
	
	_vm->waitForKeys();

	while (mainMenuStatus == 0) {
		int mouseSelectedItem, mainMenuAction = kMMANone;
	
		mouseSelectedItem = _vm->findRect(mainMenuRects, _vm->_mouseX, _vm->_mouseY, 4, kMMANone);
		if (mouseSelectedItem != kMMANone)
			_mainMenuSelectedItem = mouseSelectedItem;
			
		drawMainMenu(_mainMenuSelectedItem);		
		_vm->_screen->update();
		_vm->_system->delayMillis(40); // TODO

		_vm->handleEvents();

		if (_vm->_keyScancode == Common::KEYCODE_INVALID && !_vm->_leftButton && !_vm->_rightButton)
			continue;

		if (_vm->_rightButton) {
			mainMenuAction = kMMAExit;
		} else if (_vm->_leftButton && _mainMenuSelectedItem != kMMANone) {
			mainMenuAction = _mainMenuSelectedItem;
		}
		
		switch (_vm->_keyScancode) {
		case Common::KEYCODE_DOWN:
			if (_mainMenuSelectedItem == 3) {
				if (mouseSelectedItem == _mainMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_mainMenuSelectedItem = 0;
			} else {
				if (mouseSelectedItem == _mainMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_mainMenuSelectedItem++;
			}
			break;
		case Common::KEYCODE_UP:
			if (_mainMenuSelectedItem == 0) {
				if (mouseSelectedItem == _mainMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_mainMenuSelectedItem = 3;
			} else {
				if (mouseSelectedItem == _mainMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
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
			// TODO
			debug("main menu: save game");
			mainMenuStatus = 0;
			break;
		case kMMALoad:
			// TODO
			debug("main menu: load game");
			mainMenuStatus = 0;
			break;
		case kMMAOptions:
			// TODO? debug("main menu: options");
			_vm->_gui->runOptionsMenu();
			mainMenuStatus = 0;
			break;
		case kMMAQuit:
			// TODO
			debug("main menu: quit");
			mainMenuStatus = 0;
			break;
		}								

		_vm->waitForKeys();
	
	}

	_vm->waitForKeys();

	//_menuStatus--;
	//loadSceneBackground();

	return 0;
}

void GuiMainMenu::drawMainMenu(int selectedItem) {
	const int x = 137;
	const int y = 65;
	const int itemHeight = 23;
	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 10, 0, 0);
	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 11, x, y + selectedItem * itemHeight);
}

/* GuiOptionsMenu */

GuiOptionsMenu::GuiOptionsMenu(CometEngine *vm) : _vm(vm), _optionsMenuSelectedItem(0) {
}

GuiOptionsMenu::~GuiOptionsMenu() {
}

int GuiOptionsMenu::run() {
#if 0
	int optionsMenuStatus = 0;
	while (optionsMenuStatus == 0) {
		drawOptionsMenu(1, 5, 5, 5, 5, 0, 1);
		_vm->_screen->update();
		_vm->_system->delayMillis(40); // TODO
		_vm->handleEvents();
		switch (_vm->_keyScancode) {
		case Common::KEYCODE_ESCAPE:
			optionsMenuStatus = 1;
			break;
		default:
			break;			
		}
		_vm->waitForKeys();
	}
#endif

	const int kOMANone			= -1;
	const int kOMAExit			= -2;
	const int kOMAMusicVol		= 0;
	const int kOMASoundVol		= 1;
	const int kOMATalkie		= 2;
	const int kOMAGameSpeed		= 3;
	const int kOMALanguage		= 4;
	const int kOMAOk			= 6;
	const int kOMADefMusicVol	= 7;
	const int kOMADefSoundVol	= 8;
	const int kOMADefGameSpeed	= 9;
	const int kOMAIncMusicVol	= 10;
	const int kOMADecMusicVol	= 11;
	const int kOMAIncSoundVol	= 12;
	const int kOMADecSoundVol	= 13;
	const int kOMAIncGameSpeed	= 14;
	const int kOMADecGameSpeed	= 15;
	const int kOMAIncLanguage	= 16;
	const int kOMADecLanguage	= 17;

	static const GuiRectangle optionsMenuRects[] = {
		{127,  64, 189,  79, kOMAMusicVol},
		{127,  84, 189,  99, kOMASoundVol},
		{127, 104, 164, 119, kOMATalkie},
		{127, 124, 189, 139, kOMAGameSpeed},
		{127, 144, 142, 159, kOMALanguage},
		{127, 164, 142, 179, 5},//???
		{172, 165, 199, 178, kOMAOk},
		{106,  64, 121,  79, kOMADefMusicVol},
		{106,  84, 121,  99, kOMADefSoundVol},
		{106, 104, 121, 119, kOMATalkie},
		{106, 124, 121, 139, kOMADefGameSpeed},
		{106, 144, 121, 159, kOMALanguage},
		{106, 164, 121, 179, 35}};//???

	int optionsMenuStatus = 0;
	int musicVolumeDiv, sampleVolumeDiv, textSpeed, gameSpeed, language;
	uint animFrameCounter = 0;
	
	// TODO: Get real values
	musicVolumeDiv = 2;
	sampleVolumeDiv = 2;
	textSpeed = 2;
	gameSpeed = 2;
	language = 1;
	
	//_menuStatus++;
	
	_vm->waitForKeys();

	while (optionsMenuStatus == 0) {
		int mouseSelectedItem, optionsMenuAction = kOMANone, selectedItemToDraw;
		bool doWaitForKeys = true;

		int16 mouseX = CLIP(_vm->_mouseX, 127, 189);
	
		mouseSelectedItem = _vm->findRect(optionsMenuRects, _vm->_mouseX, _vm->_mouseY, 13, kOMANone);
		if (mouseSelectedItem != kOMANone)
			_optionsMenuSelectedItem = mouseSelectedItem;
		
		// TODO: Update music volume
		
		animFrameCounter++;
		if (animFrameCounter == 32)
			animFrameCounter = 0;
		
		selectedItemToDraw = _optionsMenuSelectedItem;
		if (selectedItemToDraw == kOMADefMusicVol)
			selectedItemToDraw = kOMAMusicVol;
		else if (selectedItemToDraw == kOMADefSoundVol)
			selectedItemToDraw = kOMASoundVol;
		else if (selectedItemToDraw == kOMADefGameSpeed)
			selectedItemToDraw = kOMAGameSpeed;

		drawOptionsMenu(selectedItemToDraw, musicVolumeDiv, sampleVolumeDiv, textSpeed, gameSpeed, language, animFrameCounter, optionsMenuRects);

		_vm->_screen->update();
		_vm->_system->delayMillis(40); // TODO

		_vm->handleEvents();

		if (_vm->_keyScancode == Common::KEYCODE_INVALID && !_vm->_leftButton && !_vm->_rightButton)
			continue;

		if (_vm->_rightButton) {
			optionsMenuAction = kOMAExit;
		} else if (_vm->_leftButton && _optionsMenuSelectedItem != kOMANone) {
			optionsMenuAction = _optionsMenuSelectedItem;
		}
		
		switch (_vm->_keyScancode) {
		case Common::KEYCODE_DOWN:
			if (_optionsMenuSelectedItem == 5) {
				if (mouseSelectedItem == _optionsMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_optionsMenuSelectedItem = 0;
			} else {
				if (mouseSelectedItem == _optionsMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_optionsMenuSelectedItem++;
			}
			break;
		case Common::KEYCODE_UP:
			if (_optionsMenuSelectedItem == 0) {
				if (mouseSelectedItem == _optionsMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_optionsMenuSelectedItem = 5;
			} else {
				if (mouseSelectedItem == _optionsMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_optionsMenuSelectedItem--;
			}
			break;
		case Common::KEYCODE_LEFT:
			if (_optionsMenuSelectedItem == 0) {
				optionsMenuAction = kOMADecMusicVol;
			} else if (_optionsMenuSelectedItem == 1) {
				optionsMenuAction = kOMADecSoundVol;
			} else if (_optionsMenuSelectedItem == 2) {
				optionsMenuAction = kOMATalkie;
			} else if (_optionsMenuSelectedItem == 3) {
				optionsMenuAction = kOMADecGameSpeed;
			} else if (_optionsMenuSelectedItem == 4) {
				optionsMenuAction = kOMADecLanguage;
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (_optionsMenuSelectedItem == 0) {
				optionsMenuAction = kOMAIncMusicVol;
			} else if (_optionsMenuSelectedItem == 1) {
				optionsMenuAction = kOMAIncSoundVol;
			} else if (_optionsMenuSelectedItem == 2) {
				optionsMenuAction = kOMATalkie;
			} else if (_optionsMenuSelectedItem == 3) {
				optionsMenuAction = kOMAIncGameSpeed;
			} else if (_optionsMenuSelectedItem == 4) {
				optionsMenuAction = kOMAIncLanguage;
			}
			break;
		case Common::KEYCODE_ESCAPE:
			optionsMenuAction = kOMAExit;
			break;
		case Common::KEYCODE_RETURN:
			if (_optionsMenuSelectedItem == 5 || _optionsMenuSelectedItem == 6) {
				optionsMenuAction = kOMAExit;
			}
			break;			
		default:
			break;			
		}

		/*		
		if (mainMenuAction >= 0) {
			drawMainMenu(_mainMenuSelectedItem);		
			_vm->_screen->update();
		}
		*/
				
		switch (optionsMenuAction) {
		case kOMANone:
			break;
		case kOMAExit:
			optionsMenuStatus = 2;
			break;
		case kOMAMusicVol:
			musicVolumeDiv = (mouseX - 127) / 4;
			doWaitForKeys = false;
			break;
		case kOMASoundVol:
			sampleVolumeDiv = (mouseX - 127) / 4;
			doWaitForKeys = false;
			break;
		case kOMATalkie:
			_vm->_talkieMode++;
			if (_vm->_talkieMode > 2)
				_vm->_talkieMode = 0;
			break;
		case kOMAGameSpeed:
			gameSpeed = (mouseX - 127) / 4;
			doWaitForKeys = false;
			break;
		case kOMALanguage:
			if (language < 4)
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
			sampleVolumeDiv = 8;
			break;
		case kOMADefGameSpeed:
			gameSpeed = 8;
			break;
		case kOMAIncMusicVol:
			if (musicVolumeDiv < 15)
				musicVolumeDiv++;
			doWaitForKeys = false;
			break;
		case kOMADecMusicVol:
			if (musicVolumeDiv > 0)
				musicVolumeDiv--;
			doWaitForKeys = false;
			break;
		case kOMAIncSoundVol:
			if (sampleVolumeDiv < 15)
				sampleVolumeDiv++;
			doWaitForKeys = false;
			break;
		case kOMADecSoundVol:
			if (sampleVolumeDiv > 0)
				sampleVolumeDiv--;
			doWaitForKeys = false;
			break;
		case kOMAIncGameSpeed:
			if (gameSpeed < 15)
				gameSpeed++;
			doWaitForKeys = false;
			break;
		case kOMADecGameSpeed:
			if (gameSpeed > 0)
				gameSpeed--;
			doWaitForKeys = false;
			break;
		case kOMAIncLanguage:
			if (language < 4)
				language++;
			break;
		case kOMADecLanguage:
			if (language > 0)
				language--;
			break;
		}								

		if (doWaitForKeys)
			_vm->waitForKeys();
	
	}

	_vm->waitForKeys();

	//_menuStatus--;
	//loadSceneBackground();

	return 0;
}

void GuiOptionsMenu::drawOptionsMenu(int selectedItem, int musicVolumeDiv, int sampleVolumeDiv, 
	int textSpeed, int gameSpeed, int language, uint animFrameCounter,
	const GuiRectangle *guiRectangles) {

	const int x = 107;
	const int y_add_val3 = 65;
	const int y_index = 20;
	const int gaugeX = 128;
	const int y = 70;

    _vm->_screen->drawAnimationElement(_vm->_iconSprite, 25, 0, 0);

	if (selectedItem == 5 || selectedItem == 6) {
		_vm->_screen->frameRect(guiRectangles[6].x, guiRectangles[6].y, guiRectangles[6].x2, guiRectangles[6].y2, 119);
	} else {
    	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 28, x, selectedItem * y_index + y_add_val3);
	}

    _vm->_screen->drawAnimationElement(_vm->_iconSprite, 27, gaugeX + musicVolumeDiv * 4, y);
    _vm->_screen->drawAnimationElement(_vm->_iconSprite, 27, gaugeX + sampleVolumeDiv * 4, y + y_index);
    _vm->_screen->drawAnimationElement(_vm->_iconSprite, 27, gaugeX + gameSpeed * 4, y + y_index * 3);
    _vm->_screen->drawAnimationElement(_vm->_iconSprite, language + 32, 129, 157);

	if (musicVolumeDiv == 0) {
    	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 55, 0, 0);
	} else if (musicVolumeDiv > 0 && musicVolumeDiv < 8) {
		if (animFrameCounter < 16) {
    		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 71, 0, 0);
		} else {
    		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 72, 0, 0);
		}
	} else if (musicVolumeDiv >= 8) {
		if (animFrameCounter < 16) {
    		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 73, 0, 0);
		} else {
    		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 74, 0, 0);
		}
	}
		                                
	if (sampleVolumeDiv == 0) {
    	_vm->drawAnimatedIcon(_vm->_iconSprite, 1, 0, 0, animFrameCounter);
	} else if (sampleVolumeDiv < 7) {
    	_vm->drawAnimatedIcon(_vm->_iconSprite, 2, 0, 0, animFrameCounter);
	} else {
    	_vm->drawAnimatedIcon(_vm->_iconSprite, 3, 0, 0, animFrameCounter);
	} 

    _vm->_screen->drawAnimationElement(_vm->_iconSprite, _vm->_talkieMode + 79, 0, 0);

	if (gameSpeed < 5) {
    	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 75, 0, 0);
	} else if (gameSpeed >= 5 && gameSpeed < 10) {
    	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 76, 0, 0);
	} else if (gameSpeed >= 10) {
    	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 77, 0, 0);
	}

}

/* GuiTownMap */

GuiTownMap::GuiTownMap(CometEngine *vm) : _vm(vm) {
}

GuiTownMap::~GuiTownMap() {
}

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
	
	int mapStatus = 0;
	// TODO: Use Common::Rect
	int16 mapRectX1 = 64, mapRectX2 = 269;
	int16 mapRectY1 = 65, mapRectY2 = 187;
	int16 cursorAddX = 8, cursorAddY = 8;
	// Init map status values from script
	uint16 sceneBitMaskStatus = _vm->_scriptVars[2];
	uint16 sceneStatus1 = _vm->_scriptVars[3];
	uint16 sceneStatus2 = _vm->_scriptVars[4];
	int16 cursorX, cursorY;
	int16 locationNumber = _vm->_sceneNumber % 30;

	// seg002:33FB
	cursorX = mapPoints[locationNumber].x;
	cursorY = mapPoints[locationNumber].y;
	
	_vm->_system->warpMouse(cursorX, cursorY);

	// TODO: Copy vga screen to work screen...

	_vm->waitForKeys();

	// seg002:344D	
	while (mapStatus == 0) {

		int16 currMapLocation, selectedMapLocation;

		_vm->handleEvents();

		cursorX = CLIP(_vm->_mouseX, mapRectX1 + 1, mapRectX2 - 1);
		cursorY = CLIP(_vm->_mouseY, mapRectY1 + 1, mapRectY2 - 1);

		// seg002:34A7

		switch (_vm->_keyScancode) {
		case Common::KEYCODE_UP:
			cursorY = MAX(cursorY - cursorAddY, mapRectY1 + 1);
			break;
		case Common::KEYCODE_DOWN:
			cursorY = MIN(cursorY + cursorAddY, mapRectY2 - 1);
			break;
		case Common::KEYCODE_LEFT:
			cursorX = MAX(cursorX - cursorAddX, mapRectX1 + 1);
			break;
		case Common::KEYCODE_RIGHT:
			cursorX = MIN(cursorX + cursorAddX, mapRectX2 - 1);
			break;
		default:
			break;			
		}						
		
		if (_vm->_mouseX != cursorX || _vm->_mouseY != cursorY)
			_vm->_system->warpMouse(cursorX, cursorY);	

		// seg002:3545
		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 50, 0, 0);
		
		if (_vm->_keyScancode == Common::KEYCODE_ESCAPE || _vm->_rightButton) {
			mapStatus = 1;
		}
				
		// seg002:3572

		currMapLocation = -1;	
		selectedMapLocation = -1;

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
			if (_vm->_keyScancode == Common::KEYCODE_RETURN || _vm->_leftButton) {
				selectedMapLocation = currMapLocation;
			}
		} else {
			_vm->_screen->drawAnimationElement(_vm->_iconSprite, 51, cursorX, cursorY);
		}

		if (selectedMapLocation != -1) {
			// seg002:36DA
			const MapExit &mapExit = mapExits[selectedMapLocation];
			_vm->_moduleNumber = mapExit.moduleNumber;
			_vm->_sceneNumber = mapExit.sceneNumber;
			if (sceneStatus1 == 1) {
				_vm->_moduleNumber += 6;
			} else {
				_vm->_sceneNumber += (sceneStatus2 - 1) * 30;
			}
			if ((locationNumber == 7 || locationNumber == 8) &&
				_vm->_scriptVars[5] == 2 && _vm->_scriptVars[6] == 0 &&
				selectedMapLocation != 6 && selectedMapLocation != 7 && selectedMapLocation != 4) {
				_vm->_sceneNumber += 36;
			}
			mapStatus = 2;
			debug("moduleNumber: %d; sceneNumber: %d", _vm->_moduleNumber, _vm->_sceneNumber);
		}

		_vm->_screen->update();
		_vm->_system->delayMillis(40); // TODO

	}
	
	_vm->waitForKeys();

	return 1;
}

/* GuiDiary */

GuiDiary::GuiDiary(CometEngine *vm) : _vm(vm) {
}

GuiDiary::~GuiDiary() {
}

int GuiDiary::run() {
	return handleReadBook();
}

int GuiDiary::handleReadBook() {

	int currPageNumber = -1, pageNumber, pageCount, talkPageNumber = -1;
	int bookStatus = 0;

	// Use values from script; this is the most current diary entry
	pageNumber = _vm->_scriptVars[1];
	pageCount = _vm->_scriptVars[1];

	bookTurnPageTextEffect(false, pageNumber, pageCount);

	// Set speech file
	_vm->setVoiceFileIndex(7);

	while (bookStatus == 0/*TODO:check for quit*/) {

		if (currPageNumber != pageNumber) {
			drawBookPage(pageNumber, pageCount, 64);
			currPageNumber = pageNumber;
		}

		do {
			// Play page speech
			if (talkPageNumber != pageNumber) {
				if (pageNumber > 0) {
					_vm->playVoice(pageNumber);
				} else {
					_vm->stopVoice();
				}
				talkPageNumber = pageNumber;
			}
			// TODO: Check mouse rectangles
			_vm->handleEvents();
			_vm->_system->delayMillis(20); // TODO: Adjust or use fps counter
		} while (_vm->_keyScancode == Common::KEYCODE_INVALID && _vm->_keyDirection == 0/*TODO:check for quit*/);
		
		// TODO: Handle mouse rectangles
		
		switch (_vm->_keyScancode) {
		case Common::KEYCODE_RETURN:
			bookStatus = 1;
			break;
		case Common::KEYCODE_ESCAPE:
			bookStatus = 2;
			break;
		case Common::KEYCODE_LEFT:
			if (pageNumber > 0) {
				bookTurnPageTextEffect(true, pageNumber, pageCount);
				bookTurnPage(false);
				pageNumber--;
				bookTurnPageTextEffect(false, pageNumber, pageCount);
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (pageNumber < pageCount) {
				bookTurnPageTextEffect(true, pageNumber, pageCount);
				bookTurnPage(true);
				pageNumber++;
				bookTurnPageTextEffect(false, pageNumber, pageCount);
			}
			break;
		default:
			break;
		}

  		_vm->waitForKeys();

	}

	_vm->waitForKeys();
	_vm->stopVoice();
 	_vm->_textActive = false;

	_vm->setVoiceFileIndex(_vm->_narFileIndex);

	return 2 - bookStatus;

}

void GuiDiary::drawBookPage(int pageTextIndex, int pageTextMaxIndex, byte fontColor) {

	int xadd = 58, yadd = 48, x = 0, lineNumber = 0;
	char pageNumberString[10];
	int pageNumberStringWidth;

	byte *pageText = _vm->_textReader->getString(2, pageTextIndex);
	
	_vm->_screen->drawAnimationElement(_vm->_iconSprite, 30, 0, 0);
	if (pageTextIndex < pageTextMaxIndex)
		_vm->_screen->drawAnimationElement(_vm->_iconSprite, 37, 0, 0);
		
	_vm->_screen->setFontColor(58);

	snprintf(pageNumberString, 10, "- %d -", pageTextIndex * 2 + 1);
	pageNumberStringWidth = _vm->_screen->getTextWidth((byte*)pageNumberString);
	_vm->_screen->drawText(xadd + (106 - pageNumberStringWidth) / 2, 180, (byte*)pageNumberString);
	
 	snprintf(pageNumberString, 10, "- %d -", pageTextIndex * 2 + 2);
	pageNumberStringWidth = _vm->_screen->getTextWidth((byte*)pageNumberString);
	_vm->_screen->drawText(xadd + 115 + (106 - pageNumberStringWidth) / 2, 180, (byte*)pageNumberString);
	
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

void GuiDiary::bookTurnPage(bool turnDirection) {
	if (turnDirection) {
		for (uint i = 38; i < 49; i++) {
			_vm->_screen->drawAnimationElement(_vm->_iconSprite, 30, 0, 0);
			_vm->_screen->drawAnimationElement(_vm->_iconSprite, i, 0, 0);
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
	} else {
		for (uint i = 49; i > 38; i--) {
			_vm->_screen->drawAnimationElement(_vm->_iconSprite, 30, 0, 0);
			_vm->_screen->drawAnimationElement(_vm->_iconSprite, i, 0, 0);
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
	}
}

void GuiDiary::bookTurnPageTextEffect(bool turnDirection, int pageTextIndex, int pageTextMaxIndex) {
	if (turnDirection) {
		for (byte fontColor = 64; fontColor < 72; fontColor++) {
			drawBookPage(pageTextIndex, pageTextMaxIndex, fontColor);
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
	} else {
		for (byte fontColor = 72; fontColor > 64; fontColor--) {
			drawBookPage(pageTextIndex, pageTextMaxIndex, fontColor);
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
	}
}

/* GuiPuzzle */

GuiPuzzle::GuiPuzzle(CometEngine *vm) : _vm(vm) {
}

GuiPuzzle::~GuiPuzzle() {
}

int GuiPuzzle::run() {
	return runPuzzle();
}

int GuiPuzzle::runPuzzle() {

#define PUZZLE_CHEAT
#ifdef PUZZLE_CHEAT
	static const uint16 puzzleInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4, 8,13, 0},
		{0, 1, 5, 9,14, 0},
		{0, 2, 6,10,15, 0},
		{0, 3, 7,11,12, 0},
		{0, 0, 0, 0, 0, 0}};
#else
	static const uint16 puzzleInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4,10,15, 0},
		{0, 5,11,13,12, 0},
		{0, 8, 1, 2, 6, 0},
		{0, 3, 7, 9,14, 0},
		{0, 0, 0, 0, 0, 0}};
#endif
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

	_puzzleSprite = _vm->_animationMan->loadAnimationResource("A07.PAK", 24);

	// Initialize the puzzle state
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			_puzzleTiles[i][j] = puzzleInitialTiles[i][j];

	_puzzleCursorX = 0;
	_puzzleCursorY = 0;

	while (puzzleStatus == 0) {

		int selectedTile;

		_vm->handleEvents();

		_puzzleCursorX = CLIP(_vm->_mouseX, 103, 231);
		_puzzleCursorY = CLIP(_vm->_mouseY, 44, 171);

		if (_vm->_mouseX != _puzzleCursorX || _vm->_mouseY != _puzzleCursorY)
			_vm->_system->warpMouse(_puzzleCursorX, _puzzleCursorY);

		selectedTile = _vm->findRect(puzzleTileRects, _vm->_mouseX, _vm->_mouseY, 21, -1);
		if (selectedTile >= 0) {
			if (selectedTile >= 0 && selectedTile < 20) {
				_puzzleTableColumn = rectToColRow[selectedTile].col;
				_puzzleTableRow = rectToColRow[selectedTile].row;
			} else if (selectedTile == 20) {
				_puzzleTableColumn = (_puzzleCursorX - 119) / 24 + 1;
				_puzzleTableRow = (_puzzleCursorY - 60) / 24 + 1; 
			} else {
				_puzzleTableColumn = 0;
				_puzzleTableRow = 0;
			}
		}

		drawField();
		_vm->_screen->update();
		_vm->_system->delayMillis(40); // TODO

		if (_vm->_keyScancode != Common::KEYCODE_INVALID) {
			
			bool selectionChanged = false;

			switch (_vm->_keyScancode) {
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
			
			if (selectionChanged) {
				selectedTile = 20;
				if (_puzzleTableColumn == 0 && _puzzleTableRow == 0)
					selectedTile = 16;
				else if (_puzzleTableColumn == 5 && _puzzleTableRow == 0)
					selectedTile = 17;
				else if (_puzzleTableColumn == 5 && _puzzleTableRow == 5)
					selectedTile = 18;
				else if (_puzzleTableColumn == 0 && _puzzleTableRow == 5)
					selectedTile = 19;
				else if (_puzzleTableColumn > 0 && _puzzleTableColumn < 5 && _puzzleTableRow == 0)
					selectedTile = _puzzleTableColumn - 1;
				else if (_puzzleTableColumn > 0 && _puzzleTableColumn < 5 && _puzzleTableRow == 5)
					selectedTile = _puzzleTableColumn + 7;
				else if (_puzzleTableRow > 0 && _puzzleTableRow < 5 && _puzzleTableColumn == 0)
					selectedTile = _puzzleTableRow + 11;
				else if (_puzzleTableRow > 0 && _puzzleTableRow < 5 && _puzzleTableColumn == 5)
					selectedTile = _puzzleTableRow + 3;

				if (selectedTile != 20) {
					_puzzleCursorX = (puzzleTileRects[selectedTile].x + puzzleTileRects[selectedTile].x2) / 2;
					_puzzleCursorY = (puzzleTileRects[selectedTile].y + puzzleTileRects[selectedTile].y2) / 2;
				} else {
					_puzzleCursorX = (_puzzleTableColumn - 1) * 24 + 130;
					_puzzleCursorY = (_puzzleTableRow - 1) * 24 + 71;
				}
				
				// Mouse warp to selected tile
				_vm->_system->warpMouse(_puzzleCursorX, _puzzleCursorY);

			}

		}

		if (_vm->_keyScancode == Common::KEYCODE_ESCAPE || _vm->_rightButton) {
			puzzleStatus = 1;
		} else if (_vm->_keyScancode == Common::KEYCODE_RETURN || _vm->_leftButton) {
			if (_puzzleTableColumn == 0 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				// TODO: playSampleFlag, play sample
				moveTileRow(_puzzleTableRow, -1);
			} else if (_puzzleTableColumn == 5 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				// TODO: playSampleFlag, play sample
				moveTileRow(_puzzleTableRow, 1);
			} else if (_puzzleTableRow == 0 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				// TODO: playSampleFlag, play sample
				moveTileColumn(_puzzleTableColumn, -1);
			} else if (_puzzleTableRow == 5 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				// TODO: playSampleFlag, play sample
				moveTileColumn(_puzzleTableColumn, 1);
			}
			if (testIsSolved())
				puzzleStatus = 2;
		} else {
			_vm->waitForKeys();
		}				
			
	}		

	delete _puzzleSprite;

	return puzzleStatus == 2 ? 2 : 0;

}

void GuiPuzzle::drawFinger() {
	_vm->_screen->drawAnimationElement(_puzzleSprite, 18, _puzzleCursorX, _puzzleCursorY);
}

void GuiPuzzle::drawField() {
	// TODO ??: memcpy(_sceneBackground, _screen->getScreen(), 320 * 200);
	_vm->_screen->drawAnimationElement(_puzzleSprite, 17, 0, 0);
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++) {
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++) {
			drawTile(columnIndex, rowIndex, 0, 0);		
		}
	}
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
			for (int rowIndex = 1; rowIndex <= 5; rowIndex++) {
				drawTile(columnIndex, rowIndex, 0, -yOffs);				
			}
			_vm->_screen->setClipY(0, 199);
			drawFinger();
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
		for (int rowIndex = 0; rowIndex <= 4; rowIndex++) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex + 1];
		}
		_puzzleTiles[columnIndex][5] = _puzzleTiles[columnIndex][1];
	} else {
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
		for (int yOffs = 0; yOffs < 24; yOffs += 2) {
			_vm->_screen->setClipY(60, 156);
			for (int rowIndex = 0; rowIndex <= 4; rowIndex++) {
				drawTile(columnIndex, rowIndex, 0, yOffs);				
			}
			_vm->_screen->setClipY(0, 199);
			drawFinger();
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
		for (int rowIndex = 5; rowIndex >= 1; rowIndex--) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex - 1];
		}
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
	}
}

void GuiPuzzle::moveTileRow(int rowIndex, int direction) {
	if (direction < 0) {
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_vm->_screen->setClipX(120, 215);
			for (int columnIndex = 1; columnIndex <= 5; columnIndex++) {
				drawTile(columnIndex, rowIndex, -xOffs, 0);				
			}
			_vm->_screen->setClipX(0, 319);
			drawFinger();
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
		for (int columnIndex = 0; columnIndex <= 4; columnIndex++) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex + 1][rowIndex];
		}
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
	} else {
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_vm->_screen->setClipX(120, 215);
			for (int columnIndex = 0; columnIndex <= 4; columnIndex++) {
				drawTile(columnIndex, rowIndex, xOffs, 0);				
			}
			_vm->_screen->setClipX(0, 319);
			drawFinger();
			_vm->_screen->update();
			_vm->_system->delayMillis(40); // TODO
		}
		for (int columnIndex = 5; columnIndex >= 1; columnIndex--) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex - 1][rowIndex];
		}
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
	}
}

bool GuiPuzzle::testIsSolved() {
	int matchingTiles = 0;
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++) {
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++) {
			if (_puzzleTiles[columnIndex][rowIndex] == (rowIndex - 1) * 4 + (columnIndex - 1))
				matchingTiles++;					
		}
	}
	return matchingTiles == 16;
}

} // End of namespace Comet
