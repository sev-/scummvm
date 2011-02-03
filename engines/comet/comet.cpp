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
#include "common/EventRecorder.h"
#include "common/keyboard.h"
#include "common/config-manager.h"

#include "engines/util.h"

#include "base/plugins.h"

#include "sound/mididrv.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "comet/comet.h"
#include "comet/console.h"
#include "comet/music.h"
#include "comet/animationmgr.h"
#include "comet/comet_gui.h"
#include "comet/dialog.h"
#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/scene.h"
#include "comet/screen.h"

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

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "comet");

	_console = 0;

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

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

	_quitGame = false;
}

CometEngine::~CometEngine() {
	DebugMan.clearAllDebugChannels();

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

Common::Error CometEngine::run() {
	Common::Event event;

	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(false);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	_console = new CometConsole(this);

	_music = new MusicPlayer(this);
	_screen = new Screen(this);
	_dialog = new Dialog(this);
	_script = new ScriptInterpreter(this);
	_scene = new Scene(this);
	_animationMan = new AnimationManager(this);
	_res = new ResourceManager();

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
	langText += ".CC4";
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
	memset(_actors, 0, sizeof(_actors));

	_clearScreenRequest = false;

	_paletteRedness = 0;
	_paletteStatus = 0;
	_animationType = 0;
	_textActive = false;

	_textBubbleActive = false;
	_itemInSight = false;

	_sceneDecorationSprite = NULL;
	_loadgameRequested = false;

	_portraitTalkCounter = 0;
	_portraitTalkAnimNumber = 0;

	_mouseX = 0;
	_mouseY = 0;
	_keyScancode = Common::KEYCODE_INVALID;
	_keyDirection = 0;
	_cursorDirection = 0;
	_mouseClick = 0;
	_scriptKeybFlag = 0;
	_mouseWalking = false;
	_mouseCursorDirection = 0;
	_leftButton = false;
	_rightButton = false;

	_cmdLook = false;
	_cmdGet = false;
	_cmdTalk = false;

	_currentInventoryItem = -1;

	_animIndex = -1;

	initSystemVars();

	_talkieMode = 1;

	_startupModuleNumber = 9;
	_startupSceneNumber = 0;

	// Unused in Comet CD
	// _beamColor = 112;
	// _beamColorIncr = 1;

	_isSaveAllowed = true;
	
	_debugRectangles = false;

	initAndLoadGlobalData();

	//DEBUG:
	//setFullPalette(_cdintroPal);
	//setFullPalette(_ctuPal);

	//TEST
	_talkieMode = 1;
	_textSpeed = 0;
	//_music->playMusic(4);

	_system->showMouse(true);

#if 1
	// Test the puzzle
	_screen->setFullPalette(_gamePalette);
	_gui->run(kGuiPuzzle);
#endif

#if 0
	byte soundFramesData[] = {3, 3, 0};
	_screen->setFullPalette(_ctuPal);
	strcpy(AName, "A00.PAK");
	playCutscene(38, 0, 32000, 3, 1, soundFramesData);
#endif

#if 0
	_screen->setFullPalette(_ctuPal);
	handleDiskMenu();
#endif

#if 0
	Animation *anim1 = _animationMan->loadAnimationResource("A00.PAK", 12);
	AnimationElement *elem1 = anim1->_elements[0];
	AnimationElement *elem2 = anim1->_elements[1];
	InterpolatedAnimationElement interElem;
	buildInterpolatedAnimationElement(elem1, elem2, &interElem);
	delete anim1;
#endif

#if 0
	// Test new resource loader
	ResourceManager res;
	GenericResource *g = new GenericResource();
	//res.loadFromPak(g, "A00.PAK", 1);
	//res.loadFromCC4(g, "E.CC4", 0);
	res.loadFromNar(g, "D00.NAR", 806);
	delete g;
#endif

#if 0
	Common::Array<Point> *poly = new Common::Array<Point>();
	poly->push_back(Point(319, 156));
	poly->push_back(Point(325, 152));
	poly->push_back(Point(325, 156));
	poly->push_back(Point(319, 156));
	for (uint i = 0; i < poly->size(); i++) {
		Point pt = (*poly)[i];
		debug("pt.x = %d; pt.y = %d", pt.x, pt.y);
	}
	_screen->clipPolygonRight(&poly, 319);
	debug("--------------------------------");
	for (uint i = 0; i < poly->size(); i++) {
		Point pt = (*poly)[i];
		debug("pt.x = %d; pt.y = %d", pt.x, pt.y);
	}
	delete poly;
#endif

#if 0
	// Anim viewer
	_screen->setFullPalette(_gamePalette);
	AnimationResource *anim;
	bool done = false;
	int frameListIndex = 0, frameIndex = 0;
	int resIndex = 9;
	//anim = _animationMan->loadAnimationResource("A05.PAK", 7);//FIRE
	anim = _animationMan->loadAnimationResource("RES.PAK", resIndex);
	while (!done) {
		int16 x, y;
		AnimationFrameList *frameList;
		handleEvents();
		// Debugging keys
		switch (_keyScancode) {
		case Common::KEYCODE_KP_PLUS:
			resIndex++;
			debug("resIndex = %d", resIndex);
			delete anim;
			anim = _animationMan->loadAnimationResource("RES.PAK", resIndex);
			frameListIndex = 0;
			frameIndex = 0;
			break;
		case Common::KEYCODE_KP_MINUS:
			if (resIndex > 0) {
				resIndex--;
				debug("resIndex = %d", resIndex);
				delete anim;
				anim = _animationMan->loadAnimationResource("A05.PAK", resIndex);
				frameListIndex = 0;
				frameIndex = 0;
			}
			break;
		case Common::KEYCODE_ESCAPE:
			done = true;
			break;
		case Common::KEYCODE_SPACE:
			frameList = anim->_anims[frameListIndex];
			debug("(%d/%d); (%d/%d); (%d/%d)", frameIndex, frameList->frames.size(), frameListIndex, anim->_anims.size(), x, y);
			break;
		case Common::KEYCODE_a:
			frameIndex++;
			if (frameIndex >= frameList->frames.size()) {
				frameIndex = 0;
				frameListIndex++;
				if (frameListIndex >= anim->_anims.size()) 
					frameListIndex = 0;
			}
			frameList = anim->_anims[frameListIndex];
			debug("(%d/%d); (%d/%d); (%d/%d)", frameIndex, frameList->frames.size(), frameListIndex, anim->_anims.size(), x, y);
			break;
		default:
			break;
		}
		//x = 141; y = 6;
		//x = 0; y = 0;
		//x = 141; 
		//y = 0;
		x = _mouseX; 
		y = _mouseY;
		//frameListIndex = 1;
		//frameIndex = 49;
		frameList = anim->_anims[frameListIndex];
		_screen->clear();
		if (frameIndex < frameList->frames.size())
			_screen->drawAnimationElement(anim, frameList->frames[frameIndex]->elementIndex, x, y);
		_screen->update();
		_system->delayMillis(40);
	}
#endif

#if 0
	// Cursor viewer
	_screen->setFullPalette(_gamePalette);
	AnimationResource *anim;
	bool done = false;
	int celIndex = 0;
	anim = _animationMan->loadAnimationResource("RES.PAK", 9);
	while (!done) {
		int16 x, y;
		AnimationCel *currCel = anim->_cels[celIndex];
		handleEvents();
		// Debugging keys
		switch (_keyScancode) {
		case Common::KEYCODE_KP_PLUS:
			celIndex++;
			if (celIndex >= anim->_cels.size())
				celIndex = 0;
			debug("celIndex = %d", celIndex);
			currCel = anim->_cels[celIndex];
			break;
		case Common::KEYCODE_KP_MINUS:
			celIndex--;
			if (celIndex < 0) 
				celIndex = anim->_cels.size() - 1;
			debug("celIndex = %d", celIndex);
			currCel = anim->_cels[celIndex];
			break;
		case Common::KEYCODE_ESCAPE:
			done = true;
			break;
		default:
			break;
		}
		x = _mouseX + 20; 
		y = _mouseY + 20;
		_screen->clear();
		_screen->drawAnimationCelSprite(*currCel, x, y, 0);
		_screen->update();
		_system->delayMillis(40);
	}
#endif

	setMouseCursor(0, _mouseCursors[0]);

	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 99) {
			loadGameState(saveSlot);
		}
	} else {
		if (ConfMan.getInt("boot_param") != 1) {
			// Play the intro
			introMainLoop();
		} else {
			_moduleNumber = 9;
			_sceneNumber = 9;
		}

		waitForKeys();

		if (_currentModuleNumber == 5)
			_sceneNumber = 2;
		else if (_currentModuleNumber == 9)
			_sceneNumber = 9;
	}

	_screen->clear();
	_screen->update();

	debug("_sceneNumber = %d; _moduleNumber = %d", _sceneNumber, _moduleNumber);
	debug("_currentSceneNumber = %d; _currentModuleNumber = %d", _currentSceneNumber, _currentModuleNumber);

	if (!_quitGame)
		gameMainLoop();

	return Common::kNoError;
}

} // End of namespace Comet
