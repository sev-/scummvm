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
 */

#ifndef GUI_MAINMENU_DIALOG_H
#define GUI_MAINMENU_DIALOG_H

#include "gui/dialog.h"
#include "gui/widget.h"
#include "engines/game.h"

namespace GUI {

class MainMenuDialog : public Dialog {
public:
	MainMenuDialog();
	virtual ~MainMenuDialog() {}

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	virtual void reflowLayout();

	void open();
};

class SettingsDialog : public Dialog {
public:
	SettingsDialog();
	virtual ~SettingsDialog() {}

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	virtual void reflowLayout();
};

class VoiceDialog : public Dialog {
public:
	VoiceDialog();
	virtual ~VoiceDialog() {}

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	virtual void reflowLayout();

	RadiobuttonGroup *_subToggleGroup;

	RadiobuttonWidget *_subToggleSubBoth;
	RadiobuttonWidget *_subToggleSpeechOnly;
	RadiobuttonWidget *_subToggleSubOnly;
};

} // End of namespace GUI

#endif
