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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "gui/s_overlay.h"
#include "gui/s_mainmenu.h"

namespace Common {
DECLARE_SINGLETON(GUI::SOverlay);
}

#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/onscreendialog.h"

#include "common/events.h"

namespace GUI {

#define CHAT_MODE_CHECK_Y 199
#define BOTTOM_TOOLBAR_CHECK_Y 199

#define POSTCARD_WINDOW_CHECK_Y 105

// Postcard
#define POSTCARD_WINDOW_CHECK_Y 105
#define POSTCARD_LOAD_START_X 62
#define POSTCARD_LOAD_START_Y 40
#define POSTCARD_LOAD_END_X 156
#define POSTCARD_LOAD_END_Y 70
#define POSTCARD_SAVE_START_X 62
#define POSTCARD_SAVE_START_Y 71
#define POSTCARD_SAVE_END_X 156
#define POSTCARD_SAVE_END_Y 110
#define POSTCARD_EXIT_START_X 157
#define POSTCARD_EXIT_START_Y 40
#define POSTCARD_EXIT_END_X 235
#define POSTCARD_EXIT_END_Y 71
#define POSTCARD_CONTINUE_CLICK_X 180
#define POSTCARD_CONTINUE_CLICK_Y 80
#define POSTCARD_CLICK_NONE 0
#define POSTCARD_CLICK_SAVE 1
#define POSTCARD_CLICK_LOAD 2
#define POSTCARD_CLICK_EXIT 3

SOverlay::SOverlay() {
	_initialized = false;
	_active = true;

	_controlPanel = 0;

	_mouseVisible = false;
	_gameInChat = false;
	_gameInPostcard = false;
	_bottomToolbarAppearing = false;
	_classicMode = false;

	_selectedChatRow = 0;

	_hitAreaHelper = new HitAreaHelper;

	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

SOverlay::~SOverlay() {
	_controlPanel->close();

	delete _controlPanel;

	g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
}

void SOverlay::init() {
	_controlPanel = new SDialog();

	_initialized = true;
}

void SOverlay::preDrawOverlayGui() {
    if (_initialized && _active) {
		g_system->showOverlay();
		g_gui.theme()->clearAll();
		g_gui.theme()->openDialog(true, GUI::ThemeEngine::kShadingNone);

		_controlPanel->updateHotspots();

		_controlPanel->drawDialog();
		g_gui.theme()->finishBuffering();
		g_gui.theme()->updateScreen();
   }
}

void SOverlay::beforeDrawTextureToScreen(Graphics::Surface* gameSurface) {
	if (!g_engine)
		return;

	// Check bottom toolbar
	checkBottomToolbar(gameSurface);

	// Check chat mode
	checkGameInChat(gameSurface);

	// Check postcard screen
	checkGameInPostcard(gameSurface);
}

void SOverlay::postDrawOverlayGui() {
    if (_initialized && _active) {
	    g_system->hideOverlay();
	}
}

bool SOverlay::notifyEvent(const Common::Event &event) {
	Common::Event event1 = event;

	event1.mouse.x = event.mouse.x * g_system->getOverlayWidth() / g_system->getWidth();
	event1.mouse.y = event.mouse.y * g_system->getOverlayHeight() / g_system->getHeight();

	if (_controlPanel && _controlPanel->_numHotspots) {
		Common::Point p = _hitAreaHelper->getClosestHotspot(event1.mouse.x, event1.mouse.y)._displayPoint;

		if (p != _prevHotspot) {
			_prevHotspot = p;

			_controlPanel->setMouseCursor(_controlPanel->_currentAction, (p.x != 0 || p.y != 0));
		}
	}

	if (!_controlPanel || !_active)
		return false;

	_controlPanel->_eventProcessed = false;

	g_gui.processEvent(event1, _controlPanel);

	if (_gameInPostcard) {
		int16 gameX = event1.mouse.x;
		int16 gameY = event1.mouse.y;

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

		if (postcardClick != POSTCARD_CLICK_NONE) {
			warning("postcardClick %d", postcardClick);

			// Post a click to the "continue" button to cancel the postcard
			pushClickEvent(POSTCARD_CONTINUE_CLICK_X, POSTCARD_CONTINUE_CLICK_Y);

			// Handle the selected option
			// TODO

			return false; //true;
		}

		return false;
	}

	return _controlPanel->_eventProcessed;
}

void SOverlay::reflowLayout() {
	if (_controlPanel)
		_controlPanel->reflowLayout();
}

void SOverlay::setEngine(Engine *engine) {
	if (_controlPanel)
		_controlPanel->setEngine(engine);
}

void SOverlay::setMouseVisibility(bool state) {
	if (_mouseVisible == state)
		return;

	_mouseVisible = state;

	reflowLayout();
}

void SOverlay::onFastFadeInStarted() {
	// If we reach the fast fade-in that happens when loading (after we loaded), we end auto-load state.
	warning("onFastFadeInStarted()");
#if 0
	if (mAutoLoadSlot == -1) {
		mAutoLoadState = false;
	}
#endif
}

bool SOverlay::isInAutoloadState() {
	// If we are auto-loading, we disable movie fade-out, graphics, etc.

	return false; // STUB
#if 0
	return mAutoLoadState;
#endif
}
void SOverlay::onActionClicked(uint16 action) {
//	addBigActionFadeAnimation(getActionIcon(action));
}

void SOverlay::onGameIdleCounter() {
	// Mark a flag if we got an "idle" count while saving
#if 0
	if (mSaveInProgress) {
		mGameIdleCountWhileSaving = true;
	}
#endif
}

void SOverlay::onActionChanged(uint16 action) {
	_controlPanel->_currentAction = action;

	_controlPanel->setMouseCursor(action);
}

void SOverlay::checkGameInChat(Graphics::Surface *gameSurface) {

	bool gameInChat;

	if (!_mouseVisible || _bottomToolbarAppearing) {
		gameInChat = false;
	} else {
		//
		// check if the game is in chat mode by checking if a certain line has only black pixels
		//

		// Get the row pointer
		byte* rowPtr = (byte *)gameSurface->getPixels()
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
	if (_gameInChat == true && gameInChat == false) {
		pushScrollEvent(0, 0);
	}

	// Check if we got into chat mode - if so, set the default line selection to 1
	if (_gameInChat == false && gameInChat == true) {
		// Get all the chat hotspots
		Common::Point points[10];
		uint16 count = _hitAreaHelper->getAllChatHotspots(points, 10);

		if (count > 0) {
			Common::Point first = points[0];
			pushScrollEvent(first.x, first.y);

			_selectedChatRow = 1;
		}

	}

	_gameInChat = gameInChat;
}

void SOverlay::checkGameInPostcard(Graphics::Surface *gameSurface) {
//
// check if the game is in the postcard screen by querying certain pixels
//

// Get the row pointer
	byte* rowPtr = (byte *)gameSurface->getPixels()
			+ POSTCARD_WINDOW_CHECK_Y * gameSurface->pitch;

// Check for a certain pixel pattern
	if (getGameType() == GAME_TYPE_SIMON1) {
		if (rowPtr[65] == 228 && rowPtr[75] == 243 && rowPtr[79] == 254
				&& rowPtr[91] == 227 && rowPtr[133] == 225 && rowPtr[200] == 225
				&& rowPtr[254] == 228) {
			_gameInPostcard = true;
		} else {
			_gameInPostcard = false;
		}
	} else if (getGameType() == GAME_TYPE_SIMON2) {
		if (rowPtr[65] == 0xEB && rowPtr[67] == 0xDB && rowPtr[100] == 0xDB
				&& rowPtr[126] == 0xEB && rowPtr[200] == 0xE4
				&& rowPtr[239] == 0xE4 && rowPtr[253] == 0xDB) {
			_gameInPostcard = true;
		} else {
			_gameInPostcard = false;
		}
	}
}

void SOverlay::checkBottomToolbar(Graphics::Surface *gameSurface) {
//
// check if the game has inventory by querying certain pixels
//

// Get the row pointer
	byte* rowPtr = (byte *)gameSurface->getPixels()
			+ BOTTOM_TOOLBAR_CHECK_Y * gameSurface->pitch;

// Check for a certain pixel pattern

#if CURRENT_GAME_TYPE == GAME_TYPE_SIMON1
	if (rowPtr[1] == 240 && rowPtr[25] == 247 && rowPtr[50] == 240
			&& rowPtr[75] == 247 && rowPtr[100] == 250 && rowPtr[125] == 240) {
		_bottomToolbarAppearing = true;
	} else {
		_bottomToolbarAppearing = false;
	}
#endif

#if CURRENT_GAME_TYPE == GAME_TYPE_SIMON2

	if (rowPtr[0] == 0xfa && rowPtr[17] == 0xf8 && rowPtr[69] == 0xf9
			&& rowPtr[123] == 0xf7 && rowPtr[188] == 0xf8
			&& rowPtr[254] == 0xf7) {
		_bottomToolbarAppearing = true;
	} else {
		_bottomToolbarAppearing = false;
	}
#endif
}

/**
 * Using GAME coordinates
 */
void SOverlay::pushScrollEvent(int x, int y) {
	Common::Event e;
	e.type = Common::EVENT_MOUSEMOVE;
	e.mouse.x = x;
	e.mouse.y = y;

	g_system->getEventManager()->pushEvent(e);
}

void SOverlay::pushClickEvent(int32 x, int32 y) {
	Common::Event e;
	e.type = Common::EVENT_MOUSEMOVE;

	e.mouse.x = x;
	e.mouse.y = y;

	g_system->getEventManager()->pushEvent(e);

	e.type = Common::EVENT_LBUTTONDOWN;
	g_system->getEventManager()->pushEvent(e);

	e.type = Common::EVENT_LBUTTONUP;
	g_system->getEventManager()->pushEvent(e);
}

void SOverlay::chatArrowClick(bool up) {
	// Get all the chat hotspots
	Common::Point points[10];
	uint16 count = _hitAreaHelper->getAllChatHotspots(points, 10);

	// Adjust the selected row number.
	// Chat hit areas are ordered from bottom to top.
	if (up) {
		--_selectedChatRow;
		if (_selectedChatRow <= 0) {
			_selectedChatRow = count;
		}
	} else {
		++_selectedChatRow;
		if (_selectedChatRow > count) {
			_selectedChatRow = 1;
		}
	}

	// Scroll the game mouse to the selected row
	Common::Point selected = points[_selectedChatRow - 1];

	pushScrollEvent(selected.x, selected.y);
}

void SOverlay::talkButtonClick() {
	// Get all the chat hotspots
	Common::Point points[10];
	uint16 count = _hitAreaHelper->getAllChatHotspots(points, 10);

	// Click the selected row
	Common::Point selected;
	if (_selectedChatRow <= 0 || _selectedChatRow > count) {
		selected = points[0];
	} else {
		selected = points[_selectedChatRow - 1];
	}

	pushClickEvent(selected.x, selected.y);

	_selectedChatRow = 0;
}

void SOverlay::revealItems() {
	_controlPanel->createHotspots(_hitAreaHelper);
}


#pragma mark --------- SDialog ---------

#define GAME_SCREEN_WIDTH 320
#define GAME_SCREEN_HEIGHT 200

#define BLACK_PANEL_START_Y 134
#define BLACK_PANEL_END_Y 144

#define BLACK_PANEL_HEIGHT (BLACK_PANEL_END_Y - BLACK_PANEL_START_Y)

#define SKIP_X 0
#define SKIP_Y 0
#define SKIP_W 0.09

#define REVEAL_ITEMS_X 0.91
#define REVEAL_ITEMS_Y 0
#define REVEAL_ITEMS_W 0.09

#define CONTROL_MODE_X 0
#define CONTROL_MODE_Y 0
#define CONTROL_MODE_W 0.09

#define MENU_X 0
#define MENU_Y 0
#define MENU_W 0.1

#define FRAME_W 0.1

#define BIG_ACTION_ICON_X 0.425
#define BIG_ACTION_ICON_Y 0.3
#define BIG_ACTION_ICON_W 0.15

#define SMALL_ACTION_ICON_W 		0.08
#define SMALL_ACTION_ICON_PADDING 		14

#define ARROW_W 0.1
#define ARROW_H 0.14

#define UP_ARROW_X (1 - ARROW_W - 0.01)
#define UP_ARROW_Y 0.37
#define DOWN_ARROW_X (1 - ARROW_W - 0.01)
#define DOWN_ARROW_Y (UP_ARROW_Y + ARROW_H + 0.01)

#define CHAT_BUTTON_W (ARROW_W * 1.6)
#define CHAT_BUTTON_H (ARROW_H * 2 + 0.01)
#define CHAT_BUTTON_X (UP_ARROW_X - CHAT_BUTTON_W - 0.01)
#define CHAT_BUTTON_Y UP_ARROW_Y

#define TOUCH_INDICATOR_W 0.035
#define TOUCH_INDICATOR_MIN_SCALE_W 0.04
#define TOUCH_INDICATOR_MAX_SCALE_W 0.05
#define TOUCH_INDICATOR_SCALE_DURATION 750
#define TOUCH_INDICATOR_INITIAL_FADE_DURATION 100

#define MUSIC_ENHANCED_BY_X 0.1
#define MUSIC_ENHANCED_BY_Y 0.7
#define MUSIC_ENHANCED_BY_W 0.8
#define MUSIC_ENHANCED_BY_BEGIN_TIME 3000
#define MUSIC_ENHANCED_BY_END_TIME 8000

#define ACTION_BITMAP_W 90
#define ACTION_BITMAP_H 90

#define CURSOR_W 0.07
#define CURSOR_BULGE 1.3

enum {
	kMenuCmd = 'MENU',
	kRevealCmd = 'REVL',
	kSkipCmd = 'SKIP',
	kUpCmd = 'UP  ',
	kDownCmd = 'DOWN',
	kTalkCmd = 'TALK',
	kSpotCmd = 'SPOT',
	kActCmd  = 'ACT '
};

SDialog::SDialog() : Dialog(0, 0, 320, 200) {
	_x = _y = 0;

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundNone;

	_menuButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kMenuCmd, 0);
	_menuButton->setButtonDisplay(false);
	_menuButton->setAGfx(g_gui.theme()->getAImageSurface("menu.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);
	_menuButton->setVisible(false);

	_revealButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kRevealCmd, 0);
	_revealButton->setButtonDisplay(false);
	_revealButton->setAGfx(g_gui.theme()->getAImageSurface("reveal_items.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);
	_revealButton->setVisible(false);

	_skipButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kSkipCmd, 0);
	_skipButton->setButtonDisplay(false);
	_skipButton->setAGfx(g_gui.theme()->getAImageSurface("skip.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);
	_skipButton->setVisible(false);

	_arrowUpButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kUpCmd, 0);
	_arrowUpButton->setButtonDisplay(false);
	_arrowUpButton->setAGfx(g_gui.theme()->getAImageSurface("arrow_up.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);
	_arrowUpButton->setVisible(false);

	_arrowDownButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kDownCmd, 0);
	_arrowDownButton->setButtonDisplay(false);
	_arrowDownButton->setAGfx(g_gui.theme()->getAImageSurface("arrow_down.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);
	_arrowDownButton->setVisible(false);

	_talkButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kTalkCmd, 0);
	_talkButton->setButtonDisplay(false);
	_talkButton->setAGfx(g_gui.theme()->getAImageSurface("talk_btn.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);
	_talkButton->setVisible(false);


	_eventProcessed = false;

	_mainMenuDialog = 0;

	_engine = 0;

	_hotspotsOn = false;
	_numHotspots = 0;
	_hotspotAlpha = 0;
	_hotspotState = 0;
	_hotspotCountdown = 0;

	for (int i = 0; i < kMaxHotspots; i++)
		_hotspotButtons[i] = 0;

	_actionIcon = 0;

	_currentAction = ACTION_WALK;

	reflowLayout();
}

void SDialog::handleMouseDown(int x, int y, int button, int clickCount) {
	_eventProcessed = _menuButton->isPointIn(x, y) || _revealButton->isPointIn(x, y);
}

void SDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	_eventProcessed = false;

	switch (cmd) {
	case kMenuCmd:
		if (!_mainMenuDialog)
			_mainMenuDialog = new MainMenuDialog(_engine);

		g_sOverlay.setActive(false);
		_mainMenuDialog->runModal();
		g_sOverlay.setActive(true);

		_eventProcessed = true;

		break;
	case kRevealCmd:

		g_sOverlay.revealItems();

		_eventProcessed = true;

		break;
	case kSkipCmd:

		performSkip();

		_eventProcessed = true;

		break;
	case kUpCmd:

		g_sOverlay.chatArrowClick(true);

		_eventProcessed = true;

		break;
	case kDownCmd:

		g_sOverlay.chatArrowClick(false);

		_eventProcessed = true;

		break;
	case kTalkCmd:

		g_sOverlay.talkButtonClick();

		_eventProcessed = true;

		break;
	case kSpotCmd: {
			warning("spot");

			createActionIcon(_mouseWidget->getAbsX(), _mouseWidget->getAbsY(), _currentAction);
		}

		break;

	case kActCmd:
		// Do nothing
		break;

	default:
		warning("uknown command: %s", tag2str(cmd));
	}
}

void SDialog::reflowLayout() {
	int ow = g_system->getOverlayWidth();
	int oh = g_system->getOverlayHeight();

	_w = ow;
	_h = oh;

	if (canSkip()) {
		_skipButton->resize(SKIP_X * ow, SKIP_Y * oh, SKIP_W * ow, SKIP_W * ow);
		_skipButton->setVisible(true);
	} else {
		_skipButton->setVisible(false);
		_skipButton->resize(0, 0, 1, 1);
	}

	_menuButton->resize(MENU_X * ow, MENU_Y * oh, MENU_W * ow, MENU_W * ow);
	_menuButton->setVisible(canShowMenuButton());

	_revealButton->resize(REVEAL_ITEMS_X * ow, REVEAL_ITEMS_Y * oh, REVEAL_ITEMS_W * ow, REVEAL_ITEMS_W * ow);
	_revealButton->setVisible(canShowRevealItems());

	// Show the chat overlay if needed
	if (canShowChatControls()) {
		_arrowUpButton->resize(UP_ARROW_X * ow, UP_ARROW_Y * oh, ARROW_W * ow, ARROW_H * oh);
		_arrowDownButton->resize(DOWN_ARROW_X * ow, DOWN_ARROW_Y * oh, ARROW_W * ow, ARROW_H * oh);
		_talkButton->resize(CHAT_BUTTON_X * ow, CHAT_BUTTON_Y * oh, CHAT_BUTTON_W * ow, CHAT_BUTTON_H * oh);
	} else {
		_arrowUpButton->resize(0, 0, 1, 1);
		_arrowDownButton->resize(0, 0, 1, 1);
		_talkButton->resize(0, 0, 1, 1);
	}

	_arrowUpButton->setVisible(canShowChatControls());
	_arrowDownButton->setVisible(canShowChatControls());
	_talkButton->setVisible(canShowChatControls());

	reflowHotspots();

	setMouseCursor(_currentAction);

	//GuiObject::reflowLayout();
}

void SDialog::close() {
	Dialog::close();
}

bool SDialog::canSkip() {
	if (_engine == NULL) {
		return false;
	}

	return (!g_sOverlay._mouseVisible && _engine->canSkip());
}

bool SDialog::canShowRevealItems() {
	return g_sOverlay._bottomToolbarAppearing && g_sOverlay._mouseVisible;
}

bool SDialog::canShowMenuButton() {
	return (g_sOverlay._bottomToolbarAppearing || g_sOverlay._gameInChat)
			&& g_sOverlay._mouseVisible;
}

bool SDialog::canShowChatControls() {
	return g_sOverlay._mouseVisible && g_sOverlay._gameInChat && !g_sOverlay._classicMode;
}

uint16 SDialog::getCurrentAction() {
	if (_engine == NULL) {
		return ACTION_WALK;
	}

	return _engine->getCurrentActionId();
}

void SDialog::createHotspots(HitAreaHelper *hitAreaHelper) {
	if (_hotspotsOn)
		return;

	_hotspotsOn = true;

	// Obtain hotspots
	_numHotspots = hitAreaHelper->getAllInteractionHotspots(_hotspots, kMaxHotspots);

	_hotspotState = 0;
	_hotspotCountdown = 0;

	for (uint i = 0; i < _numHotspots; i++) {
		_hotspotButtons[i] = new PicButtonWidget(this, 0, 0, 10, 10, 0, kSpotCmd, 0);
		_hotspotButtons[i]->setButtonDisplay(false);
		_hotspotButtons[i]->setAGfx(g_gui.theme()->getAImageSurface("touch_indicator.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);
	}

	reflowHotspots();
}

void SDialog::reflowHotspots() {
	int ow = g_system->getOverlayWidth();
	int oh = g_system->getOverlayHeight();

	for (uint i = 0; i < _numHotspots; i++) {
		_hotspotButtons[i]->resize(_hotspots[i].x * ow / 320, _hotspots[i].y * oh / 200, TOUCH_INDICATOR_W * ow, TOUCH_INDICATOR_W * ow);
	}

	if (_actionIcon) {
		// Draw action icon
		float indicatorHeight = TOUCH_INDICATOR_W * ACTION_BITMAP_H / ACTION_BITMAP_W * ow / oh;
		float hotspotPositionY =
			((_actionY + BLACK_PANEL_HEIGHT) / (float) GAME_SCREEN_HEIGHT) - indicatorHeight / 2;

		float actionHeight = SMALL_ACTION_ICON_W * (float)ACTION_BITMAP_H / (float)ACTION_BITMAP_W;
		float ax = _actionX - (SMALL_ACTION_ICON_W / 2) * GAME_SCREEN_WIDTH;

		float ay = _actionY - (0.01 - actionHeight) * GAME_SCREEN_HEIGHT;
		if (ay < (BLACK_PANEL_HEIGHT / (float) GAME_SCREEN_HEIGHT)) {
			// If we're on the upper part, switch Y value to be below the indicator
			ay = hotspotPositionY + (indicatorHeight + 0.06) * GAME_SCREEN_HEIGHT;
		}

		_actionIcon->resize(ax, ay, SMALL_ACTION_ICON_W * ow, actionHeight * oh);
	}
}

static const struct Cursor {
	int id;
	const char *action;
	const char *cursor;
	int hX;
	int hY;
} cursors[] = {
	{ ACTION_WALK,    "action_walk.png",    "action_walk-cursor.png", 0, 0 },
	{ ACTION_LOOK,    "action_look.png",    "action_look-cursor.png", 0, 0 },
	{ ACTION_OPEN,    "action_open.png",    "action_open-cursor.png", 0, 0 },
	{ ACTION_MOVE,    "action_move.png",    "action_move-cursor.png", 0, 0 },
	{ ACTION_CONSUME, "action_consume.png", "action_consume-cursor.png", 0, 0 },
	{ ACTION_PICK,    "action_pick.png",    "action_pick-cursor.png", 0, 0 },
	{ ACTION_CLOSE,   "action_close.png",   "action_close-cursor.png", 0, 0 },
	{ ACTION_USE,     "action_use.png",     "action_use-cursor.png", 0, 0 },
	{ ACTION_TALK,    "action_talk.png",    "action_talk-cursor.png", 0, 0 },
	{ ACTION_REMOVE,  "action_remove.png",  "action_remove-cursor.png", 0, 0 },
	{ ACTION_WEAR,    "action_wear.png",    "action_wear-cursor.png",    0, 0 },
	{ ACTION_GIVE,    "action_give.png",    "action_give-cursor.png", 0, 0 },
	{ -1, "", "", 0, 0 }
};

const Cursor *findCursor(int id) {
	int n = 0;

	while (cursors[n].id != -1) {
		if (cursors[n].id == id)
			return &cursors[n];
		n++;
	}

	return &cursors[n]; // Null value
}

static const char *getActionIcon(uint16 action) {
	return findCursor(action)->action;
}

void SDialog::setMouseCursor(int action, bool bulge) {
	const Cursor *c = findCursor(action);

	if (c->id == -1)
		return;

	int cursorW = (int)((float)g_system->getOverlayWidth() * CURSOR_W * (bulge ? CURSOR_BULGE : 1.0));
	const Graphics::TransparentSurface *surf = g_gui.theme()->getAImageSurface(c->cursor)->scale(cursorW, cursorW);

	g_system->setMouseCursor(surf->getPixels(), surf->w, surf->h, c->hX, c->hY, 0, true, &surf->format);
}

void SDialog::createActionIcon(int x, int y, int action) {
	if (_actionIcon)
		removeWidget(_actionIcon);

	_actionIcon = new PicButtonWidget(this, 0, 0, 10, 10, 0, kActCmd, 0);
	_actionIcon->setButtonDisplay(false);
	_actionIcon->setAGfx(g_gui.theme()->getAImageSurface(getActionIcon(action)), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);

	_actionX = x;
	_actionY = y;

	reflowHotspots();
}

void SDialog::updateHotspots() {
	if (!_hotspotsOn)
		return;

	_hotspotCountdown++;

	byte oldAlpha = _hotspotAlpha;

	switch(_hotspotState) {
	case 0: // fade in
		_hotspotAlpha = _hotspotCountdown ? _hotspotCountdown * 32 - 1 : 0;

		if (_hotspotCountdown > 8) {
			_hotspotCountdown = 0;
			_hotspotState++;

			_hotspotAlpha = 255;
		}
		break;
	case 1: // display
		if (_hotspotCountdown > 200) {
			_hotspotCountdown = 0;
			_hotspotState++;
		}
		break;
	case 2: // fade out
		if (_hotspotCountdown > 64) {
			_hotspotAlpha = 0;
			_hotspotCountdown = 0;
			_hotspotState++;
			break;
		}

		_hotspotAlpha = 255 - (_hotspotAlpha ? _hotspotCountdown * 4 - 1 : 0);

		break;
	case 3: // destroy
		for (uint i = 0; i < _numHotspots; i++)
			removeWidget(_hotspotButtons[i]);

		if (_actionIcon)
			removeWidget(_actionIcon);

		_hotspotAlpha = 0;
		_numHotspots = 0;
		_hotspotsOn = false;
		_hotspotState++;
		break;
	case 4:
		return;
	}

	if (oldAlpha != _hotspotAlpha)
		for (uint i = 0; i < _numHotspots; i++)
			_hotspotButtons[i]->useAlpha(_hotspotAlpha);
}

void SDialog::performSkip(bool showAnimation) {
	if (showAnimation) {
		//addBigActionFadeAnimation(getBitmap("skip.png"));
	}

	// Push the "esc" key event to the system
	Common::Event e1, e2, e3, e4;

	e1.kbd.keycode = Common::KEYCODE_ESCAPE;
	e1.kbd.ascii = Common::ASCII_ESCAPE;
	e1.type = Common::EVENT_KEYDOWN;
	g_system->getEventManager()->pushEvent(e1);

	e2.kbd.keycode = Common::KEYCODE_ESCAPE;
	e2.kbd.ascii = Common::ASCII_ESCAPE;
	e2.type = Common::EVENT_KEYUP;
	g_system->getEventManager()->pushEvent(e2);

	// Push a right click event to the system
	e3.type = Common::EVENT_RBUTTONDOWN;
	g_system->getEventManager()->pushEvent(e3);

	e4.type = Common::EVENT_RBUTTONUP;
	g_system->getEventManager()->pushEvent(e4);
}


#pragma mark --------- HitAreaHelper ---------

#define HIT_AREA_MAX 100
#define DISTANCE_THRESHOLD 50

HitAreaHelper::HitAreaHelper()
		: _interactionHitAreaCount(0) {
	_interactionHitAreas = new Common::Rect[HIT_AREA_MAX];
	_chatHitAreas = new Common::Rect[HIT_AREA_MAX];
}

HitAreaHelper::~HitAreaHelper() {
	delete[] _interactionHitAreas;
	delete[] _chatHitAreas;
}

uint16 HitAreaHelper::getAllChatHotspots(Common::Point *hotspots, uint16 max) {
	updateChatHitAreas();

	int maxCount = MIN(_chatHitAreaCount, max);

	for (int i = 0; i < maxCount; ++i) {
		hotspots[i].x = (_chatHitAreas[i].right + _chatHitAreas[i].left) / 2;
		hotspots[i].y = (_chatHitAreas[i].bottom + _chatHitAreas[i].top) / 2;
	}

	return maxCount;
}

uint16 HitAreaHelper::getAllInteractionHotspots(Common::Point *hotspots, uint16 max) {
	updateInteractionHitAreas();

	int maxCount = MIN(_interactionHitAreaCount, max);

	for (int i = 0; i < maxCount; ++i) {
		hotspots[i].x = (_interactionHitAreas[i].right + _interactionHitAreas[i].left) / 2;
		hotspots[i].y = (_interactionHitAreas[i].bottom + _interactionHitAreas[i].top) / 2;
	}

	return maxCount;
}

Hotspot HitAreaHelper::getClosestHotspot(int x, int y) {
	updateInteractionHitAreas();

	Rect *resultRect = NULL;

	// First, check if we're inside one of the hit areas
	for (int i = 0; i < _interactionHitAreaCount; ++i) {
		if (x >= _interactionHitAreas[i].left && y >= _interactionHitAreas[i].top
				&& x <= _interactionHitAreas[i].right && y <= _interactionHitAreas[i].bottom) {
			resultRect = _interactionHitAreas + i;
		}
	}

	if (resultRect == NULL) {
		// If not, check for the smallest distance that is below the threshold
		int closestDistanceSquare = DISTANCE_THRESHOLD * DISTANCE_THRESHOLD;

		for (int i = 0; i < _interactionHitAreaCount; ++i) {
			int centerX = (_interactionHitAreas[i].right + _interactionHitAreas[i].left) / 2;
			int centerY = (_interactionHitAreas[i].bottom + _interactionHitAreas[i].top) / 2;
			int dx = abs(x - centerX);
			int dy = abs(y - centerY);
			int distanceSquare = dx * dx + dy * dy;

			if (distanceSquare <= closestDistanceSquare) {
				resultRect = _interactionHitAreas + i;
				closestDistanceSquare = distanceSquare;
			}
		}
	}

	Hotspot resultHotspot;

	// Return the middle point, a corner (if there's a clash in the middle) or 0 point
	if (resultRect != NULL) {
		// Middle
		resultHotspot._displayPoint = Common::Point(
				(resultRect->left + resultRect->right) / 2,
				(resultRect->top + resultRect->bottom) / 2);
		resultHotspot._cursorPoint = resultHotspot._displayPoint;

		if (isPointIsolated(resultHotspot._cursorPoint, resultRect)) {
			return resultHotspot;
		}

		// top left
		resultHotspot._cursorPoint = Common::Point(resultRect->left + 1,
				resultRect->top + 1);

		if (isPointIsolated(resultHotspot._cursorPoint, resultRect)) {
			return resultHotspot;
		}

		// top right
		resultHotspot._cursorPoint = Common::Point(resultRect->right - 1,
				resultRect->top + 1);

		if (isPointIsolated(resultHotspot._cursorPoint, resultRect)) {
			return resultHotspot;
		}

		// bottom left
		resultHotspot._cursorPoint = Common::Point(resultRect->left + 1,
				resultRect->bottom - 1);

		if (isPointIsolated(resultHotspot._cursorPoint, resultRect)) {
			return resultHotspot;
		}

		// bottom right
		resultHotspot._cursorPoint = Common::Point(resultRect->right - 1,
				resultRect->bottom - 1);

		if (isPointIsolated(resultHotspot._cursorPoint, resultRect)) {
			return resultHotspot;
		}
		// Use middle anyway
		resultHotspot._cursorPoint = resultHotspot._displayPoint;

		return resultHotspot;
	} else {
		resultHotspot._cursorPoint = Common::Point();
		resultHotspot._displayPoint = Common::Point();
		return resultHotspot;
	}
}

void HitAreaHelper::updateInteractionHitAreas() {
	g_engine->getInteractionHitAreas(_interactionHitAreas, _interactionHitAreaCount);
}

void HitAreaHelper::updateChatHitAreas() {
	g_engine->getChatHitAreas(_chatHitAreas, _chatHitAreaCount);
}

bool HitAreaHelper::isPointIsolated(Common::Point p, Rect *original) {
	// Check if the point is inside another hit area beside the original
	for (int i = 0; i < _interactionHitAreaCount; ++i) {
		if (_interactionHitAreas + i != original) {
			if (p.x >= _interactionHitAreas[i].left && p.y >= _interactionHitAreas[i].top
					&& p.x <= _interactionHitAreas[i].right
					&& p.y <= _interactionHitAreas[i].bottom) {

				// Clash detected
				return false;
			}
		}
	}

	return true;
}

} // End of namespace GUI
