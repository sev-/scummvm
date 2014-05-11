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

class Simon1Dialog : public Dialog {
public:
	Simon1Dialog() : Dialog(0, 0, 320, 200) {}

protected:
	virtual void reflowLayout();
	void setSize();
};

class MainMenuDialog : public Simon1Dialog {
public:
	MainMenuDialog();
	virtual ~MainMenuDialog() {}

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	void open();
};

class SettingsDialog : public Simon1Dialog {
public:
	SettingsDialog();
	virtual ~SettingsDialog() {}

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
};

class VoiceDialog : public Simon1Dialog {
public:
	VoiceDialog();
	virtual ~VoiceDialog() {}

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	void close();
	int getSubtitleMode(bool subtitles, bool speech_mute);

	RadiobuttonGroup *_subToggleGroup;
};

class MusicDialog : public Simon1Dialog {
public:
	MusicDialog();
	virtual ~MusicDialog() {}

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	void close();

	RadiobuttonGroup *_musicToggleGroup;
};

} // End of namespace GUI

#endif
