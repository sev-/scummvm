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

#include "base/version.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/util.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/s_mainmenu.h"
#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/options.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/list.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/popup.h"
#include "gui/ThemeEval.h"

#include "graphics/cursorman.h"

using Common::ConfigManager;

namespace GUI {

enum {
	kNewGameCmd = 'NEWG',
	kContinueCmd = 'CONT',
	kSaveGameCmd = 'SAVE',
	kSettingsCmd = 'SETN',
	kLoadGameCmd = 'LOAD',
	kTutorialCmd = 'TUTR',
	kQuitCmd = 'QUIT'
};

MainMenuDialog::MainMenuDialog()
	: Dialog(0, 0, 320, 200) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_w = screenW;
	_h = screenH;

	GraphicsWidget *sep1 = new GraphicsWidget(this, "MainMenu.sep1");
	sep1->setAGfx(g_gui.theme()->getAImageSurface("seperator.png"), ThemeEngine::kAutoScaleFit);

	new StaticTextWidget(this, "MainMenu.Title", _("20TH ANNIVERSARY EDITION"));

	GraphicsWidget *sep2 = new GraphicsWidget(this, "MainMenu.sep2");
	sep2->setAGfx(g_gui.theme()->getAImageSurface("seperator.png"), ThemeEngine::kAutoScaleFit);

	new ButtonWidget(this, "MainMenu.NewGameButton", _("~N~EW GAME"), _("Start new game"), kNewGameCmd);
	new ButtonWidget(this, "MainMenu.QuitButton", _("~Q~UIT"), _("Quit"), kQuitCmd);
	new ButtonWidget(this, "MainMenu.SettingsButton", _("S~E~TTINGS"), _("Change game settings"), kSettingsCmd);
	new ButtonWidget(this, "MainMenu.ContinueButton", _("~C~ONTINUE"), _("Continue game"), kContinueCmd);
	new ButtonWidget(this, "MainMenu.LoadGameButton", _("~L~OAD"), _("Load savegame"), kLoadGameCmd);
	new ButtonWidget(this, "MainMenu.SaveGameButton", _("~S~AVE"), _("Save game"), kSaveGameCmd);
	new ButtonWidget(this, "MainMenu.TutorialButton", _("~T~UTORIAL"), _("Game tutorial"), kTutorialCmd);
}

MainMenuDialog::~MainMenuDialog() {
}

void MainMenuDialog::open() {
	CursorMan.popAllCursors();
	Dialog::open();
}

void MainMenuDialog::close() {
	Dialog::close();
}

void MainMenuDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kQuitCmd:
		setResult(-1);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void MainMenuDialog::reflowLayout() {
	_w = g_system->getOverlayWidth();
	_h = g_system->getOverlayHeight();

	Dialog::reflowLayout();
}

} // End of namespace GUI
