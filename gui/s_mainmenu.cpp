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


#include "common/config-manager.h"
#include "common/translation.h"

#include "gui/s_mainmenu.h"
#include "gui/gui-manager.h"

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
	kQuitCmd = 'QUIT',

	kBackCmd = 'BACK',
	kSubtitleToggle = 'sttg',
	kMusicToggle = 'mstg',
	kGfxToggle = 'gftg',
	kControlsToggle = 'cntg',

	kVoiceCmd = 'VOIC',
	kMusicCmd = 'MUSC',
	kLanguageCmd = 'LANG',
	kGraphicsCmd = 'GRFX',
	kControlsCmd = 'CTRL',
	kAboutCmd = 'ABOU'
};

enum {
	kSubtitlesSpeech,
	kSubtitlesSubs,
	kSubtitlesBoth
};

enum {
	kMusicEnhanced,
	kMusicOriginal,
	kMusicNone
};

enum {
	kGfxHigh,
	kGfxMedium,
	kGfxLow,
	kGfxOriginal
};

enum {
	kControlsTouch,
	kControlsClassic
};

void Simon1Dialog::setSize() {
	_w = g_system->getOverlayWidth();
	_h = g_system->getOverlayHeight();
}

void Simon1Dialog::reflowLayout() {
	setSize();

	Dialog::reflowLayout();
}

MainMenuDialog::MainMenuDialog() {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	setSize();

	GraphicsWidget *sep1 = new GraphicsWidget(this, "MainMenu.sep1");
	sep1->setAGfx(g_gui.theme()->getAImageSurface("seperator.png"), ThemeEngine::kAutoScaleStretch);

	new StaticTextWidget(this, "MainMenu.Title", _("20TH ANNIVERSARY EDITION"));

	GraphicsWidget *sep2 = new GraphicsWidget(this, "MainMenu.sep2");
	sep2->setAGfx(g_gui.theme()->getAImageSurface("seperator.png"), ThemeEngine::kAutoScaleStretch);

	new ButtonWidget(this, "MainMenu.NewGameButton", _("~N~EW GAME"), _("Start new game"), kNewGameCmd);
	new ButtonWidget(this, "MainMenu.QuitButton", _("~Q~UIT"), _("Quit"), kQuitCmd);
	new ButtonWidget(this, "MainMenu.SettingsButton", _("S~E~TTINGS"), _("Change game settings"), kSettingsCmd);
	new ButtonWidget(this, "MainMenu.ContinueButton", _("~C~ONTINUE"), _("Continue game"), kContinueCmd);
	new ButtonWidget(this, "MainMenu.LoadGameButton", _("~L~OAD"), _("Load savegame"), kLoadGameCmd);
	new ButtonWidget(this, "MainMenu.SaveGameButton", _("~S~AVE"), _("Save game"), kSaveGameCmd);
	new ButtonWidget(this, "MainMenu.TutorialButton", _("~T~UTORIAL"), _("Game tutorial"), kTutorialCmd);
}

void MainMenuDialog::open() {
	CursorMan.popAllCursors();
	Dialog::open();
}

void MainMenuDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kQuitCmd:
		setResult(-1);
		close();
		break;
	case kNewGameCmd:
		break;
	case kContinueCmd:
		break;
	case kSaveGameCmd:
		break;
	case kLoadGameCmd:
		break;
	case kTutorialCmd:
		break;
	case kSettingsCmd: {
			SettingsDialog settings;
			settings.runModal();
		}
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

SettingsDialog::SettingsDialog() {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	setSize();

	GraphicsWidget *sep1 = new GraphicsWidget(this, "Settings.sep1");
	sep1->setAGfx(g_gui.theme()->getAImageSurface("seperator.png"), ThemeEngine::kAutoScaleStretch);

	new StaticTextWidget(this, "Settings.Title", _("20TH ANNIVERSARY EDITION"));

	GraphicsWidget *sep2 = new GraphicsWidget(this, "Settings.sep2");
	sep2->setAGfx(g_gui.theme()->getAImageSurface("seperator.png"), ThemeEngine::kAutoScaleStretch);

	new ButtonWidget(this, "Settings.VoiceButton", _("VOICE"), _("Select game voice"), kVoiceCmd);
	new ButtonWidget(this, "Settings.MusicButton", _("MUSIC"), _("Select game music"), kMusicCmd);
	new ButtonWidget(this, "Settings.LanguageButton", _("LANGUAGE"), _("Select game language"), kLanguageCmd);
	new ButtonWidget(this, "Settings.GraphicsButton", _("GRAPHICS"), _("Select game graphics"), kGraphicsCmd);
	new ButtonWidget(this, "Settings.ControlsButton", _("CONTROLS"), _("Select game controls"), kControlsCmd);
	new ButtonWidget(this, "Settings.AboutButton", _("ABOUT"), _("About game"), kAboutCmd);
	new ButtonWidget(this, "Settings.BackButton", _("BACK"), _("Previous menu"), kBackCmd);
}

void SettingsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kBackCmd:
		close();
		break;
	case kVoiceCmd: {
			VoiceDialog voice;
			voice.runModal();
		}
		break;
	case kMusicCmd: {
			MusicDialog music;
			music.runModal();
		}
		break;
	case kLanguageCmd:
		break;
	case kGraphicsCmd: {
			GraphicsDialog graphics;
			graphics.runModal();
		}
		break;
	case kControlsCmd: {
			ControlsDialog controls;
			controls.runModal();
		}
		break;
	case kAboutCmd:
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

VoiceDialog::VoiceDialog() {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	setSize();

	_subToggleGroup = new RadiobuttonGroup(this, kSubtitleToggle);

	new RadiobuttonWidget(this, "VoiceDialog.subToggleSubBoth", _subToggleGroup, kSubtitlesBoth, _("VOICE AND SUBTITLES"));
	new RadiobuttonWidget(this, "VoiceDialog.subToggleSpeechOnly", _subToggleGroup, kSubtitlesSpeech, _("VOICE ONLY"));
	new RadiobuttonWidget(this, "VoiceDialog.subToggleSubOnly", _subToggleGroup, kSubtitlesSubs, _("SUBTITLES ONLY"));

	new ButtonWidget(this, "VoiceDialog.BackButton", _("BACK"), _("Previous menu"), kBackCmd);

	int subMode = getSubtitleMode(ConfMan.getBool("subtitles"), ConfMan.getBool("speech_mute"));
	_subToggleGroup->setValue(subMode);

}

void VoiceDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kBackCmd:
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void VoiceDialog::close() {
	bool subtitles, speech_mute;

	switch (_subToggleGroup->getValue()) {
	case kSubtitlesSpeech:
		subtitles = speech_mute = false;
		break;
	case kSubtitlesBoth:
		subtitles = true;
		speech_mute = false;
		break;
	case kSubtitlesSubs:
	default:
		subtitles = speech_mute = true;
		break;
	}

	ConfMan.setBool("subtitles", subtitles);
	ConfMan.setBool("speech_mute", speech_mute);

	ConfMan.flushToDisk();

	Dialog::close();
}

int VoiceDialog::getSubtitleMode(bool subtitles, bool speech_mute) {
	if (!subtitles && !speech_mute) // Speech only
		return kSubtitlesSpeech;
	else if (subtitles && !speech_mute) // Speech and subtitles
		return kSubtitlesBoth;
	else if (subtitles && speech_mute) // Subtitles only
		return kSubtitlesSubs;
	else
		warning("Wrong configuration: Both subtitles and speech are off. Assuming subtitles only");

	return kSubtitlesSubs;
}

MusicDialog::MusicDialog() {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	setSize();

	_musicToggleGroup = new RadiobuttonGroup(this, kMusicToggle);

	new RadiobuttonWidget(this, "MusicDialog.musicToggleEnhanced", _musicToggleGroup, kMusicEnhanced, _("ENHANCED MUSIC"));
	new RadiobuttonWidget(this, "MusicDialog.musicToggleOriginal", _musicToggleGroup, kMusicOriginal, _("ORIGINAL MUSIC"));
	new RadiobuttonWidget(this, "MusicDialog.musicToggleNone", _musicToggleGroup, kMusicNone, _("NO MUSIC"));

	new ButtonWidget(this, "MusicDialog.BackButton", _("BACK"), _("Previous menu"), kBackCmd);

	_musicToggleGroup->setValue(ConfMan.getInt("use-music"));

}

void MusicDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kBackCmd:
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void MusicDialog::close() {
	int music;
	switch (_musicToggleGroup->getValue()) {
	case kMusicEnhanced:
		music = 0;
		break;
	case kMusicOriginal:
		music = 1;
		break;
	case kMusicNone:
	default:
		music = 2;
		break;
	}

	ConfMan.setInt("use-music", music);

	ConfMan.flushToDisk();

	Dialog::close();
}

GraphicsDialog::GraphicsDialog() {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	setSize();

	_graphicsToggleGroup = new RadiobuttonGroup(this, kGfxToggle);

	new RadiobuttonWidget(this, "GraphicsDialog.gfxToggleHigh", _graphicsToggleGroup, kGfxHigh, _("HIGH"));
	new RadiobuttonWidget(this, "GraphicsDialog.gfxToggleMedium", _graphicsToggleGroup, kGfxMedium, _("MEDIUM"));
	new RadiobuttonWidget(this, "GraphicsDialog.gfxToggleLow", _graphicsToggleGroup, kGfxLow, _("LOW"));
	new RadiobuttonWidget(this, "GraphicsDialog.gfxToggleOriginal", _graphicsToggleGroup, kGfxOriginal, _("ORIGINAL"));

	new ButtonWidget(this, "GraphicsDialog.BackButton", _("BACK"), _("Previous menu"), kBackCmd);

	_graphicsToggleGroup->setValue(ConfMan.getInt("scaling_option"));

}

void GraphicsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kBackCmd:
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void GraphicsDialog::close() {
	int gfx;
	switch (_graphicsToggleGroup->getValue()) {
	case kGfxHigh:
		gfx = 0;
		break;
	case kGfxMedium:
		gfx = 1;
		break;
	case kGfxLow:
		gfx = 2;
		break;
	case kGfxOriginal:
	default:
		gfx = 3;
		break;
	}

	ConfMan.setInt("scaling_option", gfx);

	ConfMan.flushToDisk();

	Dialog::close();
}

ControlsDialog::ControlsDialog() {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	setSize();

	_controlsToggleGroup = new RadiobuttonGroup(this, kControlsToggle);

	new RadiobuttonWidget(this, "ControlsDialog.ctrlToggleTouch", _controlsToggleGroup, kControlsTouch, _("TOUCH"));
	new RadiobuttonWidget(this, "ControlsDialog.ctrlToggleClassic", _controlsToggleGroup, kControlsClassic, _("CLASSIC"));

	new ButtonWidget(this, "ControlsDialog.BackButton", _("BACK"), _("Previous menu"), kBackCmd);

	_controlsToggleGroup->setValue(ConfMan.getBool("touchpad_mode"));

}

void ControlsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kBackCmd:
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void ControlsDialog::close() {
	bool controls;
	switch (_controlsToggleGroup->getValue()) {
	case kControlsTouch:
		controls = true;
		break;
	case kControlsClassic:
	default:
		controls = false;
		break;
	}

	ConfMan.setBool("touchpad_mode", controls);

	ConfMan.flushToDisk();

	Dialog::close();
}

} // End of namespace GUI
