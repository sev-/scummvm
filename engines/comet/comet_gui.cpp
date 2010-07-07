#include "comet/comet.h"
#include "comet/animationmgr.h"
#include "comet/font.h"
#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/screen.h"

namespace Comet {

/* Inventory */

int CometEngine::handleInventory() {

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

	waitForKeys();
		
	// Build items array and set up variables
	for (int i = 0; i < 256; i++) {
		if (_inventoryItemStatus[i] >= 1) {
			items.push_back(i);
			if (i == _currentInventoryItem) {
				firstItem = items.size() < 5 ? 0 : items.size() - 5;
				currentItem = items.size() - 1;
			}
		}
	}

	while (inventoryStatus == 0) {
		int inventoryAction = kIANone, mouseSelectedItem;
			
		handleEvents();

		mouseSelectedItem = findRect(inventorySlotRects, _mouseX, _mouseY, MIN<int>(items.size() - firstItem, 10) + 2, kIANone);
			
		if (mouseSelectedItem >= 0) {
			currentItem = firstItem + mouseSelectedItem;
		}			
	
		drawInventory(items, firstItem, currentItem, animFrameCounter++);

		_screen->update();
		_system->delayMillis(40); // TODO: Adjust or use fps counter

		if (_rightButton) {
			inventoryAction = kIAExit;
		} else if (_leftButton) {
			if (mouseSelectedItem >= 0)
				inventoryAction = kIAUse;
			else if (mouseSelectedItem != kIANone)
				inventoryAction = mouseSelectedItem;				
		}

		switch (_keyScancode) {
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
				if (_inventoryItemStatus[i] == 2)
					_inventoryItemStatus[i] = 1;
			}
			_currentInventoryItem = items[currentItem];
			// Return just selects, U actually uses the item
			if (inventoryAction == kIAUse) {
				//debug("Use item #%d", _currentInventoryItem);
				_inventoryItemStatus[_currentInventoryItem] = 2;
			}
			inventoryStatus = 1;
			break;
		default:
			break;
		}
		
  		waitForKeys();
	}
	
	// TODO...

	return 2 - inventoryStatus;;
}

void CometEngine::drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter) {

	const uint kMaxItemsOnScreen = 10;

	uint xadd = 74, yadd = 64, itemHeight = 12;

	_screen->drawAnimationElement(_iconSprite, 16, 0, 0);

	// Draw up arrow
	if (firstItem > 0)
		_screen->drawAnimationElement(_iconSprite, 53, 0, 0);

	// Draw down arrow
	if (firstItem + kMaxItemsOnScreen < items.size())
		_screen->drawAnimationElement(_iconSprite, 52, 0, 0);

	for (uint itemIndex = 0; (itemIndex < kMaxItemsOnScreen) && (firstItem + itemIndex < items.size()); itemIndex++) {
		byte *itemName = _inventoryItemNames->getString(items[firstItem + itemIndex]);
		int itemX = xadd + 21, itemY = yadd + itemHeight * itemIndex;
		_screen->setFontColor(120);
		_screen->drawText(itemX, itemY, itemName);
		_screen->setFontColor(119);
		_screen->drawText(itemX + 1, itemY + 1, itemName);
		drawAnimatedIcon(_inventoryItemSprites, items[firstItem + itemIndex], xadd, yadd + itemHeight * itemIndex - 3, animFrameCounter);
	}
	
	if (items.size() > 0) {
		int selectionY = yadd + (currentItem - firstItem) * itemHeight - 1;
		_screen->frameRect(xadd + 16, selectionY, 253, selectionY + itemHeight - 1, _invSelectionColor);
		_invSelectionColor++;
		if (_invSelectionColor >= 96)
			_invSelectionColor = 80;
	}

}

/* Command bar */

void CometEngine::drawCommandBar(int selectedItem, int animFrameCounter) {

	const int x = 196;
	const int y = 14;

	_screen->drawAnimationElement(_iconSprite, 0, 0, 0);
	_screen->drawAnimationElement(_iconSprite, selectedItem + 1, 0, 0);

	if (_currentInventoryItem >= 0 && _inventoryItemStatus[_currentInventoryItem] == 0) {
		_currentInventoryItem = -1;
		for (int inventoryItem = 0; inventoryItem <= 255 && _currentInventoryItem == -1; inventoryItem++) {
			if (_inventoryItemStatus[inventoryItem] > 0)
				_currentInventoryItem = inventoryItem;
		}
	}	

	if (_currentInventoryItem >= 0)
		drawAnimatedIcon(_inventoryItemSprites, _currentInventoryItem, x, y, animFrameCounter);
	
}

void CometEngine::handleCommandBar() {

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
	
	_menuStatus++;
	
	waitForKeys();

	// TODO: copyScreens(vgaScreen, _sceneBackground);
	// TODO: copyScreens(vgaScreen, _workScreen);
	// TODO: setMouseCursor(1, 0);

	_commandBarSelectedItem = kCBANone;

	while (commandBarStatus == 0) {
		int mouseSelectedItem, commandBarAction = kCBANone;
	
		mouseSelectedItem = findRect(commandBarRects, _mouseX, _mouseY, commandBarItemCount + 1, kCBANone);
		if (mouseSelectedItem != kCBANone)
			_commandBarSelectedItem = mouseSelectedItem;
			
		drawCommandBar(_commandBarSelectedItem,	animFrameCounter++);		
		_screen->update();
		_system->delayMillis(40); // TODO

		handleEvents();

		if (_keyScancode == Common::KEYCODE_INVALID && !_leftButton && !_rightButton)
			continue;

		if (_rightButton) {
			commandBarAction = kCBAExit;
		} else if (_leftButton && _commandBarSelectedItem != kCBANone) {
			commandBarAction = _commandBarSelectedItem;
		}
		
		switch (_keyScancode) {
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
			_screen->update();
		}
		
		switch (commandBarAction) {
		case kCBANone:
			break;
		case kCBAExit:
			commandBarStatus = 2;
			break;
		case kCBAVerbTalk:
			_cmdTalk = true;
			commandBarStatus = 1;
			break;
		case kCBAVerbGet:
			_cmdGet = true;
			commandBarStatus = 1;
			break;
		case kCBAVerbLook:
			_cmdLook = true;
			commandBarStatus = 1;
			break;
		case kCBAUseItem:
			useCurrentInventoryItem();
			commandBarStatus = 1;
			break;
		case kCBAInventory:
			commandBarStatus = handleInventory();
			break;
		case kCBAMap:
			commandBarStatus = handleMap();
			break;
		case kCBAMenu:
			// TODO: Disk menu
			break;
		}								

		waitForKeys();
	
	}

	waitForKeys();

	_menuStatus--;

	loadSceneBackground();

}
	
/* Disk menu */

void CometEngine::drawDiskMenu(int selectedItem) {

	const int x = 137;
	const int y = 65;
	const int itemHeight = 23;

	_screen->drawAnimationElement(_iconSprite, 10, 0, 0);
	_screen->drawAnimationElement(_iconSprite, 11, x, y + selectedItem * itemHeight);

}

int CometEngine::handleDiskMenu() {
	
	const int kDMANone		= -1;
	const int kDMAExit		= -2;
	const int kDMASave		= 0;
	const int kDMALoad		= 1;
	const int kDMAOptions	= 2;
	const int kDMAQuit		= 3;

	static const GuiRectangle diskMenuRects[] = {
		{136,  64, 184,  80, kDMASave},
		{136,  87, 184, 103, kDMALoad},
		{136, 110, 184, 126, kDMAOptions},
		{136, 133, 184, 149, kDMAQuit}};		

	int diskMenuStatus = 0;
	
	_menuStatus++;
	
	waitForKeys();

	// TODO

	_diskMenuSelectedItem = kDMASave;

	while (diskMenuStatus == 0) {
		int mouseSelectedItem, diskMenuAction = kDMANone;
	
		mouseSelectedItem = findRect(diskMenuRects, _mouseX, _mouseY, 4, kDMANone);
		if (mouseSelectedItem != kDMANone)
			_diskMenuSelectedItem = mouseSelectedItem;
			
		drawDiskMenu(_diskMenuSelectedItem);		
		_screen->update();
		_system->delayMillis(40); // TODO

		handleEvents();

		if (_keyScancode == Common::KEYCODE_INVALID && !_leftButton && !_rightButton)
			continue;

		if (_rightButton) {
			diskMenuAction = kDMAExit;
		} else if (_leftButton && _diskMenuSelectedItem != kDMANone) {
			diskMenuAction = _diskMenuSelectedItem;
		}
		
		switch (_keyScancode) {
		case Common::KEYCODE_DOWN:
			if (_diskMenuSelectedItem == 3) {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem = 0;
			} else {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem++;
			}
			break;
		case Common::KEYCODE_UP:
			if (_diskMenuSelectedItem == 0) {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem = 3;
			} else {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem--;
			}
			break;
		case Common::KEYCODE_ESCAPE:
			diskMenuAction = kDMAExit;
			break;
		case Common::KEYCODE_RETURN:
			diskMenuAction = _diskMenuSelectedItem;
			break;			
		case Common::KEYCODE_s:
			diskMenuAction = kDMASave;
			break;
		case Common::KEYCODE_l:
			diskMenuAction = kDMALoad;
			break;
		case Common::KEYCODE_t:
			diskMenuAction = kDMAOptions;
			break;
		case Common::KEYCODE_x:
			diskMenuAction = kDMAQuit;
			break;
		default:
			break;			
		}
		
		if (diskMenuAction >= 0) {
			drawDiskMenu(_diskMenuSelectedItem);		
			_screen->update();
		}
		
		switch (diskMenuAction) {
		case kDMANone:
			break;
		case kDMAExit:
			diskMenuStatus = 2;
			break;
		case kDMASave:
			// TODO
			debug("disk menu: save game");
			diskMenuStatus = 0;
			break;
		case kDMALoad:
			// TODO
			debug("disk menu: load game");
			diskMenuStatus = 0;
			break;
		case kDMAOptions:
			// TODO
			debug("disk menu: options");
			diskMenuStatus = 0;
			break;
		case kDMAQuit:
			// TODO
			debug("disk menu: quit");
			diskMenuStatus = 0;
			break;
		}								

		waitForKeys();
	
	}

	waitForKeys();

	_menuStatus--;

	loadSceneBackground();

	return 0;
}

/* Town map */

int CometEngine::updateMap() {

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
	uint16 sceneBitMaskStatus = _scriptVars[2];
	uint16 sceneStatus1 = _scriptVars[3];
	uint16 sceneStatus2 = _scriptVars[4];
	int16 cursorX, cursorY;
	int16 locationNumber = _sceneNumber % 30;

	// seg002:33FB
	cursorX = mapPoints[locationNumber].x;
	cursorY = mapPoints[locationNumber].y;
	
	_system->warpMouse(cursorX, cursorY);

	// TODO: Copy vga screen to work screen...

	waitForKeys();

	// seg002:344D	
	while (mapStatus == 0) {

		int16 currMapLocation, selectedMapLocation;

		handleEvents();

		if (_mouseX > mapRectX1 && _mouseX < mapRectX2) {
			cursorX = _mouseX;
		} else if (_mouseX < mapRectX2) {
			cursorX = mapRectX1 + 1;
		} else {
			cursorX = mapRectX2 - 1;
		}			
		
		if (_mouseY > mapRectY1 && _mouseY < mapRectY2) {
			cursorY = _mouseY;
		} else if (_mouseY < mapRectY2) {
			cursorY = mapRectY1 + 1;
		} else {
			cursorY = mapRectY2 - 1;
		}			
	
		// seg002:34A7

		switch (_keyScancode) {
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
		
		if (_mouseX != cursorX || _mouseY != cursorY)
			_system->warpMouse(cursorX, cursorY);	

		// seg002:3545
		_screen->drawAnimationElement(_iconSprite, 50, 0, 0);
		
		if (_keyScancode == Common::KEYCODE_ESCAPE || _rightButton) {
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
			byte *locationName = _textReader->getString(2, 40 + currMapLocation);
			_screen->drawTextOutlined(MIN(cursorX - 2, 283 - _screen->_font->getTextWidth(locationName)), 
				cursorY - 6, locationName, 119, 120);
			if (_keyScancode == Common::KEYCODE_RETURN || _leftButton) {
				selectedMapLocation = currMapLocation;
			}
		} else {
			_screen->drawAnimationElement(_iconSprite, 51, cursorX, cursorY);
		}

		if (selectedMapLocation != -1) {
			// seg002:36DA
			const MapExit &mapExit = mapExits[selectedMapLocation];
			_moduleNumber = mapExit.moduleNumber;
			_sceneNumber = mapExit.sceneNumber;
			if (sceneStatus1 == 1) {
				_moduleNumber += 6;
			} else {
				_sceneNumber += (sceneStatus2 - 1) * 30;
			}
			if ((locationNumber == 7 || locationNumber == 8) &&
				_scriptVars[5] == 2 && _scriptVars[6] == 0 &&
				selectedMapLocation != 6 && selectedMapLocation != 7 && selectedMapLocation != 4) {
				_sceneNumber += 36;
			}
			mapStatus = 2;
			debug("moduleNumber: %d; sceneNumber: %d", _moduleNumber, _sceneNumber);
		}

		_screen->update();
		_system->delayMillis(40); // TODO

	}
	
	waitForKeys();

	return 1;
}

int CometEngine::handleMap() {

	// TODO: Proper implementation

	return updateMap();

}

/* Diary */

int CometEngine::handleReadBook() {

	int currPageNumber = -1, pageNumber, pageCount, talkPageNumber = -1;
	int bookStatus = 0;

	// Use values from script; this is the most current diary entry
	pageNumber = _scriptVars[1];
	pageCount = _scriptVars[1];

	bookTurnPageTextEffect(false, pageNumber, pageCount);

	// Set speech file
	setVoiceFileIndex(7);

	while (bookStatus == 0/*TODO:check for quit*/) {

		if (currPageNumber != pageNumber) {
			drawBookPage(pageNumber, pageCount, 64);
			currPageNumber = pageNumber;
		}

		do {
			// Play page speech
			if (talkPageNumber != pageNumber) {
				if (pageNumber > 0) {
					playVoice(pageNumber);
				} else {
					stopVoice();
				}
				talkPageNumber = pageNumber;
			}
			// TODO: Check mouse rectangles
			handleEvents();
			_system->delayMillis(20); // TODO: Adjust or use fps counter
		} while (_keyScancode == Common::KEYCODE_INVALID && _keyDirection == 0/*TODO:check for quit*/);
		
		// TODO: Handle mouse rectangles
		
		switch (_keyScancode) {
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

  		waitForKeys();

	}

	waitForKeys();
	stopVoice();
 	_textActive = false;

	setVoiceFileIndex(_narFileIndex);

	return 2 - bookStatus;

}

void CometEngine::drawBookPage(int pageTextIndex, int pageTextMaxIndex, byte fontColor) {

	int xadd = 58, yadd = 48, x = 0, lineNumber = 0;
	char pageNumberString[10];
	int pageNumberStringWidth;

	byte *pageText = _textReader->getString(2, pageTextIndex);
	
	_screen->drawAnimationElement(_iconSprite, 30, 0, 0);
	if (pageTextIndex < pageTextMaxIndex)
		_screen->drawAnimationElement(_iconSprite, 37, 0, 0);
		
	_screen->setFontColor(58);

	snprintf(pageNumberString, 10, "- %d -", pageTextIndex * 2 + 1);
	pageNumberStringWidth = _screen->_font->getTextWidth((byte*)pageNumberString);
	_screen->drawText(xadd + (106 - pageNumberStringWidth) / 2, 180, (byte*)pageNumberString);
	
 	snprintf(pageNumberString, 10, "- %d -", pageTextIndex * 2 + 2);
	pageNumberStringWidth = _screen->_font->getTextWidth((byte*)pageNumberString);
	_screen->drawText(xadd + 115 + (106 - pageNumberStringWidth) / 2, 180, (byte*)pageNumberString);
	
	_screen->setFontColor(fontColor);
	
	while (*pageText != 0 && *pageText != '*') {
		x = MAX(xadd + (106 - _screen->_font->getTextWidth(pageText)) / 2, 0);
		_screen->drawText(x, yadd + lineNumber * 10, pageText);
		if (++lineNumber == 13) {
			xadd += 115;
			yadd -= 130;
		}
		while (*pageText != 0 && *pageText != '*')
			pageText++;
		pageText++;
	}

}

void CometEngine::bookTurnPage(bool turnDirection) {
	if (turnDirection) {
		for (uint i = 38; i < 49; i++) {
			_screen->drawAnimationElement(_iconSprite, 30, 0, 0);
			_screen->drawAnimationElement(_iconSprite, i, 0, 0);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	} else {
		for (uint i = 49; i > 38; i--) {
			_screen->drawAnimationElement(_iconSprite, 30, 0, 0);
			_screen->drawAnimationElement(_iconSprite, i, 0, 0);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	}
}

void CometEngine::bookTurnPageTextEffect(bool turnDirection, int pageTextIndex, int pageTextMaxIndex) {
	if (turnDirection) {
		for (byte fontColor = 64; fontColor < 72; fontColor++) {
			drawBookPage(pageTextIndex, pageTextMaxIndex, fontColor);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	} else {
		for (byte fontColor = 72; fontColor > 64; fontColor--) {
			drawBookPage(pageTextIndex, pageTextMaxIndex, fontColor);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	}
}

/* Puzzle */

int CometEngine::runPuzzle() {

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

	_puzzleSprite = _animationMan->loadAnimationResource("A07.PAK", 24);

	// Initialize the puzzle state
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			_puzzleTiles[i][j] = puzzleInitialTiles[i][j];

	_puzzleCursorX = 0;
	_puzzleCursorY = 0;

	while (puzzleStatus == 0) {

		int selectedTile;

		handleEvents();

		_puzzleCursorX = CLIP(_mouseX, 103, 231);
		_puzzleCursorY = CLIP(_mouseY, 44, 171);

		if (_mouseX != _puzzleCursorX || _mouseY != _puzzleCursorY)
			_system->warpMouse(_puzzleCursorX, _puzzleCursorY);

		selectedTile = findRect(puzzleTileRects, _mouseX, _mouseY, 21, -1);
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

		puzzleDrawField();
		_screen->update();
		_system->delayMillis(40); // TODO

		if (_keyScancode != Common::KEYCODE_INVALID) {
			
			bool selectionChanged = false;

			switch (_keyScancode) {
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
				_system->warpMouse(_puzzleCursorX, _puzzleCursorY);

			}

		}

		if (_keyScancode == Common::KEYCODE_ESCAPE || _rightButton) {
			puzzleStatus = 1;
		} else if (_keyScancode == Common::KEYCODE_RETURN || _leftButton) {
			if (_puzzleTableColumn == 0 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileRow(_puzzleTableRow, -1);
			} else if (_puzzleTableColumn == 5 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileRow(_puzzleTableRow, 1);
			} else if (_puzzleTableRow == 0 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileColumn(_puzzleTableColumn, -1);
			} else if (_puzzleTableRow == 5 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileColumn(_puzzleTableColumn, 1);
			}
			if (puzzleTestIsSolved())
				puzzleStatus = 2;
		} else {
			waitForKeys();
		}				
			
	}		

	delete _puzzleSprite;

	return puzzleStatus == 2 ? 2 : 0;

}

void CometEngine::puzzleDrawFinger() {
	_screen->drawAnimationElement(_puzzleSprite, 18, _puzzleCursorX, _puzzleCursorY);
}

void CometEngine::puzzleDrawField() {
	memcpy(_sceneBackground, _screen->getScreen(), 320 * 200);
	_screen->drawAnimationElement(_puzzleSprite, 17, 0, 0);
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++) {
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++) {
			puzzleDrawTile(columnIndex, rowIndex, 0, 0);		
		}
	}
	puzzleDrawFinger();
}

void CometEngine::puzzleDrawTile(int columnIndex, int rowIndex, int xOffs, int yOffs) {
	_screen->drawAnimationElement(_puzzleSprite, _puzzleTiles[columnIndex][rowIndex], 
		119 + (columnIndex - 1) * 24 + xOffs, 60 + (rowIndex - 1) * 24 + yOffs);
}

void CometEngine::puzzleMoveTileColumn(int columnIndex, int direction) {
	if (direction < 0) {
		_puzzleTiles[columnIndex][5] = _puzzleTiles[columnIndex][1];
		for (int yOffs = 0; yOffs < 24; yOffs += 2) {
			_screen->setClipY(60, 156);
			for (int rowIndex = 1; rowIndex <= 5; rowIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, 0, -yOffs);				
			}
			_screen->setClipY(0, 199);
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int rowIndex = 0; rowIndex <= 4; rowIndex++) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex + 1];
		}
		_puzzleTiles[columnIndex][5] = _puzzleTiles[columnIndex][1];
	} else {
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
		for (int yOffs = 0; yOffs < 24; yOffs += 2) {
			_screen->setClipY(60, 156);
			for (int rowIndex = 0; rowIndex <= 4; rowIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, 0, yOffs);				
			}
			_screen->setClipY(0, 199);
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int rowIndex = 5; rowIndex >= 1; rowIndex--) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex - 1];
		}
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
	}
}

void CometEngine::puzzleMoveTileRow(int rowIndex, int direction) {
	if (direction < 0) {
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_screen->setClipX(120, 215);
			for (int columnIndex = 1; columnIndex <= 5; columnIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, -xOffs, 0);				
			}
			_screen->setClipX(0, 319);
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int columnIndex = 0; columnIndex <= 4; columnIndex++) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex + 1][rowIndex];
		}
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
	} else {
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_screen->setClipX(120, 215);
			for (int columnIndex = 0; columnIndex <= 4; columnIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, xOffs, 0);				
			}
			_screen->setClipX(0, 319);
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int columnIndex = 5; columnIndex >= 1; columnIndex--) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex - 1][rowIndex];
		}
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
	}
}

bool CometEngine::puzzleTestIsSolved() {
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
