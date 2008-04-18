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
#include "comet/anim.h"
#include "comet/pak.h"
#include "comet/music.h"

namespace Comet {

enum {
	kMouseNone			= 0,
	kMouseLeftDown		= 1,
	kMouseLeftUp		= 2,
	kMouseRightDown     = 3,
	kMouseRightUp		= 4
};

CometEngine::CometEngine(OSystem *syst, const CometGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc) {

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	Common::addSpecialDebugLevel(kDebugScript, "script", "Script debug level");
	
}


CometEngine::~CometEngine() {
	if (_music)
	    delete _music;
}


int CometEngine::init() {

	// Detect game
	if (!initGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(true);
		_system->initSize(320, 200);
	_system->endGFXTransaction();


    _music = new MusicPlayer(this);

	return 0;
}

int CometEngine::go() {

	/*
	int size = getPakSize("A00.PAK", 38);
	byte *temp = loadFromPak("A00.PAK", 38);
    FILE *f = fopen("Q:\\OldGames\\SotC\\scummvm\\engines\\comet\\test.va2", "wb");
    fwrite(temp, size, 1, f);
    fclose(f);
    exit(0);
    */

    Common::Event event;
    Common::EventManager *eventMan = _system->getEventManager();

	/* Init vars */
	_gameLoopCounter = 0;
	_textColorFlag = 0;
	
	_fileNumber3 = -1;
	_currentFileNumber = -1;
	_fileNumber = 0;
	_scriptNumber3 = -1;
	_currentScriptNumber = -1;
	_scriptNumber = 0;
    memset(_marcheItems, 0, sizeof(_marcheItems));
    memset(_sceneObjects, 0, sizeof(_sceneObjects));
	_scriptData = NULL;
	_scriptCount = 0;
	_curScriptNumber = -1;
	_curScript = NULL;
	_paletteValue2 = 0;
	_marcheNumber = 0;
	_paletteMode = 0;
	_screenZoomFactor = 0;
	_textFlag2 = 0;

	_flag03 = false;
	_itemInSight = false;

	_staticObjects = NULL;
	_needToLoadSavegameFlag = false;
	_loadingGameFlag = false;
 	_linesArray.clear();
	_screenTransitionEffectFlag = false;
	_screenZoomFactor = 0;
	_screenZoomXPos = 160;
	_screenZoomYPos = 100;
	
	_portraitTalkCounter = 0;
	_portraitTalkAnimNumber = 0;
	
	_blockingTestRect.left = 0;
	_blockingTestRect.right = 0;
	_blockingTestRect.top = 0;
	_blockingTestRect.bottom = 0;
	
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
	
	_dialogTextSubIndex = 0;
	_dialogTextX = 0;
	_dialogTextY = 0;
	_dialogTextColor = 79;
	_dialogTextColorInc = -1;
	_dialogRunning = false;
	_animIndex = -1;

    _scriptVars1[0] = &_scriptNumber3;
    for (int i = 0; i < 10; i++) {
    	_scriptVars1[1 + i * 3] = &_sceneObjects[i].flag;
    	_scriptVars1[2 + i * 3] = &_sceneObjects[i].x;
    	_scriptVars1[3 + i * 3] = &_sceneObjects[i].y;
    }
    _scriptVars1[31] = &_mouseButtons4;
    _scriptVars1[32] = &_scriptMouseFlag;
    _scriptVars1[33] = &_scriptRandomValue;
    _scriptVars1[34] = &_fileNumber3;

	for (int i = 0; i < 17; i++)
	    _scripts[i] = new Script(this);

    _talkieMode = 1;

    _startupFileNumber = 9;
    _startupScriptNumber = 0;

	_narFile = NULL;
	_narCount = 0;
	_narOffsets = NULL;

    initAndLoadGlobalData();

	//DEBUG:
	//setFullPalette(_cdintroPal);
	//setFullPalette(_ctuPal);

	//TEST
	_talkieMode = 1;
    //_music->playMusic(4);

//#define DOINTRO

#ifdef DOINTRO

#if 1
	/* Play intro */
    _endLoopFlag = false;
    while (!_endLoopFlag) {
        handleEvents();
        printf("l: _keyScancode = %d\n", _keyScancode); fflush(stdout);
        
        if (_keyScancode == Common::KEYCODE_ESCAPE)
            break;

        static bool slow = false;
		if (_keyScancode == Common::KEYCODE_a)
		    slow = !slow;
		if (slow)
			_system->delayMillis(40);

        if (_keyScancode == Common::KEYCODE_KP_PLUS)
        	_scriptNumber++;
        else if (_keyScancode == Common::KEYCODE_KP_MINUS)
        	if (_scriptNumber > 0) _scriptNumber--;
        	
        /* Debugging helpers ends here */
        
        updateGame();
#if 1
        //DEBUG
        if (_fileNumber == 9 && _scriptNumber == 0) {
        	_fileNumber = 5;
        	_scriptNumber = 0;
        }
#endif
#if 0
        //DEBUG - jump to scene
        if (_fileNumber == 9 && _scriptNumber == 0) {
		    memcpy(_ctuPal, _paletteBuffer, 768);
		    memcpy(_palette, _paletteBuffer, 768);
			setFullPalette(_ctuPal);
			_paletteValue2 = 0;
        	_fileNumber = 0;
        	_scriptNumber = 1;
        	//_scriptNumber = 11;
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
        printf("l: _keyScancode = %d\n", _keyScancode); fflush(stdout);
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
        	_scriptNumber++;
        } else if (_keyScancode == Common::KEYCODE_KP_MINUS) {
        	if (_scriptNumber > 0) _scriptNumber--;
        } if (_keyScancode == Common::KEYCODE_KP_MULTIPLY) {
        	_fileNumber++;
        	_scriptNumber = 0;
        } else if (_keyScancode == Common::KEYCODE_KP_DIVIDE) {
        	if (_fileNumber > 0) {
				_fileNumber--;
        		_scriptNumber = 0;
        	}
        }

        /* Debugging helpers ends here */

        updateGame();

#if 1
        //DEBUG - jump to scene
        if (_fileNumber == 9 && _scriptNumber == 0) {
		    memcpy(_ctuPal, _paletteBuffer, 768);
		    memcpy(_palette, _paletteBuffer, 768);
			setFullPalette(_ctuPal);
			_paletteValue2 = 0;
        	//_fileNumber = 1;
        	_fileNumber = 0;
        	_scriptNumber = 0;
        	//_scriptNumber = 11;
        }
#endif
	}
#endif



#if 0
    memset(getScreen(), 0, 64000);

    setFullPalette(_ctuPal);
    
    _startupFileNumber = 9;
    _startupScriptNumber = 0;

    initAndLoadGlobalData();

	Anim *anim = new Anim(this);
	anim->load("A05.PAK", 21);
	//0, 11, 21

    _font->setColor(254);

	int seqIndex = 0;
	int seqCount = READ_LE_UINT32(anim->getSection(0)) / 4;
	int xp = 240, yp = 170;


    seqIndex = 12;
    

    memset(getScreen(), 0, 64000);
    anim->runSeq1(seqIndex, xp, yp);

    bool done = false;
    while (!done) {

		while (eventMan->pollEvent(event)) {
		switch (event.type) {
			case Common::EVENT_KEYDOWN:

				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
				    return 0;
				case Common::KEYCODE_LEFT:
                    if (seqIndex > 0) {
						seqIndex--;
                    	memset(getScreen(), 0, 64000);
                    	printf("seqIndex = %d\n", seqIndex);
                    	anim->runSeq1(seqIndex, xp, yp);
                    }
				    break;
				case Common::KEYCODE_RIGHT:
                    if (seqIndex + 1 < seqCount) {
						seqIndex++;
                    	memset(getScreen(), 0, 64000);
                    	printf("seqIndex = %d\n", seqIndex);
                    	anim->runSeq1(seqIndex, xp, yp);
                    }
				    break;
				}

				break;
			case Common::EVENT_KEYUP:
				break;
			case Common::EVENT_MOUSEMOVE:
				xp = event.mouse.x;
				yp = event.mouse.y;
				anim->runSeq1(seqIndex, xp, yp);
				break;
			case Common::EVENT_LBUTTONDOWN:
				break;
			case Common::EVENT_LBUTTONUP:
				break;
			case Common::EVENT_RBUTTONDOWN:
				break;
			case Common::EVENT_RBUTTONUP:
				break;
			case Common::EVENT_QUIT:
				// TODO
				//exit(0);
				return 0;
			default:
				break;
			}
		}

		char text[512];
		sprintf(text, "seqIndex = %d; x = %d; y = %d", seqIndex, xp, yp);
        _font->drawText(0, 0, getScreen(), text);

		_system->copyRectToScreen(getScreen(), 320, 0, 0, 320, 200);

		_system->updateScreen();
		_system->delayMillis(10);
    }

	//delete anim;
#endif

	return 0;
}

} // End of namespace Comet
