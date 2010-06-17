#include "comet/comet.h"
#include "comet/font.h"
#include "comet/screen.h"

namespace Comet {

int CometEngine::handleReadBook() {

	int currPageNumber = -1, pageNumber, pageCount, talkPageNumber = -1, result = 0;

	// Use values from script; this is the most current diary entry
	pageNumber = _scriptVars[1];
	pageCount = _scriptVars[1];

	bookTurnPageTextEffect(false, pageNumber, pageCount);

	// Set speech file
	openVoiceFile(7);

	while (!result /*TODO:check for quit*/) {

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
			result = 1;
			break;
		case Common::KEYCODE_ESCAPE:
			result = 2;
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

	openVoiceFile(_narFileIndex);

	return 2 - result;

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
		x = xadd + (106 - _screen->_font->getTextWidth(pageText)) / 2;
		if (x < 0)
			x = 0;
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

} // End of namespace Comet
