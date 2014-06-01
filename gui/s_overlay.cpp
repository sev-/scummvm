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


SOverlay::SOverlay() {
	_initialized = false;
	_active = true;

	_controlPanel = 0;

	_mouseVisible = false;
	_gameInChat = false;
	_gameInPostcard = false;
	_bottomToolbarAppearing = false;

	_selectedChatRow = 0;

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
		_controlPanel->drawDialog();
		g_gui.theme()->finishBuffering();
		g_gui.theme()->updateScreen();
   }
}

void SOverlay::beforeDrawTextureToScreen(Graphics::Surface* gameSurface) {
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
	if (!_controlPanel || !_active)
		return false;

	_controlPanel->_eventProcessed = false;

	Common::Event event1 = event;

	event1.mouse.x = event.mouse.x * g_system->getOverlayWidth() / g_system->getWidth();
	event1.mouse.y = event.mouse.y * g_system->getOverlayHeight() / g_system->getHeight();

	g_gui.processEvent(event1, _controlPanel);

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
	_mouseVisible = state;

	reflowLayout();
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
		uint16 count = 0; //mHitAreaHelper.getAllChatHotspots(points, 10);

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

	//g_system->forceEvent(e);
}


#pragma mark --------- SDialog ---------

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

enum {
	kMenuCmd = 'MENU',
	kRevealCmd = 'REVL'
};

SDialog::SDialog() : Dialog(0, 0, 320, 200) {
	_x = _y = 0;

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundNone;
	_menuButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kMenuCmd, 0);
	_menuButton->setButtonDisplay(false);
	_menuButton->setAGfx(g_gui.theme()->getAImageSurface("menu.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);

	_revealButton = new PicButtonWidget(this, 0, 0, 10, 10, 0, kRevealCmd, 0);
	_revealButton->setButtonDisplay(false);
	_revealButton->setAGfx(g_gui.theme()->getAImageSurface("reveal_items.png"), kPicButtonStateEnabled, ThemeEngine::kAutoScaleFit);

	reflowLayout();

	_eventProcessed = false;

	_mainMenuDialog = 0;

	_engine = 0;
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

		warning("reveal");

		_eventProcessed = true;

		break;
	default:
		warning("uknown command: %x", cmd);
	}
}

void SDialog::reflowLayout() {
	int ow = g_system->getOverlayWidth();
	int oh = g_system->getOverlayHeight();

	_w = ow;
	_h = oh;

	_menuButton->resize((int)(MENU_X * ow), (int)(MENU_Y * oh), (int)(MENU_W * ow), (int)(MENU_W * ow));
	_menuButton->setVisible(canShowMenuButton());

	_revealButton->resize(REVEAL_ITEMS_X * ow, REVEAL_ITEMS_Y * oh, REVEAL_ITEMS_W * ow, REVEAL_ITEMS_W * ow);
	_revealButton->setVisible(canShowRevealItems());

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

uint16 SDialog::getCurrentAction() {
	if (_engine == NULL) {
		return ACTION_WALK;
	}

	return _engine->getCurrentActionId();
}

} // End of namespace GUI

