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
#include "comet/pak.h"
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

	// TODO: delete stuff at engine shutdown
	_music = new MusicPlayer(this);
	_screen = new Screen(this);
	_dialog = new Dialog(this);
	_script = new ScriptInterpreter(this);
	_scene = new Scene(this);
	_animationMan = new AnimationManager(this);
	_res = new ResourceManager();
	
	_textReader = new TextReader(this);
	_textReader->setTextFilename("E.CC4"); // TODO: Use language-specific filename
	
	_gui = new Gui(this);

	_soundResource = new SoundResource();

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

	initSystemVars();

	_talkieMode = 1;

	_startupModuleNumber = 9;
	_startupSceneNumber = 0;

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
	int frameListIndex = 0, frameIndex = 3;
	int resIndex = 6;
	//anim = _animationMan->loadAnimationResource("A05.PAK", 7);//FIRE
	anim = _animationMan->loadAnimationResource("A05.PAK", resIndex);
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
			anim = _animationMan->loadAnimationResource("A05.PAK", resIndex);
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
		x = 0; y = 0;
		//x = 141; 
		//y = 0;
		//x = _mouseX; 
		//y = _mouseY;
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

#if 1

#if 0
	// Play the intro
	introMainLoop();
#else	
	_moduleNumber = 9;
	_sceneNumber = 9;
#endif	

	waitForKeys();

	if (_currentModuleNumber == 5)
		_sceneNumber = 2;
	else if (_currentModuleNumber == 9)
		_sceneNumber = 9;				

	_screen->clear();
	_screen->update();

	debug("_sceneNumber = %d; _moduleNumber = %d", _sceneNumber, _moduleNumber);
	debug("_currentSceneNumber = %d; _currentModuleNumber = %d", _currentSceneNumber, _currentModuleNumber);

	gameMainLoop();
#endif	

	return Common::kNoError;
}

} // End of namespace Comet
