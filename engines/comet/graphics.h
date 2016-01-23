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

#ifndef COMET_GRAPHICS_H
#define COMET_GRAPHICS_H

#include "comet/resource.h"
#include "common/array.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Comet {

struct DottedLineData {
	Graphics::Surface *_surface;
	int _dotCounter;
};

class CometSurface : public Graphics::Surface {
public:
	CometSurface();
	~CometSurface();
	void grabRect(Graphics::Surface *surface, int x, int y);
	void putRect(Graphics::Surface *surface, int x, int y);
	void putPixel(int x, int y, byte color);
	void drawLine(int x1, int y1, int x2, int y2, byte color);
	void drawDottedLine(int x1, int y1, int x2, int y2, int color);
	void hLine(int x, int y, int x2, byte color);
	void fillRect(int x1, int y1, int x2, int y2, byte color);
	void frameRect(int x1, int y1, int x2, int y2, byte color);
	void drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, byte flags = 0);
	void drawAnimationCelRle(AnimationCel &cel, int16 x, int16 y);
	void drawFilledPolygon(Common::Array<Common::Point> &poly, byte fillColor, byte outlineColor);
	void drawPolygon(Common::Array<Common::Point> &poly, byte color);
	void drawRectangle(Common::Array<Common::Point> &points, byte fillColor, byte outlineColor);
	void drawPixels(Common::Array<Common::Point> &points, byte color);
	void clear();
	void setClipRect(int clipX1, int clipY1, int clipX2, int clipY2);
	void setClipX(int clipX1, int clipX2);
	void setClipY(int clipY1, int clipY2);
protected:
	int _clipX1, _clipY1, _clipX2, _clipY2;
	void clipPolygonLeft(Common::Array<Common::Point> **poly, int16 clipLeft);
	void clipPolygonRight(Common::Array<Common::Point> **poly, int16 clipRight);
	void clipPolygonTop(Common::Array<Common::Point> **poly, int16 clipTop);
	void clipPolygonBottom(Common::Array<Common::Point> **poly, int16 clipBottom);
	void drawFilledPolygonBody(Common::Array<Common::Point> &poly, byte color);
};

class InterpolatedAnimationCommand {
public:
	Common::Array<Common::Point> _points;
	InterpolatedAnimationCommand(byte cmd, byte aarg1, byte aarg2, byte barg1, byte barg2);
	void draw(CometSurface *destSurface, int16 x, int16 y, int mulValue);
protected:
	byte _cmd;
	byte _aarg1, _aarg2, _barg1, _barg2;
};

class InterpolatedAnimationElement {
public:
	~InterpolatedAnimationElement();
	void build(AnimationElement *elem1, AnimationElement *elem2);
	void draw(CometSurface *destSurface, int16 x, int16 y, int mulValue);
protected:
	Common::Array<InterpolatedAnimationCommand*> _commands;
};

class BaseMouseCursor {
public:
	void setCursor(const byte **currentCursor);
	virtual ~BaseMouseCursor() {}
protected:
	virtual Graphics::Surface *createCursorSurface() = 0;
	virtual const byte *getCursorData() = 0;
};

class AnimationCelMouseCursor : public BaseMouseCursor {
public:
	AnimationCelMouseCursor(const AnimationCel *cel) : _cel(cel) {};
protected:
	const AnimationCel *_cel;
	virtual Graphics::Surface *createCursorSurface();
	virtual const byte *getCursorData();
};

class SystemMouseCursor : public BaseMouseCursor {
protected:
	virtual Graphics::Surface *createCursorSurface();
	virtual const byte *getCursorData();
};

Graphics::Surface *decompressAnimationCel(const byte *celData, int width, int height);

}

#endif
