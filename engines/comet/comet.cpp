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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/scummsys.h"
#include "common/debug-channels.h"
#include "common/random.h"
#include "common/keyboard.h"
#include "common/config-manager.h"

#include "engines/util.h"

#include "base/plugins.h"

#include "graphics/cursorman.h"

#include "audio/mididrv.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "comet/comet.h"
#include "comet/actor.h"
#include "comet/console.h"
#include "comet/input.h"
#include "comet/music.h"
#include "comet/animationmgr.h"
#include "comet/comet_gui.h"
#include "comet/dialog.h"
#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/talktext.h"

namespace Comet {

enum {
	kMouseNone			= 0,
	kMouseLeftDown		= 1,
	kMouseLeftUp		= 2,
	kMouseRightDown		= 3,
	kMouseRightUp		= 4
};

CometEngine::CometEngine(OSystem *syst, const CometGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	DebugMan.addDebugChannel(kDebugResource, "Resource", "Resource Debug Flag");
	DebugMan.addDebugChannel(kDebugAnimation, "Animation", "Animation Debug Flag");
	DebugMan.addDebugChannel(kDebugSaveLoad, "Saveload", "Saveload Debug Flag");
	DebugMan.addDebugChannel(kDebugScript, "Script", "Script Debug Flag");
	DebugMan.addDebugChannel(kDebugText, "Text", "Text Debug Flag");
	DebugMan.addDebugChannel(kDebugCollision, "Collision", "Collision Debug Flag");
	DebugMan.addDebugChannel(kDebugScreen, "Screen", "Screen Debug Flag");

	_rnd = new Common::RandomSource("comet");

	_console = 0;

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_music = 0;
	_screen = 0;
	_dialog = 0;
	_script = 0;
	_scene = 0;
	_animationMan = 0;
	_res = 0;
	_textReader = 0;
	_gui = 0;
	_soundResource = 0;

	_tempScreen = 0;
	_screenPalette = 0;
	_backupPalette = 0;

	_sceneBackgroundResource = 0;

	_globalStrings = 0;
	_inventoryItemNames = 0;

	_bubbleSprite = 0;
	_heroSprite = 0;
	_inventoryItemSprites = 0;

	_gamePalette = 0;
	_flashbakPal = 0;
	_introPalette1 = 0;
	_introPalette2 = 0;

	_cursorSprite = 0;
	_iconSprite = 0;

	_sceneDecorationSprite = 0;

	_actors = new Actors(this);

}

CometEngine::~CometEngine() {
	DebugMan.clearAllDebugChannels();

	delete _actors;

	delete _rnd;
	delete _console;

	delete _music;
	delete _screen;
	delete _dialog;
	delete _script;
	delete _scene;
	delete _animationMan;
	delete _res;
	delete _textReader;
	delete _gui;
	delete _soundResource;

	delete[] _tempScreen;
	delete[] _screenPalette;
	delete[] _backupPalette;

	delete _sceneBackgroundResource;

	delete _globalStrings;
	delete _inventoryItemNames;

	delete _bubbleSprite;
	delete _heroSprite;
	delete _inventoryItemSprites;

	free(_gamePalette);
	free(_flashbakPal);
	free(_introPalette1);
	free(_introPalette2);

	delete _cursorSprite;
	delete _iconSprite;

	delete _sceneDecorationSprite;
}

void CometEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
}

Common::Error CometEngine::run() {
	initGraphics(320, 200, false);
	
	syncSoundSettings();

	// Default values
	ConfMan.registerDefault("text_speed", 1);
	ConfMan.registerDefault("game_speed", 2);
	
	_systemMouseCursor = new SystemMouseCursor();
	_console = new CometConsole(this);

	// Any music driver gets Adlib music except for 'No sound'
	if (MidiDriver::getMusicType(MidiDriver::detectDevice(MDT_PCSPK | MDT_MIDI | MDT_ADLIB)) != MT_NULL)
		_music = new MusicPlayer(this);
		
	_screen = new Screen(this);
	_dialog = new Dialog(this);
	_script = new ScriptInterpreter(this);
	_scene = new Scene(this);
	_animationMan = new AnimationManager(this);
	_res = new ResourceManager();
	_input = new Input(this);
	_talkText = new TalkText(this);

	_textReader = new TextReader(this);
	Common::String langText;
	switch (getLanguage()) {
		case Common::EN_ANY: // English
			langText += "E";
			break;
		case Common::DE_DEU: // German
			langText += "D";
			break;
		case Common::IT_ITA: // Italian
			langText += "I";
			break;
		case Common::ES_ESP: // Spanish
			langText += "S";
			break;
		case Common::FR_FRA: // French
			langText += "T";
			break;
		default:
			warning("Unknown Text Language - Defaulting to English");
			langText += "E";
			break;
	}
	langText += ".cc4";
	_textReader->setTextFilename(langText.c_str());  

	_gui = new Gui(this);

	_soundResource = new SoundResource();
	_currSoundResourceIndex = -1;

	// Init vars
	_gameLoopCounter = 0;
	_textColorFlag = 0;

	_prevModuleNumber = -1;
	_currentModuleNumber = -1;
	_moduleNumber = 0;
	_prevSceneNumber = -1;
	_currentSceneNumber = -1;
	_sceneNumber = 0;

	_clearScreenRequest = false;

	_paletteRedness = 0;
	_paletteStatus = 0;
	_animationType = 0;

	_itemInSight = false;

	_sceneDecorationSprite = NULL;
	_loadgameRequested = false;

	_portraitTalkCounter = 0;
	_portraitTalkAnimNumber = 0;

	_cmdLook = false;
	_cmdGet = false;
	_cmdTalk = false;

	_currentInventoryItem = -1;

	_nextTick = 0;

	initSystemVars();

	// Unused in Comet CD
	// _beamColor = 112;
	// _beamColorIncr = 1;

	_isSaveAllowed = true;

	if (getGameID() == GID_COMET) {
		if (isFloppy()) {
			_startupModuleNumber = 5;
			_startupSceneNumber = 0;
		} else {
			_startupModuleNumber = 9;
			_startupSceneNumber = 0;
		}
	} else if (getGameID() == GID_MUSEUM) {
		_startupModuleNumber = 0;
		_startupSceneNumber = 43;
	}

	initAndLoadGlobalData();

#if 0
//*** TEST ***//
	_moduleNumber = 0;
	_backgroundFileIndex = 0;
	_scenePakName = Common::String::format("d%02d.pak", _moduleNumber);
	_res->loadFromPak(_sceneBackgroundResource, _scenePakName.c_str(), _backgroundFileIndex);
	_screen->clear();
	_screen->setFullPalette(_gamePalette);
	_screen->copyFromScreenResource(_sceneBackgroundResource);
	Graphics::Surface *destSurface = new Graphics::Surface();
	destSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	Graphics::Surface *sourceSurface = new Graphics::Surface();
	sourceSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	memcpy(sourceSurface->getPixels(), _screen->_backSurface->getPixels(), 64000);
	//_screen->screenZoomEffect2x(destSurface, sourceSurface, 0, 0);
	//_screen->screenZoomEffect3x(destSurface, sourceSurface, 0, 0);
	_screen->screenZoomEffect4x(destSurface, sourceSurface, 0, 0);
	//_system->copyRectToScreen(_screen->_backSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->copyRectToScreen(destSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();
	sourceSurface->free();
	delete sourceSurface;
	destSurface->free();
	delete destSurface;
	while (!shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
		}
	}
	debug("ok.");
//*** TEST ***//
	return Common::kNoError;
#endif

	if (!isFloppy()) {
		CursorMan.showMouse(true);
		setMouseCursor(0);
	} else
		CursorMan.showMouse(false);

	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 99)
			loadGameState(saveSlot);
	} else if (getGameID() == GID_COMET) {
		if (ConfMan.getInt("boot_param") != 1) {
			// Play the intro
			introMainLoop();
		} else {
			_moduleNumber = 9;
			_sceneNumber = 9;
		}
		_input->waitForKeys();
		if (_currentModuleNumber == 5)
			_sceneNumber = 2;
		else if (_currentModuleNumber == 9)
			_sceneNumber = 9;
	}

	_screen->clear();
	_screen->update();

	if (!shouldQuit()) {
		if (getGameID() == GID_COMET) {
			cometMainLoop();
		} else if (getGameID() == GID_MUSEUM) {
			museumMainLoop();
		}
	}

	delete _talkText;
	delete _input;
	delete _systemMouseCursor;

	return Common::kNoError;
}

} // End of namespace Comet
