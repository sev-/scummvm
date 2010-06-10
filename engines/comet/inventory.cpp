#include "comet/comet.h"
#include "comet/screen.h"

namespace Comet {

int CometEngine::handleInventory() {

	Common::Array<uint16> items;
	uint firstItem = 0, currentItem = 0, maxItemsOnScreen = 10, animFrameCounter = 0;
	int result = 0;

	/*
	// DEBUG
	for (uint16 i = 1; i < 20; i++)
		items.push_back(i);
	*/
		
	waitForKeys();
		
	// Build items array and set up variables
	for (int i = 0; i < 256; i++) {
		if (_itemStatus[i] >= 1) {
			items.push_back(i);
			if (i == _invActiveItem) {
				firstItem = items.size() < 5 ? 0 : items.size() - 5;
				currentItem = items.size() - 1;
			}
		}
	}

	while (!result) {
	
		// TODO: Check mouse rectangles
	
		drawInventory(items, firstItem, currentItem, animFrameCounter++);

		// TODO: Handle mouse rectangles
		
		switch (_keyScancode) {
		case Common::KEYCODE_DOWN:
		{
			if ((currentItem - firstItem + 1 < maxItemsOnScreen) && (currentItem + 1 < items.size())) {
				// TODO: Check mouse rectangle
				currentItem++;
			} else if (firstItem + maxItemsOnScreen < items.size()) {
				firstItem++;
				currentItem++;
			}
			break;
		}
		case Common::KEYCODE_UP:
		{
			if (currentItem > firstItem) {
				// TODO: Check mouse rectangle
				currentItem--;
			} else if (firstItem > 0) {
				firstItem--;
				currentItem--;
			}
			break;
		}
		case Common::KEYCODE_ESCAPE:
			result = 2;
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_u:
			for (uint i = 0; i < 255; i++) {
				if (_itemStatus[i] == 2)
					_itemStatus[i] = 1;
			}
			_invActiveItem = items[currentItem];
			// Return just selects, U actually uses the item
			if (_keyScancode == Common::KEYCODE_u) {
				//debug("Use item #%d", _invActiveItem);
				_itemStatus[_invActiveItem] = 2;
			}
			result = 1;
			break;
		default:
			break;
		}
		
  		waitForKeys();
		handleEvents();
		_system->delayMillis(20); // TODO: Adjust or use fps counter
	}
	
	result = 2 - result;

	// TODO...

	return result;
}

void CometEngine::drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter) {

	uint xadd = 74, yadd = 64, itemHeight = 12, maxItemsOnScreen = 10;

	_screen->drawAnimationElement(_iconSprite, 16, 0, 0);

	if (firstItem > 0)
		_screen->drawAnimationElement(_iconSprite, 53, 0, 0);

	if (firstItem + maxItemsOnScreen < items.size())
		_screen->drawAnimationElement(_iconSprite, 52, 0, 0);

	for (uint i = 0; (i < maxItemsOnScreen) && (firstItem + i < items.size()); i++) {
		byte *itemName = _textBuffer3->getString(items[firstItem + i]);
		int x = xadd + 21, y = yadd + itemHeight * i;
		_screen->setFontColor(120);
		_screen->drawText(x, y, itemName);
		_screen->setFontColor(119);
		_screen->drawText(x + 1, y + 1, itemName);
		x = xadd;
		y = yadd +  + itemHeight * i - 3;
		// TODO: Implement and use drawIcon instead
		_screen->drawAnimationElement(_objectsVa2, _objectsVa2->_anims[items[firstItem + i]]->frames[0]->elementIndex, x, y);
	}
	
	if (items.size() > 0) {
		int x = xadd + 16, y = yadd + (currentItem - firstItem) * itemHeight - 1;
		_screen->frameRect(x, y, 253, y + itemHeight - 1, _invSelectionColor);
		_invSelectionColor++;
		if (_invSelectionColor >= 96)
			_invSelectionColor = 80;
	}

	_screen->update();

}

void CometEngine::invUseItem() {

	for (uint index = 0; index < 256; index++) {
		if (_itemStatus[index] == 2)
			_itemStatus[index] = 1;
	}
	
	if (_invActiveItem != -1) {
		if (_itemStatus[_invActiveItem] == 1)
			_itemStatus[_invActiveItem] = 2;
	}

}

void CometEngine::invCheckActiveItem() {

	/* If the currently selected item was disabled, scan for the preceeding item
		and set it as selected item. */
	if (_invActiveItem >= 0 && _itemStatus[_invActiveItem] == 0) {
		if (_invActiveItem >= 1) {
			for (_invActiveItem = _invActiveItem - 1; _invActiveItem >= 0 && _itemStatus[_invActiveItem] == 0;
				_invActiveItem--) {
			}
		} else {
			_invActiveItem = -1;
		}
	}

	/* Check if the player wants to read the notebook */
	if (_itemStatus[0] == 2) {
		handleReadBook();
		_itemStatus[0] = 1;
	}


}

} // End of namespace Comet
