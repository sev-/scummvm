#include "common/rect.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/screen.h"

namespace Comet {

void CometEngine::updateMap() {

	static const Common::Point mapPoints[] = {
		Common::Point(248, 126), Common::Point(226, 126), Common::Point(224, 150), Common::Point(204, 156),
		Common::Point(178, 154), Common::Point(176, 138), Common::Point(152, 136), Common::Point(124, 134),
		Common::Point(112, 148), Common::Point( 96, 132), Common::Point( 92, 114), Common::Point(146, 116),
		Common::Point(176, 106), Common::Point(138, 100), Common::Point(104,  94), Common::Point( 82,  96),
		Common::Point(172,  94), Common::Point(116,  80), Common::Point(134,  80), Common::Point(148,  86),
		Common::Point(202, 118), Common::Point(178, 120), Common::Point(190,  92)
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
	
	int result = 0;
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
	while (result == 0) {

		bool textShowing = false;

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
			result = 1;
		}
				
		// seg002:3572
		for (int16 mapItemIndex = 0; mapItemIndex < 10; mapItemIndex++) {

			int16 bitMask = 1 << mapItemIndex;
			const MapRect &mapRect = mapRects[mapItemIndex];
			
			if ((sceneBitMaskStatus & bitMask) && 
				cursorX >= mapRect.x1 && cursorX <= mapRect.x2 && 
				cursorY >= mapRect.y1 && cursorY <= mapRect.y2) {
		
				// seg002:3636
				
				int16 textXAdjust = 0;
				byte *locationName = _textReader->getString(2, 40 + mapItemIndex);
				int textWidth = _screen->_font->getTextWidth(locationName);
				
				if (cursorX - 2 + textWidth >= 283) {
					textXAdjust = cursorX + textWidth - 285;
				}

				_screen->drawTextOutlined(cursorX - 2 - textXAdjust, cursorY - 6, locationName, 119, 120);				
				
				textShowing = true;

				if (_keyScancode == Common::KEYCODE_RETURN || _leftButton) {
					// seg002:36DA
					const MapExit &mapExit = mapExits[mapItemIndex];
					_moduleNumber = mapExit.moduleNumber;
					_sceneNumber = mapExit.sceneNumber;
					if (sceneStatus1 == 1) {
						_moduleNumber += 6;
					} else {
						_sceneNumber += (sceneStatus2 - 1) * 30;
					}
					if ((locationNumber == 7 || locationNumber == 8) &&
						_scriptVars[5] == 2 && _scriptVars[6] == 0 &&
						mapItemIndex != 6 && mapItemIndex != 7 && mapItemIndex != 4) {
						_sceneNumber += 36;
					}
					result = 2;
				}
				
			}
		
		}
	
		if (!textShowing) {
			_screen->drawAnimationElement(_iconSprite, 51, cursorX, cursorY);
		}

		_screen->update();
		_system->delayMillis(40); // TODO

		// _textReader->getString(2, 40 + locationIndex);		
	
	}
	
	waitForKeys();

}

void CometEngine::handleMap() {
}

} // End of namespace Comet
