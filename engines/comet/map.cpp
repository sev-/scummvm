#include "common/rect.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/screen.h"

namespace Comet {

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

} // End of namespace Comet
