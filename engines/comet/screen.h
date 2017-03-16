#ifndef COMET_SCREEN_H
#define COMET_SCREEN_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/util.h"

#include "comet/comet.h"
#include "comet/resource.h"

namespace Comet {

enum PaletteFadeType {
	kFadeNone,
	kFadeIn,
	kFadeOut
};

struct Point {
	int16 x, y;
	Point() : x(0), y(0) {}
	Point(int16 px, int16 py) : x(px), y(py) {}
};

enum AnimationCommandType {
	kActElement			= 0,
	kActCelSprite		= 1,
	kActNop0			= 2,
	kActNop1			= 3,
	kActFilledPolygon	= 4,
	kActRectangle		= 5,
	kActPolygon			= 6,
	kActPixels			= 7,
	kActPolygon1		= 8,	// unused in Comet? / Alias for kActPolygon
	kActPolygon2		= 9,	// unused in Comet? / Alias for kActPolygon
	kActCelRle			= 10
};

struct InterpolatedAnimationCommand {
	byte _cmd;
	byte _aarg1, _aarg2, _barg1, _barg2;
	Common::Array<Point> _points;
	InterpolatedAnimationCommand(byte cmd, byte aarg1, byte aarg2, byte barg1, byte barg2);
};

struct InterpolatedAnimationElement {
	Common::Array<InterpolatedAnimationCommand*> commands;
	~InterpolatedAnimationElement();
};

class Screen {
public:
	Screen(CometEngine *vm);
	~Screen();
	
	void update();
	
	void enableTransitionEffect();
	void setZoom(int zoomFactor, int x, int y);
	void setFadeType(PaletteFadeType fadeType);
	PaletteFadeType getFadeType() const { return _fadeType; }
	void setFadeStep(int fadeStep);
	int getZoomFactor() const { return _zoomFactor; }

	void screenZoomEffect2x(int x, int y);
	void screenZoomEffect3x(int x, int y);
	void screenZoomEffect4x(int x, int y);
	void screenTransitionEffect();
	void screenScrollEffect(byte *newScreen, int scrollDirection);

	void buildPalette(byte *sourcePal, byte *destPal, int value);
	void buildRedPalette(byte *sourcePal, byte *destPal, int value);
	void paletteFadeIn();
	void paletteFadeOut();
	void setWhitePalette(int value);

	void putPixel(int x, int y, byte color);
	void drawLine(int x1, int y1, int x2, int y2, byte color);
	void drawDottedLine(int x1, int y1, int x2, int y2, int color);
	void hLine(int x, int y, int x2, byte color);
	void vLine(int x, int y, int y2, byte color);
	void fillRect(int x1, int y1, int x2, int y2, byte color);
	void frameRect(int x1, int y1, int x2, int y2, byte color);
	void filledPolygonColor(Common::Array<Point> &poly, byte color);

	void setPartialPalette(byte *palette, int start, int count);
	void setFullPalette(byte *palette);

	void clear();

	byte *getScreen() const {
		return _workScreen;
	}

	void loadFont(const char *pakName, int index);
	void setFontColor(byte color);
	void drawText(int x, int y, byte *text);
	void drawTextOutlined(int x, int y, byte *text, byte color1, byte color2);
	int drawText3(int x, int y, byte *text, byte color, int flag);

	static void plotProc(int x, int y, int color, void *data);
	static void dottedPlotProc(int x, int y, int color, void *data);

	// New Animation drawing code
	void drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, byte flags = 0);
	void drawAnimationCelRle(AnimationCel &cel, int16 x, int16 y);
	void drawAnimationElement(AnimationResource *animation, int16 elementIndex, int16 x, int16 y, byte parentFlags = 0);
	void drawAnimationCommand(AnimationResource *animation, AnimationCommand *cmd, int16 x, int16 y, byte parentFlags = 0);
	void drawInterpolatedAnimationElement(InterpolatedAnimationElement *interElem, int16 x, int16 y, int mulValue);
	void drawInterpolatedAnimationCommand(InterpolatedAnimationCommand *interCmd, int16 x, int16 y, int mulValue, byte color1, byte color2);
	void buildInterpolatedAnimationElement(AnimationElement *elem1, AnimationElement *elem2,
		InterpolatedAnimationElement *interElem);

	int drawAnimation(AnimationResource *animation, AnimationFrameList *frameList, int frameIndex, int interpolationStep, int x, int y, int frameCount);

	void setClipRect(int clipX1, int clipY1, int clipX2, int clipY2);
	void setClipX(int clipX1, int clipX2);
	void setClipY(int clipY1, int clipY2);

//protected:

	CometEngine *_vm;

	byte *_workScreen;

	bool _transitionEffect;
	int _zoomFactor, _zoomX, _zoomY;
	
	PaletteFadeType _fadeType;
	int _fadeStep;
	bool _palFlag;
	
	int _clipX1, _clipY1, _clipX2, _clipY2;

	int _dotFlag;

	//byte *_paletteBuffer;

// FIXME
public:
	Font *_font;

protected:
	// FIXME: Remove static vars
	static int *gfxPrimitivesPolyInts;
	static uint gfxPrimitivesPolyAllocated;

};

}

#endif
