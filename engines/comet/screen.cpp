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

#include "comet/comet.h"
#include "comet/screen.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/primitives.h"

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

	_currFontResource = new FontResource();

}

Screen::~Screen() {
	delete _currFontResource;
}

void Screen::update() {
	if (_transitionEffect && _zoomFactor == 0 && _fadeType == kFadeNone) {
		screenTransitionEffect();
		_transitionEffect = false;
	} else if (_fadeType == kFadeIn) {
		paletteFadeIn();
	} else if (_fadeType == kFadeOut) {
		paletteFadeOut();
	} else if (_zoomFactor == 0) {
		_vm->_system->copyRectToScreen(getPixels(), 320, 0, 0, 320, 200);
		_vm->_system->updateScreen();
		_vm->syncUpdate(false);
	} else {
		updateZoomEffect(_zoomFactor, _zoomX, _zoomY);
		_vm->syncUpdate(false);
	}
}

void Screen::copyFromScreenResource(ScreenResource *screenResource) {
	memcpy(getPixels(), screenResource->getScreen(), 64000);
}

void Screen::copyFromScreen(byte *source) {
	memcpy(getPixels(), source, 64000);
}

void Screen::copyToScreen(byte *dest) {
	memcpy(dest, getPixels(), 64000);
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

void Screen::screenZoomEffect2x(Graphics::Surface *destSurface, Graphics::Surface *sourceSurface, int x, int y) {
	if (x - 80 < 0) x = 0;
	if (x > 159) x = 159;
	if (y - 80 < 0) y = 0;
	if (y > 99) y = 99;
	for (int yc = 0; yc < 100; ++yc) {
		byte *sourceRow = (byte*)sourceSurface->getBasePtr(x, y + yc);
		byte *destRow1 = (byte*)destSurface->getBasePtr(0, y + 2 * yc + 0);
		byte *destRow2 = (byte*)destSurface->getBasePtr(0, y + 2 * yc + 1);
		for (int xc = 0; xc < 160; ++xc) {
			const byte pixel = *sourceRow++;
			for (int p = 0; p < 2; ++p) {
				*destRow1++ = pixel;
				*destRow2++ = pixel;
			}
		}
	}
}

void Screen::screenZoomEffect3x(Graphics::Surface *destSurface, Graphics::Surface *sourceSurface, int x, int y) {
	if (x - 53 < 0) x = 0;
	if (x > 213) x = 213;
	if (y - 50 < 0) y = 0;
	if (y > 133) y = 133;
	for (int yc = 0; yc < 66; ++yc) {
		byte *sourceRow = (byte*)sourceSurface->getBasePtr(x, y + yc);
		byte *destRow1 = (byte*)destSurface->getBasePtr(0, y + 3 * yc + 0);
		byte *destRow2 = (byte*)destSurface->getBasePtr(0, y + 3 * yc + 1);
		byte *destRow3 = (byte*)destSurface->getBasePtr(0, y + 3 * yc + 2);
		byte pixel = 0;
		for (int xc = 0; xc < 106; ++xc) {
			pixel = *sourceRow++;
			for (int p = 0; p < 3; ++p) {
				*destRow1++ = pixel;
				*destRow2++ = pixel;
				*destRow3++ = pixel;
			}
		}
		for (int p = 0; p < 2; ++p) {
			*destRow1++ = pixel;
			*destRow2++ = pixel;
			*destRow3++ = pixel;
		}
	}
	// The remaining rows are not scaled at all
	for (int p = 0; p < 2; ++p) {
		byte *destRow = (byte*)destSurface->getBasePtr(0, 198 + p);
		memset(destRow, 0, 320);
	}
}

void Screen::screenZoomEffect4x(Graphics::Surface *destSurface, Graphics::Surface *sourceSurface, int x, int y) {
	if (x - 40 < 0) x = 0;
	if (x > 239) x = 239;
	if (y - 44 < 0) y = 0;
	if (y > 149) y = 149;
	for (int yc = 0; yc < 50; ++yc) {
		byte *sourceRow = (byte*)sourceSurface->getBasePtr(x, y + yc);
		byte *destRow1 = (byte*)destSurface->getBasePtr(0, y + 4 * yc + 0);
		byte *destRow2 = (byte*)destSurface->getBasePtr(0, y + 4 * yc + 1);
		byte *destRow3 = (byte*)destSurface->getBasePtr(0, y + 4 * yc + 2);
		byte *destRow4 = (byte*)destSurface->getBasePtr(0, y + 4 * yc + 3);
		for (int xc = 0; xc < 80; ++xc) {
			const byte pixel = *sourceRow++;
			for (int p = 0; p < 4; ++p) {
				*destRow1++ = pixel;
				*destRow2++ = pixel;
				*destRow3++ = pixel;
				*destRow4++ = pixel;
			}
		}
	}
}

void Screen::updateZoomEffect(int zoomFactor, int zoomX, int zoomY) {
	Graphics::Surface *destSurface = new Graphics::Surface();
	destSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	switch (zoomFactor) {
	case 1:
		screenZoomEffect2x(destSurface, this, zoomX, zoomY);
		break;
	case 2:
		screenZoomEffect3x(destSurface, this, zoomX, zoomY);
		break;
	case 3:
		screenZoomEffect4x(destSurface, this, zoomX, zoomY);
		break;
	default:
		break;
	}
	_vm->_system->copyRectToScreen(destSurface->getPixels(), 320, 0, 0, 320, 200);
	_vm->_system->updateScreen();
	destSurface->free();
	delete destSurface;
}

void Screen::screenTransitionEffect() {

	byte *vgaScreen = new byte[64000];

	Graphics::Surface *screen = _vm->_system->lockScreen();
	memcpy(vgaScreen, screen->getPixels(), 320 * 200);
	_vm->_system->unlockScreen();

	for (int i = 0; i < 7; i++) {
		byte *sourceBuf = (byte*)getBasePtr(i, 0);
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

}

void Screen::screenScrollEffect(byte *newScreen, int scrollDirection) {

	const int kScrollStripWidth = 40;

	Graphics::Surface *screen = _vm->_system->lockScreen();
	memcpy(getPixels(), screen->getPixels(), 320 * 200);
	_vm->_system->unlockScreen();

	int copyOfs = 0;
	
	while (copyOfs < 320) {
		if (scrollDirection < 0) {
			for (int y = 0; y < 200; y++) {
				memmove(getBasePtr(0, y), getBasePtr(kScrollStripWidth, y), 320 - kScrollStripWidth);
				memcpy(getBasePtr(320 - kScrollStripWidth, y), &newScreen[y * 320 + copyOfs], kScrollStripWidth);
			}
		} else {
			for (int y = 0; y < 200; y++) {
				memmove(getBasePtr(kScrollStripWidth, y), getBasePtr(0, y), 320 - kScrollStripWidth);
				memcpy(getBasePtr(0, y), &newScreen[y * 320 + (320 - kScrollStripWidth - copyOfs)], kScrollStripWidth);
			}
		}
		_vm->_system->copyRectToScreen(getPixels(), 320, 0, 0, 320, 200);
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
	setFadePalette(0);
	_vm->_system->copyRectToScreen(getPixels(), 320, 0, 0, 320, 200);
	paletteFadeCore(_fadeStep);
	setFadePalette(255);
	_fadeType = kFadeNone;
	_palFlag = false;
}

void Screen::paletteFadeOut() {
	setFadePalette(255);
	paletteFadeCore(-_fadeStep);
	setFadePalette(0);
	_fadeType = kFadeNone;
	_palFlag = true;
}

void Screen::paletteFadeCore(int fadeStep) {
	const uint32 kFadeUpdateDuration = 10;
	int value = fadeStep < 0 ? 255 : 0;
	uint32 nextTick = _vm->_system->getMillis();
	while ((fadeStep < 0 && value > 0) || (fadeStep >= 0 && value < 255)) {
		uint32 currTick = _vm->_system->getMillis();
		if (currTick >= nextTick) {
			if (currTick < nextTick + kFadeUpdateDuration)
				setFadePalette(value);
			nextTick = currTick + kFadeUpdateDuration;
			value += fadeStep;
		}
	}
}

void Screen::setFadePalette(int value) {
	buildPalette(_vm->_gamePalette, _vm->_screenPalette, value);
	setFullPalette(_vm->_screenPalette);
	_vm->_system->updateScreen();
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
	_currFontResource->drawText(x, y, (byte*)getPixels(), text, _currFontColor);
}

void Screen::drawTextOutlined(int x, int y, const byte *text, byte color1, byte color2) {
	byte *destBuffer = (byte*)getPixels();
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

Graphics::Surface *Screen::decompressAnimationCel(const byte *celData, int16 width, int16 height) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	const byte *src = celData;
	byte *dst = (byte*)surface->getPixels();
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

	Common::Array<Common::Point> points;

	// The commands' points need to be adjusted according to the parentFlags and the x/y position
	points.reserve(cmd->points.size() + 1);
	for (uint i = 0; i < cmd->points.size(); i++) {
		int16 ax = cmd->points[i].x;
		int16 ay = cmd->points[i].y;
		if (parentFlags & 0x80)
			ax = -ax;
		if (parentFlags & 0x20)
			ay = -ay;
		points.push_back(Common::Point(x + ax, y + ay));
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
		drawFilledPolygon(points, cmd->arg2, cmd->arg1);
		break;

	case kActRectangle:
		drawRectangle(points, cmd->arg2, cmd->arg1);
		break;

	case kActPolygon:
		drawPolygon(points, cmd->arg2);
		break;

	case kActPixels:
		drawPixels(points, cmd->arg2); 
		break;

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
		color1 = interCmd->_aarg1;
		color2 = interCmd->_aarg2;
		drawInterpolatedAnimationCommand(interCmd, x, y, mulValue, color1, color2);
	}
}

void Screen::drawInterpolatedAnimationCommand(InterpolatedAnimationCommand *interCmd, int16 x, int16 y, int mulValue, byte color1, byte color2) {

	debug(8, "Screen::drawInterpolatedAnimationCommand() cmd = %d; points = %d", interCmd->_cmd, interCmd->_points.size());

	Common::Array<Common::Point> points;

	// The commands' points need to be adjusted according to the x/y position
	points.reserve(interCmd->_points.size() + 1);
	for (uint pointIndex = 0; pointIndex < interCmd->_points.size(); pointIndex += 2) {
		int x1, y1;
		Common::Point *pt1 = &interCmd->_points[pointIndex + 0];
		Common::Point *pt2 = &interCmd->_points[pointIndex + 1];
		x1 = x + pt1->x + ((pt2->x - pt1->x) * mulValue) / 256;
		y1 = y + pt1->y + ((pt2->y - pt1->y) * mulValue) / 256;
		points.push_back(Common::Point(x1, y1));
	}

	switch (interCmd->_cmd) {

	case kActElement:
		warning("Screen::drawInterpolatedAnimationCommand() kActElement not supported here");
		break;

	case kActCelSprite:
		warning("Screen::drawInterpolatedAnimationCommand() kActCelSprite not supported here");
		break;

	case kActFilledPolygon:
		drawFilledPolygon(points, color2, color1);
		break;

	case kActRectangle:
		drawRectangle(points, color2, color1);
		break;

	case kActPolygon:
		drawPolygon(points, color2);
		break;

	case kActPixels:
		drawPixels(points, color2); 
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

} // End of namespace Comet
