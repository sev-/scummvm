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

#ifndef COMET_SCREEN_H
#define COMET_SCREEN_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/util.h"

#include "comet/comet.h"
#include "comet/graphics.h"
#include "comet/resource.h"

namespace Comet {

enum PaletteFadeType {
	kFadeNone,
	kFadeIn,
	kFadeOut
};

class Screen : public CometSurface {
public:
	Screen(CometEngine *vm);
	~Screen();
	
	void update();

	void copyFromScreenResource(ScreenResource *screenResource);
	void copyFromScreen(byte *source);
	void copyToScreen(byte *dest);
	
	void enableTransitionEffect();
	void setZoom(int zoomFactor, int x, int y);
	void setFadeType(PaletteFadeType fadeType);
	PaletteFadeType getFadeType() const { return _fadeType; }
	void setFadeStep(int fadeStep);
	int getZoomFactor() const { return _zoomFactor; }

	void screenTransitionEffect();
	void screenScrollEffect(byte *newScreen, int scrollDirection);

	void buildPalette(byte *sourcePal, byte *destPal, int value);
	void buildRedPalette(byte *sourcePal, byte *destPal, int value);
	void paletteFadeIn();
	void paletteFadeOut();
	void paletteFadeCore(int fadeStep);
	void setFadePalette(int value);
	void setWhitePalette(int value);

	void setPartialPalette(byte *palette, int start, int count);
	void setFullPalette(byte *palette);

	void loadFont(const char *pakName, int index);
	void loadFontFromRaw(const byte *rawData, uint32 rawDataSize, int maxCount, int index);
	void setFontColor(byte color);
	void drawText(int x, int y, const byte *text);
	void drawTextOutlined(int x, int y, const byte *text, byte color1, byte color2);
	int drawText3(int x, int y, const byte *text, byte color, int flag);
	int getTextWidth(const byte *text);
	int getTextHeight(const byte *text);

	void drawAnimationElement(AnimationResource *animation, int16 elementIndex, int16 x, int16 y, byte parentFlags = 0);
	void drawAnimationCommand(AnimationResource *animation, AnimationCommand *cmd, int16 x, int16 y, byte parentFlags = 0);
	void drawInterpolatedAnimationElement(InterpolatedAnimationElement *interElem, int16 x, int16 y, int mulValue);
	void drawInterpolatedAnimationCommand(InterpolatedAnimationCommand *interCmd, int16 x, int16 y, int mulValue, byte color1, byte color2);
	void buildInterpolatedAnimationElement(AnimationElement *elem1, AnimationElement *elem2,
		InterpolatedAnimationElement *interElem);

	int drawAnimation(AnimationResource *animation, AnimationFrameList *frameList, int frameIndex, int interpolationStep, int x, int y, int frameCount);

	void screenZoomEffect2x(Graphics::Surface *destSurface, Graphics::Surface *sourceSurface, int x, int y);
	void screenZoomEffect3x(Graphics::Surface *destSurface, Graphics::Surface *sourceSurface, int x, int y);
	void screenZoomEffect4x(Graphics::Surface *destSurface, Graphics::Surface *sourceSurface, int x, int y);
	void updateZoomEffect(int zoomFactor, int zoomX, int zoomY);

protected:
	CometEngine *_vm;
	bool _transitionEffect;
	PaletteFadeType _fadeType;
	int _fadeStep;
	FontResource *_currFontResource;
	byte _currFontColor;
public:
	bool _palFlag;
	int _zoomFactor, _zoomX, _zoomY;
};

}

#endif
