#include "graphics/surface.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/pak.h"

#include "comet/screen.h"

namespace Comet {

int *Screen::gfxPrimitivesPolyInts = NULL;
int Screen::gfxPrimitivesPolyAllocated = 0;

Screen::Screen(CometEngine *vm) : _vm(vm) {

	_fadeType = kFadeNone;
	_screenTransitionEffectFlag = false;
	_screenZoomFactor = 0;
	_screenZoomXPos = 160;
	_screenZoomYPos = 100;

    _workScreen = new byte[64320];
	_font = new Font();

}

Screen::~Screen() {
	delete[] _workScreen;
	delete _font;
}

void Screen::update() {

    if (_screenTransitionEffectFlag && _screenZoomFactor == 0 && _fadeType == kFadeNone) {
        screenTransitionEffect();
	} else if (_fadeType == kFadeIn) {
	    paletteFadeIn();
	} else if (_fadeType == kFadeOut) {
	    paletteFadeOut();
	} else {

	    switch (_screenZoomFactor) {
	    case 0:
	        _vm->_system->copyRectToScreen(_workScreen, 320, 0, 0, 320, 200);
	        _vm->_system->updateScreen();
	        break;
		case 1:
      		screenZoomEffect2x(_screenZoomXPos, _screenZoomYPos);
      		break;
		case 2:
      		screenZoomEffect3x(_screenZoomXPos, _screenZoomYPos);
      		break;
		case 3:
      		screenZoomEffect4x(_screenZoomXPos, _screenZoomYPos);
      		break;
		default:
		    break;
		}

    	//_system->delayMillis(40);
    	_vm->_system->delayMillis(5);//DEBUG
	}

}

void Screen::enableTransitionEffect() {
    _screenTransitionEffectFlag = true;
}

void Screen::setZoom(int zoomFactor, int x, int y) {
    _screenZoomFactor = zoomFactor;
	_screenZoomXPos = x;
	_screenZoomYPos = y;
}

void Screen::setFadeType(PaletteFadeType fadeType) {
    _fadeType = fadeType;
}

void Screen::setFadeValue(int fadeValue) {
    _fadeValue = fadeValue;
}

void Screen::screenZoomEffect2x(int x, int y) {

    if (x - 80 < 0) x = 0;
    if (x > 159) x = 159;
    if (y - 80 < 0) y = 0;
    if (y > 99) y = 99;

    byte *vgaScreen = (byte*)malloc(64000);
    byte *sourceBuf = _workScreen + x + (y * 320);
    byte *destBuf = vgaScreen;

    for (int yc = 0; yc < 100; yc++) {

        for (int xc = 0; xc < 160; xc++) {
            byte pixel = *sourceBuf++;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
        }

        sourceBuf -= 160;

        for (int xc = 0; xc < 160; xc++) {
            byte pixel = *sourceBuf++;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
        }

        sourceBuf += 160;

    }

	_vm->_system->copyRectToScreen(vgaScreen, 320, 0, 0, 320, 200);
	_vm->_system->updateScreen();

    delete[] vgaScreen;

}

void Screen::screenZoomEffect3x(int x, int y) {

    if (x - 53 < 0) x = 0;
    if (x > 213) x = 213;
    if (y - 50 < 0) y = 0;
    if (y > 133) y = 133;

    byte *vgaScreen = (byte*)malloc(64000);
    byte *sourceBuf = _workScreen + x + (y * 320);
    byte *destBuf = vgaScreen;

    for (int yc = 0; yc < 66; yc++) {

        byte pixel;

        for (int xc = 0; xc < 106; xc++) {
            pixel = *sourceBuf++;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
        }
        *destBuf++ = pixel;
        *destBuf++ = pixel;

        sourceBuf -= 106;

        for (int xc = 0; xc < 106; xc++) {
            pixel = *sourceBuf++;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
        }
        *destBuf++ = pixel;
        *destBuf++ = pixel;

        sourceBuf -= 106;

        for (int xc = 0; xc < 106; xc++) {
            pixel = *sourceBuf++;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
        }
        *destBuf++ = pixel;
        *destBuf++ = pixel;

        sourceBuf += 214;

    }

    memset(destBuf, 0, 640);

	_vm->_system->copyRectToScreen(vgaScreen, 320, 0, 0, 320, 200);
	_vm->_system->updateScreen();

    delete[] vgaScreen;

}

void Screen::screenZoomEffect4x(int x, int y) {

    if (x - 40 < 0) x = 0;
    if (x > 239) x = 239;
    if (y - 44 < 0) y = 0;
    if (y > 149) y = 149;

    byte *vgaScreen = (byte*)malloc(64000);
    byte *sourceBuf = _workScreen + x + (y * 320);
    byte *destBuf = vgaScreen;

    for (int yc = 0; yc < 50; yc++) {

        byte pixel;

        for (int xc = 0; xc < 80; xc++) {
            pixel = *sourceBuf++;

            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            destBuf += 316;

            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            destBuf += 316;

            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            destBuf += 316;

            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            *destBuf++ = pixel;
            destBuf -= 960;

        }

        sourceBuf += 240;
        destBuf += 960;

    }

	_vm->_system->copyRectToScreen(vgaScreen, 320, 0, 0, 320, 200);
	_vm->_system->updateScreen();

    delete[] vgaScreen;

}

void Screen::screenTransitionEffect() {

	byte *vgaScreen = (byte*)malloc(64000);

	Graphics::Surface *screen = _vm->_system->lockScreen();
	memcpy(vgaScreen, screen->pixels, 320 * 200);
	_vm->_system->unlockScreen();

    for (int i = 0; i < 7; i++) {
        byte *sourceBuf = _workScreen + i;
        byte *destBuf = vgaScreen + i;
        for (int x = 0; x < 320 * 200 / 6; x++) {
            *destBuf = *sourceBuf;
            sourceBuf += 6;
            destBuf += 6;
        }
		_vm->_system->copyRectToScreen(vgaScreen, 320, 0, 0, 320, 200);
		_vm->_system->updateScreen();
    	_vm->_system->delayMillis(40);
    }

    free(vgaScreen);

	_screenTransitionEffectFlag = false;

}

void Screen::buildPalette(byte *sourcePal, byte *destPal, int value) {
	for (int i = 0; i < 768; i++)
	    destPal[i] = (sourcePal[i] * value) >> 8;
}

void Screen::paletteFadeIn() {

	buildPalette(_vm->_ctuPal, _vm->_palette, 0);
	setFullPalette(_vm->_palette);
	_vm->_system->updateScreen();
	_vm->_system->copyRectToScreen(_workScreen, 320, 0, 0, 320, 200);

	int value = 0;
	while (value < 255) {
		buildPalette(_vm->_ctuPal, _vm->_palette, value);
		setFullPalette(_vm->_palette);
		_vm->_system->updateScreen();
		value += _fadeValue;
		_vm->_system->delayMillis(10);
	}

	buildPalette(_vm->_ctuPal, _vm->_palette, 255);
	setFullPalette(_vm->_palette);
	_vm->_system->updateScreen();

	_fadeType = kFadeNone;
	_palFlag = false;

}

void Screen::paletteFadeOut() {

	buildPalette(_vm->_ctuPal, _vm->_palette, 255);
	setFullPalette(_vm->_palette);
	_vm->_system->updateScreen();

	int value = 255;
	while (value > 0) {
		buildPalette(_vm->_ctuPal, _vm->_palette, value);
		setFullPalette(_vm->_palette);
		_vm->_system->updateScreen();
		value -= _fadeValue;
		_vm->_system->delayMillis(10);
	}

	buildPalette(_vm->_ctuPal, _vm->_palette, 0);
	setFullPalette(_vm->_palette);
	_vm->_system->updateScreen();

	_fadeType = kFadeNone;
	_palFlag = true;

}

void Screen::setPartialPalette(byte *palette, int start, int count) {
    byte colors[1024];
    for (int i = start; i < count; i++) {
		colors[i * 4 + 0] = palette[i * 3 + 0];
		colors[i * 4 + 1] = palette[i * 3 + 1];
		colors[i * 4 + 2] = palette[i * 3 + 2];
		colors[i * 4 + 3] = 0;
    }
    _vm->_system->setPalette(colors, start, count);
}

void Screen::setFullPalette(byte *palette) {
    setPartialPalette(palette, 0, 256);
}

void Screen::clearScreen() {
    memset(_workScreen, 0, 64320);
}

void Screen::hLine(int x, int y, int x2, byte color) {
	// Clipping
	if (y < 0 || y >= 200)
		return;

	if (x2 < x)
		SWAP(x2, x);

	if (x < 0)
		x = 0;
	if (x2 >= 320)
		x2 = 320 - 1;

    byte *ptr = getScreen() + x + y * 320;
	if (x2 >= x)
		memset(ptr, color, x2 - x + 1);

}

void Screen::vLine(int x, int y, int y2, byte color) {
	// Clipping
	if (x < 0 || x >= 320)
		return;

	if (y2 < y)
		SWAP(y2, y);

	if (y < 0)
		y = 0;
	if (y2 >= 200)
		y2 = 200 - 1;

    byte *ptr = getScreen() + x + y * 320;
	while (y++ <= y2) {
		*ptr = (byte)color;
		ptr += 320;
	}

}

void Screen::fillRect(int x1, int y1, int x2, int y2, byte color) {

    if (x1 < 0) x1 = 0;
    else if (x1 >= 320) x1 = 319;
    if (x2 < 0) x2 = 0;
    else if (x2 >= 320) x2 = 319;
    if (y1 < 0) y1 = 0;
    else if (y1 >= 200) y1 = 199;

	if (x2 < x1)
		SWAP(x2, x1);

	if (y2 < y1)
		SWAP(y2, y1);

	int width = x2 - x1;
	int height = y2 - y1 + 1;

    byte *ptr = getScreen() + x1 + y1 * 320;
	while (height--) {
		memset(ptr, color, width);
		ptr += 320;
	}

}

void Screen::frameRect(int x1, int y1, int x2, int y2, byte color) {
	hLine(x1, y1, x2 - 1, color);
	hLine(x1, y2 - 1, x2 - 1, color);
	vLine(x1, y1, y2 - 1, color);
	vLine(x2 - 1, y1, y2 - 1, color);
}

int gfxPrimitivesCompareInt(const void *a, const void *b) {
    return (*(const int *) a) - (*(const int *) b);
}

void Screen::filledPolygonColor(PointArray &poly, byte color) {
    int i, y, xa, xb, miny, maxy;
    int x1, y1, x2, y2;
    int ind1, ind2;
    int ints;

    /* Sanity check */
    if (poly.size() < 3)
    	return;

    /* Allocate temp array, only grow array */
    if (!gfxPrimitivesPolyAllocated) {
	   gfxPrimitivesPolyInts = (int*)malloc(sizeof(int) * poly.size());
	   gfxPrimitivesPolyAllocated = poly.size();
    } else {
        if (gfxPrimitivesPolyAllocated < poly.size()) {
            gfxPrimitivesPolyInts = (int*)realloc(gfxPrimitivesPolyInts, sizeof(int) * poly.size());
	       gfxPrimitivesPolyAllocated = poly.size();
	    }
    }

    /* Determine Y maxima */
    miny = poly[0].y;
    maxy = poly[0].y;
    for (i = 1; i < poly.size(); i++) {
        if (poly[i].y < miny) {
            miny = poly[i].y;
        } else if (poly[i].y > maxy) {
            maxy = poly[i].y;
        }
    }

    /* Draw, scanning y */

    for (y = miny; y <= maxy; y++) {

    	ints = 0;

    	for (i = 0; i < poly.size(); i++) {
            if (!i) {
        		ind1 = poly.size() - 1;
        		ind2 = 0;
    	    } else {
        		ind1 = i - 1;
        		ind2 = i;
    	    }
    	    y1 = poly[ind1].y;
    	    y2 = poly[ind2].y;
    	    if (y1 < y2) {
        		x1 = poly[ind1].x;
        		x2 = poly[ind2].x;
    	    } else if (y1 > y2) {
        		y2 = poly[ind1].y;
        		y1 = poly[ind2].y;
        		x2 = poly[ind1].x;
        		x1 = poly[ind2].x;
    	    } else {
                continue;
    	    }

    	    if ( (y >= y1 && y < y2) || (y == maxy && y > y1 && y <= y2) ) {
                gfxPrimitivesPolyInts[ints++] = ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
    	    }

    	}

    	qsort(gfxPrimitivesPolyInts, ints, sizeof(int), gfxPrimitivesCompareInt);

    	for (i = 0; i < ints; i += 2) {
    	    xa = gfxPrimitivesPolyInts[i] + 1;
    	    xa = (xa >> 16) + ((xa & 32768) >> 15);
    	    xb = gfxPrimitivesPolyInts[i+1] - 1;
    	    xb = (xb >> 16) + ((xb & 32768) >> 15);
    	    hLine(xa, y, xb, color);
    	}

    }

}

void Screen::loadFont(const char *pakName, int index) {
    _font->load(pakName, index);
}

void Screen::setFontColor(byte color) {
	_font->setColor(color);
}

void Screen::drawText(int x, int y, const char *text) {
    _font->drawText(x, y, getScreen(), (char*)text);
}

int Screen::drawText3(int x, int y, char *text, byte color, int flag) {

	int tw = 0, linesDrawn = 0, textX = x, textY = y;

	if (textY < 3)
	    textY = 3;

    setFontColor(color);

	while (*text != '*' && ++linesDrawn <= 3) {

	    int textWidth, textWidth2;

	    if (flag == 0) {
	        textWidth = _font->getTextWidth(text);
            textWidth2 = textWidth / 2;
            textX = x - textWidth2;
            tw = x + textWidth2;
			if (textX < 3)
			    textX = 3;
			if (tw > 316)
			    textX -= tw - 316;
		} else {
			textWidth2 = 1;//HACK
		}

		if (textWidth2 != 0) {
		    drawText(textX, textY, text);
		    textY += 8;
		}

		text += strlen(text) + 1;

	}

	return textY;

}

}
