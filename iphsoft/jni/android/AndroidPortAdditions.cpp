/*
 * AndroidPortAdditions.cpp
 *
 *  Created on: Jan 14, 2013
 *      Author: omergilad
 */

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni.h"

#include "AndroidPortAdditions.h"

#include "graphics/AlphaAnimation.h"
#include "graphics/ScaleAnimation.h"
#include "graphics/WaitForConditionAnimation.h"
#include "graphics/ParallelAnimation.h"

#include "graphics/SequenceAnimationComposite.h"
#include "graphics/RepeatAnimationWrapper.h"
#include "graphics/DeccelerateInterpolator.h"
#include "graphics/AccelerateInterpolator.h"

#include "event_types.h"

using std::shared_ptr;

// Suppress the compiler warning about an (unsigned int >= 0).
// The constant that contains 0 might change later.
#pragma GCC diagnostic ignored "-Wtype-limits"

static inline GLfixed xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}

AndroidPortAdditions* AndroidPortAdditions::sInstance = NULL;

AndroidPortAdditions::AndroidPortAdditions()
		:
				mDisplayRatio(0),
				mRenderMeasureSkippedFrameCount(0),
				mShouldUseModifiedGamePixels(false),
				mSelectedChatRow(0),
				mBitmaps(),
				mGlTextures(NULL),
				mShouldPerformRevealItems(false),
				mMouseVisible(false),
				mClassicMode(false),
				mTouchTranslationOffset(0),
				mGameInChat(false),
				mDumpGameScreenToFile(false),
				mMagnifierX(-1),
				mMagnifierY(-1),
				mMagnifierTouchX(0),
				mMagnifierTouchY(0),
				mCurrentAction(ACTION_WALK),
				mTouchState(NONE),
				mDisallowNextTouchEvent(false),
				mCurrentHotspot(Point(0, 0), Point(0, 0)),
				mLastTouchMoveTimestamp(0),
				mBottomToolbarAppearing(false),
				mActionIconFadeCounter(0),
				mRevealItemsFadeCounter(0),
				mFirstDrawTimestamp(0),
				mGameInPostcard(false),
				mSkipIconFadeCounter(0),
				mGameDisplayStarted(false),
				mAutoLoadSlot(-1),
				mAutoLoadSpamSkipCounter(0),
				mShaderTestStartTime(0),
				mScalingFactor(2),
				mLQShaderScalingFactor(2.0),
				mScalingOption(SCALING_OPTION_SOFT),
				mScaledOutput(NULL),
				mScalerPlugin(NULL),
				mShouldTestShader(false),
				mRenderMeasureCount(0),
				mRenderMeasureAverage(0.0),
				mSkipPressedAtLeastOnce(false),
				mSlotToLoad(-1),
				mSlotToSave(-1),
				mLastIdlePreventionTimestamp(0),
				mSaveStartTimestamp(0),
				mSaveInProgress(false),
				mGameIdleCountWhileSaving(false),
				mCurrentGameArea(OTHER),
				mGameStartedInAutoload(false),
				mForceSaveLoad(false),
				mAutoLoadState(false),
				mCurrentHotspotDrawable(NULL),
				mCurrentActionDrawable(NULL),
				mShouldPerformSimon1PuddleWorkaround(false),
				mShaderScalingMaxResolutionW(SHADER_SCALING_MAX_RESOLUTION_W),
				mShaderScalingMaxResolutionH(SHADER_SCALING_MAX_RESOLUTION_H) {

}

bool AndroidPortAdditions::shouldUseWalkAction() {
	return (mLastTouchMoveTimestamp != 0
			&& (AndroidPortUtils::getTimeOfDayMillis() - mLastTouchMoveTimestamp
					>= WALK_ACTION_MIN_TOUCH_TIME));
}

void AndroidPortAdditions::clearCurrentActionIconAnimation(
		bool actionPerformed) {

	if (mCurrentActionDrawable == NULL) {
		return;
	}

	AnimationPtr actionEndAnimation;
	if (actionPerformed) {

		// Create the initial fade in animation
		shared_ptr<AlphaAnimation> initialFadeInAnimation(new AlphaAnimation());
		initialFadeInAnimation->setDuration(ACTION_BLINK_FADE_DURATION);
		initialFadeInAnimation->setStartAlpha(
				mCurrentActionDrawable->getAlpha());
		initialFadeInAnimation->setEndAlpha(1);

		// Create the loop fade in animation
		shared_ptr<AlphaAnimation> loopFadeInAnimation(new AlphaAnimation());
		loopFadeInAnimation->setDuration(ACTION_BLINK_FADE_DURATION);
		loopFadeInAnimation->setStartAlpha(0);
		loopFadeInAnimation->setEndAlpha(1);

		// Create a fade out animation
		shared_ptr<AlphaAnimation> fadeOutAnimation(new AlphaAnimation());
		fadeOutAnimation->setDuration(ACTION_BLINK_FADE_DURATION);
		fadeOutAnimation->setStartAlpha(1);
		fadeOutAnimation->setEndAlpha(0);

		// Create a sequence for fade in \ fade out
		shared_ptr<SequenceAnimationComposite> fadeSequenceAnimation(
				new SequenceAnimationComposite());
		fadeSequenceAnimation->addAnimation(loopFadeInAnimation);
		fadeSequenceAnimation->addAnimation(fadeOutAnimation);

		// Create a repeatable animation
		// We repeat times - 1 because the first "repeat" is done separately, to consider the starting alpha value
		shared_ptr<RepeatAnimationWrapper> repeatAnimation(
				new RepeatAnimationWrapper(fadeSequenceAnimation,
						ACTION_BLINK_REPEAT_COUNT - 1));

		// Create a sequence of the initial fade in, and the loop
		shared_ptr<SequenceAnimationComposite> actionBlinkAnimation(
				new SequenceAnimationComposite());
		actionBlinkAnimation->addAnimation(initialFadeInAnimation);
		actionBlinkAnimation->addAnimation(fadeOutAnimation);
		actionBlinkAnimation->addAnimation(repeatAnimation);

		actionEndAnimation = actionBlinkAnimation;
	} else {
		// Create a fade out animation
		shared_ptr<AlphaAnimation> fadeOutAnimation(new AlphaAnimation());
		fadeOutAnimation->setDuration(ACTION_LEAVE_FADEOUT_DURATION);
		fadeOutAnimation->setStartAlpha(mCurrentActionDrawable->getAlpha());
		fadeOutAnimation->setEndAlpha(0);
		fadeOutAnimation->setInterpolator(
				InterpolatorPtr(new DeccelerateInterpolator));

		actionEndAnimation = fadeOutAnimation;
	}

	actionEndAnimation->start(AndroidPortUtils::getTimeOfDayMillis());
	mCurrentActionDrawable->setAnimation(actionEndAnimation);
	mCurrentActionDrawable = NULL;
}

bool AndroidPortAdditions::canSkip() {
	if (g_engine == NULL) {
		return false;
	}

	return (!mMouseVisible && g_engine->canSkip());
}

bool AndroidPortAdditions::canShowRevealItems() {
	return mBottomToolbarAppearing && mMouseVisible;
}

bool AndroidPortAdditions::canShowMenuButton() {
	return (mBottomToolbarAppearing || mGameInChat) && mMouseVisible;
}

uint16 AndroidPortAdditions::getCurrentAction() {
	if (g_engine == NULL) {
		return ACTION_WALK;
	}

	return g_engine->getCurrentActionId();
}

void AndroidPortAdditions::onActionChanged(uint16 action) {
	mCurrentAction = action;
}

void AndroidPortAdditions::onActionClicked(uint16 action) {

//	addBigActionFadeAnimation(getActionIcon(action));
}

void AndroidPortAdditions::addBigActionFadeAnimation(AndroidBitmap* bitmap) {

	// Add a fade out animation for the action
	DrawablePtr actionDrawable(new Drawable);
	actionDrawable->setBitmap(bitmap);
	actionDrawable->setPositionX(BIG_ACTION_ICON_X);
	actionDrawable->setPositionY(BIG_ACTION_ICON_Y);
	actionDrawable->setWidth(BIG_ACTION_ICON_W);

	shared_ptr<AlphaAnimation> fadeInAnimation(new AlphaAnimation);
	fadeInAnimation->setDuration(BIG_ACTION_ICON_FADEIN_DURATION);
	fadeInAnimation->setStartAlpha(0);
	fadeInAnimation->setEndAlpha(BIG_ACTION_ICON_FADE_MAX_ALPHA);
	fadeInAnimation->setInterpolator(
			InterpolatorPtr(new DeccelerateInterpolator));

	shared_ptr<AlphaAnimation> fadeOutAnimation(new AlphaAnimation);
	fadeOutAnimation->setDuration(BIG_ACTION_ICON_FADEOUT_DURATION);
	fadeOutAnimation->setStartAlpha(BIG_ACTION_ICON_FADE_MAX_ALPHA);
	fadeOutAnimation->setEndAlpha(0);
	fadeOutAnimation->setInterpolator(
			InterpolatorPtr(new AccelerateInterpolator));

	shared_ptr<SequenceAnimationComposite> actionAnimation(
			new SequenceAnimationComposite);
	actionAnimation->addAnimation(fadeInAnimation);
	actionAnimation->addAnimation(fadeOutAnimation);

	// Add to animation list
	actionAnimation->start(AndroidPortUtils::getTimeOfDayMillis());
	actionDrawable->setAnimation(AnimationPtr(actionAnimation));
	mAnimatedDrawables.push_back(actionDrawable);
}

AndroidPortAdditions::~AndroidPortAdditions() {
	LOGD("AndroidPortAdditions::~AndroidPortAdditions()");

	// Delete all the Android bitmap pointers
	for (BitmapHashMap::iterator x = mBitmaps.begin(); x != mBitmaps.end();
			++x) {

		AndroidBitmap* bitmap = x->_value;
		delete bitmap;
	}

	mBitmaps.clear();

	GLCALL(glDeleteTextures(1, mGlTextures));
	delete[] mGlTextures;
	mGlTextures = NULL;

	mModifiedGameSurface.free();

	delete mScaledOutput;

	mScalerPlugin->deinitialize();
	delete mScalerPlugin;

	delete mDefaultShaderProgram;

	delete mTouchEventMutex;
}

void AndroidPortAdditions::onSystemInitialized() {
	// Mutex depends on game engine
	mTouchEventMutex = new Common::Mutex();
}

void AndroidPortAdditions::setDisplayDimensions(uint32 width, uint32 height) {
	LOGD("AndroidPortAdditions::setDisplayDimensions: %d %d", width, height);

	mDisplayWidth = width;
	mDisplayHeight = height;

	mDisplayRatio = width / (float) height;
	Drawable::setDisplayRatio(mDisplayRatio);

	mGameToDisplayRatioW = (float) GAME_SCREEN_WIDTH / width;
	mGameToDisplayRatioH = (float) GAME_SCREEN_HEIGHT / height;

	// Calculate the touch translation offset based on the height of the black panel, game screen and display
	int16 blackPanelHeight = BLACK_PANEL_END_Y - BLACK_PANEL_START_Y;
	// The offset is negative - we fool the system that the touch event was above by this offset
	mTouchTranslationOffset = blackPanelHeight / mGameToDisplayRatioH;
	mTouchTranslationLowerBound = BLACK_PANEL_END_Y / mGameToDisplayRatioH;

	LOGD(
			"AndroidPortAdditions::setDisplayDimensions: mGameToDisplayRatioH %f mTouchTranslationOffset %d mTouchTranslationLowerBound %d",
			mGameToDisplayRatioH, mTouchTranslationOffset, mTouchTranslationLowerBound);

	// Scaling factor for SW scaler (currently minimal, devices are not fast enough)
	mScalingFactor = 2;

	// Calculate scaling factor for LQ shader (according to width)
	mLQShaderScalingFactor = LOW_QUALITY_SHADER_SCALING_FACTOR; //(float)mDisplayWidth / GAME_SCREEN_WIDTH;

	LOGD(
			"AndroidPortAdditions::setDisplayDimensions: mLQShaderScalingFactor %f",
			mLQShaderScalingFactor);

	if (!mScaledOutput) {
		mScaledOutput = new uint16[GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT
				* mScalingFactor * mScalingFactor];
	}

	if (!mScalerPlugin) {
		mScalerPlugin = new HQPlugin();
		// Set the pixel format for RGB565
		Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
		mScalerPlugin->initialize(format);

		if (mScalingFactor > 2) {
			mScalerPlugin->increaseFactor();
		}
	}

	LOGD("AndroidPortAdditions::setDisplayDimensions: mScalingFactor = %d",
			mScalingFactor);
}

void AndroidPortAdditions::addBitmapResource(String key,
		AndroidBitmap* bitmap) {
	mBitmaps[key] = bitmap;

	// Set display dimensions as uninitialized
	bitmap->displayWidth = 0;
	bitmap->displayHeight = 0;
}

AndroidBitmap* AndroidPortAdditions::getBitmap(String key) {
	return mBitmaps[key];
}

void AndroidPortAdditions::addShaderSource(const char* source, uint32 type) {
	if (type == SHADER_TYPE_VERTEX) {
		mScalerVertexSource = source;
	} else if (type == SHADER_TYPE_FRAGMENT) {
		mScalerFragmentSource = source;
	} else if (type == SHADER_TYPE_LQ_VERTEX) {
		mScalerLQVertexSource = source;
	} else if (type == SHADER_TYPE_LQ_FRAGMENT) {
		mScalerLQFragmentSource = source;
	}
}

void AndroidPortAdditions::release() {
	LOGD("AndroidPortAdditions: release");

	delete sInstance;
	sInstance = NULL;
}

void AndroidPortAdditions::gameEvent(int32 type) {
	switch (type) {
	case GAME_EVENT_SHOULD_TEST_SHADER:

		mShouldTestShader = true;
		break;
	case GAME_EVENT_USE_ULTRA_MODE:

		mShaderScalingMaxResolutionW = SHADER_SCALING_ULTRA_MAX_RESOLUTION_W;
		mShaderScalingMaxResolutionH = SHADER_SCALING_ULTRA_MAX_RESOLUTION_H;
		break;
	}
}

void AndroidPortAdditions::onRenderTimeMeasure(uint64 time) {
	LOGD("AndroidPortAdditions::onRenderTimeMeasure: %lld", time);

	if (!mShouldTestShader) {
		return;
	}

	uint64 currentTime = AndroidPortUtils::getTimeOfDayMillis();

	// If this is the first test frame, keep the timestamp
	if (mShaderTestStartTime == 0) {
		mShaderTestStartTime = currentTime;
	}

	// Safety check to make sure that the test doesn't take more than a certain time - in order to ensure good user experience
	if (currentTime - mShaderTestStartTime >= RENDER_MEASURE_MAX_TEST_TIME) {
		// Failure in this case
		mShouldTestShader = false;
		mScalingOption == SCALING_OPTION_SHADER ?
				fallbackToLQHardwareScaler() : fallbackToSoftwareScaler();
		return;
	}

	// Skip a few frames in the beginning to ignore the time it takes for ScummVM to initialize
	if (mRenderMeasureSkippedFrameCount < RENDER_MEASURE_SKIPPED_FRAMES) {
		++mRenderMeasureSkippedFrameCount;
		return;
	}

	// Add to the render time average
	mRenderMeasureAverage += time / (double) RENDER_MEASURE_COUNT_MAX;
	++mRenderMeasureCount;

	// Check if we should finish testing
	if (mRenderMeasureCount >= RENDER_MEASURE_COUNT_MAX) {
		mShouldTestShader = false;

		LOGD("AndroidPortAdditions::mRenderMeasureAverage: %f",
				mRenderMeasureAverage);

		// Check the result
		if (mRenderMeasureAverage <= RENDER_MEASURE_THRESHOLD) {
			// The shader has good performance
			JNI::gameEventJNIToJava(GAME_EVENT_SHADER_TEST_SUCCESS);
		} else {
			mScalingOption == SCALING_OPTION_SHADER ?
					fallbackToLQHardwareScaler() : fallbackToSoftwareScaler();

		}
	}
}

void AndroidPortAdditions::fallbackToLQHardwareScaler() {

	LOGD("AndroidPortAdditions::fallbackToLQHardwareScaler: ");

	mScalerShaderProgram = OpenGLESHelper::createProgram(mScalerLQVertexSource,
			mScalerLQFragmentSource);

	// Delete the unneeded LQ sources
	delete[] mScalerLQVertexSource;
	delete[] mScalerLQFragmentSource;

	if (mScalerShaderProgram == NULL) {

		LOGD(
				"AndroidPortAdditions::fallbackToLQHardwareScaler: couldn't compile LQ scaler, fallback to SW");

		// Use SW scaler in case of compile error
		fallbackToSoftwareScaler();
	} else {
		LOGD("AndroidPortAdditions::fallbackToLQHardwareScaler: success");

		JNI::gameEventJNIToJava(GAME_EVENT_SCALER_LQ_FALLBACK);
	}
}

void AndroidPortAdditions::fallbackToSoftwareScaler() {
	// The shader has bad performance - fallback to software and inform the UI
	mScalingOption = SCALING_OPTION_SOFT;
	mScalerShaderProgram = mDefaultShaderProgram;

	OSystem_Android* android = (OSystem_Android*) g_system;
	android->reinitGameTextureSize(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

	JNI::gameEventJNIToJava(GAME_EVENT_SHADER_TEST_FAILURE);
}

bool AndroidPortAdditions::checkClick(uint32 x, uint32 y, uint32 buttonX,
		uint32 buttonY, uint32 buttonW, uint32 buttonH) {
	return (x >= buttonX && x < buttonX + buttonW && y >= buttonY
			&& y < buttonY + buttonH);
}

void AndroidPortAdditions::onMouseClick(int x, int y) {
	touchEventSafety(x, y);

	// If we are in touch mode and not in chat, recognize an action press
	/*	if (mMouseVisible && mClassicMode && !mGameInChat) {
	 recognizeActionPress(x, y);
	 }*/
}

bool AndroidPortAdditions::checkGameOverlayClicks(int x, int y,
		bool performOverlayAction) {
	if (canSkip()) {
		// Check for "skip" button click
		AndroidBitmap* skip = getBitmap("skip.png");
		if (checkClick(x, y, SKIP_X * mDisplayWidth, SKIP_Y * mDisplayHeight,
				skip->displayWidth, skip->displayHeight)) {

			if (performOverlayAction) {
				performSkip();

				// Mark this flag
				mSkipPressedAtLeastOnce = true;
			}

			return true;
		}
	} else {

		if (canShowRevealItems()) {
			// Check for "reveal items" button click
			AndroidBitmap* revealItems = getBitmap("reveal_items.png");
			if (checkClick(x, y, REVEAL_ITEMS_X * mDisplayWidth,
					REVEAL_ITEMS_Y * mDisplayHeight, revealItems->displayWidth,
					revealItems->displayHeight)) {

				if (performOverlayAction) {
					mShouldPerformRevealItems = true;
				}

				return true;
			}
		}
		if (canShowMenuButton()) {
			// Check for menu click
			AndroidBitmap* menu = getBitmap("menu.png");
			if (checkClick(x, y, MENU_X * mDisplayWidth,
					MENU_Y * mDisplayHeight, menu->displayWidth,
					menu->displayHeight)) {
				if (performOverlayAction) {
					// Start the menu
					JNI::gameEventJNIToJava(GAME_EVENT_SHOW_MENU);
				}

				return true;
			}
		}

		// Check for chat overlay clicks
		if (mGameInChat && !mClassicMode) {
			AndroidBitmap* upArrow = getBitmap("arrow_up.png");
			if (checkClick(x, y, UP_ARROW_X * mDisplayWidth,
					UP_ARROW_Y * mDisplayHeight, upArrow->displayWidth,
					upArrow->displayHeight)) {

				if (performOverlayAction) {
					chatArrowClick(true);
				}

				return true;
			}
			AndroidBitmap* downArrow = getBitmap("arrow_down.png");
			if (checkClick(x, y, DOWN_ARROW_X * mDisplayWidth,
					DOWN_ARROW_Y * mDisplayHeight, downArrow->displayWidth,
					downArrow->displayHeight)) {

				if (performOverlayAction) {
					chatArrowClick(false);
				}

				return true;

			}
			AndroidBitmap* chatButton = getBitmap("talk_btn.png");
			if (checkClick(x, y, CHAT_BUTTON_X * mDisplayWidth,
					CHAT_BUTTON_Y * mDisplayHeight, chatButton->displayWidth,
					chatButton->displayHeight)) {

				if (performOverlayAction) {
					chatButtonClick();
				}

				return true;
			}
		}
	}

	if (mGameInPostcard) {

		int16 gameX = x * mGameToDisplayRatioW;
		int16 gameY = y * mGameToDisplayRatioH;

		// Check for clicks on "save", "load" or "exit"
		uint16 postcardClick = POSTCARD_CLICK_NONE;

		if (gameX >= POSTCARD_SAVE_START_X && gameY >= POSTCARD_SAVE_START_Y
				&& gameX <= POSTCARD_SAVE_END_X && gameY <= POSTCARD_SAVE_END_Y) {
			postcardClick = POSTCARD_CLICK_SAVE;
		} else if (gameX >= POSTCARD_LOAD_START_X
				&& gameY >= POSTCARD_LOAD_START_Y
				&& gameX <= POSTCARD_LOAD_END_X && gameY <= POSTCARD_LOAD_END_Y) {
			postcardClick = POSTCARD_CLICK_LOAD;
		} else if (gameX >= POSTCARD_EXIT_START_X
				&& gameY >= POSTCARD_EXIT_START_Y
				&& gameX <= POSTCARD_EXIT_END_X && gameY <= POSTCARD_EXIT_END_Y) {
			postcardClick = POSTCARD_CLICK_EXIT;
		}

		if (performOverlayAction && postcardClick != POSTCARD_CLICK_NONE) {
			LOGD("AndroidPortAdditions::onTapEvent: postcardClick %d",
					postcardClick);

			// Post a click to the "continue" button to cancel the postcard
			pushClickEvent(POSTCARD_CONTINUE_CLICK_X,
					POSTCARD_CONTINUE_CLICK_Y);

			// Handle the selected option
			JNI::onGameOption(postcardClick);

			return true;
		}

		return false;
	}

	return false;
}

bool AndroidPortAdditions::onTapEvent(int x, int y, bool doubleTap) {
	//LOGD("AndroidPortAdditions::onTapEvent %d %d", x, y);

	// Dump screen to file on debug mode
#ifdef ANDROID_PORT_DEBUG
	if (doubleTap)
		mDumpGameScreenToFile = true;
#endif

	// Make sure touch events fall within the actual surface
	touchEventSafety(x, y);

	if (checkGameOverlayClicks(x, y)) {
		return true;
	}

	// Send game event
	if (!mClassicMode && mMouseVisible) {
		gameTouchEvent(x, y, 0, 0, doubleTap ? DOUBLE_TAP : TAP);
	}

	return false;
}

bool AndroidPortAdditions::onLongClickEvent(int x, int y) {

	// Make sure touch events fall within the actual surface
	touchEventSafety(x, y);

	if (checkGameOverlayClicks(x, y)) {
		return true;
	}

	// Send game event
	if (!mClassicMode && mMouseVisible) {
		gameTouchEvent(x, y, 0, 0, UP);
	}

	return false;
}

bool AndroidPortAdditions::onFlingEvent(int x1, int y1, int x2, int y2) {
	touchEventSafety(x1, y1);
	touchEventSafety(x2, y2);

	// Send game event
	if (!mClassicMode && mMouseVisible) {
		gameTouchEvent(x2, y2, x1, y1, FLING);
	}

	return false;
}

bool AndroidPortAdditions::onScrollEvent(int x, int y) {
//	LOGD("AndroidPortAdditions::onScrollEvent %d %d", x, y);

	touchEventSafety(x, y);

	// Send game event
	if (!mClassicMode && mMouseVisible) {
		gameTouchEvent(x, y, 0, 0, DOWN);
	}

	return false;
}

bool AndroidPortAdditions::onDownEvent(int x, int y) {
//	LOGD("AndroidPortAdditions::onDownEvent %d %d", x, y);

	touchEventSafety(x, y);

	// Reset walk icon timestamp
	mLastTouchMoveTimestamp = AndroidPortUtils::getTimeOfDayMillis();

	// Check if the 'down' was performed on an overlay part - if so, do not pass to game touch behavior
	if (checkGameOverlayClicks(x, y, false)) {
		return false;
	}

	// Send game event
	if (!mClassicMode && mMouseVisible) {

		gameTouchEvent(x, y, 0, 0, DOWN);
	}

	return false;
}

bool AndroidPortAdditions::onUpEvent(int x, int y) {
	//LOGD("AndroidPortAdditions::onUpEvent %d %d", x, y);

	touchEventSafety(x, y);

	// Send game event
	if (!mClassicMode && mMouseVisible) {
		gameTouchEvent(x, y, 0, 0, UP);
	}

	return false;
}

/**
 * Using GAME coordinates
 */
void AndroidPortAdditions::pushClickEvent(int32 x, int32 y) {
	OSystem_Android* android = (OSystem_Android*) g_system;

//	LOGD("AndroidPortAdditions::pushClickEvent: %d %d", x, y);
	android->pushClick(x, y);
}

/**
 * Using GAME coordinates
 */
void AndroidPortAdditions::pushScrollEvent(int x, int y) {

	Common::Event e;
	e.type = Common::EVENT_MOUSEMOVE;
	e.mouse.x = x;
	e.mouse.y = y;

	OSystem_Android* android = (OSystem_Android*) g_system;
	android->forceEvent(e);

}

void AndroidPortAdditions::onShowMouse(bool visible) {
	mMouseVisible = visible;
}

void AndroidPortAdditions::onGameIdleCounter() {
	// Mark a flag if we got an "idle" count while saving
	if (mSaveInProgress) {
		mGameIdleCountWhileSaving = true;
	}
}

bool AndroidPortAdditions::saveGame(int32 slot) {
	// Generate a simple savename (internal)
	char saveName[20];
	g_engine->generateSaveSlotName(saveName, slot);

	// Save the game and check for error
	return (g_engine->saveGameState(slot, saveName).getCode()
			== Common::kNoError);
}

bool AndroidPortAdditions::loadGame(int32 slot) {
	// Load the game
	return (g_engine->loadGameState(slot).getCode() == Common::kNoError);
}

void AndroidPortAdditions::setSlotToLoad(int32 slot, bool force) {
	mSlotToLoad = slot;
	mForceSaveLoad = force;
}

void AndroidPortAdditions::setSlotToSave(int32 slot, bool force) {
	mSlotToSave = slot;
	mForceSaveLoad = force;
}

void AndroidPortAdditions::setAutoLoadSlot(int32 slot) {
	LOGD("AndroidPortAdditions::setAutoLoadSlot %d", slot);

	mAutoLoadSlot = slot;

	if (slot != -1) {
		mGameStartedInAutoload = true;
		mAutoLoadState = true;
	}
}

bool AndroidPortAdditions::isInAutoloadState() {
	// If we are auto-loading, we disable movie fade-out, graphics, etc.

	return mAutoLoadState;
}

void AndroidPortAdditions::onFastFadeInStarted() {
	// If we reach the fast fade-in that happens when loading (after we loaded), we end auto-load state.
	if (mAutoLoadSlot == -1) {
		mAutoLoadState = false;
	}
}

void AndroidPortAdditions::initGLESResources() {
	// Initialize the GL texture if we didn't yet
	if (mGlTextures == NULL) {
		initGlTextures();
	}

	// Init the default program
	mDefaultShaderProgram = OpenGLESHelper::createProgram(
			OpenGLESHelper::DEFAULT_VERTEX_SHADER,
			OpenGLESHelper::DEFAULT_FRAGMENT_SHADER);

	// Init the scaler program
	if (mScalingOption == SCALING_OPTION_SHADER) {
		//
		// HQ setting behavior
		//

		LOGD("AndroidPortAdditions::initGLESResources: HQ setting behavior");

		mScalerShaderProgram = OpenGLESHelper::createProgram(
				mScalerVertexSource, mScalerFragmentSource);

		if (mScalerShaderProgram == NULL) {
			LOGD(
					"AndroidPortAdditions::initGLESResources: couldn't compile HQ scaler, trying LQ");

			// Load LQ setting in case of compile error
			mScalerShaderProgram = OpenGLESHelper::createProgram(
					mScalerLQVertexSource, mScalerLQFragmentSource);

			// Delete the unneeded LQ sources
			delete[] mScalerLQVertexSource;
			delete[] mScalerLQFragmentSource;

			if (mScalerShaderProgram == NULL) {
				LOGD(
						"AndroidPortAdditions::initGLESResources: couldn't compile LQ scaler, fallback to SW");

				// Use SW scaler in case of compile error
				mScalerShaderProgram = mDefaultShaderProgram;
				mScalingOption = SCALING_OPTION_SOFT;
				mShouldTestShader = false;
				JNI::gameEventJNIToJava(GAME_EVENT_SCALER_FALLBACK);
			} else {

				LOGD(
						"AndroidPortAdditions::initGLESResources: fallback to LQ is successful");

				// Inform of LQ fallback
				mScalingOption = SCALING_OPTION_LQ_SHADER;
				mShouldTestShader = false;
				JNI::gameEventJNIToJava(GAME_EVENT_SCALER_LQ_FALLBACK);
			}

		}
	} else if (mScalingOption == SCALING_OPTION_LQ_SHADER) {
		//
		// LQ setting behavior
		//

		LOGD("AndroidPortAdditions::initGLESResources: LQ setting behavior");

		mScalerShaderProgram = OpenGLESHelper::createProgram(
				mScalerLQVertexSource, mScalerLQFragmentSource);

		// Delete the unneeded LQ sources
		delete[] mScalerLQVertexSource;
		delete[] mScalerLQFragmentSource;

		if (mScalerShaderProgram == NULL) {

			LOGD(
					"AndroidPortAdditions::initGLESResources: couldn't compile LQ scaler, fallback to SW");

			// Use SW scaler in case of compile error
			mScalerShaderProgram = mDefaultShaderProgram;
			mScalingOption = SCALING_OPTION_SOFT;
			mShouldTestShader = false;
			JNI::gameEventJNIToJava(GAME_EVENT_SCALER_FALLBACK);
		}

	} else {

		//
		// Other setting behavior
		//

		LOGD(
				"AndroidPortAdditions::initGLESResources: SW or original setting behavior");

		mScalerShaderProgram = mDefaultShaderProgram;
	}

	// Init the black program
	mBlackShaderProgram = OpenGLESHelper::createProgram(
			OpenGLESHelper::BLACK_VERTEX_SHADER,
			OpenGLESHelper::BLACK_FRAGMENT_SHADER);

	// Delete the unneeded HQ sources
	delete[] mScalerVertexSource;
	delete[] mScalerFragmentSource;

	GLCALL(
			glEnableVertexAttribArray(mDefaultShaderProgram->mPositionAttributeHandle));
	GLCALL(
			glEnableVertexAttribArray(mDefaultShaderProgram->mTexCoordAttributeHandle));
	GLCALL(
			glEnableVertexAttribArray(mScalerShaderProgram->mPositionAttributeHandle));
	GLCALL(
			glEnableVertexAttribArray(mScalerShaderProgram->mTexCoordAttributeHandle));
	GLCALL(
			glEnableVertexAttribArray(mBlackShaderProgram->mPositionAttributeHandle));

}

void AndroidPortAdditions::onDrawTextureToScreen(GLshort x, GLshort y,
		GLshort w, GLshort h) {

//	LOGD("AndroidPortAdditions::onDrawTextureToScreen %d %d %d %d", x, y, w ,h);

// Get the current time
	uint64 currentTime = AndroidPortUtils::getTimeOfDayMillis();

	// Set the beginning timestamp if it wasn't set yet
	if (mFirstDrawTimestamp == 0) {
		mFirstDrawTimestamp = AndroidPortUtils::getTimeOfDayMillis();
	}

	GLCALL(glBindTexture(GL_TEXTURE_2D, *mGlTextures));

	// Draw all animations
	drawAnimations();

	// Draw the mouse pointer if needed
	if (mMouseVisible && mClassicMode) {
		const Common::Point &mouse = g_system->getEventManager()->getMousePos();
		drawBitmapAsGlTexture(getBitmap("cursor.png"),
				mouse.x / (float) GAME_SCREEN_WIDTH,
				mouse.y / (float) GAME_SCREEN_HEIGHT);
	}

	// UPDATE: removed for now
	// Check if this is the right time to show the label "music enhanced by..."
	/*	uint64 relativeTime = currentTime - mFirstDrawTimestamp;
	 if (relativeTime >= MUSIC_ENHANCED_BY_BEGIN_TIME
	 && relativeTime < MUSIC_ENHANCED_BY_END_TIME
	 && !mSkipPressedAtLeastOnce && !mGameStartedInAutoload) {
	 drawBitmapAsGlTexture(getBitmap("music_enhanced_by.png"),
	 MUSIC_ENHANCED_BY_X, MUSIC_ENHANCED_BY_Y);
	 }*/

	// draw the "reveal items", "menu" or "skip" buttons according to state
	if (canSkip()) {

		drawBitmapAsGlTexture(getBitmap("skip.png"), SKIP_X, SKIP_Y);
	} else {
		if (canShowRevealItems()) {
			drawBitmapAsGlTexture(getBitmap("reveal_items.png"), REVEAL_ITEMS_X,
					REVEAL_ITEMS_Y);
		}
		if (canShowMenuButton()) {
			drawBitmapAsGlTexture(getBitmap("menu.png"), MENU_X, MENU_Y);
		}
	}

// Show the chat overlay if needed
	if (mMouseVisible && mGameInChat && !mClassicMode) {
		drawBitmapAsGlTexture(getBitmap("arrow_up.png"), UP_ARROW_X, UP_ARROW_Y,
				ARROW_W, ARROW_H);
		drawBitmapAsGlTexture(getBitmap("arrow_down.png"), DOWN_ARROW_X,
				DOWN_ARROW_Y, ARROW_W, ARROW_H);
		drawBitmapAsGlTexture(getBitmap("talk_btn.png"), CHAT_BUTTON_X,
				CHAT_BUTTON_Y, CHAT_BUTTON_W, CHAT_BUTTON_H);
	}

// Notify the Java layer on the first time we get called here
	if (!mGameDisplayStarted) {
		JNI::onGameDisplayStarted();
		mGameDisplayStarted = true;
	}
}

void AndroidPortAdditions::addHotspotFadeoutAnimation(Point hotspot,
		long duration) {
	DrawablePtr hotspotDrawable(new Drawable), actionDrawable(new Drawable);
	generateHotspotIndicatorDrawables(getBitmap("touch_indicator.png"),
			hotspot.x, hotspot.y, NULL, hotspotDrawable, actionDrawable);

// Add an alpha animation to the hotspot
	AlphaAnimation* alphaAnimation = new AlphaAnimation();
	alphaAnimation->start(AndroidPortUtils::getTimeOfDayMillis());
	alphaAnimation->setDuration(duration);
	alphaAnimation->setStartAlpha(1);
	alphaAnimation->setEndAlpha(0);
	hotspotDrawable->setAnimation(AnimationPtr(alphaAnimation));

// Add to animation list
	mAnimatedDrawables.push_back(hotspotDrawable);
}

void AndroidPortAdditions::addWalkFadeoutAnimation(int x, int y) {
	DrawablePtr drawable(new Drawable);

	AndroidBitmap* bitmap = getBitmap("walk.png");
	drawable->setBitmap(bitmap);
	drawable->setWidth(SMALL_ACTION_ICON_W);

// Center
	float height = SMALL_ACTION_ICON_W * bitmap->ratio * mDisplayRatio;
	float xPos = (x / (float) GAME_SCREEN_WIDTH) - SMALL_ACTION_ICON_W / 2;
	float yPos = (y / (float) GAME_SCREEN_HEIGHT) - height / 2;

// Adjust to bounds
	xPos = MAX(xPos, 0.0f);
	yPos = MAX(yPos, (float) BLACK_PANEL_HEIGHT / GAME_SCREEN_HEIGHT);

	drawable->setPositionX(xPos);
	drawable->setPositionY(yPos);

// Add an alpha animation to the walk icon
	shared_ptr<AlphaAnimation> fadeInAnimation(new AlphaAnimation);
	fadeInAnimation->setDuration(WALK_ICON_SINGLE_TAP_FADEIN_DURATION);
	fadeInAnimation->setStartAlpha(0);
	fadeInAnimation->setEndAlpha(WALK_ICON_SINGLE_TAP_MAX_ALPHA);
	fadeInAnimation->setInterpolator(
			InterpolatorPtr(new DeccelerateInterpolator));

	shared_ptr<AlphaAnimation> fadeOutAnimation(new AlphaAnimation);
	fadeOutAnimation->setDuration(WALK_ICON_SINGLE_TAP_FADEOUT_DURATION);
	fadeOutAnimation->setStartAlpha(WALK_ICON_SINGLE_TAP_MAX_ALPHA);
	fadeOutAnimation->setEndAlpha(0);
	fadeOutAnimation->setInterpolator(
			InterpolatorPtr(new DeccelerateInterpolator));

	shared_ptr<SequenceAnimationComposite> walkAnimation(
			new SequenceAnimationComposite);
	walkAnimation->addAnimation(fadeInAnimation);
	walkAnimation->addAnimation(fadeOutAnimation);

// Add to animation list
	walkAnimation->start(AndroidPortUtils::getTimeOfDayMillis());
	drawable->setAnimation(AnimationPtr(walkAnimation));
	mAnimatedDrawables.push_back(drawable);
}

void AndroidPortAdditions::assistWalkCoordinates(int16& x, int16& y) {

//	LOGD("AndroidPortAdditions::assistWalkCoordinates: old %d %d", x ,y);

// Adjust according to border size to help walking to edges
	if (y < BLACK_PANEL_START_Y) {
		if (x <= WALK_ASSIST_BORDER_SIZE_X) {
			x = 0;
		} else if (x >= GAME_SCREEN_WIDTH - WALK_ASSIST_BORDER_SIZE_X) {
			x = GAME_SCREEN_WIDTH - 1;
		}

		if (y >= BLACK_PANEL_START_Y - WALK_ASSIST_BORDER_SIZE_Y) {
			y = BLACK_PANEL_START_Y - 2;
		}
	}

//LOGD("AndroidPortAdditions::assistWalkCoordinates: new %d %d", x ,y);

}

void AndroidPortAdditions::gameTouchBehavior() {

// Filter states we don't run in
	if (!mMouseVisible || mClassicMode || mTouchState == NONE) {
		return;
	}

// Obtain last touch event
	mTouchEventMutex->lock();
	int16 gameTouchX = mGameTouchX;
	int16 gameTouchY = mGameTouchY;
	TouchState gameTouchState = mTouchState;
	mTouchState = NONE;
	mTouchEventMutex->unlock();

// Flag for whether the current action was performed or not
	bool actionUsed = false;

//	LOGD("AndroidPortAdditions::gameTouchBehavior: %d %d %d",
//		gameTouchX, gameTouchY, gameTouchState);

	switch (gameTouchState) {
	case FLING:
		LOGE(
				"AndroidPortAdditions::gameTouchBehavior: got FLING state here - bug!");
	case NONE:
		// Nothing
		return;
	case DOWN:

		if (mGameInPostcard || mGameInChat || gameTouchY >= BLACK_PANEL_END_Y) {
			// Just a mouse move in non-game area or state
			pushScrollEvent(gameTouchX, gameTouchY);

		} else {

			Hotspot newHotspot = mHitAreaHelper.getClosestHotspot(gameTouchX,
					gameTouchY);

			if (mCurrentHotspot.mDisplayPoint != newHotspot.mDisplayPoint) {

				// Reset timestamp for walk action
				mLastTouchMoveTimestamp =
						AndroidPortUtils::getTimeOfDayMillis();

				// If there was a previous hotspot, add a fadeout animation
				if (!mCurrentHotspot.mDisplayPoint.isOrigin()) {

					clearCurrentHotspotAnimation(
							HOTSPOT_SWITCH_FADEOUT_DURATION, false);
					clearCurrentActionIconAnimation(false);
				}

				if (!newHotspot.mDisplayPoint.isOrigin()) {
					// Hotspot changed - setup the current animation
					setupCurrentHotspotAnimation(newHotspot.mDisplayPoint);
				}
			}

			// On the game area, check for the nearest hotspot and move the mouse there
			mCurrentHotspot = newHotspot;
			if (!mCurrentHotspot.mCursorPoint.isOrigin()) {

				pushScrollEvent(mCurrentHotspot.mCursorPoint.x,
						mCurrentHotspot.mCursorPoint.y);
			} else {
				// If there's no near hotspot, scroll over the touch location
				pushScrollEvent(gameTouchX, gameTouchY);
			}
		}

		break;

	case UP:

		if (mGameInPostcard || mGameInChat) {
			// Just a click in non-game area or state
			pushClickEvent(gameTouchX, gameTouchY);
		} else if (gameTouchY >= BLACK_PANEL_END_Y) {
			bottomToolbarClickBehavior(gameTouchX, gameTouchY);

		} else {

			// Check if we're near a hotspot
			mCurrentHotspot = mHitAreaHelper.getClosestHotspot(gameTouchX,
					gameTouchY);

			if (!mCurrentHotspot.mDisplayPoint.isOrigin()) {

				// Check if an action is selected
				if (mCurrentAction == ACTION_WALK) {
					// If we're on 'walk', walk to the hotspot only if timeout has passes
					if (shouldUseWalkAction()) {
						pushClickEvent(mCurrentHotspot.mCursorPoint.x,
								mCurrentHotspot.mCursorPoint.y);
						actionUsed = true;
					}
				} else {
					// Otherwise, perform a click on the hotspot to perform the action
					pushClickEvent(mCurrentHotspot.mCursorPoint.x,
							mCurrentHotspot.mCursorPoint.y);
					actionUsed = true;

				}
			} else {
				// Check if an action is selected
				if (mCurrentAction == ACTION_WALK) {
					// If 'walk' is selected and we're not on a hotspot, walk only if the press is long
					// UPDATE: disabled for now, user can walk using single-tap
					/*if (shouldUseWalkAction()) {
					 // Walk to the current coordinates
					 addWalkFadeoutAnimation(gameTouchX, gameTouchY);
					 assistWalkCoordinates(gameTouchX, gameTouchY);
					 pushClickEvent(gameTouchX, gameTouchY);
					 }*/
				} else {
					// Perform the action on the current coordinates
					pushClickEvent(gameTouchX, gameTouchY);
				}
			}
		}

		// If there was a previous hotspot, fade it out
		clearCurrentHotspotAnimation(HOTSPOT_LEAVE_FADEOUT_DURATION, false);
		clearCurrentActionIconAnimation(actionUsed);

		mCurrentHotspot.clear();

		break;

	case TAP:

		if (mGameInPostcard || mGameInChat) {
			// Just a click in non-game area or state
			pushClickEvent(gameTouchX, gameTouchY);
		} else if (gameTouchY >= BLACK_PANEL_END_Y) {
			bottomToolbarClickBehavior(gameTouchX, gameTouchY);

		} else {
			// Check if we're near a hotspot
			mCurrentHotspot = mHitAreaHelper.getClosestHotspot(gameTouchX,
					gameTouchY);

			if (!mCurrentHotspot.mCursorPoint.isOrigin()) {
				// Check if an action is selected
				if (mCurrentAction == ACTION_WALK) {
					// If we're on 'walk', walk to the touched point
					addWalkFadeoutAnimation(gameTouchX, gameTouchY);
					assistWalkCoordinates(gameTouchX, gameTouchY);

					// Check if we need outside dragon cave workaround
					if (mCurrentGameArea == OUTSIDE_DRAGON_CAVE_WITH_HOOK) {
						outsideDragonCaveWorkaround(gameTouchX, gameTouchY);
					}

					pushClickEvent(gameTouchX, gameTouchY);
				} else {
					// Otherwise, perform a click on the hotspot to perform the action
					pushClickEvent(mCurrentHotspot.mCursorPoint.x,
							mCurrentHotspot.mCursorPoint.y);
					actionUsed = true;

				}
			} else {

				if (mCurrentAction == ACTION_WALK) {
					addWalkFadeoutAnimation(gameTouchX, gameTouchY);
					assistWalkCoordinates(gameTouchX, gameTouchY);
				}

				// Perform action on the touched point
				pushClickEvent(gameTouchX, gameTouchY);

			}
		}

		// If there was a previous hotspot, fade it out
		clearCurrentHotspotAnimation(HOTSPOT_LEAVE_FADEOUT_DURATION, true);
		clearCurrentActionIconAnimation(actionUsed);

		mCurrentHotspot.clear();

		break;
	case DOUBLE_TAP:

		// Force a click regadless of everything
		pushClickEvent(gameTouchX, gameTouchY);

		// If there was a previous hotspot, fade it out
		clearCurrentHotspotAnimation(HOTSPOT_LEAVE_FADEOUT_DURATION, false);
		clearCurrentActionIconAnimation(actionUsed);

		mCurrentHotspot.clear();

		break;
	}

}

void AndroidPortAdditions::bottomToolbarClickBehavior(int16 gameTouchX,
		int16 gameTouchY) {
	// Click on the bottom toolbar
	pushClickEvent(gameTouchX, gameTouchY);
}

void AndroidPortAdditions::setGameTextureInfo(GLuint gameTexture,
		uint16 texWidth, uint16 texHeight) {
	mGameTexture = gameTexture;
	mGameTextureW = texWidth;
	mGameTextureH = texHeight;
}

void AndroidPortAdditions::drawAnimations() {
// Get current time
	long time = AndroidPortUtils::getTimeOfDayMillis();

	std::list<DrawablePtr>::iterator it = mAnimatedDrawables.begin();
	while (it != mAnimatedDrawables.end()) {
		DrawablePtr drawable = *it;
		drawable->updateAnimation(time);

		if (drawable->isAnimationFinished()) {
			// If animation was finished, erase element
			mAnimatedDrawables.erase(it++);
		} else {
			// Draw the element
			drawAnimationDrawable(drawable);

			++it;
		}
	}
}

void AndroidPortAdditions::drawAnimationDrawable(const DrawablePtr drawable) {

	drawBitmapAsGlTexture(drawable->getBitmap(), drawable->getPositionX(),
			drawable->getPositionY(), drawable->getWidth(), 0,
			drawable->getAlpha());
}

void AndroidPortAdditions::drawBitmapAsGlTexture(AndroidBitmap* bitmap, float x,
		float y, float width, float height, float alpha) {
//LOGD("AndroidPortAdditions::drawBitmapAsGlTexture: x %d y %d bitmap: %s tex x %d tex y %d w %d h %d", x, y, bitmap->bitmapName.c_str(), bitmap->glTextureX, bitmap->glTextureY, bitmap->width, bitmap->height);

	// Use a blending mode which will workaround Android's premultiply alpha behavior
	// http://stackoverflow.com/questions/15915337/using-translucent-pngs-as-textures-and-fading-them-opengl-es-2-0
	if (bitmap->sourceContainsAlpha) {
		GLCALL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
	}

// Texture coordinates should be in GL tex coordinate system(0.0 - 1.0)
	const GLfloat tex_x = bitmap->glTextureX / (GLfloat) GL_TEXTURE_WIDTH;
	const GLfloat tex_y = bitmap->glTextureY / (GLfloat) GL_TEXTURE_HEIGHT;
	const GLfloat tex_width = bitmap->width / (GLfloat) GL_TEXTURE_WIDTH;
	const GLfloat tex_height = bitmap->height / (GLfloat) GL_TEXTURE_HEIGHT;

// define texCoords (triangle strip) - what part of the texture to crop
// (invert y axis)
	const GLfloat texcoords[] = { tex_x, tex_y, tex_x + tex_width, tex_y, tex_x,
			tex_y + tex_height, tex_x + tex_width, tex_y + tex_height, };

	GLfloat bmpDisplayWidth, bmpDisplayHeight;
	if (width <= 0) {
		// In this scenario, the bitmap dimension are specified in pixels (normal screen coordinates)
		bmpDisplayWidth = bitmap->displayWidth / (GLfloat) mDisplayWidth;
		bmpDisplayHeight = bitmap->displayHeight / (GLfloat) mDisplayHeight;
	} else {
		// Set a custom display width for the bitmap, just for now
		bmpDisplayWidth = width;
		if (height <= 0) {
			bmpDisplayHeight = bmpDisplayWidth * bitmap->ratio * mDisplayRatio;
		} else {
			bmpDisplayHeight = height;
		}
	}

// Convert x, y, width, height to default GL vertex coordinate system (-1.0 - 1.0)
	x = x * 2 - 1;
	y = y * (-2) + 1;
	bmpDisplayWidth = bmpDisplayWidth * 2;
	bmpDisplayHeight = bmpDisplayHeight * 2;

// define vertices for drawing to the screen
	const GLfloat vertices[] = { x, y, x + bmpDisplayWidth, y, x, y
			- bmpDisplayHeight, x + bmpDisplayWidth, y - bmpDisplayHeight, };

// Use the program
	GLCALL(glUseProgram(mDefaultShaderProgram->mProgramHandle));

// Set the texture uniform
	GLCALL(glUniform1i(mDefaultShaderProgram->mTextureUniformHandle, 0));

// Set the alpha uniform
	GLCALL(
			glUniform1f(mDefaultShaderProgram->mAlphaFactorUniformHandle, alpha));

// Pass the position attributes
	GLCALL(
			glVertexAttribPointer(mDefaultShaderProgram->mPositionAttributeHandle, 2, GL_FLOAT, false, 0, vertices));

// Pass the texture coordinate attributes
	GLCALL(
			glVertexAttribPointer(mDefaultShaderProgram->mTexCoordAttributeHandle, 2, GL_FLOAT, false, 0, texcoords));

	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	// Revert to default blending mode
	if (bitmap->sourceContainsAlpha) {
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}
}

void AndroidPortAdditions::setupCurrentHotspotAnimation(Point hotspot) {

// Generate the base hotspot and action drawables
	if (mCurrentHotspotDrawable == NULL) {
		mCurrentHotspotDrawable.reset(new Drawable);
	}
	if (mCurrentActionDrawable == NULL) {
		mCurrentActionDrawable.reset(new Drawable);
	}

	AndroidBitmap* action = getActionIcon(mCurrentAction);
	AndroidBitmap* touchIndicator = getBitmap("touch_indicator.png");
	generateHotspotIndicatorDrawables(touchIndicator, hotspot.x, hotspot.y,
			action, mCurrentHotspotDrawable, mCurrentActionDrawable);

//
// Setup the hotspot animation
//

// Calculate the point that should be the indicator center
	float centerX = hotspot.x / (float) GAME_SCREEN_WIDTH;
	float centerY = (hotspot.y + BLACK_PANEL_HEIGHT)
			/ (float) GAME_SCREEN_HEIGHT;

// Create a fade in animation
	shared_ptr<AlphaAnimation> hotspotFadeInAnimation(new AlphaAnimation);
	hotspotFadeInAnimation->setDuration(TOUCH_INDICATOR_INITIAL_FADE_DURATION);
	hotspotFadeInAnimation->setStartAlpha(0);
	hotspotFadeInAnimation->setEndAlpha(1);
	hotspotFadeInAnimation->setInterpolator(
			InterpolatorPtr(new DeccelerateInterpolator));

// Create the initial scale up animation
	shared_ptr<ScaleAnimation> initialScaleUpAnimation(new ScaleAnimation);
	initialScaleUpAnimation->setCenterX(centerX);
	initialScaleUpAnimation->setCenterY(centerY);
	initialScaleUpAnimation->setStartWidth(TOUCH_INDICATOR_MIN_SCALE_W);
	initialScaleUpAnimation->setEndWidth(TOUCH_INDICATOR_MAX_SCALE_W);
	initialScaleUpAnimation->setDuration(TOUCH_INDICATOR_INITIAL_FADE_DURATION);

// Create the looped scale up animation
	shared_ptr<ScaleAnimation> scaleUpAnimation(new ScaleAnimation);
	scaleUpAnimation->setCenterX(centerX);
	scaleUpAnimation->setCenterY(centerY);
	scaleUpAnimation->setStartWidth(TOUCH_INDICATOR_MIN_SCALE_W);
	scaleUpAnimation->setEndWidth(TOUCH_INDICATOR_MAX_SCALE_W);
	scaleUpAnimation->setDuration(TOUCH_INDICATOR_SCALE_DURATION);

// Create a scale down animation
	shared_ptr<ScaleAnimation> scaleDownAnimation(new ScaleAnimation);
	scaleDownAnimation->setCenterX(centerX);
	scaleDownAnimation->setCenterY(centerY);
	scaleDownAnimation->setStartWidth(TOUCH_INDICATOR_MAX_SCALE_W);
	scaleDownAnimation->setEndWidth(TOUCH_INDICATOR_MIN_SCALE_W);
	scaleDownAnimation->setDuration(TOUCH_INDICATOR_SCALE_DURATION);

// Create the initial enter animation
	shared_ptr<ParallelAnimation> hotspotEnterAnimation(new ParallelAnimation);
	hotspotEnterAnimation->addAnimation(hotspotFadeInAnimation);
	hotspotEnterAnimation->addAnimation(initialScaleUpAnimation);

// Create a sequence
	shared_ptr<SequenceAnimationComposite> scaleUpDownAnimation(
			new SequenceAnimationComposite);
	scaleUpDownAnimation->addAnimation(scaleDownAnimation);
	scaleUpDownAnimation->addAnimation(scaleUpAnimation);

// Create a loop
	shared_ptr<RepeatAnimationWrapper> scaleLoop(
			new RepeatAnimationWrapper(scaleUpDownAnimation, 0));

// Create the sequence of enter + loop
	shared_ptr<SequenceAnimationComposite> hotspotAnimation(
			new SequenceAnimationComposite);
	hotspotAnimation->addAnimation(hotspotEnterAnimation);
	hotspotAnimation->addAnimation(scaleLoop);

// Add the animation
	hotspotAnimation->start(AndroidPortUtils::getTimeOfDayMillis());
	mCurrentHotspotDrawable->setAnimation(hotspotAnimation);
	mAnimatedDrawables.push_back(mCurrentHotspotDrawable);

//
// Setup the action icon animation
//

	AnimationPtr actionIconAnimation;

// Create a fade in animation
	shared_ptr<AlphaAnimation> actionFadeInAnimation(new AlphaAnimation);
	actionFadeInAnimation->setDuration(ACTION_ENTER_FADEIN_DURATION);
	actionFadeInAnimation->setStartAlpha(0);
	actionFadeInAnimation->setEndAlpha(1);
	actionFadeInAnimation->setInterpolator(
			InterpolatorPtr(new DeccelerateInterpolator));
	actionFadeInAnimation->start(AndroidPortUtils::getTimeOfDayMillis());

	if (mCurrentAction == ACTION_WALK) {

		mCurrentActionDrawable->setAlpha(0);

		// If using WALK, create a conditional wait animation for using before the fade-in (using a local class)
		class CheckIfWalkCondition: public Condition {
		public:
			virtual bool evaluate() {
				return AndroidPortAdditions::instance()->shouldUseWalkAction();
			}
		};

		shared_ptr<WaitForConditionAnimation> walkConditionAnimation(
				new WaitForConditionAnimation);
		shared_ptr<CheckIfWalkCondition> walkCondition(
				new CheckIfWalkCondition);
		walkConditionAnimation->setCondition(walkCondition);

		// Create the sequence of wait + fade in
		shared_ptr<SequenceAnimationComposite> sequence(
				new SequenceAnimationComposite);
		sequence->addAnimation(walkConditionAnimation);
		sequence->addAnimation(actionFadeInAnimation);
		actionIconAnimation = sequence;
	} else {
		actionIconAnimation = actionFadeInAnimation;
	}

// Add the animation
	actionIconAnimation->setFinishOnEnd(false);
	actionIconAnimation->start(AndroidPortUtils::getTimeOfDayMillis());
	mCurrentActionDrawable->setAnimation(actionIconAnimation);
	mAnimatedDrawables.push_back(mCurrentActionDrawable);
}

void AndroidPortAdditions::clearCurrentHotspotAnimation(long fadeoutDuration,
		bool completeFadeIn) {

// Make sure we have a current hotspot reference
	if (mCurrentHotspotDrawable == NULL) {
		return;
	}

// Set a fadeout + scale down animation that will erase the drawable at the end
	shared_ptr<AlphaAnimation> alphaAnimation(new AlphaAnimation);
	alphaAnimation->setDuration(fadeoutDuration);
	alphaAnimation->setStartAlpha(mCurrentHotspotDrawable->getAlpha());
	alphaAnimation->setEndAlpha(0);
	alphaAnimation->setInterpolator(
			InterpolatorPtr(new DeccelerateInterpolator));

// Calculate the point that should be the indicator center
	float centerX = mCurrentHotspotDrawable->getPositionX()
			+ mCurrentHotspotDrawable->getWidth() / 2;
	float centerY = mCurrentHotspotDrawable->getPositionY()
			+ mCurrentHotspotDrawable->getHeightByRatio() / 2;

// Create a scale down animation
	shared_ptr<ScaleAnimation> scaleDownAnimation(new ScaleAnimation);
	scaleDownAnimation->setCenterX(centerX);
	scaleDownAnimation->setCenterY(centerY);
	scaleDownAnimation->setStartWidth(mCurrentHotspotDrawable->getWidth());
	scaleDownAnimation->setEndWidth(TOUCH_INDICATOR_MIN_SCALE_W);
	scaleDownAnimation->setDuration(fadeoutDuration);
	scaleDownAnimation->setInterpolator(
			InterpolatorPtr(new DeccelerateInterpolator));

	shared_ptr<ParallelAnimation> hotspotFadeOutAnimation(
			new ParallelAnimation);
	hotspotFadeOutAnimation->addAnimation(alphaAnimation);
	hotspotFadeOutAnimation->addAnimation(scaleDownAnimation);

	AnimationPtr hotspotClearingAnimation;

	// Check whether to complete a fade-in before clearing the hotspot
	if (completeFadeIn) {
		// Create a fade in animation
		shared_ptr<AlphaAnimation> hotspotFadeInAnimation(new AlphaAnimation);
		hotspotFadeInAnimation->setDuration(
				TOUCH_INDICATOR_INITIAL_FADE_DURATION);
		hotspotFadeInAnimation->setStartAlpha(
				mCurrentHotspotDrawable->getAlpha());
		hotspotFadeInAnimation->setEndAlpha(1);
		hotspotFadeInAnimation->setInterpolator(
				InterpolatorPtr(new DeccelerateInterpolator));

		shared_ptr<SequenceAnimationComposite> fadeInOutSequence(
				new SequenceAnimationComposite);
		fadeInOutSequence->addAnimation(hotspotFadeInAnimation);
		fadeInOutSequence->addAnimation(hotspotFadeOutAnimation);

		hotspotClearingAnimation = fadeInOutSequence;
	} else {
		hotspotClearingAnimation = hotspotFadeOutAnimation;
	}

	hotspotClearingAnimation->start(AndroidPortUtils::getTimeOfDayMillis());
	mCurrentHotspotDrawable->setAnimation(hotspotClearingAnimation);

// Clear the reference
	mCurrentHotspotDrawable = NULL;
}

bool AndroidPortAdditions::generateHotspotIndicatorDrawables(
		AndroidBitmap* bitmap, int x, int y, AndroidBitmap* action,
		DrawablePtr hotspotDrawable, DrawablePtr actionDrawable, float alpha) {

// Draw an indicator on the center of the target
// (calculate the starting corrdinates for drawing so that the bitmap is centered on the hotspot)
	float indicatorHeight = TOUCH_INDICATOR_W * bitmap->ratio * mDisplayRatio;
	float hotspotPositionY =
			((y + BLACK_PANEL_HEIGHT)/ (float) GAME_SCREEN_HEIGHT)-indicatorHeight / 2;

	hotspotDrawable->setPositionX(
			(x / (float) GAME_SCREEN_WIDTH) - TOUCH_INDICATOR_W / 2);
	hotspotDrawable->setPositionY(hotspotPositionY);
	hotspotDrawable->setWidth(TOUCH_INDICATOR_W);
	hotspotDrawable->setAlpha(alpha);
	hotspotDrawable->setBitmap(bitmap);

	if (action != NULL) {
		// Draw action icon
		float actionHeight = SMALL_ACTION_ICON_W * action->ratio
				* mDisplayRatio;
		actionDrawable->setPositionX(
				(x / (float) GAME_SCREEN_WIDTH) - SMALL_ACTION_ICON_W / 2);
		float actionY = hotspotPositionY - 0.01 - actionHeight;
		if (actionY < (BLACK_PANEL_HEIGHT / (float) GAME_SCREEN_HEIGHT)) {
			// If we're on the upper part, switch Y value to be below the indicator
			actionY = hotspotPositionY + indicatorHeight + 0.06;
		}

		actionDrawable->setPositionY(actionY);
		actionDrawable->setWidth(SMALL_ACTION_ICON_W);
		actionDrawable->setBitmap(action);

		return true;
	}

	return false;
}

void AndroidPortAdditions::drawHotspotIndicator(AndroidBitmap* bitmap, int x,
		int y, AndroidBitmap* action, float alpha) {

// Draw an indicator on the center of the target
// (calculate the starting corrdinates for drawing so that the bitmap is centered on the hotspot)
	float indicatorHeight = TOUCH_INDICATOR_W * bitmap->ratio * mDisplayRatio;
	float indicatorX = (x / (float) GAME_SCREEN_WIDTH) - TOUCH_INDICATOR_W / 2;
	float indicatorY = ((y + BLACK_PANEL_HEIGHT)/ (float) GAME_SCREEN_HEIGHT)-indicatorHeight / 2;

	drawBitmapAsGlTexture(bitmap, indicatorX, indicatorY, TOUCH_INDICATOR_W, 0,
			alpha);

	if (action != NULL) {
		// Draw action icon
		float actionHeight = SMALL_ACTION_ICON_W * action->ratio
				* mDisplayRatio;
		float actionX = (x / (float) GAME_SCREEN_WIDTH)
				- SMALL_ACTION_ICON_W / 2;
		float actionY = indicatorY - 0.01 - actionHeight;
		if (actionY < (BLACK_PANEL_HEIGHT / (float) GAME_SCREEN_HEIGHT)) {
			// If we're on the upper part, switch Y value to be below the indicator
			actionY = indicatorY + indicatorHeight + 0.06;
		}

		drawBitmapAsGlTexture(action, actionX, actionY);
	}
}

uint16* AndroidPortAdditions::scale(const uint8 *srcPtr, uint16 x, uint16 y,
		uint16 w, uint16 h) {
	uint32 srcPitch = GAME_SCREEN_WIDTH * 2;
	uint32 dstPitch = w * mScalingFactor * 2;

	mScalerPlugin->scale(srcPtr, srcPitch, (uint8*) mScaledOutput, dstPitch, w,
			h, x, y);

	return mScaledOutput;
}

void AndroidPortAdditions::setScalingOption(uint16 option) {
	LOGD("AndroidPortAdditions::setScalingOption %d", option);

	mScalingOption = option;
}

void AndroidPortAdditions::initGlTextures() {
	LOGD("AndroidPortAdditions::initGlTextures");

	mGlTextures = new GLuint[1];
	GLCALL(glGenTextures(1, mGlTextures));
	GLCALL(glBindTexture(GL_TEXTURE_2D, *mGlTextures));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

	byte* combinedGlTexturePixels = new byte[GL_TEXTURE_WIDTH
			* GL_TEXTURE_HEIGHT * 4];

	GLCALL(
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GL_TEXTURE_WIDTH, GL_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, combinedGlTexturePixels));

	delete[] combinedGlTexturePixels;

	initBitmapInGlTextures(getBitmap("skip.png"), SKIP_W);
	initBitmapInGlTextures(getBitmap("reveal_items.png"), REVEAL_ITEMS_W);
	initBitmapInGlTextures(getBitmap("mouse_mode.png"), CONTROL_MODE_W);
	initBitmapInGlTextures(getBitmap("touch_mode.png"), CONTROL_MODE_W);
	initBitmapInGlTextures(getBitmap("close.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("consume.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("give.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("look.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("move.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("open.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("pick.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("remove.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("talk.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("talk_btn.png"), CHAT_BUTTON_W, true);
	initBitmapInGlTextures(getBitmap("menu.png"), MENU_W);

	initBitmapInGlTextures(getBitmap("use.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("walk.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("wear.png"), SMALL_ACTION_ICON_W);
	initBitmapInGlTextures(getBitmap("music_enhanced_by.png"),
			MUSIC_ENHANCED_BY_W);
	initBitmapInGlTextures(getBitmap("cursor.png"), CURSOR_W);
	initBitmapInGlTextures(getBitmap("touch_indicator.png"), TOUCH_INDICATOR_W);
	initBitmapInGlTextures(getBitmap("touch_indicator_selected.png"),
			TOUCH_INDICATOR_W);
	initBitmapInGlTextures(getBitmap("arrow_up.png"), ARROW_W, true);
	initBitmapInGlTextures(getBitmap("arrow_down.png"), ARROW_W, true);

// Initialize the "copy rect" of the magnifier, based on the magnifier's size on the screen, multiplied by game \ display ratio
// (to know how many pixels to copy)

	AndroidBitmap* frame = getBitmap("frame.png");
	initBitmapInGlTextures(frame, FRAME_W);

	mMagnifierCopyRectWidth = frame->displayWidth / (float) mDisplayWidth;
	mMagnifierCopyRectHeight = frame->displayHeight / (float) mDisplayHeight;
}

void AndroidPortAdditions::initBitmapInGlTextures(AndroidBitmap* bitmap,
		float width, bool sourceContainsAlpha) {
// This generates the right coordinates of the bitmap inside the GL texture
	mGlTextureHelper.allocateBitmapInGlTexture(bitmap);

	GLCALL(
			glTexSubImage2D(GL_TEXTURE_2D, 0, bitmap->glTextureX, bitmap->glTextureY, bitmap->width, bitmap->height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->pixels));

// Calculate ratio
	bitmap->ratio = (float) bitmap->height / (float) bitmap->width;

// Calculate sizes
	bitmap->displayWidth = mDisplayWidth * width;
	bitmap->displayHeight = bitmap->displayWidth * bitmap->ratio; // Get the height by multiplying the image aspect ratio

	// Save this flag per bitmap
	bitmap->sourceContainsAlpha = sourceContainsAlpha;

	// We don't need the bitmap in memory anymore
	delete[] bitmap->pixels;
}

void AndroidPortAdditions::performSkip(bool showAnimation) {
	LOGD("AndroidPortAdditions::performSkip()");

	if (showAnimation) {
		addBigActionFadeAnimation(getBitmap("skip.png"));
	}

	OSystem_Android* android = (OSystem_Android*) g_system;

// Push the "esc" key event to the system
	Common::Event e1, e2, e3, e4;
	e1.kbd.keycode = Common::KEYCODE_ESCAPE;
	e1.kbd.ascii = Common::ASCII_ESCAPE;
	e1.type = Common::EVENT_KEYDOWN;
	android->forceEvent(e1);

	e2.kbd.keycode = Common::KEYCODE_ESCAPE;
	e2.kbd.ascii = Common::ASCII_ESCAPE;
	e2.type = Common::EVENT_KEYUP;
	android->forceEvent(e2);

// Push a right click event to the system
	e3.type = Common::EVENT_RBUTTONDOWN;
	android->forceEvent(e3);

	e4.type = Common::EVENT_RBUTTONUP;
	android->forceEvent(e4);
}

void AndroidPortAdditions::performRevealItems() {
	LOGD("AndroidPortAdditions::performRevealItems()");

	// Obtain hotspots
	uint16 maxHotspots = 30;
	Point hotspots[maxHotspots];
	uint16 count = mHitAreaHelper.getAllInteractionHotspots(hotspots,
			maxHotspots);

	AndroidBitmap* indicator = getBitmap("touch_indicator.png");

	// Draw indicators on all of them with alphas
	for (int i = 0; i < count; ++i) {

		DrawablePtr hotspotDrawable(new Drawable), actionDrawable(new Drawable);

		generateHotspotIndicatorDrawables(indicator, hotspots[i].x,
				hotspots[i].y, NULL, hotspotDrawable, actionDrawable);

		// Create a fade in animation
		shared_ptr<AlphaAnimation> fadeInAnimation(new AlphaAnimation());
		fadeInAnimation->setDuration(REVEAL_ITEMS_FADEIN_DURATION);
		fadeInAnimation->setStartAlpha(0);
		fadeInAnimation->setEndAlpha(1);
		fadeInAnimation->setInterpolator(
				InterpolatorPtr(new DeccelerateInterpolator));

		// Create a stay animation
		shared_ptr<AlphaAnimation> stayAnimation(new AlphaAnimation());
		stayAnimation->setDuration(REVEAL_ITEMS_STAY_DURATION);
		stayAnimation->setStartAlpha(1);
		stayAnimation->setEndAlpha(1);

		// Create a fade out animation
		shared_ptr<AlphaAnimation> fadeOutAnimation(new AlphaAnimation());
		fadeOutAnimation->setDuration(REVEAL_ITEMS_FADEOUT_DURATION);
		fadeOutAnimation->setStartAlpha(1);
		fadeOutAnimation->setEndAlpha(0);
		fadeOutAnimation->setInterpolator(
				InterpolatorPtr(new AccelerateInterpolator));

		// Create a sequence
		shared_ptr<SequenceAnimationComposite> revealItemsAnimation(
				new SequenceAnimationComposite);
		revealItemsAnimation->addAnimation(fadeInAnimation);
		revealItemsAnimation->addAnimation(stayAnimation);
		revealItemsAnimation->addAnimation(fadeOutAnimation);
		revealItemsAnimation->start(AndroidPortUtils::getTimeOfDayMillis());

		hotspotDrawable->setAnimation(revealItemsAnimation);
		mAnimatedDrawables.push_back(hotspotDrawable);
	}
}

void AndroidPortAdditions::performInventoryScroll(bool up) {
	if (up) {
		pushClickEvent(INVENTORY_UP_ARROW_X, INVENTORY_UP_ARROW_Y);
	} else {
		pushClickEvent(INVENTORY_DOWN_ARROW_X, INVENTORY_DOWN_ARROW_Y);

	}
}

void AndroidPortAdditions::copyPixelsBetweenSurfaces(Surface* src, Surface* dst,
		Rect srcRect, Point dstPoint, bool startFromBottom) {
//	LOGD("AndroidPortAdditions::copyPixelsBetweenSurfaces: src: %d %d %d %d dst: %d %d startFromBottom: %d", srcRect.left, srcRect.top, srcRect.right, srcRect.bottom, dstPoint.x, dstPoint.y, startFromBottom);

	byte* srcPixels = (byte*) src->pixels;
	byte* dstPixels = (byte*) dst->pixels;

	byte bytesPerPixel = src->format.bytesPerPixel;

// Check that the pixel formats match (only in terms of bytes per pixel)
	if (dst->format.bytesPerPixel != bytesPerPixel) {
		LOGE(
				"AndroidPortAdditions::copyPixelsBetweenSurfaces: pixel formats do not match, pixels per byte: %d and %d",
				bytesPerPixel, dst->format.bytesPerPixel);
		return;
	}

	for (uint16 y = 0; y < srcRect.height(); ++y) {
		// Decide whether to count the rows from bottom or from top
		uint16 rowIndex = startFromBottom ? (srcRect.height() - 1 - y) : y;

		uint16 srcRow = srcRect.top + rowIndex;
		uint16 dstRow = dstPoint.y + rowIndex;

		byte* srcPtr = srcPixels + srcRow * src->pitch
				+ srcRect.left * bytesPerPixel;
		byte* dstPtr = dstPixels + dstRow * dst->pitch
				+ dstPoint.x * bytesPerPixel;

		memcpy(dstPtr, srcPtr, srcRect.width() * bytesPerPixel);
	}
}

void AndroidPortAdditions::beforeDrawTextureToScreen(Surface* gameSurface) {
// Debugging
#ifdef ANDROID_PORT_DEBUG

	if (mDumpGameScreenToFile) {
		AndroidPortUtils::dumpBytesToFile((byte*) gameSurface->pixels,
				gameSurface->h * gameSurface->pitch
						* gameSurface->format.bytesPerPixel,
				"mnt/sdcard/gamescreen.clut8");
		mDumpGameScreenToFile = false;
	}
#endif

	// Check bottom toolbar
	checkBottomToolbar(gameSurface);

// Check chat mode
	checkGameInChat(gameSurface);

// Check postcard screen
	checkGameInPostcard(gameSurface);

// Identify the current game area
	GameArea previousArea = mCurrentGameArea;
	mCurrentGameArea = GameAreaIdentifier::identifyGameArea(
			(const char*) gameSurface->pixels);

	//LOGD("AndroidPortAdditions::beforeDrawTextureToScreen: game area: %d", mCurrentGameArea);

	// Workaround for Simon1 puddle save bug
	if (mCurrentGameArea == PUDDLE_WATER)
	{
		mShouldPerformSimon1PuddleWorkaround = true;
	}

// If we were on hebrew title and it's gone, notify
	if (previousArea == HEBREW_TITLE && mCurrentGameArea != HEBREW_TITLE) {
		JNI::gameEventJNIToJava(GAME_EVENT_HEBREW_TITLE_GONE);
	}

	if (mBottomToolbarAppearing && !mClassicMode) {
		// Move the middle black panel to the top.
		// This will also generate the modified game pixels.
		moveBlackPanel(gameSurface);

		mShouldUseModifiedGamePixels = true;

	} else {
		mShouldUseModifiedGamePixels = false;
	}

// Auto load if needed
	autoloadBehavior();
	loadIfNeeded();
	saveIfNeeded();

// Prevent idle mode if needed
	uint64 currentTime = AndroidPortUtils::getTimeOfDayMillis();
	if (currentTime - mLastIdlePreventionTimestamp
			>= IDLE_PREVENTION_CLICK_INTERVAL && mBottomToolbarAppearing
			&& mMouseVisible && !mGameInPostcard) {
		preventIdleMode();
		mLastIdlePreventionTimestamp = currentTime;
	}

	// Add reveal items animation if needed
	if (mShouldPerformRevealItems) {
		performRevealItems();
		mShouldPerformRevealItems = false;
	}

// All logic related to touch events in the game itself
	gameTouchBehavior();
}

void AndroidPortAdditions::autoloadBehavior() {
// Logic for auto-load behavior
	if (mAutoLoadSlot != -1) {
		// Spam "skip" events every few frames in case we are in auto-load.
		// This is to skip the starting animation and get fast into a state where loading can be done.
		if (mAutoLoadSpamSkipCounter % 5 == 0) {
			performSkip(false);
		}

		++mAutoLoadSpamSkipCounter;

		// If we got the botton toolbar, and the mouse is visible, we are after the opening movie.
		// Auto-load the slot, and get out of the auto-load state
		if (mBottomToolbarAppearing && mMouseVisible) {
			// We set the ending condition before performing the load, in order to avoid recursion
			// (because of the way the AGOS engine is implemented - loading results in direct screen update).
			int32 slotToLoadNow = mAutoLoadSlot;
			mAutoLoadSlot = -1;

			if (loadGame(slotToLoadNow)) {
				JNI::gameEventJNIToJava(GAME_EVENT_LOAD_SUCCESS);
			} else {
				JNI::gameEventJNIToJava(GAME_EVENT_LOAD_FAILURE);
				LOGE(
						"AndroidPortAdditions::beforeDrawTextureToScreen: Error auto-loading slot %d",
						slotToLoadNow);
			}
		}
	}
}

void AndroidPortAdditions::loadIfNeeded() {
// Load chosen slot if needed
	if (mSlotToLoad != -1) {
		int32 slotToLoadNow = mSlotToLoad;
		mSlotToLoad = -1;

		// Check conditions for loading
		if (mBottomToolbarAppearing && mMouseVisible) {

			if (!loadGame(slotToLoadNow)) {
				JNI::gameEventJNIToJava(GAME_EVENT_LOAD_FAILURE);
				LOGE(
						"AndroidPortAdditions::beforeDrawTextureToScreen: Error loading slot %d",
						slotToLoadNow);
			} else {
				JNI::gameEventJNIToJava(GAME_EVENT_LOAD_SUCCESS);
			}
		} else {
			JNI::gameEventJNIToJava(GAME_EVENT_LOAD_FAILURE);
			LOGD(
					"AndroidPortAdditions::beforeDrawTextureToScreen: could not load at this time");
		}
	}
}

void AndroidPortAdditions::saveIfNeeded() {
// Check if there is a request to save
	if (mSlotToSave == -1) {
		return;
	}

	if (mForceSaveLoad) {
		int32 slotToSaveNow = mSlotToSave;
		mSlotToSave = -1;
		if (!saveGame(slotToSaveNow)) {
			JNI::gameEventJNIToJava(GAME_EVENT_SAVE_FAILURE);
			LOGE(
					"AndroidPortAdditions::beforeDrawTextureToScreen: Error saving slot %d",
					slotToSaveNow);
		} else {
			JNI::gameEventJNIToJava(GAME_EVENT_SAVE_SUCCESS);
		}

		return;
	}

	uint64 currentTime = AndroidPortUtils::getTimeOfDayMillis();
// Check saving conditions for the first time
	if (!mSaveInProgress) {
		if (!checkSaveConditions()) {
			resetSaveState();
			JNI::gameEventJNIToJava(GAME_EVENT_SAVE_FAILURE);
			LOGD(
					"AndroidPortAdditions::beforeDrawTextureToScreen: could not save at this time");

			return;
		}

		mSaveInProgress = true;
		mSaveStartTimestamp = currentTime;

		// Stop Simon from walking when beginning saving
		g_engine->stopWalking();
	}

// Check if the game idle counter ticked since saving started
	if (mGameIdleCountWhileSaving) {
		// If so, check conditions again and then save if possible
		if (checkSaveConditions()) {
			int32 slotToSaveNow = mSlotToSave;
			mSlotToSave = -1;
			if (!saveGame(slotToSaveNow)) {
				JNI::gameEventJNIToJava(GAME_EVENT_SAVE_FAILURE);
				LOGE(
						"AndroidPortAdditions::beforeDrawTextureToScreen: Error saving slot %d",
						slotToSaveNow);
			} else {
				JNI::gameEventJNIToJava(GAME_EVENT_SAVE_SUCCESS);
			}
		} else {
			JNI::gameEventJNIToJava(GAME_EVENT_SAVE_FAILURE);
			LOGD(
					"AndroidPortAdditions::beforeDrawTextureToScreen: could not save at this time");
		}

		resetSaveState();
	}

// Check if the timeout has passed, and fail if it did
	if (currentTime - mSaveStartTimestamp >= SAVE_TIMEOUT) {
		resetSaveState();
		JNI::gameEventJNIToJava(GAME_EVENT_SAVE_FAILURE);
		LOGD(
				"AndroidPortAdditions::beforeDrawTextureToScreen: could not save at this time");
	}
}

void AndroidPortAdditions::resetSaveState() {
	mSlotToSave = -1;
	mSaveInProgress = false;
	mGameIdleCountWhileSaving = false;
}

void AndroidPortAdditions::copyPixelsToMagnifier() {
//	LOGD("copyPixelsToMagnifier mMagnifierX %d mMagnifierY %d mMagnifierCopyRectWidth %f mMagnifierCopyRectHeight %f", mMagnifierX, mMagnifierY, mMagnifierCopyRectWidth, mMagnifierCopyRectHeight);

//
// Source rect calculations
//

	uint16 scalingFactor;
	if (mScalingOption == SCALING_OPTION_SOFT)
		scalingFactor = mScalingFactor;
	else
		scalingFactor = 1;

// Calculate the ratio between game pixels and texture size
	float gameToTextureRatioW = GAME_SCREEN_WIDTH * scalingFactor
			/ (float) mGameTextureW;
	float gameToTextureRatioH = GAME_SCREEN_HEIGHT * scalingFactor
			/ (float) mGameTextureH;

//	LOGD("copyPixelsToMagnifier mCurrentTouchX %d mCurrentTouchY %d gameToTextureRatioW %f gameToTextureRatioH %f", mCurrentTouchX, mCurrentTouchY, gameToTextureRatioW, gameToTextureRatioH);
//	LOGD("copyPixelsToMagnifier mGameTextureW %d mGameTextureH %d", mGameTextureW, mGameTextureH);

// Get the center of the copy source rect - based on the touched point (as texture coordinates)
// This is factored to account for the ratio between game screen and the containing game texture.
	float copySrcX = ((float) mMagnifierTouchX / mDisplayWidth)
			* gameToTextureRatioW;
	float copySrcY = ((float) mMagnifierTouchY / mDisplayHeight)
			* gameToTextureRatioH;

// Get the source width \ height as texture coordinates
	float copySrcW = mMagnifierCopyRectWidth * gameToTextureRatioW;
	float copySrcH = mMagnifierCopyRectHeight * gameToTextureRatioH;

//	LOGD("copyPixelsToMagnifier: initial tex coords %f %f %f %f", copySrcX, copySrcY, copySrcW, copySrcH);

// Shift the src point to the upper left corner of the rect.
	copySrcX -= copySrcW / 2;
	copySrcY -= copySrcH / 2;

//	LOGD("copyPixelsToMagnifier: tex coords after shift to corner %f %f %f %f", copySrcX, copySrcY, copySrcW, copySrcH);

/// Calculate the copy padding (the areas that should not be copied from the texture)
// These values are as texture coordinate values
	float leftPadding = MAX(0.0f - copySrcX, 0.0f);
	float rightPadding = MAX(copySrcX + copySrcW - gameToTextureRatioW, 0.0f);
	float topPadding = MAX(
			BLACK_PANEL_HEIGHT / (float) GAME_SCREEN_HEIGHT
					* gameToTextureRatioH - copySrcY, 0.0f);
	float bottomPadding = MAX(
			copySrcY + copySrcH
					- BLACK_PANEL_END_Y / (float) GAME_SCREEN_HEIGHT
							* gameToTextureRatioH, 0.0f);

//	LOGD("copyPixelsToMagnifier: padding left %f right %f top %f bottom %f", leftPadding, rightPadding, topPadding, bottomPadding);

// Adjust the texture coordinates according to padding
	copySrcX = MAX(copySrcX, 0.0f);
	copySrcY = MAX(copySrcY, 0.0f);
	copySrcW = copySrcW - leftPadding - rightPadding;
	copySrcH = copySrcH - topPadding - bottomPadding;

// Create the texture coordinates
	GLfloat texCoords[] = { copySrcX, copySrcY, copySrcX + copySrcW, copySrcY,
			copySrcX, copySrcY + copySrcH, copySrcX + copySrcW, copySrcY
					+ copySrcH };

//	LOGD("copyPixelsToMagnifier: tex coords %f %f %f %f", copySrcX, copySrcY, copySrcW, copySrcH);

//
// Dest rect calculations
//

// Get the magnifier coordinates in GL coordinates (this is our copy destination)
	float copyDestX = (float) mMagnifierX / mDisplayWidth * 2.0 - 1.0;
	float copyDestY = (float) mMagnifierY / mDisplayHeight * (-2.0) + 1.0;

// Get the magnifier width\height in GL coordinates
	float copyDestW = (float) mMagnifierCopyRectWidth * 2.0;
	float copyDestH = (float) mMagnifierCopyRectHeight * 2.0;

// Create the vertex array for the black rect
	GLfloat blackVertices[] = { copyDestX, copyDestY, copyDestX + copyDestW,
			copyDestY, copyDestX, copyDestY - copyDestH, copyDestX + copyDestW,
			copyDestY - copyDestH };

//	LOGD("copyPixelsToMagnifier: black vertices %f %f %f %f", copyDestX, copyDestY, copyDestW, copyDestH);

// Convert the padding values to vertex coordinates
	float leftPaddingVertex = leftPadding / gameToTextureRatioW * 2;
	float rightPaddingVertex = rightPadding / gameToTextureRatioW * 2;
	float topPaddingVertex = topPadding / gameToTextureRatioH * 2;
	float bottomPaddingVertex = bottomPadding / gameToTextureRatioH * 2;

// Adjust the copy destinations according to padding
	copyDestX += leftPaddingVertex;
	copyDestY -= topPaddingVertex;
	copyDestW = copyDestW - leftPaddingVertex - rightPaddingVertex;
	copyDestH = copyDestH - topPaddingVertex - bottomPaddingVertex;

// Create the vertex array for the black rect
	GLfloat copyVertices[] = { copyDestX, copyDestY, copyDestX + copyDestW,
			copyDestY, copyDestX, copyDestY - copyDestH, copyDestX + copyDestW,
			copyDestY - copyDestH };

//	LOGD("copyPixelsToMagnifier: vertices %f %f %f %f", copyDestX, copyDestY, copyDestW, copyDestH);

	GLCALL(glBindTexture(GL_TEXTURE_2D, mGameTexture));

// Draw black on the rect
	GLCALL(glUseProgram(mBlackShaderProgram->mProgramHandle));
	GLCALL(
			glVertexAttribPointer(mBlackShaderProgram->mPositionAttributeHandle, 2, GL_FLOAT, false, 0, blackVertices));
	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

// Make sure we use the scaling shade program
	GLCALL(glUseProgram(mScalerShaderProgram->mProgramHandle));

// Set the texture uniform
	GLCALL(glUniform1i(mScalerShaderProgram->mTextureUniformHandle, 0));

// Set the alpha uniform
	GLCALL( glUniform1f(mScalerShaderProgram->mAlphaFactorUniformHandle, 1.0));

// Pass the position attributes
	GLCALL(
			glVertexAttribPointer(mScalerShaderProgram->mPositionAttributeHandle, 2, GL_FLOAT, false, 0, copyVertices));

// Pass the texture coordinate attributes
	GLCALL(
			glVertexAttribPointer(mScalerShaderProgram->mTexCoordAttributeHandle, 2, GL_FLOAT, false, 0, texCoords));

	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

void AndroidPortAdditions::checkGameInChat(Surface* gameSurface) {

	bool gameInChat;

	if (!mMouseVisible || mBottomToolbarAppearing) {
		gameInChat = false;
	} else {
		//
		// check if the game is in chat mode by checking if a certain line has only black pixels
		//

		// Get the row pointer
		byte* rowPtr = (byte*) gameSurface->pixels
				+ CHAT_MODE_CHECK_Y * gameSurface->pitch;
		byte* rowEnd = rowPtr
				+ gameSurface->w * gameSurface->format.bytesPerPixel;

		// Check if all bytes are 0 (applies to all pixel formats)
		gameInChat = true;
		while (rowPtr < rowEnd) {

			if (*rowPtr) {
				// Byte != 0, so pixel is not black
				gameInChat = false;
				return;
			}

			++rowPtr;
		}
	}

	// Check if we got out of chat mode - if so, reposition the cursor in the game area.
	// This is done to prevent false highlighting of a chat choice in the next chat screen.
	if (mGameInChat == true && gameInChat == false) {
		pushScrollEvent(0, 0);
	}

	// Check if we got into chat mode - if so, set the default line selection to 1
	if (mGameInChat == false && gameInChat == true) {
		// Get all the chat hotspots
		Point points[10];
		uint16 count = mHitAreaHelper.getAllChatHotspots(points, 10);

		if (count > 0) {
			Point first = points[0];
			pushScrollEvent(first.x, first.y);

			mSelectedChatRow = 1;
		}

	}

	mGameInChat = gameInChat;
}

void AndroidPortAdditions::checkGameInPostcard(Surface* gameSurface) {
//
// check if the game is in the postcard screen by querying certain pixels
//

// Get the row pointer
	byte* rowPtr = (byte*) gameSurface->pixels
			+ POSTCARD_WINDOW_CHECK_Y * gameSurface->pitch;

// Check for a certain pixel pattern
	if (rowPtr[65] == 228 && rowPtr[75] == 243 && rowPtr[79] == 254
			&& rowPtr[91] == 227 && rowPtr[133] == 225 && rowPtr[200] == 225
			&& rowPtr[254] == 228) {
		mGameInPostcard = true;
	} else {
		mGameInPostcard = false;
	}
}

void AndroidPortAdditions::checkBottomToolbar(Surface* gameSurface) {
//
// check if the game has inventory by querying certain pixels
//

// Get the row pointer
	byte* rowPtr = (byte*) gameSurface->pixels
			+ BOTTOM_TOOLBAR_CHECK_Y * gameSurface->pitch;

// Check for a certain pixel pattern
	if (rowPtr[1] == 240 && rowPtr[25] == 247 && rowPtr[50] == 240
			&& rowPtr[75] == 247 && rowPtr[100] == 250 && rowPtr[125] == 240) {
		mBottomToolbarAppearing = true;
	} else {
		mBottomToolbarAppearing = false;
	}
}

void AndroidPortAdditions::moveBlackPanel(Surface* gameSurface) {
//	LOGD("AndroidPortAdditions::moveBlackPanel: surface: w %d h %d pitch %d", gameSurface->w, gameSurface->h, gameSurface->pitch);

//
// In order to move the black panel, copy it to a temp pixel buffer, move the whole screen down,
// then copy the black panel to the top.
//

	uint16 panelHeight = BLACK_PANEL_END_Y - BLACK_PANEL_START_Y;

// Allocate the modified pixels if needed
	if (mModifiedGameSurface.pixels == NULL) {
		mModifiedGameSurface.create(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT,
				gameSurface->format);

		PixelFormat format = gameSurface->format;
		/*	LOGD(
		 "AndroidPortAdditions::moveBlackPanel: bpp %d losses r %d g %d b %d a %d shifts r %d g %d b %d a %d",
		 format.bytesPerPixel, format.rBits(), format.gBits(), format.bBits(), format.aBits(), format.rShift, format.gShift, format.bShift, format.aShift);*/
	}

// Copy the black panel to the top
	Rect srcRect;
	srcRect.left = 0;
	srcRect.top = BLACK_PANEL_START_Y;
	srcRect.setWidth(GAME_SCREEN_WIDTH);
	srcRect.setHeight(panelHeight);

	Point dstPoint;
	dstPoint.x = 0;
	dstPoint.y = 0;

	copyPixelsBetweenSurfaces(gameSurface, &mModifiedGameSurface, srcRect,
			dstPoint);

// Copy the upper game screen to the previous black panel location (move down)
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.setWidth(GAME_SCREEN_WIDTH);
	srcRect.setHeight(BLACK_PANEL_START_Y);

	dstPoint.x = 0;
	dstPoint.y = panelHeight;

	copyPixelsBetweenSurfaces(gameSurface, &mModifiedGameSurface, srcRect,
			dstPoint);

// Copy the remains of the screen to the same position
	srcRect.left = 0;
	srcRect.top = BLACK_PANEL_END_Y;
	srcRect.setWidth(GAME_SCREEN_WIDTH);
	srcRect.setHeight(GAME_SCREEN_HEIGHT - BLACK_PANEL_END_Y);

	dstPoint.x = 0;
	dstPoint.y = BLACK_PANEL_END_Y;

	copyPixelsBetweenSurfaces(gameSurface, &mModifiedGameSurface, srcRect,
			dstPoint);
}

bool AndroidPortAdditions::shouldUseModifiedGamePixels() {
	if (mModifiedGameSurface.pixels == NULL) {
		// Not initialized yet
		return false;
	}

	return mShouldUseModifiedGamePixels;
}

byte* AndroidPortAdditions::getModifiedGamePixels() {
	return (byte*) mModifiedGameSurface.pixels;
}

void AndroidPortAdditions::setTouchpadMode(bool touchpadMode) {
	LOGD("AndroidPortAdditions::setTouchpadMode: %d", touchpadMode);

	mClassicMode = touchpadMode;
}

bool AndroidPortAdditions::getTouchpadMode() {
	return mClassicMode;
}

void AndroidPortAdditions::translateTouchCoordinates(int& x, int& y) {
//	LOGD("AndroidPortAdditions::translateTouchCoordinates: before: %d %d", x, y);

// Check if we need to translate at that position
	if (mShouldUseModifiedGamePixels && y <= mTouchTranslationLowerBound) {
		// Translate the real coordinates into our fake coordinates (because we moved screen parts)
		y -= mTouchTranslationOffset;
		if (y < 0) {
			y = 0;
		}
	}

//	LOGD("AndroidPortAdditions::translateTouchCoordinates: after: %d %d", x, y);
}
/*
 void AndroidPortAdditions::requestMagnifierForCoordinates(int32 x, int32 y) {
 //	LOGD("AndroidPortAdditions::requestMagnifierForCoordinates: %d %d", x ,y);

 AndroidBitmap* magnifier = getBitmap("frame.png");

 // If a magnifier is already displayed, but we scrolled to the inventory area,
 // assume that the touched point is at the toolbar top edge.
 // Set a flag for this special case.
 bool touchInToolbarArea = false;
 if (mMagnifierX != -1 && mMagnifierY != -1
 && y >= mTouchTranslationLowerBound) {
 touchInToolbarArea = true;

 // The Y value used to calculate the magnifier position has a maximum
 if (y >= mTouchTranslationLowerBound + magnifier->displayHeight / 2) {
 y = mTouchTranslationLowerBound + magnifier->displayHeight / 2 - 1;
 }
 }

 mMagnifierX = -1;
 mMagnifierY = -1;

 // Check that we're in the right state for displaying a magnifier
 if (mClassicMode || !mMouseVisible || mGameInChat || mGameInPostcard) {
 return;
 }

 // Check that the coordinates are valid for a magnifier (Except when scrolling to the toolbar area)
 if (!isValidMagnifierCoordinates(x, y) && !touchInToolbarArea) {
 return;
 }

 // Check that the magnifier display dimensions are initialized;
 if (magnifier->displayWidth == 0 || magnifier->displayHeight == 0) {
 return;
 }

 // Set the magnifier coordinates as requested, based on screen positions.
 // Normal position is right above the touched coordinates.

 if (x >= mDisplayWidth - magnifier->displayWidth / 2) {
 // Pin to right edge
 mMagnifierX = mDisplayWidth - magnifier->displayWidth;
 } else if (x <= magnifier->displayWidth / 2) {
 // Pin to the left edge
 mMagnifierX = 0;
 } else {
 // Normal X value
 mMagnifierX = x - magnifier->displayWidth / 2;
 }

 if (y <= mTouchTranslationOffset + magnifier->displayHeight * 3 / 2) {
 // Case when we are near the top black panel
 if (x >= magnifier->displayWidth * 3 / 2) {
 // Top and not close to the left - left of touch point, pinned to the top
 mMagnifierX = x - magnifier->displayWidth * 3 / 2;
 mMagnifierY = mTouchTranslationOffset;
 } else {
 // Top and close to the left - below touch point
 mMagnifierX = 0;
 mMagnifierY = y + magnifier->displayHeight / 2;
 }
 } else {
 // Normal Y value
 mMagnifierY = y - magnifier->displayHeight * 3 / 2;
 }

 // Check to see if we need to get a new timestamp because of movement - based on sensitivity
 if (mMagnifierTouchX != 0 && mMagnifierTouchY != 0) {
 if (mMagnifierTouchX < x - WALK_ACTION_TOUCH_SENSITIVITY_PIXELS
 || mMagnifierTouchX > x + WALK_ACTION_TOUCH_SENSITIVITY_PIXELS
 || mMagnifierTouchY < y - WALK_ACTION_TOUCH_SENSITIVITY_PIXELS
 || mMagnifierTouchY > y + WALK_ACTION_TOUCH_SENSITIVITY_PIXELS) {
 // Record the timestamp where the magnifier was requested
 mLastTouchMoveTimestamp = AndroidPortUtils::getTimeOfDayMillis();
 }
 }

 mMagnifierTouchX = x;
 mMagnifierTouchY = y;

 // The touched Y can never be inside the toolbar area or the upper panel
 if (mMagnifierTouchY <= BLACK_PANEL_HEIGHT / mGameToDisplayRatioH) {
 mMagnifierTouchY = BLACK_PANEL_HEIGHT / mGameToDisplayRatioH;
 }
 if (mMagnifierTouchY >= mTouchTranslationLowerBound) {
 mMagnifierTouchY = mTouchTranslationLowerBound - 1;
 }

 //	LOGD("AndroidPortAdditions::requestMagnifierForCoordinates: mag x %d mag y %d touch x %d touch y %d", mMagnifierX, mMagnifierY, mCurrentTouchX, mCurrentTouchY);
 }*/

/*
 bool AndroidPortAdditions::isValidMagnifierCoordinates(int32 x, int32 y) {
 int32 minY = mTouchTranslationOffset;
 int32 maxY = mTouchTranslationLowerBound;
 return (y >= minY && y < maxY);
 }
 */

/*
 Rect AndroidPortAdditions::getDirtyRect()
 {
 // If we draw a magnifier, we return a dirty rect that represents it so the texture will update
 if (mMagnifierX >= 0 && mMagnifierY >= 0)
 {
 return Rect(mMagnifierX, mMagnifierY, mMagnifierX + mMagnifierCopyRectWidth, mMagnifierY + mMagnifierCopyRectHeight);

 }
 else
 {
 return Rect();
 }
 }
 */

/*
 Action AndroidPortAdditions::recognizeActionPress(int32 x, int32 y) {
 LOGD("AndroidPortAdditions::recognizeActionPress: %d %d", x, y);

 // Calculate the grid position of the action based on the click
 uint16 row, col;
 switch (x) {
 case 0 ... 49:
 col = 0;
 break;
 case 50 ... 100:
 col = 1;
 break;
 case 101 ... 135:
 col = 2;
 break;
 case 136 ... 169:
 col = 3;
 break;
 default:
 return WALK; //unknown col
 }

 switch (y) {
 case 145 ... 161:
 row = 0;
 break;
 case 162 ... 179:
 row = 1;
 break;
 case 180 ... 200:
 row = 2;
 break;
 default:
 return WALK; //unknown row
 }

 // If we got here, an action was pressed
 mCurrentAction = mActionGrid[row][col];

 // Set the fade counter for the action icon - it will be faded on the screen
 mActionIconFadeCounter = ACTION_ICON_FADEOUT_COUNT;

 LOGD("AndroidPortAdditions::recognizeActionPress: current action %d",
 mCurrentAction);

 return mCurrentAction;
 }*/

AndroidBitmap* AndroidPortAdditions::getActionIcon(uint16 action) {
	switch (action) {
	case ACTION_WALK:
		return getBitmap("walk.png");
	case ACTION_LOOK:
		return getBitmap("look.png");
	case ACTION_OPEN:
		return getBitmap("open.png");
	case ACTION_MOVE:
		return getBitmap("move.png");
	case ACTION_CONSUME:
		return getBitmap("consume.png");
	case ACTION_PICK:
		return getBitmap("pick.png");
	case ACTION_CLOSE:
		return getBitmap("close.png");
	case ACTION_USE:
		return getBitmap("use.png");
	case ACTION_TALK:
		return getBitmap("talk.png");
	case ACTION_REMOVE:
		return getBitmap("remove.png");
	case ACTION_WEAR:
		return getBitmap("wear.png");
	case ACTION_GIVE:
		return getBitmap("give.png");
	default:
		return NULL;
	}
}

void AndroidPortAdditions::preventIdleMode() {
	LOGD("AndroidPortAdditions::preventIdleMode()");

	g_engine->preventIdleMode();
}

bool AndroidPortAdditions::checkSaveConditions() {
	LOGD("AndroidPortAdditions::checkSaveConditions: game area %d",
			mCurrentGameArea);

// Override condition - never allow saving in goblin fortress with paper on floor due to engine bug
// http://wiki.scummvm.org/index.php/AGOS/Bugs
	if (mCurrentGameArea == GOBLIN_FORTRESS_WITH_PAPER_ON_FLOOR) {
		return false;
	}

	// Override condition - never allow saving in puddle area after water was poured
	if (mShouldPerformSimon1PuddleWorkaround)
	{
		if (mCurrentGameArea == PUDDLE_AFTER_WATER || mCurrentGameArea == BEFORE_PUDDLE)
		{
			return false;
		}
	}

	int32 timerEvents = g_engine->getTimerEventCount();

	LOGD(
			"AndroidPortAdditions::checkSaveConditions: toolbar %d mouse %d timer events %d",
			mBottomToolbarAppearing, mMouseVisible, timerEvents);

	return mBottomToolbarAppearing && mMouseVisible && timerEvents <= 1;
}

bool AndroidPortAdditions::checkLoadConditions() {
	return mBottomToolbarAppearing && mMouseVisible;
}

void AndroidPortAdditions::gameTouchEvent(int16 x, int16 y, int16 origX,
		int16 origY, TouchState touchState) {

	if (mDisallowNextTouchEvent) {
		// Ignore the current touch event - workaround for scenarios where we handle a gesture and want to ignore its touch components.
		mDisallowNextTouchEvent = false;
		return;
	}

	mTouchEventMutex->lock();

	int gameTouchX = x * mGameToDisplayRatioW;
	int gameTouchY = y * mGameToDisplayRatioH;

// Check for fling on inventory
	if (touchState == FLING) {
		int gameOrigX = origX * mGameToDisplayRatioW;
		int gameOrigY = origY * mGameToDisplayRatioH;

		if (gameTouchX >= INVENTORY_FLING_MIN_X
				&& gameOrigX >= INVENTORY_FLING_MIN_X
				&& gameTouchY >= INVENTORY_FLING_MIN_Y
				&& gameOrigY >= INVENTORY_FLING_MIN_Y) {

			// Check fling direction
			bool up = (gameOrigY <= gameTouchY);
			performInventoryScroll(up);

			// We don't deal with this event any more, and ignore the next "UP"
			touchState = NONE;
			mDisallowNextTouchEvent = true;
		} else {
			// Fling was not on inventory, we treat it as a normal scroll
			touchState = DOWN;
		}
	}

	mGameTouchX = gameTouchX;
	mGameTouchY = gameTouchY;

// Adjust Y if it's inside the game area, because we move the black panel
	if (!mGameInChat && mGameTouchY <= BLACK_PANEL_END_Y) {
		mGameTouchY -= BLACK_PANEL_HEIGHT;
		if (mGameTouchY < 0) {
			mGameTouchY = 0;
		}
	}

// Check for any event inside the chat area, and reset the selected row if needed
	if (mGameInChat && mGameTouchY >= CHAT_HIT_AREAS_START_Y) {
		mSelectedChatRow = 0;
	}

// Adjust X to the right side if we're in the inventory arrow area.
// This is done to increase the arrow hit zone.
	if (!mGameInChat && mGameTouchY > BLACK_PANEL_END_Y
			&& mGameTouchX >= INVENTORY_SCROLL_ARROW_START_X) {
		mGameTouchX = INVENTORY_SCROLL_ARROW_END_X;
	}

	mTouchState = touchState;

	mTouchEventMutex->unlock();

}

void AndroidPortAdditions::outsideDragonCaveWorkaround(int16 &x, int16 &y) {

	LOGD("AndroidPortAdditions::outsideDragonCaveWorkaround: %d %d", x, y);

	// Temporary workaround - make the boulder hit area bigger
	if (x >= OUTSIDE_DRAGON_CAVE_WORKAROUND_MIN_X
			&& x <= OUTSIDE_DRAGON_CAVE_WORKAROUND_MAX_X
			&& y >= OUTSIDE_DRAGON_CAVE_WORKAROUND_MIN_Y
			&& y <= OUTSIDE_DRAGON_CAVE_WORKAROUND_MAX_Y) {

		x = OUTSIDE_DRAGON_CAVE_WORKAROUND_TARGET_X;
		y = OUTSIDE_DRAGON_CAVE_WORKAROUND_TARGET_Y;
	}
}

void AndroidPortAdditions::chatArrowClick(bool up) {
// Get all the chat hotspots
	Point points[10];
	uint16 count = mHitAreaHelper.getAllChatHotspots(points, 10);

// Adjust the selected row number.
// Chat hit areas are ordered from bottom to top.
	if (up) {
		--mSelectedChatRow;
		if (mSelectedChatRow <= 0) {
			mSelectedChatRow = count;
		}
	} else {
		++mSelectedChatRow;
		if (mSelectedChatRow > count) {
			mSelectedChatRow = 1;
		}
	}

// Scroll the game mouse to the selected row
	Point selected = points[mSelectedChatRow - 1];

	mTouchEventMutex->lock();
	mTouchState = DOWN;
	mGameTouchX = selected.x;
	mGameTouchY = selected.y;
	mTouchEventMutex->unlock();
}

void AndroidPortAdditions::chatButtonClick() {
// Get all the chat hotspots
	Point points[10];
	uint16 count = mHitAreaHelper.getAllChatHotspots(points, 10);

// Click the selected row
	Point selected;
	if (mSelectedChatRow <= 0 || mSelectedChatRow > count) {
		selected = points[0];
	} else {
		selected = points[mSelectedChatRow - 1];
	}

	pushClickEvent(selected.x, selected.y);

	mSelectedChatRow = 0;
}

#endif

