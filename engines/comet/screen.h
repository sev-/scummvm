#ifndef COMET_SCREEN_H
#define COMET_SCREEN_H

#include "common/scummsys.h"
#include "common/util.h"

#include "comet/comet.h"

namespace Comet {

enum PaletteFadeType {
	kFadeNone,
	kFadeIn,
	kFadeOut
};

class Screen {
public:
    Screen(CometEngine *vm);
    ~Screen();
    
    void update();
    
    void enableTransitionEffect();
	void setZoom(int zoomFactor, int x, int y);
	void setFadeType(PaletteFadeType fadeType);
	void setFadeValue(int fadeValue);

	void screenZoomEffect2x(int x, int y);
	void screenZoomEffect3x(int x, int y);
	void screenZoomEffect4x(int x, int y);
	void screenTransitionEffect();

	void buildPalette(byte *sourcePal, byte *destPal, int value);
	void paletteFadeIn();
	void paletteFadeOut();

    void hLine(int x, int y, int x2, byte color);
    void vLine(int x, int y, int y2, byte color);
    void fillRect(int x1, int y1, int x2, int y2, byte color);
    void frameRect(int x1, int y1, int x2, int y2, byte color);
    void filledPolygonColor(PointArray &poly, byte color);
    void setPartialPalette(byte *palette, int start, int count);
    void setFullPalette(byte *palette);

	void clearScreen();

    byte *getScreen() const {
        return _workScreen;
	}

	void loadFont(const char *pakName, int index);
	void setFontColor(byte color);
    void drawText(int x, int y, const char *text);
    int drawText3(int x, int y, char *text, byte color, int flag);

protected:

    CometEngine *_vm;

	byte *_workScreen;

	bool _screenTransitionEffectFlag;
	int _screenZoomFactor, _screenZoomXPos, _screenZoomYPos;
	
	PaletteFadeType _fadeType;
    int _fadeValue;
	bool _palFlag;

    //byte *_paletteBuffer;

// FIXME
public:
    Font *_font;

protected:
    static int *gfxPrimitivesPolyInts;
    static int gfxPrimitivesPolyAllocated;

};

}

#endif
