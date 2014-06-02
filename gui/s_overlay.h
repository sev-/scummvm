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

#ifndef GUI_S_OVERLAY_H
#define GUI_S_OVERLAY_H

#define GAME_TYPE_SIMON1 0
#define GAME_TYPE_SIMON2 1

#define CURRENT_GAME_TYPE GAME_TYPE_SIMON1

#include "common/system.h"

#include "common/singleton.h"

#include "gui/dialog.h"
#include "gui/widget.h"

#include "common/events.h"

#include "engines/engine.h"

#define g_sOverlay (GUI::SOverlay::instance())

namespace GUI {

/**
 * Simon 1 action ids
 */
#define ACTION_WALK 101
#define ACTION_LOOK 102
#define ACTION_OPEN 103
#define ACTION_MOVE 104
#define ACTION_CONSUME 105
#define ACTION_PICK 106
#define ACTION_CLOSE 107
#define ACTION_USE 108
#define ACTION_TALK 109
#define ACTION_REMOVE 110
#define ACTION_WEAR 111
#define ACTION_GIVE 112

class HitAreaHelper;
class SDialog;

class SOverlay : public Common::Singleton<SOverlay>, public Common::EventObserver {
	friend class Common::Singleton<SingletonBaseType>;
	SOverlay();
	~SOverlay();

public:

	/** Hooks for intercepting into GUI processing, so required events could be shoot
	 *  or filtered out */
	void preDrawOverlayGui();
	void postDrawOverlayGui();

	void init();

	virtual bool notifyEvent(const Common::Event &event);

	void reflowLayout();

	void setEngine(Engine *engine);
	void setActive(bool active) { _active = active; }
	void setMouseVisibility(bool state);
	void beforeDrawTextureToScreen(Graphics::Surface *gameSurface);

	int getGameType() { return CURRENT_GAME_TYPE; }

private:
	SDialog *_controlPanel;
	bool _initialized;
	bool _active;
	HitAreaHelper *_hitAreaHelper;

	void checkBottomToolbar(Graphics::Surface *gameSurface);
	void checkGameInChat(Graphics::Surface *gameSurface);
	void checkGameInPostcard(Graphics::Surface *gameSurface);
	void pushScrollEvent(int x, int y);

public:
	bool _mouseVisible;
	bool _gameInChat;
	bool _bottomToolbarAppearing;
	bool _gameInPostcard;
	bool _classicMode;

	int _selectedChatRow;
};

class SDialog : public Dialog {
public:
	SDialog();
	virtual ~SDialog() {}
	virtual void close();
	virtual void reflowLayout();

	virtual bool isVisible() const { return true; }

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleMouseDown(int x, int y, int button, int clickCount);

	void setEngine(Engine *engine) { _engine = engine; }

	bool canSkip();
	bool canShowRevealItems();
	bool canShowMenuButton();
	bool canShowChatControls();

	uint16 getCurrentAction();

	bool _eventProcessed;

	Dialog *_mainMenuDialog;

private:
	GUI::PicButtonWidget *_menuButton;
	GUI::PicButtonWidget *_revealButton;
	GUI::PicButtonWidget *_skipButton;
	GUI::PicButtonWidget *_arrowUpButton;
	GUI::PicButtonWidget *_arrowDownButton;
	GUI::PicButtonWidget *_talkButton;

	Engine *_engine;
};

struct Hotspot {
	Common::Point _displayPoint;
	Common::Point _cursorPoint;

	Hotspot() {}

	Hotspot(Common::Point display, Common::Point cursor)
			: _displayPoint(display), _cursorPoint(cursor) {
	}

	void clear() {
		_displayPoint = Common::Point();
		_cursorPoint = Common::Point();
	}

};

class HitAreaHelper {
public:
	HitAreaHelper();
	virtual ~HitAreaHelper();

	/**
	 * Returns the closest hit area to a game coordinate, according to a defined distance threshold
	 */
	Hotspot getClosestHotspot(int x, int y);

	uint16 getAllInteractionHotspots(Common::Point *hotspots, uint16 max);

	uint16 getAllChatHotspots(Common::Point *hotspots, uint16 max);

private:

	bool isPointIsolated(Common::Point p, Common::Rect *original);

	void updateInteractionHitAreas();
	void updateChatHitAreas();


	Common::Rect *_interactionHitAreas;
	uint16 _interactionHitAreaCount;

	Common::Rect *_chatHitAreas;
	uint16 _chatHitAreaCount;
};

} // End of namespace GUI

#endif
