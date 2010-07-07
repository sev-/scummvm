#include "comet/comet.h"
#include "comet/screen.h"
#include "comet/text.h"

namespace Comet {

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

void CometEngine::useCurrentInventoryItem() {

	for (uint index = 0; index < 256; index++) {
		if (_inventoryItemStatus[index] == 2)
			_inventoryItemStatus[index] = 1;
	}
	
	if (_currentInventoryItem != -1) {
		if (_inventoryItemStatus[_currentInventoryItem] == 1)
			_inventoryItemStatus[_currentInventoryItem] = 2;
	}

}

void CometEngine::checkCurrentInventoryItem() {

	/* If the currently selected item was disabled, scan for the preceeding item
		and set it as selected item. */
	if (_currentInventoryItem >= 0 && _inventoryItemStatus[_currentInventoryItem] == 0) {
		if (_currentInventoryItem >= 1) {
			for (_currentInventoryItem = _currentInventoryItem - 1; _currentInventoryItem >= 0 && _inventoryItemStatus[_currentInventoryItem] == 0;
				_currentInventoryItem--) {
			}
		} else {
			_currentInventoryItem = -1;
		}
	}

	/* Check if the player wants to read the notebook */
	if (_inventoryItemStatus[0] == 2) {
		handleReadBook();
		_inventoryItemStatus[0] = 1;
	}

}

} // End of namespace Comet
