/*
	Global TODO:
*/


#include "common/scummsys.h"

#include "base/plugins.h"
#include "common/keyboard.h"
#include "common/config-manager.h"
#include "sound/mididrv.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/pak.h"
#include "comet/music.h"

#include "comet/screen.h"
#include "comet/dialog.h"

#include "comet/animation.h"

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

	syst->getEventManager()->registerRandomSource(_rnd, "comet");

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

#ifdef TEST_CODE
	_screen = new Screen(this);


	_ctuPal = loadFromPak("RES.PAK", 5);
	_screen->setFullPalette(_ctuPal);

	Animation *anim = new Animation();


	const char *pakName = "D00.PAK";
	const int pakIndex = 3;

	byte *buffer = loadFromPak(pakName, pakIndex);
	int size = getPakSize(pakName, pakIndex);
#if 0
	FILE *xf = fopen("dump.0", "wb");
	fwrite(buffer, size, 1, xf);
	fclose(xf);
#endif
	debug("size = %d", size);
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(buffer, size);
	anim->load(*stream, size);
	delete stream;
	free(buffer);

	/*
	int x = 0, y = 50, maxHeight = 0;
	for (int i = 0;  i < anim->_cels.size(); i++) {
		AnimationCel *cel = anim->_cels[i];
		if (x + cel->width > 320) {
			x = 0;
			y += maxHeight + 4;
			maxHeight = 0;
			if (y > 200)
				break;
		}
		_screen->drawAnimationCelSprite(*cel, x, y);
		x += cel->width + 4;
		if (cel->height > maxHeight)
			maxHeight = cel->height;
	}
	*/

	/*
	_screen->drawAnimationCelRle(*anim->_cels[0], 0, 0);
	*/
	
	/*
	for (int i = 0;  i < anim->_elements.size(); i++) {
		_screen->clearScreen();
		_screen->drawAnimationElement(*anim, i, 50, 120);
		_system->delayMillis(200);
		_screen->update();
	}
	*/
	
	// Scene foreground graphics
	_screen->drawAnimationElement(anim, 0, 0, 0);
	
	_screen->update();

	while (1) {
		handleEvents();
	}

	delete anim;

	delete _screen;

#endif

#ifdef OLD_CODE
	_music = new MusicPlayer(this);
	_screen = new Screen(this);
	_dialog = new Dialog(this);
	_script = new ScriptInterpreter(this);

	/* Init vars */
	_gameLoopCounter = 0;
	_textColorFlag = 0;
	
	_prevChapterNumber = -1;
	_currentChapterNumber = -1;
	_chapterNumber = 0;
	_prevSceneNumber = -1;
	_currentSceneNumber = -1;
	_sceneNumber = 0;
	memset(_marcheItems, 0, sizeof(_marcheItems));
	memset(_sceneObjects, 0, sizeof(_sceneObjects));

	_paletteValue2 = 0;
	_marcheNumber = 0;
	_textActive = false;

	_flag03 = false;
	_itemInSight = false;

	_sceneObjectsSprite = NULL;
	_needToLoadSavegameFlag = false;
	_loadingGameFlag = false;
 	_sceneExits.clear();

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

	_scriptVars1[0] = &_prevSceneNumber;
	for (int i = 0; i < 10; i++) {
		_scriptVars1[1 + i * 3] = &_sceneObjects[i].flag;
		_scriptVars1[2 + i * 3] = &_sceneObjects[i].x;
		_scriptVars1[3 + i * 3] = &_sceneObjects[i].y;
	}
	_scriptVars1[31] = &_mouseButtons4;
	_scriptVars1[32] = &_scriptMouseFlag;
	_scriptVars1[33] = &_scriptRandomValue;
	_scriptVars1[34] = &_prevChapterNumber;

	_talkieMode = 1;

	_startupChapterNumber = 9;
	_startupSceneNumber = 0;

	_narFile = NULL;
	_narCount = 0;
	_narOffsets = NULL;

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
			_paletteValue2 = 0;
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

		// DEBUG
		if (_keyScancode == Common::KEYCODE_KP_PLUS) {
			_sceneNumber++;
			debug(4, "## New _sceneNumber = %d", _sceneNumber);
		} else if (_keyScancode == Common::KEYCODE_KP_MINUS) {
			if (_sceneNumber > 0) {
				debug(4, "## New _sceneNumber = %d", _sceneNumber);
				_sceneNumber--;
			}
		} if (_keyScancode == Common::KEYCODE_KP_MULTIPLY) {
			_chapterNumber++;
			_sceneNumber = 0;
		} else if (_keyScancode == Common::KEYCODE_KP_DIVIDE) {
			if (_chapterNumber > 0) {
				_chapterNumber--;
				_sceneNumber = 0;
			}
		}
		/* Debugging helpers ends here */

		updateGame();

#if 1
		//DEBUG - jump to scene
		if (_chapterNumber == 9 && _sceneNumber == 0) {
			memcpy(_ctuPal, _paletteBuffer, 768);
			memcpy(_palette, _paletteBuffer, 768);
			_screen->setFullPalette(_ctuPal);
			_paletteValue2 = 0;
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
