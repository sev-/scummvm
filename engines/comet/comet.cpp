/*
	Global TODO:
*/


#include "common/scummsys.h"
#include "common/EventRecorder.h"
#include "common/keyboard.h"
#include "common/config-manager.h"

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

	Common::addDebugChannel(kDebugScript, "script", "Script debug level");
	
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
	
	_prevChapterNumber = -1;
	_currentChapterNumber = -1;
	_chapterNumber = 0;
	_prevSceneNumber = -1;
	_currentSceneNumber = -1;
	_sceneNumber = 0;
	memset(_sceneObjects, 0, sizeof(_sceneObjects));

	_clearScreenRequest = false;

	_introPaletteState = 0;
	_animationType = 0;
	_textActive = false;

	_flag03 = false;
	_itemInSight = false;

	_sceneObjectsSprite = NULL;
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
	
	_cmdLook = false;
	_cmdGet = false;
	_cmdTalk = false;
	
	_invActiveItem = -1;
	
	_animIndex = -1;

	_systemVars[0] = &_prevSceneNumber;
	for (int i = 0; i < 10; i++) {
		_systemVars[1 + i * 3] = &_sceneObjects[i].life;
		_systemVars[2 + i * 3] = &_sceneObjects[i].x;
		_systemVars[3 + i * 3] = &_sceneObjects[i].y;
	}
	_systemVars[31] = &_mouseButtons4;
	_systemVars[32] = &_scriptMouseFlag;
	_systemVars[33] = &_scriptRandomValue;
	_systemVars[34] = &_prevChapterNumber;

	_talkieMode = 1;

	_startupChapterNumber = 9;
	_startupSceneNumber = 0;

	_narFile = NULL;
	_narCount = 0;
	_narOffsets = NULL;
	
	_debugRectangles = false;

	initAndLoadGlobalData();

	//DEBUG:
	//setFullPalette(_cdintroPal);
	//setFullPalette(_ctuPal);

	//TEST
	_talkieMode = 1;
	_textSpeed = 0;
	//_music->playMusic(4);

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
		if (_chapterNumber == 9 && _sceneNumber == 0) {
			_chapterNumber = 5;
			_sceneNumber = 0;
		}
#endif
#if 0
		//DEBUG - jump to scene
		if (_chapterNumber == 9 && _sceneNumber == 0) {
			memcpy(_ctuPal, _paletteBuffer, 768);
			memcpy(_palette, _paletteBuffer, 768);
			setFullPalette(_ctuPal);
			_introPaletteState = 0;
			_chapterNumber = 0;
			_sceneNumber = 1;
			//_sceneNumber = 11;
		}
#endif
	}
#endif

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

		if (_keyScancode == Common::KEYCODE_RETURN)
			skipText();

		if (_keyScancode == Common::KEYCODE_t)
	  		_cmdTalk = true;
		else if (_keyScancode == Common::KEYCODE_g)
	  		_cmdGet = true;
		else if (_keyScancode == Common::KEYCODE_l)
	  		_cmdLook = true;
		else if (_keyScancode == Common::KEYCODE_o) {
			// Inventory test code
			/*
			for (int i = 0; i < 255; i++) {
				if (_itemStatus[i] == 1) {
					debug("item[%03d] = [%s]", i, _textBuffer3->getString(i));
				}
			}
			*/
			handleInventory();
		} else if (_keyScancode == Common::KEYCODE_b) {
			// DEBUG only
			handleReadBook();
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
			_chapterNumber++;
			_sceneNumber = 0;
			debug("## New _chapterNumber = %d", _chapterNumber);
		} else if (_keyScancode == Common::KEYCODE_KP_DIVIDE) {
			if (_chapterNumber > 0) {
				_chapterNumber--;
				_sceneNumber = 0;
				debug("## New _chapterNumber = %d", _chapterNumber);
			}
		}
		/* Debugging helpers ends here */

		updateGame();
		invCheckActiveItem();

#if 1
		//DEBUG - jump to scene
		if (_chapterNumber == 9 && _sceneNumber == 0) {
			memcpy(_ctuPal, _paletteBuffer, 768);
			memcpy(_palette, _paletteBuffer, 768);
			_screen->setFullPalette(_ctuPal);
			_introPaletteState = 0;
			//_chapterNumber = 1;
			_chapterNumber = 0;
			_sceneNumber = 0;
			//_sceneNumber = 11;
		}
#endif
	}
#endif

#endif // OLD_CODE

	return Common::kNoError;
}

} // End of namespace Comet
