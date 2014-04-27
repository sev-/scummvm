/*
 * AndroidPortAdditions.h
 *
 *  Created on: Jan 14, 2013
 *      Author: omergilad
 */

#ifndef ANDROIDPORTADDITIONS_H_
#define ANDROIDPORTADDITIONS_H_

#include "common/rect.h"
#include "common/hash-str.h"
#include "graphics/surface.h"
#include "graphics/scaler/hq.h"

#include "engines/engine.h"
#include "common/error.h"
#include "common/mutex.h"

#include "AndroidPortGlTextureHelper.h"
#include "AndroidPortUtils.h"
#include "GameAreaIdentifier.h"
#include "HitAreaHelper.h"
#include "graphics/Drawable.h"
#include "graphics/AlphaAnimation.h"

#include <GLES/gl.h>

#include <list>

#include "AndroidBitmap.h"

#include "Constants.h"

using Graphics::Surface;
using Graphics::PixelFormat;

using namespace Common;

/**
 * NONE - no touch going on
 * DOWN - touch is currently on (scroll, tap or hold)
 * UP - finger was just raised
 */
enum TouchState {
	NONE = 0, DOWN, UP, TAP, DOUBLE_TAP, FLING
};

typedef Common::HashMap<String, AndroidBitmap*, IgnoreCase_Hash,
		IgnoreCase_EqualTo> BitmapHashMap;

class AndroidPortAdditions {

public:

	static inline AndroidPortAdditions* instance() {
		if (sInstance == NULL) {
			sInstance = new AndroidPortAdditions();
		}

		return sInstance;
	}

	static void release();

	bool shouldUseModifiedGamePixels();

	void setTouchpadMode(bool touchpadMode);

	bool getTouchpadMode();

	/**
	 * Translates the received touch coordinates if the game screen was adjusted.
	 */
	void translateTouchCoordinates(int& x, int& y);

	byte* getModifiedGamePixels();

	void onShowMouse(bool visible);

	void setDisplayDimensions(uint32 width, uint32 height);

//	Rect getDirtyRect();

	void initGLESResources();

	inline ShaderProgram* getDefaultShaderProgram() {
		return mDefaultShaderProgram;
	}

	inline ShaderProgram* getScalerShaderProgram() {
		return mScalerShaderProgram;
	}

	void setGameTextureInfo(GLuint gameTexture, uint16 texWidth,
			uint16 texHeight);

	void onDrawTextureToScreen(GLshort x, GLshort y, GLshort w, GLshort h);

	void addBitmapResource(String key, AndroidBitmap* bitmap);

	void onMouseClick(int x, int y);

	bool onTapEvent(int x, int y, bool doubleTap = false);

	bool onLongClickEvent(int x, int y);

	bool onDoubleTapEvent(int x, int y);

	bool onScrollEvent(int x, int y);

	bool onFlingEvent(int x1, int y1, int x2, int y2);

	bool onDownEvent(int x, int y);

	bool onUpEvent(int x, int y);

	void beforeDrawTextureToScreen(Surface* gameSurface);

	bool saveGame(int32 slot);

	bool loadGame(int32 slot);

	void setSlotToLoad(int32 slot, bool force = false);

	void setSlotToSave(int32 slot, bool force = false);

	void setAutoLoadSlot(int32 slot);

	/**
	 * Returns whether we need to skip movies immediately without a fade out animation - used for auto-load
	 */
	bool isInAutoloadState();

	uint16* scale(const uint8 *srcPtr, uint16 x, uint16 y, uint16 w, uint16 h);

	inline uint16 getScalingFactor() {

		return mScalingFactor;
	}

	inline float getLQShaderScalingFactor() {

		return mLQShaderScalingFactor;
	}

	void setScalingOption(uint16 option);

	inline uint16 getScalingOption() {
		return mScalingOption;
	}

	inline void setGameType(uint16 gameType)
	{
		mGameType = gameType;
	}

	inline uint16 getGameType() {
		return mGameType;
	}

	void addShaderSource(const char* source, uint32 type);

	void gameEvent(int32 type);

	inline bool shouldMeasureRenderTime() {
		return mShouldTestShader;
	}

	void onRenderTimeMeasure(uint64 time);

	void onGameIdleCounter();

	bool checkLoadConditions();

	void onFastFadeInStarted();

	void onSystemInitialized();

	bool shouldUseWalkAction();

	/**
	 * Should be called by the engine when the chosen action has changed
	 */
	void onActionChanged(uint16 action);

	/**
	 * Should be called by the engine when an action was clicked
	 */
	void onActionClicked(uint16 action);

	inline uint16 getShaderScalingMaxResolutionW()
	{
		return mShaderScalingMaxResolutionW;
	}

	inline uint16 getShaderScalingMaxResolutionH()
		{
			return mShaderScalingMaxResolutionH;
		}

private:

	bool checkGameOverlayClicks(int x, int y, bool performOverlayAction = true);

	bool checkClick(uint32 x, uint32 y, uint32 buttonX, uint32 buttonY,
			uint32 buttonW, uint32 buttonH);

	AndroidBitmap* getBitmap(String key);

	void drawBitmapAsGlTexture(AndroidBitmap* bitmap, float x, float y,
			float width = 0, float height = 0, float alpha = 1.0);

	void drawAnimationDrawable(const DrawablePtr drawable);

	void drawAnimations();

	void drawHotspotIndicator(AndroidBitmap* bitmap, int x, int y,
			AndroidBitmap* action, float alpha = 1.0);

	void setupCurrentHotspotAnimation(Point hotspot);

	void clearCurrentHotspotAnimation(long fadeoutDuration,
			bool completeFadeIn);

	void clearCurrentActionIconAnimation(bool actionPerformed);

	/**
	 * hotspotDrawable, actionDrawable - output arguments
	 *
	 * @return True if action was generated
	 */
	bool generateHotspotIndicatorDrawables(AndroidBitmap* bitmap, int x, int y,
			AndroidBitmap* action, DrawablePtr hotspotDrawable,
			DrawablePtr actionDrawable, float alpha = 1.0);

	void addBigActionFadeAnimation(AndroidBitmap* bitmap);

	void addHotspotFadeoutAnimation(Point hotspot, long duration);

	void addActionBlinkAnimation(Point hotspot, AndroidBitmap* action);

	void addWalkFadeoutAnimation(int x, int y);

	void initGlTextures();

	void initBitmapInGlTextures(AndroidBitmap* bitmap, float width,
			bool sourceContainsAlpha = false);

	void performSkip(bool showAnimation = true);

	void performRevealItems();

	void performInventoryScroll(bool up);

	void moveBlackPanel(Surface* gameSurface);

	void checkGameInChat(Surface* gameSurface);

	void checkGameInPostcard(Surface* gameSurface);

	void checkBottomToolbar(Surface* gameSurface);

	/**
	 * Copy a rectangle between 2 surfaces.
	 * Assumes same pixel format.
	 */
	void copyPixelsBetweenSurfaces(Surface* src, Surface* dst, Rect srcRect,
			Point dstPoint, bool startFromBottom = false);

	void copyPixelsToMagnifier();

	void requestMagnifierForCoordinates(int32 x, int32 y);

	bool isValidMagnifierCoordinates(int32 x, int32 y);

	void pushClickEvent(int32 x, int32 y);

	void pushScrollEvent(int x, int y);

	//Action recognizeActionPress(int32 x, int32 y);

	AndroidBitmap* getActionIcon(uint16 action);

	void preventIdleMode();

	void gameTouchBehavior();

	void bottomToolbarClickBehavior(int16 gameTouchX, int16 gameTouchY);

	bool canSkip();

	bool canShowRevealItems();

	bool canShowMenuButton();

	/**
	 * Safety measure for touch events - makes sure the events fall into the known surface dimensions.
	 * Solves an issue on Jelly Bean where soft buttons trigger irrelevant touch events.
	 */
	inline void touchEventSafety(int& x, int& y) {
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;
		if (x >= mDisplayWidth)
			x = mDisplayWidth - 1;
		if (y >= mDisplayHeight)
			y = mDisplayHeight - 1;
	}

	void autoloadBehavior();

	void loadIfNeeded();

	void saveIfNeeded();

	bool checkSaveConditions();

	void resetSaveState();

	/**
	 * Set the current game touched point according to screen touch
	 */
	void gameTouchEvent(int16 x, int16 y, int16 origX, int16 origY,
			TouchState touchState);

	void assistWalkCoordinates(int16& x, int16& y);

	void chatArrowClick(bool up);

	void chatButtonClick();

	uint16 getCurrentAction();

	void fallbackToSoftwareScaler();

	void fallbackToLQHardwareScaler();

	void outsideDragonCaveWorkaround(int16 &x, int16 &y);

	uint16 mGameType;

	BitmapHashMap mBitmaps;
	GLuint* mGlTextures;

	AndroidPortGlTextureHelper mGlTextureHelper;

	bool mMouseVisible;

	uint16 mDisplayWidth;
	uint16 mDisplayHeight;

	// Width / height
	float mDisplayRatio;

	Surface mBlackPanelTempSurface;
	Surface mModifiedGameSurface;

	bool mShouldUseModifiedGamePixels;

	bool mClassicMode;

	bool mGameInChat;
	bool mBottomToolbarAppearing;
	bool mGameInPostcard;

	GameArea mCurrentGameArea;

	uint16 mTouchTranslationOffset;
	uint16 mTouchTranslationLowerBound;

	bool mDumpGameScreenToFile;

	// Coordinates for drawing magnifier.
	// -1 means no magnifier should be drawn
	int16 mMagnifierX;
	int16 mMagnifierY;
	// Current magnifier touch coordinates (only inside game area)
	int16 mMagnifierTouchX;
	int16 mMagnifierTouchY;

	int16 mGameTouchX;
	int16 mGameTouchY;
	TouchState mTouchState;
	Common::Mutex* mTouchEventMutex;
	Hotspot mCurrentHotspot;
	bool mDisallowNextTouchEvent;

	DrawablePtr mCurrentHotspotDrawable;
	DrawablePtr mCurrentActionDrawable;

	uint16 mRevealItemsFadeCounter;

	float mMagnifierCopyRectWidth;
	float mMagnifierCopyRectHeight;

	float mGameToDisplayRatioW;
	float mGameToDisplayRatioH;

	uint16 mCurrentAction;

	uint16 mActionIconFadeCounter;
	uint16 mSkipIconFadeCounter;

	bool mGameDisplayStarted;

	uint64 mLastTouchMoveTimestamp;

	uint64 mFirstDrawTimestamp;

	int32 mSlotToLoad;
	int32 mSlotToSave;
	uint64 mSaveStartTimestamp;
	bool mSaveInProgress;
	bool mGameIdleCountWhileSaving;
	bool mForceSaveLoad;

	int32 mAutoLoadSlot;
	uint mAutoLoadSpamSkipCounter;
	bool mAutoLoadState;

	bool mGameStartedInAutoload;

	uint16* mScaledOutput;

	ScalerPluginObject* mScalerPlugin;

	uint16 mScalingFactor;
	float mLQShaderScalingFactor;

	uint16 mScalingOption;

	ShaderProgram* mDefaultShaderProgram;
	ShaderProgram* mScalerShaderProgram;
	ShaderProgram* mBlackShaderProgram;

	bool mShouldTestShader;
	uint16 mRenderMeasureSkippedFrameCount;
	uint16 mRenderMeasureCount;
	double mRenderMeasureAverage;
	uint64 mShaderTestStartTime;

	uint64 mLastIdlePreventionTimestamp;

	bool mShouldPerformRevealItems;

	const char* mScalerVertexSource;
	const char* mScalerFragmentSource;
	const char* mScalerLQVertexSource;
	const char* mScalerLQFragmentSource;

	uint16 mGameTextureW;
	uint16 mGameTextureH;
	GLuint mGameTexture;

	bool mSkipPressedAtLeastOnce;

	HitAreaHelper mHitAreaHelper;

	std::list<DrawablePtr> mAnimatedDrawables;

	// 1-based index, 0 means no selection
	int16 mSelectedChatRow;

	bool mShouldPerformSimon1PuddleWorkaround;

	// Maximum resolution for shader scaling (for performance reasons)
	uint16 mShaderScalingMaxResolutionW;
	uint16 mShaderScalingMaxResolutionH;

	static AndroidPortAdditions* sInstance;

	AndroidPortAdditions();
	virtual ~AndroidPortAdditions();
};

#endif /* ANDROIDPORTADDITIONS_H_ */
