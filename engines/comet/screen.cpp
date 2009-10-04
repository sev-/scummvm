#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/pak.h"

#include "comet/screen.h"

namespace Comet {

int *Screen::gfxPrimitivesPolyInts = NULL;
uint Screen::gfxPrimitivesPolyAllocated = 0;

Screen::Screen(CometEngine *vm) : _vm(vm) {

	_fadeType = kFadeNone;
	_transitionEffect = false;
	_zoomFactor = 0;
	_zoomX = 160;
	_zoomY = 100;

	_workScreen = new byte[64320];
	_font = new Font();
	
	setClipRect(0, 0, 320, 200);

}

Screen::~Screen() {
	delete[] _workScreen;
	delete _font;
}

void Screen::update() {

	if (_transitionEffect && _zoomFactor == 0 && _fadeType == kFadeNone) {
		screenTransitionEffect();
	} else if (_fadeType == kFadeIn) {
		paletteFadeIn();
	} else if (_fadeType == kFadeOut) {
		paletteFadeOut();
	} else {

		switch (_zoomFactor) {
		case 0:
			_vm->_system->copyRectToScreen(_workScreen, 320, 0, 0, 320, 200);
			_vm->_system->updateScreen();
			break;
		case 1:
	  		screenZoomEffect2x(_zoomX, _zoomY);
	  		break;
		case 2:
	  		screenZoomEffect3x(_zoomX, _zoomY);
	  		break;
		case 3:
	  		screenZoomEffect4x(_zoomX, _zoomY);
	  		break;
		default:
			break;
		}

		//_system->delayMillis(40);
		_vm->_system->delayMillis(5);//DEBUG
	}

}

void Screen::enableTransitionEffect() {
	_transitionEffect = true;
}

void Screen::setZoom(int zoomFactor, int x, int y) {
	_zoomFactor = zoomFactor;
	_zoomX = x;
	_zoomY = y;
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

		byte pixel = 0;

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

	byte *vgaScreen = new byte[64000];

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

	delete vgaScreen;

	_transitionEffect = false;

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

void Screen::putPixel(int x, int y, byte color) {
	if (x >= 0 && x < 320 && y >= 0 && y < 200)
		_workScreen[x + y * 320] = color;
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

void Screen::drawLine(int x1, int y1, int x2, int y2, byte color) {
	Graphics::drawLine(x1, y1, x2, y2, color, Screen::plotProc, (void*)this);
}

void Screen::fillRect(int x1, int y1, int x2, int y2, byte color) {

	// FIXME: We allow the rectangle to be 200 pixels hight, but only 319
	//        pixels wide? Is that correct?

	if (x1 < 0) x1 = 0;
	else if (x1 >= 320) x1 = 319;
	if (x2 < 0) x2 = 0;
	else if (x2 >= 320) x2 = 319;
	if (y1 < 0) y1 = 0;
	else if (y1 >= 200) y1 = 199;
	if (y2 < 0) y2 = 0;
	else if (y2 >= 200) y2 = 199;

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

void Screen::filledPolygonColor(Common::Array<Point> &poly, byte color) {

	/* NOTE: This polygon drawing code is not REd but taken from SDL_Gfx
		ATM it's better than nothing but sometimes gives wrong results.
		Eventually the original engine's filled polygon drawing code might
		have to be rewritten. */

	int y, xa, xb, miny, maxy;
	int x1, y1, x2, y2;
	int ind1, ind2;
	uint ints;

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
	for (uint i = 1; i < poly.size(); i++) {
		if (poly[i].y < miny) {
			miny = poly[i].y;
		} else if (poly[i].y > maxy) {
			maxy = poly[i].y;
		}
	}

	/* Draw, scanning y */

	for (y = miny; y <= maxy; y++) {

		ints = 0;

		for (uint i = 0; i < poly.size(); i++) {
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

		for (uint i = 0; i < ints; i += 2) {
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

void Screen::drawText(int x, int y, byte *text) {
	_font->drawText(x, y, getScreen(), text);
}

int Screen::drawText3(int x, int y, byte *text, byte color, int flag) {

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

		text += strlen((char*)text) + 1;

	}

	return textY;

}

void Screen::plotProc(int x, int y, int color, void *data) {
	Screen *screen = (Screen*)data;
	screen->putPixel(x, y, color);
}

// New Animation drawing code

void Screen::drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, byte flags) {

	byte *frameData = cel.data;

	int width = cel.width;
	int lineWidth = width;
	int height = cel.height;
	int skipX = 0;

	flags ^= (cel.flags >> 8);

	// TODO: More clipping
	y -= height;
	y++;

	if (x + width >= _clipX2)
		width = _clipX2 - x;

	if (y + height >= _clipY2)
		height = _clipY2 - y;

	if (y < 0) {
		if (y + height - 1 < 0)
			return;
		height -= -y;
		while (y < 0) {
			// Skip the clipped RLE-compressed line
			byte chunks = *frameData++;
			while (chunks--)
				frameData += 3 + frameData[1] * 4 + frameData[2];
			frameData++;
			y++;
		}
	}

	if (x < 0) {
		if (x + width - 1 < 0)
			return;
		skipX = -x;
		x = 0;
	}

	if (x >= _clipX2 || y >= _clipY2)
		return;

	byte *screenDestPtr = _workScreen + x + (y * 320);
	byte lineBuffer[320];

	while (height--) {

		memset(lineBuffer, 0, lineWidth);

		// TODO: Possibly merge decompression and drawing of pixels

		// Decompress the current pixel row
		byte chunks = *frameData++;
		byte *lineBufferPtr = lineBuffer;
		while (chunks--) {
			byte skip = frameData[0];
			int count = frameData[1] * 4 + frameData[2];
			frameData += 3;
			lineBufferPtr += skip;
			memcpy(lineBufferPtr, frameData, count);
			lineBufferPtr += count;
			frameData += count;
		}
		memset(lineBufferPtr, 0, *frameData++);

		// Draw the decompressed pixels
		if (flags & 0x80) {
			for (int xc = skipX; xc < width; xc++) {
				if (lineBuffer[lineWidth-xc-1] != 0)
					screenDestPtr[xc-skipX] = lineBuffer[lineWidth-xc-1];
			}
		} else {
			for (int xc = skipX; xc < width; xc++) {
				if (lineBuffer[xc] != 0)
					screenDestPtr[xc-skipX] = lineBuffer[xc];
			}
		}

		screenDestPtr += 320;
	}

}

void Screen::drawAnimationCelRle(AnimationCel &cel, int16 x, int16 y) {

	// TODO: Clean up this messy function

	int line = 0;
	byte *offsets[200];
	byte bh = 0, bl = 0;
	byte cl = 0, dh = 0, dl = 0;
	bool doMemset = false;
	byte *rleData = cel.data;

	for (int i = 0; i < 200; i++)
		offsets[i] = _workScreen + x + (y + i) * 320;

	do {

		byte flags = *rleData++;
		byte count = *rleData++;

		if (flags & 0x80)
			line = *rleData++;

		byte pixel = *rleData++;
		dh = 1;

		do {

			byte cmd = *rleData++;

			dl = cmd;

			cmd = ((cmd & 0xC0) | (flags & 0x3E)) >> 1;

			//debug(2, "cmd = %d", cmd);

			switch (cmd) {

			case 0:
			case 1:
			case 8:
			case 9:
			case 22:
			case 23:
				doMemset = (cmd % 2 == 0) ? true : false;	// 0, 8, 22 and 1, 9, 23
				for (int i = 0; i < 6; i++) {
					cl = dh + (dl & 1);
					dl >>= 1;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 11:
			case 13:
			case 15:
			case 35:
			case 57:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl -= (dl & 1);
					dl >>= 1;
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 32:
			case 40:
			case 54:
				for (int i = 0; i < 6; i++) {
					cl = dh - (dl & 1);
					dl >>= 1;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 2:
			case 4:
			case 6:
			case 24:
			case 26:
			case 42:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl += (dl & 1);
					dl >>= 1;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 48:
			case 50:
			case 52:
				bh = dh - (dl & 1);
				dl >>= 1;
				bl = dh - (dl & 1);
				dl >>= 1;
				dh = dh - (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					memset(offsets[line], pixel, bh);
					offsets[line++] += bh;
					memset(offsets[line], pixel, bl);
					offsets[line++] += bl;
					memset(offsets[line], pixel, dh);
					offsets[line++] += dh;
				}
				break;

			case 3:
			case 5:
			case 7:
			case 25:
			case 27:
			case 43:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl += (dl & 1);
					dl >>= 1;
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 10:
			case 12:
			case 14:
			case 34:
			case 56:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl -= (dl & 1);
					dl >>= 1;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 16:
			case 18:
			case 20:
				bh = dh + (dl & 1);
				dl >>= 1;
				bl = dh + (dl & 1);
				dl >>= 1;
				dh = dh + (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					memset(offsets[line], pixel, bh);
					offsets[line++] += bh;
					memset(offsets[line], pixel, bl);
					offsets[line++] += bl;
					memset(offsets[line], pixel, dh);
					offsets[line++] += dh;
				}
				break;

			case 17:
			case 19:
			case 21:
				bh = dh + (dl & 1);
				dl >>= 1;
				bl = dh + (dl & 1);
				dl >>= 1;
				dh = dh + (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					offsets[line++] += bh;
					offsets[line++] += bl;
					offsets[line++] += dh;
				}
				break;

			case 28:
			case 60:
			case 92:
			case 124:
				do {
					memset(offsets[line], pixel, count & 7);
					offsets[line++] += count & 7;
					count >>= 3;
				} while (count & 7);
				rleData--;
				count = 1;
				break;

			case 30:
			case 36:
			case 38:
			case 64:
			case 66:
			case 78:
			case 80:
				for (int i = 0; i < 3; i++) {
					cl = dh + (dl & 3);
					dl >>= 2;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 96:
			case 98:
			case 100:
			case 102:
			case 104:
			case 106:
			case 108:
			case 110:
			case 112:
			case 114:
			case 116:
			case 118:
			case 120:
			case 122:
			case 126:
				dh = dl & 0x3F;
				memset(offsets[line], pixel, dh);
				offsets[line++] += dh;
				break;

			case 44:
			case 46:
			case 70:
			case 72:
			case 74:
			case 82:
				for (int i = 0; i < 3; i++) {
					cl = dh - (dl & 3);
					dl >>= 2;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 49:
			case 51:
			case 53:
				bh = dh - (dl & 1);
				dl >>= 1;
				bl = dh - (dl & 1);
				dl >>= 1;
				dh = dh - (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					offsets[line++] += bh;
					offsets[line++] += bl;
					offsets[line++] += dh;
				}
				break;

			case 33:
			case 41:
			case 55:
				for (int i = 0; i < 6; i++) {
					cl = dh - (dl & 1);
					dl >>= 1;
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 58:
			case 59:
			case 86:
			case 87:
			case 88:
			case 89:
				doMemset = (cmd % 2 == 0) ? true : false;	// 58, 86, 88 and 59, 87, 89
				for (int i = 0; i < 3; i++) {
					cl = dh + (dl & 3) - 1;
					dl >>= 2;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 76:
			case 77:
			case 84:
			case 85:
			case 94:
			case 95:
				doMemset = (cmd % 2 == 0) ? true : false;	// 76, 84, 94 and 77, 85, 95
				for (int i = 0; i < 2; i++) {
					cl = dh + (dl & 7);
					dl >>= 3;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 31:
			case 37:
			case 39:
			case 65:
			case 67:
			case 79:
			case 81:
				for (int i = 0; i < 3; i++) {
					cl = dh + (dl & 3);
					dl >>= 2;
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 62:
			case 63:
			case 68:
			case 69:
			case 90:
			case 91:
				doMemset = (cmd % 2 == 0) ? true : false;	// 62, 68, 90 and 63, 69, 91
				for (int i = 0; i < 2; i++) {
					cl = dh - (dl & 7);
					dl >>= 3;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 45:
			case 47:
			case 71:
			case 73:
			case 75:
			case 83:
				for (int i = 0; i < 3; i++) {
					cl = dh - (dl & 3);
					dl >>= 2;
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 97:
			case 99:
			case 101:
			case 103:
			case 105:
			case 107:
			case 109:
			case 111:
			case 113:
			case 115:
			case 117:
			case 119:
			case 121:
			case 123:
			case 127:
				dh = dl & 0x3F;
				offsets[line++] += dh;
				break;

			case 29:
			case 61:
			case 93:
			case 125:
				do {
					offsets[line++] += count & 7;
					count >>= 3;
				} while (count & 7);
				rleData--;
				count = 1;
				break;

			default:
				error("Screen::drawAnimationCelRle() Unknown RLE command %d", cmd);

			}

		} while (--count > 0);

	} while (*rleData != 0);

}

void Screen::drawAnimationElement(Animation *animation, int16 elementIndex, int16 x, int16 y, byte parentFlags) {
	AnimationElement *element = animation->_elements[elementIndex];

	byte flags = element->flags | (parentFlags & 0xA0);
	debug(8, "Screen::drawAnimationElement() flags = %02X", flags);

	for (Common::Array<AnimationCommand*>::iterator iter = element->commands.begin(); iter != element->commands.end(); iter++) {
		drawAnimationCommand(animation, (*iter), x, y, flags);
	}

}

void Screen::drawAnimationCommand(Animation *animation, AnimationCommand *cmd, int16 x, int16 y, byte parentFlags) {

	debug(8, "Screen::drawAnimationCommand() cmd = %d; points = %d", cmd->cmd, cmd->points.size());

	Common::Array<Point> points;

	// The commands' points need to be adjusted accoding to the parentFlags and the x/y position
	points.reserve(cmd->points.size() + 1);
	for (uint i = 0; i < cmd->points.size(); i++) {
		int16 ax = cmd->points[i].x;
		int16 ay = cmd->points[i].y;
		if (parentFlags & 0x80)
			ax = -ax;
		if (parentFlags & 0x20)
			ay = -ay;
		points.push_back(Point(x + ax, y + ay));
	}

	switch (cmd->cmd) {

	case kActElement:
	{
		int16 subElementIndex = (cmd->arg2 << 8) | cmd->arg1;
		drawAnimationElement(animation, subElementIndex & 0x0FFF, points[0].x, points[0].y, parentFlags | (cmd->arg2 & 0xA0));
		break;
	}

	case kActCelSprite:
	{
		int16 celIndex = ((cmd->arg2 << 8) | cmd->arg1) & 0x0FFF;
		int16 celX = points[0].x, celY = points[0].y;
		if (parentFlags & 0x80)
			celX -= animation->getCelWidth(celIndex);
		if (parentFlags & 0x20)
			celY += animation->getCelHeight(celIndex);
		drawAnimationCelSprite(*animation->_cels[celIndex], celX, celY, parentFlags | (cmd->arg2 & 0xA0));
		break;
	}

	case kActFilledPolygon:
	{
		filledPolygonColor(points, cmd->arg2);
		if (cmd->arg1 != 0xFF) {
			points.push_back(points[0]);
			for (uint i = 0; i < points.size() - 1; i++)
				drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, cmd->arg1);
		}
		break;
	}

	case kActRectangle:
	{
		fillRect(points[0].x, points[0].y, points[1].x, points[1].y, cmd->arg2);
		if (cmd->arg1 != 0xFF)
			frameRect(points[0].x, points[0].y, points[1].x, points[1].y, cmd->arg1);
		break;
	}

	case kActPolygon:
	{
		for (uint i = 0; i < points.size() - 1; i++)
			drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, cmd->arg2);
		break;
	}

	case kActPixels:
	{
		for (uint i = 0; i < points.size(); i++)
			putPixel(points[i].x, points[i].y, cmd->arg2);
		break;
	}

	case kActCelRle:
	{
		AnimationCel *cel = animation->_cels[(cmd->arg2 << 8) | cmd->arg1];
		drawAnimationCelRle(*cel, points[0].x, points[0].y - cel->height + 1);
		break;
	}

	default:
		warning("Screen::drawAnimationCommand() Unknown command %d", cmd->cmd);

	}

}

void Screen::setClipRect(int clipX1, int clipY1, int clipX2, int clipY2) {
	// The clipping rect is only used in drawAnimationCelSprite
	_clipX1 = clipX1;
	_clipY1 = clipY1;
	_clipX2 = clipX2;
	_clipY2 = clipY2;
}

} // End of namespace Comet
