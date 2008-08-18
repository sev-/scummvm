#ifndef COMET_FONT_H
#define COMET_FONT_H

#include "common/scummsys.h"
#include "common/util.h"

namespace Comet {

class Font {

public:

	Font();
	~Font();
	
	void load(const char *pakFilename, int fileIndex);
	void setColor(byte color);
	void drawText(int x, int y, byte *destBuffer, byte *text);
	void drawTextOutlined(int x, int y, byte *destBuffer, byte *text, byte color2, byte color);
	//void drawTextColor(int x, int y, char *text, byte *destBuffer, uint8 color);
	int getTextWidth(byte *text);

private:
	byte *_fontData;
	byte *_charData;
	byte *_charInfo;
	byte _color;
	int _charHeight, _bytesPerLine;
};

}

#endif
