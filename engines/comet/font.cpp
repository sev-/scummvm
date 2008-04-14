#include "comet/comet.h"
#include "comet/font.h"
#include "comet/pak.h"

namespace Comet {

Font::Font() : _fontData(NULL) {
}

Font::~Font() {
	if (_fontData)
	    free(_fontData);
}

void Font::load(const char *pakFilename, int fileIndex) {
	if (_fontData)
	    free(_fontData);
    _fontData = loadFromPak(pakFilename, fileIndex);
	uint16 skipChars = _fontData[0] * 2;
	_charHeight = _fontData[2];
	_bytesPerLine = _fontData[3];
	if (_bytesPerLine == 0)
		_bytesPerLine = READ_LE_UINT16(_fontData + 4);
	_charData = _fontData + 8;
	_charInfo = _fontData + READ_BE_UINT16(_fontData + 6) - skipChars;
}

void Font::setColor(byte color) {
  _color = color;
}

void Font::drawText(int x, int y, byte *destBuffer, char *text) {

    static const byte startFlags[] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };

    while (*text) {

        uint16 charOfs = text[0] * 2;
        uint16 charInfo = READ_BE_UINT16(_charInfo + charOfs);
        byte charWidth = (charInfo & 0xF000) >> 12;

        if (charWidth > 0) {
            charInfo = charInfo & 0x0FFF;
            byte *charData = _charData + (charInfo >> 3);
            for (int h = 0; h < _charHeight; h++) {
                byte charMask = startFlags[charInfo & 7];
                byte charByte = charData[0];
                byte dataOfs = 0;
                for (int w = 0; w < charWidth; w++) {
                    if (charMask & charByte)
                        destBuffer[(x + w) + (y + h) * 320] = _color;
                    if (charMask & 1) {
                        dataOfs++;
                        charByte = charData[dataOfs];
                        charMask = 0x80;
                    } else {
                        charMask >>= 1;
                    }
                }
                charData += _bytesPerLine;
            }
            x += charWidth + 1;
        } else {
            x += 3;
        }
        text++;
    }

}

void Font::drawTextOutlined(int x, int y, byte *destBuffer, char *text, byte color2, byte color) {
    setColor(color);
    drawText(x + 1, y + 1, destBuffer, text);
    drawText(x + 1, y - 1, destBuffer, text);
    drawText(x + 1, y, destBuffer, text);
    drawText(x - 1, y, destBuffer, text);
    drawText(x, y + 1, destBuffer, text);
    drawText(x, y - 1, destBuffer, text);
    drawText(x - 1, y + 1, destBuffer, text);
    drawText(x - 1, y - 1, destBuffer, text);
    setColor(color2);
    drawText(x, y, destBuffer, text);
}

int Font::getTextWidth(char *text) {
    int textWidth = 0;
    while (*text) {
        uint16 charOfs = text[0] * 2;
        uint16 charInfo = READ_BE_UINT16(_charInfo + charOfs);
        byte charWidth = (charInfo & 0xF000) >> 12;
        if (charWidth > 0)
            textWidth += charWidth + 1;
        else
            textWidth += 3;
        text++;
    }
    return textWidth;
}

}
