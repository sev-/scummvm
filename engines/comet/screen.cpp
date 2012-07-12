/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/screen.h"

namespace Comet {

InterpolatedAnimationCommand::InterpolatedAnimationCommand(byte cmd, byte aarg1, byte aarg2, byte barg1, byte barg2)
	: _cmd(cmd), _aarg1(aarg1), _aarg2(aarg2), _barg1(barg1), _barg2(barg2) {
}

InterpolatedAnimationElement::~InterpolatedAnimationElement() {
	for (Common::Array<InterpolatedAnimationCommand*>::iterator iter = commands.begin(); iter != commands.end(); ++iter)
		delete (*iter);
}

Screen::Screen(CometEngine *vm) : _vm(vm) {
	_palFlag = false;
	_fadeType = kFadeNone;
	_transitionEffect = false;
	_zoomFactor = 0;
	_zoomX = 160;
	_zoomY = 100;

	_workScreen = new byte[64320];
	_currFontResource = new FontResource();
	
	setClipRect(0, 0, 320, 200);

}

Screen::~Screen() {
	delete[] _workScreen;
	delete _currFontResource;
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

		_vm->_system->delayMillis(40); // TODO
	}

}

void Screen::copyFromScreenResource(ScreenResource *screenResource) {
	memcpy(getScreen(), screenResource->getScreen(), 64000);
}

void Screen::copyFromScreen(byte *source) {
	memcpy(getScreen(), source, 64000);
}

void Screen::copyToScreen(byte *dest) {
	memcpy(dest, getScreen(), 64000);
}

void Screen::grabRect(Graphics::Surface *surface, int x, int y) {
	for (int yc = 0; yc < surface->h; yc++)
		memcpy(surface->getBasePtr(0, yc), getScreen() + x + (y + yc) * 320, surface->w);
}

void Screen::putRect(Graphics::Surface *surface, int x, int y) {
	for (int yc = 0; yc < surface->h; yc++)
		memcpy(getScreen() + x + (y + yc) * 320, surface->getBasePtr(0, yc), surface->w);
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

void Screen::setFadeStep(int fadeStep) {
	_fadeStep = fadeStep;
}

void Screen::screenZoomEffect2x(int x, int y) {

	if (x - 80 < 0) x = 0;
	if (x > 159) x = 159;
	if (y - 80 < 0) y = 0;
	if (y > 99) y = 99;

	byte *vgaScreen = new byte[64000];
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

	byte *vgaScreen = new byte[64000];
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

	byte *vgaScreen = new byte[64000];
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
		_vm->syncUpdate(false);
	}

	delete[] vgaScreen;

	_transitionEffect = false;

}

void Screen::screenScrollEffect(byte *newScreen, int scrollDirection) {

	const int kScrollStripWidth = 40;

	Graphics::Surface *screen = _vm->_system->lockScreen();
	memcpy(_workScreen, screen->pixels, 320 * 200);
	_vm->_system->unlockScreen();

	int copyOfs = 0;
	
	while (copyOfs < 320) {
		if (scrollDirection < 0) {
			for (int y = 0; y < 200; y++) {
				memmove(&_workScreen[y * 320], &_workScreen[y * 320 + kScrollStripWidth], 320 - kScrollStripWidth);
				memcpy(&_workScreen[y * 320 + 320 - kScrollStripWidth], &newScreen[y * 320 + copyOfs], kScrollStripWidth);
			}
		} else {
			for (int y = 0; y < 200; y++) {
				memmove(&_workScreen[y * 320 + kScrollStripWidth], &_workScreen[y * 320], 320 - kScrollStripWidth);
				memcpy(&_workScreen[y * 320], &newScreen[y * 320 + (320 - kScrollStripWidth - copyOfs)], kScrollStripWidth);
			}
		}
		_vm->_system->copyRectToScreen(_workScreen, 320, 0, 0, 320, 200);
		_vm->_system->updateScreen();
		_vm->syncUpdate(false);
		copyOfs += kScrollStripWidth;
	}

}

void Screen::buildPalette(byte *sourcePal, byte *destPal, int value) {
	for (int i = 0; i < 768; i++)
		destPal[i] = (sourcePal[i] * value) >> 8;
}

void Screen::buildRedPalette(byte *sourcePal, byte *destPal, int value) {
	byte *src = sourcePal;
	for (int i = 0; i < 256; i++) {
		byte r = *src++;
		byte g = *src++;
		byte b = *src++;
		*destPal++ = ((r + g + b + 100) / 4 - r) * value / 16 + r;
		*destPal++ = g * (16 - value) / 16;
		*destPal++ = b * (16 - value) / 16;
	}
}

void Screen::paletteFadeIn() {
	buildPalette(_vm->_gamePalette, _vm->_screenPalette, 0);
	setFullPalette(_vm->_screenPalette);
	_vm->_system->updateScreen();
	_vm->_system->copyRectToScreen(_workScreen, 320, 0, 0, 320, 200);

	int value = 0;
	while (value < 255) {
		buildPalette(_vm->_gamePalette, _vm->_screenPalette, value);
		setFullPalette(_vm->_screenPalette);
		_vm->_system->updateScreen();
		value += _fadeStep;
		_vm->_system->delayMillis(10); // TODO
	}

	buildPalette(_vm->_gamePalette, _vm->_screenPalette, 255);
	setFullPalette(_vm->_screenPalette);
	_vm->_system->updateScreen();

	_fadeType = kFadeNone;
	_palFlag = false;

}

void Screen::paletteFadeOut() {
	buildPalette(_vm->_gamePalette, _vm->_screenPalette, 255);
	setFullPalette(_vm->_screenPalette);
	_vm->_system->updateScreen();

	int value = 255;
	while (value > 0) {
		buildPalette(_vm->_gamePalette, _vm->_screenPalette, value);
		setFullPalette(_vm->_screenPalette);
		_vm->_system->updateScreen();
		value -= _fadeStep;
		_vm->_system->delayMillis(10); // TODO
	}

	buildPalette(_vm->_gamePalette, _vm->_screenPalette, 0);
	setFullPalette(_vm->_screenPalette);
	_vm->_system->updateScreen();

	_fadeType = kFadeNone;
	_palFlag = true;

}

void Screen::setWhitePalette(int value) {
	for (int i = 0; i < 768; i++)
		_vm->_screenPalette[i] = _vm->_gamePalette[i] + (256 - _vm->_gamePalette[i]) / 16 * value;
	setFullPalette(_vm->_screenPalette);
	_vm->_system->updateScreen();
}

void Screen::setPartialPalette(byte *palette, int start, int count) {
	_vm->_system->getPaletteManager()->setPalette(palette+(start*3), start, count);
}

void Screen::setFullPalette(byte *palette) {
	setPartialPalette(palette, 0, 256);
}

void Screen::clear() {
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

void Screen::drawDottedLine(int x1, int y1, int x2, int y2, int color) {
	_dotFlag = 1;
	Graphics::drawLine(x1, y1, x2, y2, color, Screen::dottedPlotProc, (void*)this);
}

void Screen::fillRect(int x1, int y1, int x2, int y2, byte color) {

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

	int width = x2 - x1 + 1;
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

void Screen::clipPolygonLeft(Common::Array<Point> **poly, int16 clipLeft) {
	Common::Array<Point> *points = *poly; 
	Common::Array<Point> *clipPoints = new Common::Array<Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currX < clipLeft) {
			doClip = newX >= clipLeft;
		} else {
			clipPoints->push_back(Point(currX, currY));
			doClip = newX < clipLeft;
		}
		if (doClip) {
			if (newX <= currX) {
				SWAP(newX, currX);
				SWAP(newY, currY);
			}
			clipPoints->push_back(Point(
				clipLeft,
				-(currX - clipLeft) * (newY - currY) / (newX - currX) + currY));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void Screen::clipPolygonRight(Common::Array<Point> **poly, int16 clipRight) {
	Common::Array<Point> *points = *poly; 
	Common::Array<Point> *clipPoints = new Common::Array<Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currX > clipRight) {
			doClip = newX <= clipRight;
		} else {
			clipPoints->push_back(Point(currX, currY));
			doClip = newX > clipRight;
		}
		if (doClip) {
			if (newX <= currX) {
				SWAP(newX, currX);
				SWAP(newY, currY);
			}
			clipPoints->push_back(Point(
				clipRight,
				-(currX - clipRight) * (newY - currY) / (newX - currX) + currY));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void Screen::clipPolygonTop(Common::Array<Point> **poly, int16 clipTop) {
	Common::Array<Point> *points = *poly; 
	Common::Array<Point> *clipPoints = new Common::Array<Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currY < clipTop) {
			doClip = newY >= clipTop;
		} else {
			clipPoints->push_back(Point(currX, currY));
			doClip = newY < clipTop;
		}
		if (doClip) {
			clipPoints->push_back(Point(
				-(currY - clipTop) * (newX - currX) / (newY - currY) + currX, 
				clipTop));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void Screen::clipPolygonBottom(Common::Array<Point> **poly, int16 clipBottom) {
	Common::Array<Point> *points = *poly; 
	Common::Array<Point> *clipPoints = new Common::Array<Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currY > clipBottom) {
			doClip = newY <= clipBottom;
		} else {
			clipPoints->push_back(Point(currX, currY));
			doClip = newY > clipBottom;
		}
		if (doClip) {
			clipPoints->push_back(Point(
				(-(currY - clipBottom)) * (newX - currX) / (newY - currY) + currX, 
				clipBottom));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void Screen::filledPolygonColor(Common::Array<Point> &poly, byte color) {

	Common::Array<Point> *workPoints = new Common::Array<Point>();
	int16 x1buffer[200], x2buffer[200];
	int16 minX = 32767, maxX = -32767;
	int16 minY = 32767, maxY = -32767;
	int16 currX, currY;
	int16 *x1dst, *x2dst;
	bool clipped = false;

	workPoints->reserve(poly.size() + 1);
	for (uint i = 0; i < poly.size(); i++) {
		Point pt = poly[i];
		if (pt.x < minX) minX = pt.x;
		if (pt.x > maxX) maxX = pt.x;
		if (pt.y < minY) minY = pt.y;
		if (pt.y > maxY) maxY = pt.y;
		workPoints->push_back(pt);
	}
	workPoints->push_back((*workPoints)[0]);
					  
	if (minY == maxY) {
		delete workPoints;
		return;
	}

	if (minX < _clipX1 && workPoints->size() > 2) {
		clipPolygonLeft(&workPoints, _clipX1);
		clipped = true;
	}

	if (maxX >= _clipX2 && workPoints->size() > 2) {
		clipPolygonRight(&workPoints, _clipX2 - 1);
		clipped = true;
	}

	if (minY < _clipY1 && workPoints->size() > 2) {
		clipPolygonTop(&workPoints, _clipY1);
		clipped = true;
	}
	
	if (maxY >= _clipY2 && workPoints->size() > 2) {
		clipPolygonBottom(&workPoints, _clipY2 - 1);
		clipped = true;
	}

	if (workPoints->size() <= 2) {
		delete workPoints;
		return;
	}

	if (clipped) {
		// Recalc min/max y
		minX = 32767;
		maxX = -32767;
		minY = 32767;
		maxY = -32767;
		for (uint i = 0; i < workPoints->size(); i++) {
			Point pt = (*workPoints)[i];
			if (pt.x < minX) minX = pt.x;
			if (pt.x > maxX) maxX = pt.x;
			if (pt.y < minY) minY = pt.y;
			if (pt.y > maxY) maxY = pt.y;
		}
	}

	x1dst = x1buffer;
	x2dst = x2buffer;
	currX = (*workPoints)[0].x;
	currY = (*workPoints)[0].y;
	for (uint i = 1; i < workPoints->size(); i++) {
		int16 newX, newY, sax, sdx;
		newX = (*workPoints)[i].x;
		newY = (*workPoints)[i].y;
		sdx = newX;
		sax = newY;
		if (newY != currY) {
			int16 err;
			if (currY == minY || currY == maxY)
				SWAP(x1dst, x2dst);
			if (currX >= newX) {
				SWAP(currX, newX);
				SWAP(currY, newY);
			}
			int16 *di = x2dst + currY;
			int dir = 1;
			if (currY >= newY) {
				dir = -1;
				SWAP(currY, newY);
			}
			SWAP(currY, newY);
			currY -= newY;
			*di = currX;
			di += dir;
			newX -= currX;
			err = currY / 2;
			for (int y = 0; y < currY; y++) {
				err += newX;
				while (err >= currY) {
					err -= currY;
					currX++;
				}
				*di = currX;
				di += dir;
			}
		}
		currX = sdx;
		currY = sax;
	}

	int16 *x1src = x1dst + minY;
	int16 *x2src = x2dst + minY;
	int16 rowCount = maxY - minY + 1;
	do {
		int16 x1 = *x1src++;
		int16 x2 = *x2src++;
		if (x1 > x2) SWAP(x1, x2);
		hLine(x1, minY, x2, color);
		minY++;
	} while (--rowCount);

	delete workPoints;
	
}

void Screen::loadFont(const char *pakName, int index) {
	_vm->_res->loadFromPak(_currFontResource, pakName, index);
}

void Screen::loadFontFromRaw(const byte *rawData, uint32 rawDataSize, int maxCount, int index) {
	_vm->_res->loadFromRaw(_currFontResource, rawData, rawDataSize, maxCount, index);
}

void Screen::setFontColor(byte color) {
	_currFontColor = color;
}

void Screen::drawText(int x, int y, const byte *text) {
	_currFontResource->drawText(x, y, getScreen(), text, _currFontColor);
}

void Screen::drawTextOutlined(int x, int y, const byte *text, byte color1, byte color2) {
	byte *destBuffer = getScreen();
	_currFontResource->drawText(x + 1, y + 1, destBuffer, text, color2);
	_currFontResource->drawText(x + 1, y - 1, destBuffer, text, color2);
	_currFontResource->drawText(x + 1, y, destBuffer, text, color2);
	_currFontResource->drawText(x - 1, y, destBuffer, text, color2);
	_currFontResource->drawText(x, y + 1, destBuffer, text, color2);
	_currFontResource->drawText(x, y - 1, destBuffer, text, color2);
	_currFontResource->drawText(x - 1, y + 1, destBuffer, text, color2);
	_currFontResource->drawText(x - 1, y - 1, destBuffer, text, color2);
	_currFontResource->drawText(x, y, destBuffer, text, color1);
}

int Screen::drawText3(int x, int y, const byte *text, byte color, int flag) {
	debugC(kDebugText, "Screen::drawText3(x: %d, y: %d, byte *text, color: %d, flag: %d)", x, y, color, flag);
	int tw = 0, linesDrawn = 0, textX = x, textY = y;
	if (textY < 3)
		textY = 3;
	setFontColor(color);
	while (*text != '*' && ++linesDrawn <= 3) {
		int textWidth, textWidth2;
		if (flag == 0) {
			textWidth = getTextWidth(text);
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
			debugC(kDebugText, "\tdrawText(textX: %d, textY: %d, text: \"%s\"", textX, textY, text);
			drawText(textX, textY, text);
			textY += 8;
		}
		text += strlen((const char*)text) + 1;
	}
	debugC(kDebugText, "End of Screen::drawText3()");
	return textY;
}

int Screen::getTextWidth(const byte *text) {
	return _currFontResource->getTextWidth(text);
}

int Screen::getTextHeight(const byte *text) {
	int textHeight = 0, linesDrawn = 0;
	while (*text != '*' && ++linesDrawn <= 3) {
		textHeight += 8;
		text += strlen((const char*)text) + 1;
	}
	return textHeight;
}

void Screen::plotProc(int x, int y, int color, void *data) {
	Screen *screen = (Screen*)data;
	screen->putPixel(x, y, color);
}

void Screen::dottedPlotProc(int x, int y, int color, void *data = NULL) {
	Screen *screen = (Screen*)data;
	if (x >= 0 && x < 320 && y >= 0 && y < 200) {
		screen->_dotFlag++;
		if (screen->_dotFlag & 2)
			screen->getScreen()[x + y * 320] = color;
	}
}

Graphics::Surface *Screen::decompressAnimationCel(const byte *celData, int16 width, int16 height) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	const byte *src = celData;
	byte *dst = (byte*)surface->pixels;
	while (height--) {
		byte *row = dst;
		byte chunks = *src++;
		while (chunks--) {
			byte skip = src[0];
			int count = src[1] * 4 + src[2];
			src += 3;
			row += skip;
			memcpy(row, src, count);
			row += count;
			src += count;
		}
		src++;
		dst += surface->pitch;
	}
	return surface;
}

void Screen::drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, byte flags) {
	byte *frameData = cel.data;

	int width = cel.width;
	int lineWidth = width;
	int height = cel.height;
	int skipX = 0;

	flags ^= (cel.flags >> 8);

	y -= height;
	y++;

	if (x + width >= _clipX2)
		width = _clipX2 - x;

	if (y + height >= _clipY2)
		height = _clipY2 - y;

	if (y < _clipY1) {
		if (y + height - 1 < _clipY1)
			return;
		height -= _clipY1 - y;
		while (y < _clipY1) {
			// Skip the clipped RLE-compressed line
			byte chunks = *frameData++;
			while (chunks--)
				frameData += 3 + frameData[1] * 4 + frameData[2];
			frameData++;
			y++;
		}
	}

	if (x < _clipX1) {
		if (x + width - 1 < _clipX1)
			return;
		skipX = _clipX1 - x;
		x = _clipX1;
	}

	if (x >= _clipX2 || y >= _clipY2)
		return;

	byte *screenDestPtr = _workScreen + x + (y * 320);
	byte lineBuffer[320];

	while (height--) {

		memset(lineBuffer, 0, lineWidth);

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
			for (int xc = skipX; xc < width; xc++)
				if (lineBuffer[lineWidth-xc-1] != 0)
					screenDestPtr[xc-skipX] = lineBuffer[lineWidth-xc-1];
		} else {
			for (int xc = skipX; xc < width; xc++)
				if (lineBuffer[xc] != 0)
					screenDestPtr[xc-skipX] = lineBuffer[xc];
		}

		screenDestPtr += 320;
	}

}

void Screen::drawAnimationCelRle(AnimationCel &cel, int16 x, int16 y) {

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

void Screen::drawAnimationElement(AnimationResource *animation, int16 elementIndex, int16 x, int16 y, byte parentFlags) {
	AnimationElement *element = animation->_elements[elementIndex];

	byte flags = element->flags | (parentFlags & 0xA0);
	debug(8, "Screen::drawAnimationElement() flags = %02X", flags);

	for (Common::Array<AnimationCommand*>::iterator iter = element->commands.begin(); iter != element->commands.end(); ++iter) {
		drawAnimationCommand(animation, (*iter), x, y, flags);
	}

}

void Screen::drawAnimationCommand(AnimationResource *animation, AnimationCommand *cmd, int16 x, int16 y, byte parentFlags) {

	debug(8, "Screen::drawAnimationCommand() cmd = %d; points = %d", cmd->cmd, cmd->points.size());

	Common::Array<Point> points;

	// The commands' points need to be adjusted according to the parentFlags and the x/y position
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

void Screen::drawInterpolatedAnimationElement(InterpolatedAnimationElement *interElem, int16 x, int16 y, int mulValue) {
	for (Common::Array<InterpolatedAnimationCommand*>::iterator iter = interElem->commands.begin(); iter != interElem->commands.end(); ++iter) {
		InterpolatedAnimationCommand *interCmd = *iter;
		byte color1, color2;
		// TODO: int limit = (mul_val * cmdCount) / 256;
		color1 = interCmd->_aarg1;
		color2 = interCmd->_aarg2;
		drawInterpolatedAnimationCommand(interCmd, x, y, mulValue, color1, color2);
	}
}

void Screen::drawInterpolatedAnimationCommand(InterpolatedAnimationCommand *interCmd, int16 x, int16 y, int mulValue, byte color1, byte color2) {

	debug(8, "Screen::drawInterpolatedAnimationCommand() cmd = %d; points = %d", interCmd->_cmd, interCmd->_points.size());

	Common::Array<Point> points;

	//int limit = (mul_val * cmdCount) / 256;

	// The commands' points need to be adjusted according to the x/y position
	points.reserve(interCmd->_points.size() + 1);
	for (uint pointIndex = 0; pointIndex < interCmd->_points.size(); pointIndex += 2) {
		int x1, y1;
		Point *pt1 = &interCmd->_points[pointIndex + 0];
		Point *pt2 = &interCmd->_points[pointIndex + 1];
		x1 = x + pt1->x + ((pt2->x - pt1->x) * mulValue) / 256;
		y1 = y + pt1->y + ((pt2->y - pt1->y) * mulValue) / 256;
		points.push_back(Point(x1, y1));
	}

	switch (interCmd->_cmd) {

	case kActElement:
		warning("Screen::drawInterpolatedAnimationCommand() kActElement not supported here");
		break;

	case kActCelSprite:
		warning("Screen::drawInterpolatedAnimationCommand() kActCelSprite not supported here");
		break;

	case kActFilledPolygon:
		filledPolygonColor(points, color2);
		if (color1 != 0xFF) {
			points.push_back(points[0]);
			for (uint i = 0; i < points.size() - 1; i++)
				drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color1);
		}
		break;

	case kActRectangle:
		fillRect(points[0].x, points[0].y, points[1].x, points[1].y, color2);
		if (color1 != 0xFF)
			frameRect(points[0].x, points[0].y, points[1].x, points[1].y, color1);
		break;

	case kActPolygon:
		for (uint i = 0; i < points.size() - 1; i++)
			drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color2);
		break;

	case kActPixels:
		for (uint i = 0; i < points.size(); i++)
			putPixel(points[i].x, points[i].y, color2);
		break;

	case kActCelRle:
		warning("Screen::drawInterpolatedAnimationCommand() kActCelRle not supported here");
		break;

	default:
		warning("Screen::drawAnimationCommand() Unknown command %d", interCmd->_cmd);

	}

}

void Screen::buildInterpolatedAnimationElement(AnimationElement *elem1, AnimationElement *elem2, 
	InterpolatedAnimationElement *interElem) {

	uint minCmdCount, maxCmdCount;
	uint minPointsCount, maxPointsCount;

	minCmdCount = MIN(elem1->commands.size(), elem2->commands.size());
	maxCmdCount = MAX(elem1->commands.size(), elem2->commands.size());

	for (uint cmdIndex = 0; cmdIndex < maxCmdCount; cmdIndex++) {

		AnimationCommand *cmd1 = NULL, *cmd2 = NULL;
		InterpolatedAnimationCommand *interCmd;

		if (cmdIndex < elem1->commands.size())
			cmd1 = elem1->commands[cmdIndex];
		
		if (cmdIndex < elem2->commands.size())
			cmd2 = elem2->commands[cmdIndex];

		if (!cmd1 || !cmd2)
			continue;

		minPointsCount = MIN(cmd1->points.size(), cmd2->points.size());
		maxPointsCount = MAX(cmd1->points.size(), cmd2->points.size());

		if (cmdIndex < minCmdCount) {
			if (cmd1->cmd == cmd2->cmd) {
				interCmd = new InterpolatedAnimationCommand(cmd1->cmd, 
					cmd1->arg1, cmd1->arg2, cmd2->arg1, cmd2->arg2);
				interCmd->_points.reserve(maxPointsCount * 2);
				for (uint currPointIndex = 0; currPointIndex < maxPointsCount; currPointIndex++) {
					if (currPointIndex < minPointsCount) {
						interCmd->_points.push_back(cmd1->points[currPointIndex]);
						interCmd->_points.push_back(cmd2->points[currPointIndex]);
					} else if (minPointsCount == cmd1->points.size()) {
						interCmd->_points.push_back(cmd1->points[cmd1->points.size() - 1]);
						interCmd->_points.push_back(cmd2->points[currPointIndex]);
					} else {
						interCmd->_points.push_back(cmd1->points[currPointIndex]);
						interCmd->_points.push_back(cmd2->points[cmd2->points.size() - 1]);
					}
				}
				interElem->commands.push_back(interCmd);
			} else {
				interCmd = new InterpolatedAnimationCommand(cmd1->cmd, 
					cmd1->arg1, cmd1->arg2, cmd2->arg1, cmd2->arg2);
				interCmd->_points.reserve(cmd1->points.size() * 2);
				for (uint currPointIndex = 0; currPointIndex < cmd1->points.size(); currPointIndex++) {
					interCmd->_points.push_back(cmd1->points[currPointIndex]);
					interCmd->_points.push_back(cmd2->points[0]);
				}
				interElem->commands.push_back(interCmd);
				interCmd = new InterpolatedAnimationCommand(cmd2->cmd, 
					cmd2->arg1, cmd2->arg2, cmd2->arg1, cmd2->arg2);
				interCmd->_points.reserve(cmd2->points.size() * 2);
				for (uint currPointIndex = 0; currPointIndex < cmd2->points.size(); currPointIndex++) {
					interCmd->_points.push_back(cmd1->points[0]);
					interCmd->_points.push_back(cmd2->points[currPointIndex]);
				}
				interElem->commands.push_back(interCmd);
			}
		} else if (minCmdCount == elem1->commands.size()) {
			interCmd = new InterpolatedAnimationCommand(cmd2->cmd, 
				cmd2->arg1, cmd2->arg2, cmd2->arg1, cmd2->arg2);
			interCmd->_points.reserve(cmd2->points.size() * 2);
			for (uint currPointIndex = 0; currPointIndex < cmd2->points.size(); currPointIndex++) {
				interCmd->_points.push_back(cmd2->points[0]);
				interCmd->_points.push_back(cmd2->points[currPointIndex]);
			}
			interElem->commands.push_back(interCmd);
		} else {
			interCmd = new InterpolatedAnimationCommand(cmd1->cmd, 
				cmd1->arg1, cmd1->arg2, cmd1->arg1, cmd1->arg2);
			interCmd->_points.reserve(cmd1->points.size() * 2);
			for (uint currPointIndex = 0; currPointIndex < cmd1->points.size(); currPointIndex++) {
				interCmd->_points.push_back(cmd1->points[currPointIndex]);
				interCmd->_points.push_back(cmd1->points[0]);
			}
			interElem->commands.push_back(interCmd);
		}
		
	}

}

int Screen::drawAnimation(AnimationResource *animation, AnimationFrameList *frameList, int frameIndex, int interpolationStep, int x, int y, int frameCount) {

	AnimationFrame *frame = frameList->frames[frameIndex];

	int drawX = x, drawY = y;
	int index = frame->elementIndex;
	int maxInterpolationStep = frame->flags & 0x3FFF;
	int gfxMode = frame->flags >> 14;
	int result = 0;

	for (int i = 0; i <= frameIndex; i++) {
		drawX += frameList->frames[i]->xOffs;
		drawY += frameList->frames[i]->yOffs;
	}

	debug(0, "gfxMode = %d; x = %d; y = %d; drawX = %d; drawY = %d; gfxMode = %d; maxInterpolationStep = %d",
		gfxMode, x, y, drawX, drawY, gfxMode, maxInterpolationStep);

	switch (gfxMode) {
	case 0:
		// Normal drawing
		drawAnimationElement(animation, index, drawX, drawY);
		break;
	case 1:
	{
		// Interpolated vector drawing
		int nextFrameIndex = frameIndex + 1;
		if (nextFrameIndex >= frameCount)
			nextFrameIndex = frameIndex;
		AnimationFrame *nextFrame = frameList->frames[nextFrameIndex];
		InterpolatedAnimationElement interElem;
		AnimationElement *elem1 = animation->_elements[frame->elementIndex];
		AnimationElement *elem2 = animation->_elements[nextFrame->elementIndex];
	
		buildInterpolatedAnimationElement(elem1, elem2, &interElem);
		drawInterpolatedAnimationElement(&interElem, drawX, drawY, maxInterpolationStep == 0 ? 1 : maxInterpolationStep);
		
		interpolationStep++;
		if (interpolationStep >= maxInterpolationStep)
			interpolationStep = 0;
			
		result = interpolationStep;			

		break;		
	}				
	default:
		debug("Screen::drawAnimation() gfxMode == %d not yet implemented", gfxMode);
	}

	return result;
}

void Screen::setClipRect(int clipX1, int clipY1, int clipX2, int clipY2) {
	_clipX1 = clipX1;
	_clipY1 = clipY1;
	_clipX2 = clipX2;
	_clipY2 = clipY2;
}

void Screen::setClipX(int clipX1, int clipX2) {
	_clipX1 = clipX1;
	_clipX2 = clipX2;
}

void Screen::setClipY(int clipY1, int clipY2) {
	_clipY1 = clipY1;
	_clipY2 = clipY2;
}

} // End of namespace Comet
