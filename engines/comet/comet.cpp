/*
	Global TODO:
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
#include "comet/font.h"
#include "comet/pak.h"
#include "comet/music.h"

#include "comet/animation.h"
#include "comet/animationmgr.h"
#include "comet/dialog.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/text.h"

namespace Comet {

enum {
	kMouseNone			= 0,
	kMouseLeftDown		= 1,
	kMouseLeftUp		= 2,
	kMouseRightDown		= 3,
	kMouseRightUp		= 4
};

CometEngine::CometEngine(OSystem *syst, const CometGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc) {

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "comet");

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	DebugMan.addDebugChannel(kDebugScript, "script", "Script debug level");
	
}

CometEngine::~CometEngine() {
	delete _rnd;
	delete _music;
	delete _screen;
	delete _dialog;
}

Common::Error CometEngine::run() {

	Common::Event event;

	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(false);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

#if 0
	{
		int size = getPakSize("A00.PAK", 12);
		byte *buf = loadFromPak("A00.PAK", 12);
		FILE *x = fopen("SEAGULLS.VA2", "wb");
		fwrite(buf, size, 1, x);
		fclose(x);
	}

#endif

#define OLD_CODE
//#define TEST_CODE

#ifdef OLD_CODE
	// TODO: delete stuff at engine shutdown
	_music = new MusicPlayer(this);
	_screen = new Screen(this);
	_dialog = new Dialog(this);
	_script = new ScriptInterpreter(this);
	_scene = new Scene(this);
	_animationMan = new AnimationManager(this);
	_textReader = new TextReader();
	_textReader->open("E.CC4"); // TODO: Use language-specific filename

	/* Init vars */
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

	_introPaletteState = 0;
	_animationType = 0;
	_textActive = false;

	_textBubbleActive = false;
	_itemInSight = false;

	_sceneDecorationSprite = NULL;
	_needToLoadSavegameFlag = false;
 	//_sceneExits.clear();

	_portraitTalkCounter = 0;
	_portraitTalkAnimNumber = 0;

	_mouseX = 0;
	_mouseY = 0;
	_keyScancode = Common::KEYCODE_INVALID;
	_keyDirection = 0;
	_keyDirection2 = 0;
	_mouseButtons4 = 0;
	_mouseButtons5 = 0;
	_scriptMouseFlag = false;
	_leftButton = false;
	_rightButton = false;
	
	_cmdLook = false;
	_cmdGet = false;
	_cmdTalk = false;
	
	_currentInventoryItem = -1;
	
	_animIndex = -1;

	_systemVars[0] = &_prevSceneNumber;
	for (int i = 0; i < 10; i++) {
		_systemVars[1 + i * 3] = &_actors[i].life;
		_systemVars[2 + i * 3] = &_actors[i].x;
		_systemVars[3 + i * 3] = &_actors[i].y;
	}
	_systemVars[31] = &_mouseButtons4;
	_systemVars[32] = &_scriptMouseFlag;
	_systemVars[33] = &_scriptRandomValue;
	_systemVars[34] = &_prevModuleNumber;

	_talkieMode = 1;

	_startupModuleNumber = 9;
	_startupSceneNumber = 0;

	_narFile = NULL;
	_narCount = 0;
	_narOffsets = NULL;

	/* Unused in Comet CD
	_beamColor = 112;
	_beamColorIncr = 1;
	*/
	
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

//#define DOINTRO

#ifdef DOINTRO

#if 1
	/* Play intro */
	_endLoopFlag = false;
	while (!_endLoopFlag) {
		handleEvents();
		
		if (_keyScancode == Common::KEYCODE_ESCAPE)
			break;

		static bool slow = false;
		if (_keyScancode == Common::KEYCODE_a)
			slow = !slow;
		if (_keyScancode == Common::KEYCODE_r)
			_debugRectangles = !_debugRectangles;
		if (slow)
			_system->delayMillis(40);

		if (_keyScancode == Common::KEYCODE_KP_PLUS) {
			_sceneNumber++;
			debug(4, "## New _sceneNumber = %d", _sceneNumber);
		} else if (_keyScancode == Common::KEYCODE_KP_MINUS) {
			if (_sceneNumber > 0) {
				debug(4, "## New _sceneNumber = %d", _sceneNumber);
				_sceneNumber--;
			}
		}

		/* Debugging helpers ends here */
		
		updateGame();

#if 1
		//DEBUG
		if (_moduleNumber == 9 && _sceneNumber == 0) {
			_moduleNumber = 5;
			_sceneNumber = 0;
		}
#endif
#if 0
		//DEBUG - jump to scene
		if (_moduleNumber == 9 && _sceneNumber == 0) {
			memcpy(_ctuPal, _paletteBuffer, 768);
			memcpy(_palette, _paletteBuffer, 768);
			setFullPalette(_ctuPal);
			_introPaletteState = 0;
			_moduleNumber = 0;
			_sceneNumber = 1;
			//_sceneNumber = 11;
		}
#endif
	}
#endif

#endif

#if 0
	// Test the puzzle
	_screen->setFullPalette(_ctuPal);
	runPuzzle();
#endif

#if 0
	_scriptVars[2] = 0xFFFF;
	_scriptVars[3] = 0;
	_scriptVars[4] = 1;
	_screen->setFullPalette(_ctuPal);
	updateMap();
#endif

#if 0
	static byte soundFramesData[] = {3, 3, 0};
	_screen->setFullPalette(_ctuPal);
	strcpy(AName, "A00.PAK");
	playCutscene(38, 0, 32000, 3, 1, soundFramesData);
#endif

#if 0
	_screen->setFullPalette(_ctuPal);
	handleCommandBar();
#endif

#if 0
	Animation *anim1 = _animationMan->loadAnimationResource("A00.PAK", 12);
	AnimationElement *elem1 = anim1->_elements[0];
	AnimationElement *elem2 = anim1->_elements[1];
	InterpolatedAnimationElement interElem;
	buildInterpolatedAnimationElement(elem1, elem2, &interElem);
	delete anim1;
#endif	

#if 1
	/* Hacked together main loop */
	_endLoopFlag = false;
	while (!_endLoopFlag) {
		handleEvents();

		/* Debugging helpers follows */
		if (_keyScancode == Common::KEYCODE_ESCAPE)
			break;

		static bool slow = false;
		if (_keyScancode == Common::KEYCODE_a)
			slow = !slow;
		if (_keyScancode == Common::KEYCODE_r)
			_debugRectangles = !_debugRectangles;
		if (slow)
			_system->delayMillis(40);

		/*
		if (_keyScancode == Common::KEYCODE_RETURN)
			skipText();

		if (_keyScancode == Common::KEYCODE_t)
	  		_cmdTalk = true;
		else if (_keyScancode == Common::KEYCODE_g)
	  		_cmdGet = true;
		else if (_keyScancode == Common::KEYCODE_l)
	  		_cmdLook = true;
		else if (_keyScancode == Common::KEYCODE_o) {
			handleInventory();
		} else if (_keyScancode == Common::KEYCODE_b) {
			// DEBUG only
			handleReadBook();
		}
		*/
		

		if (!_dialog->isRunning() && _currentModuleNumber != 3 && _actors[0].value6 != 4 && !_screen->_palFlag && !_textActive) {
			handleKeyInput();
		} else if (_keyScancode == Common::KEYCODE_RETURN || (_rightButton && _textActive)) {
			skipText();
		}

		switch (_keyScancode) {
		case Common::KEYCODE_F7:
			savegame("comet.000", "Quicksave");
			break;
		case Common::KEYCODE_F9:
			loadgame("comet.000");
			break;
		default:
			break;
		}

		// DEBUG
		if (_keyScancode == Common::KEYCODE_KP_PLUS) {
			_sceneNumber++;
			debug("## New _sceneNumber = %d", _sceneNumber);
		} else if (_keyScancode == Common::KEYCODE_KP_MINUS) {
			if (_sceneNumber > 0) {
				debug("## New _sceneNumber = %d", _sceneNumber);
				_sceneNumber--;
			}
		} if (_keyScancode == Common::KEYCODE_KP_MULTIPLY) {
			_moduleNumber++;
			_sceneNumber = 0;
			debug("## New _moduleNumber = %d", _moduleNumber);
		} else if (_keyScancode == Common::KEYCODE_KP_DIVIDE) {
			if (_moduleNumber > 0) {
				_moduleNumber--;
				_sceneNumber = 0;
				debug("## New _moduleNumber = %d", _moduleNumber);
			}
		}
		/* Debugging helpers ends here */

		updateGame();
		checkCurrentInventoryItem();

#if 1
		//DEBUG - jump to scene
		if (_moduleNumber == 9 && _sceneNumber == 0) {
			memcpy(_ctuPal, _paletteBuffer, 768);
			memcpy(_palette, _paletteBuffer, 768);
			_screen->setFullPalette(_ctuPal);
			_introPaletteState = 0;
#if 1			
			_moduleNumber = 0;
			_sceneNumber = 0;
#endif			
#if 0
			// Test the "beam-room"
			_scriptVars[116] = 1;
			_scriptVars[139] = 1;
			_moduleNumber = 7;
			_sceneNumber = 4;
#endif			
		}
#endif
	}
#endif

#endif // OLD_CODE

	return Common::kNoError;
}

} // End of namespace Comet
