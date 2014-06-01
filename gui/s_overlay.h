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

#include "common/system.h"

#include "common/singleton.h"

#include "gui/dialog.h"
#include "gui/widget.h"

#include "common/events.h"


#define g_sOverlay (GUI::SOverlay::instance())

namespace GUI {

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
	virtual bool notifyPoll();

	void reflowLayout();

private:
	SDialog *_controlPanel;
	bool _initialized;
};

class SDialog : public Dialog {
public:
	SDialog();
	virtual ~SDialog() {}
	virtual void close();
	virtual void reflowLayout();

	virtual bool isVisible() const { return true; }

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	bool _eventProcessed;

private:
	GUI::PicButtonWidget *_menuButton;
	GUI::PicButtonWidget *_revealButton;
};


} // End of namespace GUI

#endif
