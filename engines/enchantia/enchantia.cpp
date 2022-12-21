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
 */

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/func.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"

#include "engines/util.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/fontman.h"
#include "graphics/font.h"

#include "enchantia/enchantia.h"
#include "enchantia/datfile.h"
#include "enchantia/decompress.h"
#include "enchantia/music.h"
#include "enchantia/resource.h"

namespace Enchantia {

static const MenuSlotAction kMenuSlotActions[] = {
	kMenuInventory,
	kMenuTake,
	kMenuUse,
	kMenuLook,
	kMenuTalk,
	kMenuFight,
	kMenuJump,
	kMenuDisk,
	kMenuActionSound,
	kMenuActionInfo,
	kMenuActionDiskLoad,
	kMenuActionDiskSave,
	kMenuActionDiskDelete,
	kMenuActionDiskNothing,
	kMenuActionDiskNothing,
	// 15
	kMenuActionUseCommand,
	kMenuActionUseCommand,
	kMenuActionUseCommand,
	kMenuActionUseCommand,
	kMenuActionUseCommand,
	kMenuActionUseCommand,
	kMenuActionUseCommand,
	kMenuActionUseCommand
};

EnchantiaEngine::EnchantiaEngine(OSystem *syst, const ADGameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc), _rnd("enchantia"),
	_stdSfx(NULL), _sceneSfx(NULL), _adlibMusic(NULL), _midiPlayer(NULL) {

}

EnchantiaEngine::~EnchantiaEngine() {
	DebugMan.removeAllDebugChannels();
}

void EnchantiaEngine::quit() {
	// TODO
}

void EnchantiaEngine::updateEvents() {
	Common::EventManager *eventMan = _system->getEventManager();
	_mouseButton = kNoButton;
	Common::Event event;
	while (eventMan->pollEvent(event)) {
		switch(event.type) {
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quit();
			break;
		case Common::EVENT_KEYDOWN:
			// DEBUG to switch scenes
			switch (event.kbd.keycode) {
			case Common::KEYCODE_PAGEUP:
				if (_currSceneLink < 126)
					_nextSceneLink = _currSceneLink + 1;
				break;
			case Common::KEYCODE_PAGEDOWN:
				if (_currSceneLink > 0)
					_nextSceneLink = _currSceneLink - 1;
				break;
			case Common::KEYCODE_LEFT:
				scrollScene(kScrollLeft);
				break;
			case Common::KEYCODE_RIGHT:
				scrollScene(kScrollRight);
				break;
			case Common::KEYCODE_HOME:
				actorSprite().setPos(_mouseX, _mouseY - actorSprite().height);
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButton = kLeftButton;
			_buttonDown = true;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButton = kRightButton;
			_buttonDown = true;
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_buttonDown = false;
			break;
		case Common::EVENT_WHEELUP:
			break;
		case Common::EVENT_WHEELDOWN:
			break;
		default:
			break;
		}
	}

}

Common::Error EnchantiaEngine::run() {
	syncSoundSettings();

	initGraphics(320, 200);

	_dirtyRects1 = new RectArray();
	_dirtyRects2 = new RectArray();

	_isSaveAllowed = false;

	_screen = new Graphics::Surface();
	_screen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	_background = new Graphics::Surface();
	_background->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	_menuSurface = new Graphics::Surface();
	_menuSurface->create(320, 32, Graphics::PixelFormat::createFormatCLUT8());

	_menuSprite = loadBitmap("menu.dat", 320, 32);

	_dat = new DatFile();
	_dat->load("enchantia.dat");

	_currRolandMusicFilenameIndex = -1;

	// Use either the AdLib emulator or a MIDI device
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	if (MidiDriver::getMusicType(dev) == MT_ADLIB) {
		debug("AdLib music");
		_adlibMusic = new AdlibMusicPlayer(_mixer);
	} else {
		debug("MIDI music");
		_midiPlayer = new MidiPlayer();
	}

	CursorMan.showMouse(true);

	_sceneMap = NULL;

	_stdBrd = new SpriteResource();
	_stdBrd->load("std.brd");

	_currBrdIndex = -1;
	_currBrd = new SpriteResource();

	_sceneSpr = new SpriteResource();

	_iconsSpr = new SpriteResource();
	_iconsSpr->load("icons.spr");

	_mouseSpr = new SpriteResource();
	_mouseSpr->load("mouse.spr");

	_fontSpr = new SpriteResource();
	_fontSpr->load("font.spr");

	loadRaw("std.sfx", &_stdSfx);
	_currSoundPriority = 0;

	// Initialize the cursor sprite
	cursorSprite().status = 1;
	cursorSprite().frameIndex = 0;
	cursorSprite().setPos(152, 92);
	cursorSprite().width = 16;
	cursorSprite().height = 16;
	cursorSprite().heightAdd = 0;
	cursorSprite().yAdd = 0;
	cursorSprite().id = -1;
	cursorSprite().anim.set(0, 0, 1, 1);
	cursorSprite().moveX.set(0, 0, 1, 1);
	cursorSprite().moveY.set(0, 0, 1, 1);
	cursorSprite().spriteResource = _mouseSpr;

	_currMouseSpriteIndex = -1;
	_currMouseCursorVisible = false;

	loadPalette("sprites.pal", _palette);

#if 0
	debug("Item ID -> Sprite mapping:");
	for (uint i = 0; i < _dat->getSceneItemInfoCount(); i++)
		debug("id %d -> %d", i, _dat->getSceneItemInfo(i).id);
#endif

	_talkTable[0] = idTalkHi;
	_talkTable[1] = idTalkHelp;
	for (uint i = 2; i < kTalkTableCount; i++)
		_talkTable[i] = -1;

	_talkTable[2] = idTalkOpenSesame;//DEBUG

	_actorQueuedAnim.set(0, 0, 1, 2);
	_actorQueuedMoveX.set(0, 0, 1, 2);
	_actorQueuedMoveY.set(0, 0, 1, 2);
	_queryBoxX1 = 0;
	_queryBoxY1 = 0;
	_queryBoxX2 = 0;
	_queryBoxY2 = 0;
	_queryBoxDefValue = 0;

	_animQueueCount = 0;

	_flags1 = 0;
	_flags2 = 0;
	_scene25Value = 0;
	_scene63BabySleepCounter = 0;
	_rockBasherCounters[0] = 4;
	_rockBasherCounters[1] = 4;
	_rockBasherCounters[2] = 4;
	_theScore = 0;
	_useItemId = idNone;

	clearInventory();
#if 0
	// TEST: Give some inventory items
	//addInventoryItem(idFishBowl);
	//addInventoryItem(idMeat);
	addInventoryItem(idGarlic);
	//addInventoryItem(idMarblesBag);
	//addInventoryItem(idTray);
	//addInventoryItem(idWoodenPlank);
	//addInventoryItem(idDice);
	//addInventoryItem(idDirtySock);
	//addInventoryItem(idLetter);
	addInventoryItem(idWishCoin);
	//addInventoryItem(idMoneyPouch);
	//addInventoryItem(idDress);
	//addInventoryItem(idLetter);
	//addInventoryItem(idStamp);
	addInventoryItem(idMagnifier);
	addInventoryItem(idGlass);
	addInventoryItem(idIceCube);
	addInventoryItem(idIcicle);
	//addInventoryItem(idCross);
	//addInventoryItem(idSockWithCoins);
	//addInventoryItem(idRock27);
	//addInventoryItem(idRock28);
	//addInventoryItem(idRock29);
#endif

	_sceneIndex = -1;
	loadScene(-1, false);
	//loadScene(26, false);
	//loadScene(69, false);
	//loadScene(61, false);

	if (_adlibMusic) {
		// There's only one AdLib music track which is looped infinitely
		_adlibMusic->init();
		_adlibMusic->play((const byte*)_dat->getMusic(1)->data, 0);
	}

#if 0
	playIntro();
#endif

#if 1
	gameLoop();
#endif

	// Free memory
	delete[] _stdSfx;
	delete[] _sceneSfx;
	delete _adlibMusic;
	delete _midiPlayer;
	delete _dat;
	delete _iconsSpr;
	delete _fontSpr;
	delete _sceneSpr;
	_menuSurface->free();
	delete _menuSurface;
	_menuSprite->free();
	delete _menuSprite;
	_screen->free();
	delete _screen;
	_background->free();
	delete _background;

	delete _dirtyRects1;
	delete _dirtyRects2;

	debug("Ok.");

	return Common::kNoError;
}

void EnchantiaEngine::setVgaPalette(byte *palette) {
	// TODO Rework
	byte xpalette[768];
	for (int j = 0; j < 768; j++)
		xpalette[j] = palette[j] << 2;
	_system->getPaletteManager()->setPalette(xpalette, 0, 256);
}

void EnchantiaEngine::palFadeOut() {
	const uint kPalFadeSteps = 32;
	byte *fadePal = new byte[768];
	uint16 *workPal = new uint16[768];
	uint16 *palSubs = new uint16[768];
	for (uint i = 0; i < 768; i++) {
		workPal[i] = _palette[i] << 8;
		palSubs[i] = workPal[i] / kPalFadeSteps;
	}
	for (uint step = 0; step < kPalFadeSteps; step++) {
		for (uint i = 0; i < 768; i++) {
			workPal[i] -= palSubs[i];
			fadePal[i] = workPal[i] >> 8;
		}
		setVgaPalette(fadePal);
		_system->updateScreen();
		_system->delayMillis(10);
	}
	delete[] palSubs;
	delete[] workPal;
	delete[] fadePal;
}

int EnchantiaEngine::getRandom(int max) {
	return _rnd.getRandomNumber(max - 1);
}

uint32 EnchantiaEngine::getMillis() {
	return _system->getMillis();
}

void EnchantiaEngine::gameLoop() {

	//DEBUG
	//_flgCanRunBoxClick = true;
	//_flgWalkBoxEnabled = true; // Enabled by animation
	_nextSceneLink = -1;
	//--DEBUG

	while (!shouldQuit()) {
		int16 currActorX, currActorY;

		_isSaveAllowed = true;

		updateEvents();

		if (_flgCanRunMenu && _mouseButton == kRightButton) {
			runMenuBar();
			_mouseButton = kNoButton;
			_buttonDown = false;
		}

		updateSceneBackground();
		currActorX = actorSprite().x;
		currActorY = actorSprite().y;

		// DEBUG
		if (_nextSceneLink >= 0) {
			loadScene(_nextSceneLink, false);
			_nextSceneLink = -1;
			continue;
		}

		// TODO Handle keyboard

		if (_flgCanRunBoxClick && _mouseButton == kLeftButton) {
			checkBoxClick();
			if (_currWalkInfo)
				startActorWalk(_currWalkInfo);
		}

		updateActorWalk();
		updateAnimations();

		if (_nextSceneLink >= 0) {
			loadScene(_nextSceneLink, false);
			_nextSceneLink = -1;
			continue;
		}

		if (_flgWalkBoxEnabled) {
			checkWalkBoxes(currActorX, currActorY);
		}

		for (uint i = 9; i > 0; --i)
			_actorXTable[i] = _actorXTable[i - 1];
		_actorXTable[0] = actorSprite().x;

		updateSprites();
		updateScene();

	}

}

SpriteResource *EnchantiaEngine::loadBrd(int16 index) {
	if (index != _currBrdIndex) {
		const Common::String &brdFilename = _dat->getBrdFilename(index);
		_currBrd->load(brdFilename.c_str());
	}
	return _currBrd;
}

void EnchantiaEngine::initializeSprite(Sprite *sprite, SpriteDef *spriteDef, int16 cameraX) {
	Graphics::Surface *spriteFrame = _sceneSpr->getFrame(spriteDef->frameIndex & 0x7F);
	SpriteTemplate *spriteTemplate = _dat->getSpriteTemplate(spriteDef->templateId);

	sprite->status = spriteDef->status;
	sprite->frameIndex = spriteDef->frameIndex;
	sprite->x = spriteDef->x - cameraX;
	sprite->y = spriteDef->y;
	sprite->width = spriteFrame->w;
	sprite->height = spriteFrame->h;
	sprite->heightAdd = spriteTemplate->heightAdd;
	sprite->yAdd = spriteTemplate->yAdd;
	sprite->id = spriteTemplate->id;
	sprite->anim.set(spriteTemplate->animListId, 0,
		spriteTemplate->animListTicks, spriteTemplate->animListInitialTicks);
	sprite->moveX.set(spriteTemplate->moveXListId, 0,
		spriteTemplate->moveXListTicks, spriteTemplate->moveXListInitialTicks);
	sprite->moveY.set(spriteTemplate->moveYListId, 0,
		spriteTemplate->moveYListTicks, spriteTemplate->moveYListInitialTicks);
	sprite->spriteResource = _sceneSpr;
	if (spriteDef->type & 2) {
		// Select random list index
		byte *code = _dat->getAnimCode(sprite->anim.codeId);
		byte count = 1;
		while (*code++ != 0x7F)
			count++;
		sprite->anim.index = getRandom(count);
		sprite->moveX.index = sprite->anim.index;
		sprite->moveY.index = sprite->anim.index;
	}
	if (spriteDef->type & 4) {
		// Select random ticks
		byte randomTicks = getRandom(sprite->anim.initialTicks);
		sprite->anim.ticks = randomTicks;
		sprite->moveX.ticks = randomTicks;
		sprite->moveY.ticks = randomTicks;
	}

}

Sprite *EnchantiaEngine::insertSprite(SpriteDef *spriteDef) {
	Sprite *sprite = NULL;
	// Find an empty sprite slot
	for (uint i = _sceneSpritesCount; i < kSpriteCount; i++)
		if (_sprites[i].status == 0) {
			sprite = &_sprites[i];
			break;
		}
	if (sprite)
		initializeSprite(sprite, spriteDef);
	return sprite;
}

void EnchantiaEngine::insertSpriteList(const SpriteListItem *spriteList, uint count, uint16 spriteDefId, uint spriteIndex, byte ticks) {
	Sprite *newSprite;
	Sprite *sprite = &_sprites[spriteIndex];
	SpriteDef *spriteDef = _dat->getSpriteDef(spriteDefId);
	SpriteTemplate *spriteTemplate = _dat->getSpriteTemplate(spriteDef->templateId);
	for (uint i = 0; i < count; i++) {
		spriteDef->setPos(spriteList[i].x + sprite->x, spriteList[i].y + sprite->y);
		spriteTemplate->moveXListId = spriteList[i].moveXListId;
		spriteTemplate->moveYListId = spriteList[i].moveYListId;
		newSprite = insertSprite(spriteDef);
		if (newSprite)
			newSprite->anim.ticks = ticks;
	}
}

void EnchantiaEngine::loadScene(int16 sceneLink, bool isLoadGame) {

	static const int16 kLinkToSceneMap[] = {
		0, 1, 2, 0, 3, 8, 5, 9, 6, 4, 10, 5, 7, 11, 12, 6, 13,
		14, 4, 4, 5, 6, 6, 7, 7, 15, 16, 21, 24, 18, 19, 20,
		21, 22, 23, 17, 24, 25, 17, 26, 27, 17, 28, 29, 17,
		18, 18, 19, 30, 20, 20, 21, 21, 35, 33, 32, 34, 33,
		32, 35, 34, 34, 35, 36, 33, 37, 32, 38, 35, 39, 16,
		32, 33, 31, 27, 43, 41, 40, 42, 41, 43, 42, 44, 45,
		46, 47, 46, 48, 47, 49, 48, 56, 50, 47, 51, 47, 52,
		48, 53, 48, 54, 49, 55, 49, 57, 58, 59, 60, 63, 61,
		62, 58, 24, 64, 65, 77, 65, 66, 67, 68, 69, 70, 69,
		71, 69, 72, 73
	};

	static const int32 kSceneToRolandMusicMap[] = {
		0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3,
		3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 3, 3, 3, 3,
		0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 3, 0, 0, 0,
		0, 4, 0, 0, 0, 1
	};

	static const struct {
		int32 tempo;
		const char *filename;
	} kRolandMusicFilenames[] = {
		{140, "curse.rs1"},
		{135, "curse.rs2"},
		{140, "curse.rs3"},
		{125, "curse.rs4"},
		{110, "curse.rs5"}
	};

#if 0
	// DEBUG >>>
	for (int16 i = 0; i < ARRAYSIZE(kLinkToSceneMap); i++) {
		if (kLinkToSceneMap[i] == 75) {
			sceneLink = i - 1;
			break;
		}
	}
	// <<< DEBUG
#endif

	_currSceneLink = sceneLink;

	debug("loading scene %d", sceneLink);

	cursorSprite().frameIndex = 0;
	cursorSprite().anim.codeId = 0;

	_mixer->stopHandle(_soundHandle);
	palFadeOut();

	_screen->fillRect(Common::Rect(0, 0, 320, 200), 0);

	if (isLoadGame) {
		// Don't initialize the main actor sprite because it's been loaded from the savegame
	} else {
		// Initialize the main actor sprite
		SceneInitItem &sceneInitItem = _dat->getSceneInitItem(sceneLink + 1); // sceneLink may be -1
		handleSceneInit(sceneInitItem, sceneLink);
		actorSprite().status = sceneInitItem.status;
		actorSprite().frameIndex = sceneInitItem.frameIndex;
		actorSprite().setPos(sceneInitItem.x, sceneInitItem.y);
		actorSprite().width = 32;
		actorSprite().height = 48;
		actorSprite().heightAdd = 0;
		actorSprite().yAdd = 0;
		actorSprite().id = 0;
		actorSprite().anim.set(sceneInitItem.animListId, 0, 1, 1);
		actorSprite().moveX.set(sceneInitItem.moveXListId, 0, 1, 1);
		actorSprite().moveY.set(sceneInitItem.moveYListId, 0, 1, 1);
		_cameraStripNum = sceneInitItem.cameraStripNum;
		_cameraStripX = _cameraStripNum * 32;
		//debug("_cameraStripNum = %d; _cameraStripX = %d", _cameraStripNum, _cameraStripX);
		_spriteResourceType = sceneInitItem.spriteResourceType;
		// Fill the weird actorXTable
		for (uint i = 0; i < 10; i++)
			_actorXTable[i] = actorSprite().x;
	}

	if (_spriteResourceType == 255) {
		actorSprite().spriteResource = _stdBrd;
	} else {
		actorSprite().anim.initialTicks = 2;
		actorSprite().moveX.initialTicks = 2;
		actorSprite().moveY.initialTicks = 2;
		if (_spriteResourceType == 254) {
			/*
			actorSprite().spriteResource = _sceneSpr;
			actorSprite().width = _sceneSpr->getFrame(actorSprite().frameIndex & 0x7F)->w;
			actorSprite().height = _sceneSpr->getFrame(actorSprite().frameIndex & 0x7F)->h;
			*/
		} else {
			actorSprite().spriteResource = loadBrd(_spriteResourceType);
			actorSprite().width = actorSprite().spriteResource->getFrame(actorSprite().frameIndex & 0x7F)->w;
			actorSprite().height = actorSprite().spriteResource->getFrame(actorSprite().frameIndex & 0x7F)->h;
		}
	}

	if (_sceneIndex != kLinkToSceneMap[sceneLink + 1]) {
		// Only load if needed (Some scenes use the same resources)
		_sceneIndex = kLinkToSceneMap[sceneLink + 1];
		const SceneFilenames &sceneFilenames = _dat->getSceneFilenames(_sceneIndex);
		debug("%s, %s, %s", sceneFilenames.mapFilename.c_str(), sceneFilenames.sprFilename.c_str(), sceneFilenames.sfxFilename.c_str());
		if (_sceneIndex == kSceneTheEnd) {
			clearInventory();
			_talkTable[0] = -1;
			_talkTable[1] = -1;
		}
		// Play the scene music if it's different from the music currently playing
		if (_midiPlayer && kSceneToRolandMusicMap[_sceneIndex] != _currRolandMusicFilenameIndex) {
			_currRolandMusicFilenameIndex = kSceneToRolandMusicMap[_sceneIndex];
			// TODO Set music tempo kRolandMusicFilenames[_currRolandMusicFilenameIndex].tempo
			_midiPlayer->playMusic(kRolandMusicFilenames[_currRolandMusicFilenameIndex].filename);
		}
		_sceneStripTable = _dat->getSceneStrips(_sceneIndex);
		_sceneStripTableCount = *_sceneStripTable++;
		if (_sceneStripTableCount - 10 - _cameraStripNum < 0) {
			// CHECKME/Rework
			int16 stripD = -(_sceneStripTableCount - 10 - _cameraStripNum);
			_cameraStripNum -= stripD;
			actorSprite().x += stripD * 32;
		}
		// Load scene map
		loadSceneMap(sceneFilenames.mapFilename.c_str());
		// Load scene sprites
		_sceneSpr->load(sceneFilenames.sprFilename.c_str());
		if (_spriteResourceType == 254) {
			actorSprite().spriteResource = _sceneSpr;
			actorSprite().width = _sceneSpr->getFrame(actorSprite().frameIndex & 0x7F)->w;
			actorSprite().height = _sceneSpr->getFrame(actorSprite().frameIndex & 0x7F)->h;
		}
		// Load scene sounds
		if (sceneFilenames.sfxFilename.size())
			loadRaw(sceneFilenames.sfxFilename.c_str(), &_sceneSfx);
		else {
			delete[] _sceneSfx;
			_sceneSfx = NULL;
		}
		// Load scene boxes
		loadWalkBoxes(_sceneIndex);
	}

	for (int16 stripNum = 0; stripNum < 10; stripNum++)
		drawStrip(_sceneStripTable[_cameraStripNum + stripNum], stripNum * 32);

	if (!isLoadGame) {
		_scrollBorderLeft = 64;
		_scrollBorderRight = 223;
		_flgCanRunBoxClick = false;
		_flgCanRunMenu = false;
		_flgWalkBoxEnabled = false;
		_collectItemIgnoreActorPosition = false;
		_walkDistance = 0;
		_currWalkInfo = NULL;
		_screenShakeStatus = 0;
		initSceneSprites();
	}

	memcpy(_screen->getPixels(), _background->getPixels(), 320 * 200);
	_system->copyRectToScreen((const byte*)_screen->getBasePtr(0, 0), 320, 0, 0, 320, 200);

	_dirtyRects1->clear();
	_dirtyRects2->clear();

	drawSprites();
	setVgaPalette(_palette);

	debug("_sceneIndex is %d; sceneLink is %d", _sceneIndex, sceneLink);

}

void EnchantiaEngine::loadSceneMap(const char *filename) {
	Common::File fd;
	uint32 sceneMapSize;
	delete[] _sceneMap;
	if (!fd.open(filename))
		error("EnchantiaEngine::loadSceneMap() Could not open %s", filename);
	fd.read(_palette, 576);
	sceneMapSize = fd.readUint16LE() * 16;
	_sceneMap = new byte[sceneMapSize];
	fd.read(_sceneMap, sceneMapSize);
	fd.close();
}

void EnchantiaEngine::initSceneSprites() {
	int16 cameraX = _cameraStripNum * 32;
	SpriteDefList &spriteDefList = _dat->getSceneSpriteDefTable(_sceneIndex);
	// Disable all scene sprites
	for (uint i = 2; i < kSpriteCount; i++)
		_sprites[i].status = 0;
	_sceneCountdown1 = 1;
	_sceneSpritesCount = 2;
	_sceneDecorationCount = 0;
	for (uint i = 0; i < spriteDefList.size(); i++) {
		SpriteDef *spriteDef = _dat->getSpriteDef(spriteDefList[i]);
		if (spriteDef->type == 0) {
			// Add scene decoration
			_sceneDecorations[_sceneDecorationCount].x = spriteDef->x - cameraX;
			_sceneDecorations[_sceneDecorationCount].y = spriteDef->y;
			_sceneDecorations[_sceneDecorationCount].frameIndex = spriteDef->frameIndex;
			_sceneDecorations[_sceneDecorationCount].spriteResource = _sceneSpr;
			_sceneDecorationCount++;
		} else {
			// TODO: Check quality setting? Probably not needed speed-wise
			initializeSprite(&_sprites[_sceneSpritesCount++], spriteDef, cameraX);
		}
	}
}

void EnchantiaEngine::buildDrawItems() {
	for (uint i = 0; i < kSpriteCount - 1; i++) {
		Sprite *sprite = &_sprites[i + 1];
		_drawItems[i].sprite = sprite;
		_drawItems[i].priority = sprite->y + sprite->height + sprite->heightAdd;
	}
	// Sort the sprites by priority using the stable insertion sort, else the sprites may flicker
	int j;
	for (int i = 1; i < (int)kSpriteCount - 1; i++) {
		DrawItem temp = _drawItems[i];
		j = i - 1;
		while (temp.priority < _drawItems[j].priority && j >= 0) {
			_drawItems[j + 1] = _drawItems[j];
			j = j - 1;
		}
		_drawItems[j + 1] = temp;
	}
}

void EnchantiaEngine::drawSprites() {
	buildDrawItems(); // TODO Maybe merge with here

	for (uint i = 0; i < _sceneDecorationCount; i++)
		_sceneDecorations[i].redraw = false;

	for (uint i = 0; i < kSpriteCount - 1; i++) {
		Sprite *sprite = _drawItems[i].sprite;
		if (sprite->status && !(sprite->status & 2) && sprite->frameIndex != 255) {
			Common::Rect clipRect;
			drawSprite(_screen, sprite->spriteResource->getFrame(sprite->frameIndex & 0x7F),
				sprite->x, sprite->y, sprite->frameIndex & 0x80, &clipRect);
			if (clipRect.width() > 0 && clipRect.height() > 0)
				_dirtyRects2->push_back(clipRect);
			for (uint j = 0; j < _sceneDecorationCount; j++) {
				SceneDecoration &sceneDecoration = _sceneDecorations[j];
				if (!sceneDecoration.redraw) {
					Common::Rect decorationRect = Common::Rect(sceneDecoration.x, sceneDecoration.y,
						sceneDecoration.x + sceneDecoration.spriteResource->getFrame(sceneDecoration.frameIndex & 0x7F)->w,
						sceneDecoration.y + sceneDecoration.spriteResource->getFrame(sceneDecoration.frameIndex & 0x7F)->h);
					if (clipRect.intersects(decorationRect))
						sceneDecoration.redraw = true;
				}
			}
		}
	}

	// Draw scene decorations; only draw one if a sprite intersects it
	for (uint i = 0; i < _sceneDecorationCount; i++) {
		SceneDecoration &sceneDecoration = _sceneDecorations[i];
		if (sceneDecoration.redraw)
			drawSprite(_screen, sceneDecoration.spriteResource->getFrame(sceneDecoration.frameIndex & 0x7F),
				sceneDecoration.x, sceneDecoration.y, sceneDecoration.frameIndex & 0x80);
	}

}

void EnchantiaEngine::drawSurface(Graphics::Surface *destSurface, Graphics::Surface *surface, int16 x, int16 y) {
	// TODO Clipping
	// TODO Merge with drawSprite
	byte *src = (byte*)surface->getPixels();
	byte *dst = (byte*)destSurface->getBasePtr(x, y);
	int h = surface->h;
	while (h--) {
		memcpy(dst, src, surface->w);
		src += surface->pitch;
		dst += destSurface->pitch;
	}
}

void EnchantiaEngine::copySurfaceRect(Graphics::Surface *destSurface, Graphics::Surface *surface,
	Common::Rect &r) {
	byte *src = (byte*)surface->getBasePtr(r.left, r.top);
	byte *dst = (byte*)destSurface->getBasePtr(r.left, r.top);
	int w = r.width();
	int h = r.height();
	while (h--) {
		memcpy(dst, src, w);
		src += surface->pitch;
		dst += destSurface->pitch;
	}
}

void EnchantiaEngine::drawSprite(Graphics::Surface *destSurface, Graphics::Surface *sprite, int16 x, int16 y, byte flags,
	Common::Rect *clipRect) {

	int16 sourceX = 0, sourceY = 0;
	int16 w = sprite->w, h = sprite->h;

	// TODO FIXME Left clipping for flipped sprites

	// Reject sprites not inside the screen
	if (x >= destSurface->w || y >= destSurface->h || x + sprite->w < 0 || y + sprite->h < 0)
		return;

	if (x < 0) {
		sourceX = -x;
		w -= sourceX;
		x = 0;
	}

	if (y < 0) {
		sourceY = -y;
		h -= sourceY;
		y = 0;
	}

	if (x + w >= destSurface->w)
		w -= (x + w) - destSurface->w;

	if (y + h >= destSurface->h)
		h -= (y + h) - destSurface->h;

	if (clipRect)
		*clipRect = Common::Rect(x, y, x + w, y + h);

	if (w == 0 || h == 0)
		return;

	byte *src = (byte*)sprite->getBasePtr(sourceX, sourceY);
	byte *dst = (byte*)destSurface->getBasePtr(x, y);
	while (h--) {
		if (flags) {
			for (int xc = 0; xc < w; xc++) {
				if (src[xc] != 0)
					dst[w - xc - 1] = src[xc];
			}
		} else {
			for (int xc = 0; xc < w; xc++) {
				if (src[xc] != 0)
					dst[xc] = src[xc];
			}
		}
		src += sprite->pitch;
		dst += destSurface->pitch;
	}
}

void EnchantiaEngine::drawStrip(int16 stripNum, int16 x) {
	Graphics::Surface *strip = new Graphics::Surface();
	strip->create(32, 200, Graphics::PixelFormat::createFormatCLUT8());
	if (stripNum != 255) {
		byte *stripSource = _sceneMap + 16 * READ_LE_UINT16(_sceneMap + stripNum * 2);
		unpackRnc(stripSource, (byte*)strip->getPixels());
	}
	drawSurface(_background, strip, x, 0);
	strip->free();
	delete strip;
}

void EnchantiaEngine::drawMouseCursor() {
	if (_currMouseCursorVisible && cursorSprite().status == 0) {
		CursorMan.showMouse(false);
		_currMouseCursorVisible = false;
	} else if (!_currMouseCursorVisible && cursorSprite().status) {
		CursorMan.showMouse(true);
		_currMouseCursorVisible = true;
	}
	if (cursorSprite().frameIndex != _currMouseSpriteIndex) {
		Graphics::Surface *cursorSurface = _mouseSpr->getFrame(cursorSprite().frameIndex);
		CursorMan.replaceCursor((const byte*)cursorSurface->getPixels(),
			cursorSurface->w, cursorSurface->h, 0, 0, 0);
		_currMouseSpriteIndex = cursorSprite().frameIndex;
	}
}

void EnchantiaEngine::queueWalk(int16 x, int16 y) {
	_walkInfos[0].x = x;
	_walkInfos[0].y = y;
}

void EnchantiaEngine::queueWalkToSprite(uint spriteIndex, int16 deltaX, int16 deltaY) {
	queueWalk(_sprites[spriteIndex].x + deltaX, _sprites[spriteIndex].y + deltaY);
}

void EnchantiaEngine::queueActor(uint16 animListId, uint16 moveXListId, uint16 moveYListId) {
	_actorQueuedAnim.codeId = animListId;
	_actorQueuedMoveX.codeId = moveXListId;
	_actorQueuedMoveY.codeId = moveYListId;
}

void EnchantiaEngine::queueAnim(uint spriteIndex, byte ticks, byte initialTicks,
	uint16 animListId, uint16 moveXListId, uint16 moveYListId) {
	_animQueue[_animQueueCount].sprite = &_sprites[spriteIndex];
	_animQueue[_animQueueCount].ticks = ticks;
	_animQueue[_animQueueCount].initialTicks = initialTicks;
	_animQueue[_animQueueCount].animListId = animListId;
	_animQueue[_animQueueCount].moveXListId = moveXListId;
	_animQueue[_animQueueCount].moveYListId = moveYListId;
	_animQueueCount++;
}

void EnchantiaEngine::updateSpriteAnim(Sprite &sprite) {
	if (sprite.anim.codeId && --sprite.anim.ticks <= 0) {
		bool breakLoop = false;
		byte randomRange, randomBase, soundNum;
		int newFrameIndex = -1;
		sprite.anim.ticks = sprite.anim.initialTicks;
		while (!breakLoop) {
			byte *code = _dat->getAnimCode(sprite.anim.codeId);
			byte opcode = code[sprite.anim.index++];
			switch (opcode) {
			case 0x7F:
				sprite.anim.index = 0;
				break;
			case 0x7E:
				sprite.anim.codeId = READ_LE_UINT16(code + sprite.anim.index);
				sprite.anim.index = 0;
				break;
			case 0x7D:
				sprite.status = 0;
				sprite.anim.codeId = 0;
				sprite.moveX.codeId = 0;
				sprite.moveY.codeId = 0;
				breakLoop = true;
				break;
			case 0x7C:
				sprite.anim.codeId = 0;
				breakLoop = true;
				break;
			case 0x7B:
				sprite.anim.ticks = code[sprite.anim.index++] * sprite.anim.initialTicks;
				breakLoop = true;
				break;
			case 0x7A:
				sprite.anim.initialTicks = code[sprite.anim.index++];
				sprite.anim.ticks = sprite.anim.initialTicks;
				break;
			case 0x79:
				sprite.anim.index -= 2;
				break;
			case 0x78:
				randomRange = code[sprite.anim.index++];
				randomBase = code[sprite.anim.index++];
				sprite.anim.ticks = randomBase + getRandom(randomRange);
				sprite.moveX.ticks = sprite.anim.ticks;
				sprite.moveY.ticks = sprite.anim.ticks;
				breakLoop = true;
				break;
			case 0xFF:
				sprite.status &= ~2;
				breakLoop = true;
				break;
			case 0xFE:
				sprite.status |= 2;
				breakLoop = true;
				break;
			case 0xFD:
				_flgCanRunBoxClick = true;
				_flgCanRunMenu = true;
				_flgWalkBoxEnabled = true;
				break;
			case 0xFC:
				_flgCanRunMenu = true;
				break;
			case 0xFB:
				soundNum = code[sprite.anim.index++];
				playSound(soundNum, sprite);
				break;
			case 0xFA:
				_screenShakeStatus = 1;
				break;
			case 0xF9:
				_screenShakeStatus = 0;
				break;
			case 0xF8:
				_spriteResourceType = code[sprite.anim.index++];
				if (_spriteResourceType == 255) {
					sprite.spriteResource = _stdBrd;
					sprite.anim.initialTicks = 1;
					sprite.anim.ticks = 1;
					sprite.moveX.initialTicks = 1;
					sprite.moveY.initialTicks = 1;
				} else if (_spriteResourceType == 254) {
					sprite.spriteResource = _sceneSpr;
					sprite.anim.initialTicks = 2;
					sprite.anim.ticks = 2;
					sprite.moveX.initialTicks = 2;
					sprite.moveY.initialTicks = 2;
				} else {
					sprite.spriteResource = loadBrd(_spriteResourceType);
					sprite.anim.initialTicks = 2;
					sprite.anim.ticks = 2;
					sprite.moveX.initialTicks = 2;
					sprite.moveY.initialTicks = 2;
				}
				break;
			case 0xF7:
				randomRange = code[sprite.anim.index++];
				randomBase = code[sprite.anim.index++];
				do {
					newFrameIndex = randomBase + getRandom(randomRange);
				} while (newFrameIndex == sprite.frameIndex);
				breakLoop = true;
				break;
			case 0xF6:
				_nextSceneLink = code[sprite.anim.index++];
				breakLoop = true;
				break;
			default:
				newFrameIndex = opcode;
				breakLoop = true;
				break;
			}
		}
		if (newFrameIndex >= 0) {
			sprite.frameIndex = newFrameIndex;
			sprite.width = sprite.spriteResource->getFrame(sprite.frameIndex & 0x7F)->w;
			sprite.height = sprite.spriteResource->getFrame(sprite.frameIndex & 0x7F)->h;
		}
	}
}

void EnchantiaEngine::updateSpriteMoveX(Sprite &sprite) {
	if (sprite.moveX.codeId && --sprite.moveX.ticks <= 0) {
		bool breakLoop = false;
		byte randomRange, randomBase;
		int16 xAccu = 0, xDelta = 0;
		sprite.moveX.ticks = sprite.moveX.initialTicks;
		while (!breakLoop) {
			byte *code = _dat->getMoveXCode(sprite.moveX.codeId);
			byte opcode = code[sprite.moveX.index++];
			switch (opcode) {
			case 0x80:
				xAccu += 100;
				break;
			case 0x7F:
				sprite.moveX.index = 0;
				break;
			case 0x7E:
				sprite.moveX.codeId = READ_LE_UINT16(code + sprite.moveX.index);
				sprite.moveX.index = 0;
				break;
			case 0x7D:
				sprite.status = 0;
				sprite.anim.codeId = 0;
				sprite.moveX.codeId = 0;
				sprite.moveY.codeId = 0;
				breakLoop = true;
				break;
			case 0x7C:
				sprite.moveX.codeId = 0;
				breakLoop = true;
				break;
			case 0x7B:
				sprite.moveX.ticks = code[sprite.moveX.index++] * sprite.moveX.initialTicks;
				breakLoop = true;
				break;
			case 0x7A:
				sprite.moveX.initialTicks = code[sprite.moveX.index++];
				sprite.moveX.ticks = sprite.moveX.initialTicks;
				break;
			case 0x79:
				sprite.moveX.index -= 2;
				break;
			case 0x78:
				randomRange = code[sprite.moveX.index++];
				randomBase = code[sprite.moveX.index++];
				sprite.moveX.ticks = randomBase + getRandom(randomRange);
				sprite.moveY.ticks = sprite.moveX.ticks;
				breakLoop = true;
				break;
			default:
				xDelta = (int8)opcode;
				if (xDelta < 0)
					xDelta -= xAccu;
				else
					xDelta += xAccu;
				sprite.x += xDelta;
				breakLoop = true;
				break;
			}
		}
	}
}

void EnchantiaEngine::updateSpriteMoveY(Sprite &sprite) {
	if (sprite.moveY.codeId && --sprite.moveY.ticks <= 0) {
		bool breakLoop = false;
		byte randomRange, randomBase;
		int16 yAccu = 0, yDelta = 0;
		sprite.moveY.ticks = sprite.moveY.initialTicks;
		while (!breakLoop) {
			byte *code = _dat->getMoveYCode(sprite.moveY.codeId);
			byte opcode = code[sprite.moveY.index++];
			switch (opcode) {
			case 0x80:
				yAccu += 100;
				break;
			case 0x7F:
				sprite.moveY.index = 0;
				break;
			case 0x7E:
				sprite.moveY.codeId = READ_LE_UINT16(code + sprite.moveY.index);
				sprite.moveY.index = 0;
				break;
			case 0x7D:
				sprite.status = 0;
				sprite.anim.codeId = 0;
				sprite.moveY.codeId = 0;
				breakLoop = true;
				break;
			case 0x7C:
				sprite.moveY.codeId = 0;
				breakLoop = true;
				break;
			case 0x7B:
				sprite.moveY.ticks = code[sprite.moveY.index++] * sprite.moveY.initialTicks;
				breakLoop = true;
				break;
			case 0x7A:
				sprite.moveY.initialTicks = code[sprite.moveY.index++];
				sprite.moveY.ticks = sprite.moveY.initialTicks;
				break;
			case 0x79:
				sprite.moveY.index -= 2;
				break;
			case 0x78:
				randomRange = code[sprite.moveY.index++];
				randomBase = code[sprite.moveY.index++];
				sprite.moveY.ticks = randomBase + getRandom(randomRange);
				breakLoop = true;
				break;
			default:
				yDelta = (int8)opcode;
				if (yDelta < 0)
					yDelta -= yAccu;
				else
					yDelta += yAccu;
				sprite.y += yDelta;
				breakLoop = true;
				// Weird
				if (sprite.y >= 300 && sprite.y + sprite.height <= -100)
					sprite.status = 0;
				break;
			}
		}
	}
}

void EnchantiaEngine::updateAnimations() {
	for (uint i = 0; i < kSpriteCount; i++) {
		Sprite &sprite = _sprites[i];
		if (sprite.status != 0) {
			updateSpriteAnim(sprite);
			updateSpriteMoveX(sprite);
			updateSpriteMoveY(sprite);
		}
	}
	// TODO: Disable sprites out of x/y bounds
}

void EnchantiaEngine::updateSceneBackground() {

	// Restore the background
	for (RectArray::iterator it = _dirtyRects1->begin(); it != _dirtyRects1->end(); ++it)
		copySurfaceRect(_screen, _background, *it);

	while (1) {
		int16 actorX = actorSprite().x + actorSprite().width / 2;
		if (actorX <= _scrollBorderLeft && _cameraStripNum > 0)
			scrollScene(kScrollLeft);
		else if (actorX >= _scrollBorderRight && _cameraStripNum + 10 < _sceneStripTableCount)
			scrollScene(kScrollRight);
		else {
			drawSprites();
			break;
		}
	}
	drawMouseCursor();

	switch (_screenShakeStatus) {
	case 0:
		_system->setShakePos(0, 0);
		break;
	case 1:
		_system->setShakePos(0, 2);
		_screenShakeStatus = 2;
		break;
	case 2:
		_system->setShakePos(0, 0);
		_screenShakeStatus = 1;
		break;
	}

	// Draw background and sprite rects to the screen

	for (RectArray::iterator it = _dirtyRects1->begin(); it != _dirtyRects1->end(); ++it) {
		Common::Rect &r = *it;
		_system->copyRectToScreen((const byte*)_screen->getBasePtr(r.left, r.top), 320, r.left, r.top, r.width(), r.height());
	}

	for (RectArray::iterator it = _dirtyRects2->begin(); it != _dirtyRects2->end(); ++it) {
		Common::Rect &r = *it;
		_system->copyRectToScreen((const byte*)_screen->getBasePtr(r.left, r.top), 320, r.left, r.top, r.width(), r.height());
	}

	SWAP(_dirtyRects1, _dirtyRects2);

	_dirtyRects2->clear();

#if 0
	debugDrawWalkBoxes();
	debugDrawSceneItems();
	// Full screen update needed here
	_system->copyRectToScreen((const byte*)_screen->getBasePtr(0, 0), 320, 0, 0, 320, 200);
#endif

	_system->updateScreen();
	_system->delayMillis(80);

}

void EnchantiaEngine::debugDrawWalkBoxes() {
	// DEBUG: Draw some walk boxes
	for (uint i = 0; i < _walkBoxes.size(); i++) {
		const WalkBox &walkBox = _walkBoxes[i];
		byte color = 0;
		switch (walkBox.type) {
		case 0:
			color = 0xE0;
			break;
		case 1:
			color = 0xE3;
			break;
		case 2:
			color = 0xE6;
			break;
		case 3:
			color = 0xD7;
			break;
		default:
			color = 0xFF;
		}

		int16 x1 = walkBox.x1 - _cameraStripX, x2 = walkBox.x2 - _cameraStripX;
		int16 y1 = walkBox.y1, y2 = walkBox.y2;
		_screen->hLine(x1, y1, x2 - 1, color);
		_screen->hLine(x1, y2 - 1, x2 - 1, color);
		_screen->vLine(x1, y1, y2 - 1, color);
		_screen->vLine(x2 - 1, y1, y2 - 1, color);
	}
}

void EnchantiaEngine::debugDrawSceneItems() {
	// DEBUG: Show all usable items in the current scene
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	int16 itemX = 5;
	int16 x1 = actorSprite().x - 5;
	int16 x2 = actorSprite().x + actorSprite().width - 1 + 5;
	int16 y1 = actorSprite().y + actorSprite().height - 4 - 4;
	int16 y2 = actorSprite().y + actorSprite().height - 1 + 4;
	// itemFlags: look/use=1;take=2;jump=5
	for (uint i = 2; i < kSpriteCount; i++) {
		Sprite &sprite = _sprites[i];
		if (sprite.status && ((sprite.status & 4) || !(sprite.status & 2)) &&
			sprite.id >= 0 && sprite.id < 0x400 &&
			((_dat->getSceneItemInfo(sprite.id).flags & 1) ||
				(_dat->getSceneItemInfo(sprite.id).flags & 2) ||
				(_dat->getSceneItemInfo(sprite.id).flags & 5))) {
			byte color = 224;
			// Change color if we're close enough
			if (sprite.x <= x2 && sprite.x + sprite.width > x1 &&
				sprite.y - sprite.yAdd <= y2 && sprite.y + sprite.height + sprite.yAdd > y1)
				color = 216;
			_screen->hLine(sprite.x, sprite.y, sprite.x + sprite.width - 1, color);
			_screen->hLine(sprite.x, sprite.y + sprite.height - 1, sprite.x + sprite.width - 1, color);
			_screen->vLine(sprite.x, sprite.y, sprite.y + sprite.height - 1, color);
			_screen->vLine(sprite.x + sprite.width - 1, sprite.y, sprite.y + sprite.height - 1, color);
			int16 w = _iconsSpr->getFrame(_dat->getSceneItemInfo(sprite.id).frameIndex)->w;
			Common::String s = Common::String::format("ID: %d", sprite.id);
			font->drawString(_screen, s, itemX, 5 + 24 + 2, _screen->w - itemX, 254);
			w = MAX<int16>(w, font->getStringWidth(s));
			if (sprite.x <= x2 && sprite.x + sprite.width > x1 &&
				sprite.y - sprite.yAdd <= y2 && sprite.y + sprite.height + sprite.yAdd > y1)
				_screen->fillRect(Common::Rect(itemX, 5, itemX + w, 5 + 24), 216);
			if (_mouseX >= sprite.x && _mouseX <= sprite.x + sprite.width - 1 &&
				_mouseY >= sprite.y && _mouseY <= sprite.y + sprite.height - 1) {
				_screen->fillRect(Common::Rect(itemX, 5, itemX + w, 5 + 24), 223);
				debugN("- item id: %d", sprite.id);
				if ((_dat->getSceneItemInfo(sprite.id).flags & 1))
					debugN(" USE");
				if ((_dat->getSceneItemInfo(sprite.id).flags & 2))
					debugN(" TAKE");
				if ((_dat->getSceneItemInfo(sprite.id).flags & 5))
					debugN(" JUMP");
				debug(".");
			}
			drawSprite(_screen, _iconsSpr->getFrame(_dat->getSceneItemInfo(sprite.id).frameIndex), itemX, 5);
			itemX += w + 2;
		}
	}
	// Draw the current scene number
	Common::String s = Common::String::format("Scene: %d; Link: %d", _sceneIndex, _currSceneLink);
	font->drawString(_screen, s, 1, 200 - 20, 319, 254);
}

void EnchantiaEngine::scrollScene(ScrollDirection direction) {
	int16 scrollX;
	if (direction == kScrollLeft) {
		_cameraStripNum--;
		_background->move(32, 0, _background->h);
		drawStrip(_sceneStripTable[_cameraStripNum], 0);
		scrollX = +32;
	} else {
		_cameraStripNum++;
		_background->move(-32, 0, _background->h);
		drawStrip(_sceneStripTable[_cameraStripNum + 9], 320 - 32);
		scrollX = -32;
	}
	_someX += scrollX;
	_walkInfos[0].x += scrollX;
	_walkInfos[1].x += scrollX;
	_walkInfos[2].x += scrollX;
	_cameraStripX -= scrollX;
	for (uint i = 1; i < kSpriteCount; i++)
		_sprites[i].x += scrollX;
	for (uint i = 0; i < _sceneDecorationCount; i++)
		_sceneDecorations[i].x += scrollX;
	_dirtyRects1->clear();
	_dirtyRects2->clear();
	memcpy(_screen->getPixels(), _background->getPixels(), 320 * 200);
	drawSprites();
	_system->copyRectToScreen((const byte*)_screen->getBasePtr(0, 0), 320, 0, 0, 320, 200);
	_system->updateScreen();
}

int16 EnchantiaEngine::startActorWalk(WalkInfo *walkInfo) {
	int16 deltaX, deltaY;
	int16 absDeltaX, absDeltaY;

	_walkIncrY2 = 2;

	deltaX = walkInfo->x - actorSprite().x - (actorSprite().width / 2 - 1);
	absDeltaX = ABS(deltaX);
	deltaY = walkInfo->y - actorSprite().y - actorSprite().height;
	absDeltaY = ABS(deltaY);

	debug(1, "startActorWalk() (%d, %d) -> (%d, %d); (%d, %d)",
		actorSprite().x - (actorSprite().width / 2 - 1), actorSprite().y - actorSprite().height, walkInfo->x, walkInfo->y, deltaX, deltaY);

	if (_sceneIndex == kSceneInsideWreck && _spriteResourceType == 254) {
		// TODO seg000:0F9F
	}

	if (absDeltaX >= absDeltaY) {
		if (_spriteResourceType != 20)
			absDeltaX /= 2;
		absDeltaX = (absDeltaX + 1) / 2;
		if (absDeltaX != 0) {
			_walkDistance = absDeltaX;
			if (deltaX >= 0) {
				if (actorSprite().frameIndex >= 128 && actorSprite().frameIndex <= 136)
					_walkDistance++;
				setActorWalkDirection(4);
			} else {
				if (actorSprite().frameIndex <= 8)
					_walkDistance++;
				setActorWalkDirection(8);
			}
			_walkSlopeErrY = ((deltaY * 256) / _walkDistance) % 256 * 256;
			_walkSlopeY = ((deltaY * 256) / _walkDistance) / 256;
			_someY = actorSprite().y;
			_walkErrorY = 0;
			_someX = actorSprite().x;
			_walkErrorX = 0;
			_walkSlopeX = 0;
			_walkSlopeErrX = 0;
		}
	} else {
		if (_spriteResourceType != 20)
			absDeltaY = (absDeltaY + 1) / 2;
		if (absDeltaY != 0) {
			_walkDistance = absDeltaY;
			if (deltaY >= 0) {
				if (actorSprite().frameIndex >= 18 && actorSprite().frameIndex <= 26)
					_walkDistance++;
				setActorWalkDirection(1);
			} else {
				if (actorSprite().frameIndex >= 9 && actorSprite().frameIndex <= 17)
					_walkDistance++;
				setActorWalkDirection(2);
			}
			_walkSlopeErrX = ((deltaX * 256) / _walkDistance) % 256 * 256;
			_walkSlopeX = ((deltaX * 256) / _walkDistance) / 256;
			_someX = actorSprite().x;
			_walkErrorX = 0;
			_someY = actorSprite().y;
			_walkErrorY = 0;
			_walkSlopeY = 0;
			_walkSlopeErrY = 0;
		}
	}

	debug(1, "_walkDistance = %d; _walkSlopeErrX = %d; _walkSlopeX = %d; _walkSlopeErrY = %d; _walkSlopeY = %d",
		_walkDistance, _walkSlopeErrX, _walkSlopeX, _walkSlopeErrY, _walkSlopeY);

	return _walkDistance == 0 ? 1 : _walkDistance;
}

void EnchantiaEngine::updateActorWalk() {

	if (_walkDistance > 0) {
		_walkErrorX += ABS(_walkSlopeErrX);
		actorSprite().x += _walkSlopeX + (_walkErrorX >> 16) * (_walkSlopeErrX > 0 ? 1 : -1);
		_walkErrorX &= 0xFFFF;
		_walkErrorY += ABS(_walkSlopeErrY);
		actorSprite().y += _walkSlopeY + (_walkErrorY >> 16) * (_walkSlopeErrY > 0 ? 1 : -1);
		_walkErrorY &= 0xFFFF;
		--_walkDistance;
	}

	// Check if the hero is still walking
	if (_walkDistance != 0 || !_currWalkInfo)
		return;

	// The actor has reached the current walk point
	if (_currWalkInfo->next) {
		// Process the next walk point
		_currWalkInfo = _currWalkInfo->next;
		startActorWalk(_currWalkInfo);
	} else {
		// No more points to walk
		if (_actorQueuedAnim.codeId) {
			// Run post-walk sprite animations for the hero
			actorSprite().setPos(_currWalkInfo->x - (actorSprite().width / 2 - 1),
				_currWalkInfo->y - actorSprite().height);
			actorSprite().anim = _actorQueuedAnim;
			actorSprite().moveX = _actorQueuedMoveX;
			actorSprite().moveY = _actorQueuedMoveY;
			_actorQueuedAnim.codeId = 0;
			_actorQueuedMoveX.codeId = 0;
			_actorQueuedMoveY.codeId = 0;
			// Run post-walk sprite animations for the other sprites
			for (uint i = 0; i < kAnimQueueCount && _animQueue[i].sprite; i++) {
				AnimQueueItem &animQueueItem = _animQueue[i];
				if (animQueueItem.animListId) {
					animQueueItem.sprite->anim.set(animQueueItem.animListId, 0,
						animQueueItem.ticks, animQueueItem.initialTicks);
					animQueueItem.animListId = 0;
				}
				if (animQueueItem.moveXListId) {
					animQueueItem.sprite->moveX.set(animQueueItem.moveXListId, 0,
						animQueueItem.ticks, animQueueItem.initialTicks);
					animQueueItem.moveXListId = 0;
				}
				if (animQueueItem.moveYListId) {
					animQueueItem.sprite->moveY.set(animQueueItem.moveYListId, 0,
						animQueueItem.ticks, animQueueItem.initialTicks);
					animQueueItem.moveYListId = 0;
				}
				animQueueItem.sprite = NULL;
			}
			_animQueueCount = 0;
		} else {
			// Just idle
			setActorWalkDirection(0);
		}
		_currWalkInfo = NULL;
	}

}

void EnchantiaEngine::setActorWalkDirection(byte direction) {

	struct WalkAnim {
		byte type;
		uint16 moveXCodeId, moveYCodeId, walkAnimCodeId, turnAnimCodeId;
		int16 turnFirstFrame, turnLastFrame;
	};

	static const WalkAnim walkAnims[] = {
		{0, 0, 0, 0, 0, 0, 0},
		{2, 0, 0x0AB6, 0x0AA0, 0x0A8A, 18, 26},
		{2, 0, 0x0AB8, 0x0AA9, 0x0A86,  9, 17},
		{0, 0, 0, 0, 0, 0, 0},
		{1, 0x0AB2, 0x0000, 0x0A8E, 0x0A82, 128, 136},
		{1, 0x0AB2, 0x0AB6, 0x0A8E, 0x0A82, 128, 136},
		{1, 0x0AB2, 0x0AB8, 0x0A8E, 0x0A82, 128, 136},
		{0, 0, 0, 0, 0, 0, 0},
		{1, 0x0AB4, 0x0000, 0x0A97, 0x0A7E, 0, 8},
		{1, 0x0AB4, 0x0AB6, 0x0A97, 0x0A7E, 0, 8},
		{1, 0x0AB4, 0x0AB8, 0x0A97, 0x0A7E, 0, 8}
	};

	if (_sceneIndex == kSceneInsideWreck && _spriteResourceType == 254) {
		// TODO seg000:0C07
	} else {
		if (direction > 10 || walkAnims[direction].type == 0) {
			if (actorSprite().anim.codeId) {
				if (actorSprite().frameIndex < 9)
					actorSprite().frameIndex = 0;
				else if (actorSprite().frameIndex < 18)
					actorSprite().frameIndex = 9;
				else if (actorSprite().frameIndex < 128)
					actorSprite().frameIndex = 18;
				else
					actorSprite().frameIndex = 128;
			}
			actorSprite().anim.codeId = 0;
			actorSprite().moveX.codeId = 0;
			actorSprite().moveY.codeId = 0;
		} else if (walkAnims[direction].type == 1) {
			const WalkAnim &walkAnim = walkAnims[direction];
			if (actorSprite().anim.codeId != walkAnim.walkAnimCodeId && actorSprite().anim.codeId != walkAnim.turnAnimCodeId) {
				if (actorSprite().frameIndex >= walkAnim.turnFirstFrame && actorSprite().frameIndex <= walkAnim.turnLastFrame) {
					actorSprite().anim.set(walkAnim.turnAnimCodeId, 0, 1, 1);
					actorSprite().moveX.set(walkAnim.moveXCodeId, 0, 2, 1);
					actorSprite().moveY.set(walkAnim.moveYCodeId, 0, 2, 1);
				} else {
					actorSprite().anim.set(walkAnim.walkAnimCodeId, 0, 1, 1);
					actorSprite().moveX.set(walkAnim.moveXCodeId, 0, 1, 1);
					actorSprite().moveY.set(walkAnim.moveYCodeId, 0, 1, 1);
				}
			}
		} else if (walkAnims[direction].type == 2) {
			const WalkAnim &walkAnim = walkAnims[direction];
			if (actorSprite().anim.codeId != walkAnim.walkAnimCodeId && actorSprite().anim.codeId != walkAnim.turnAnimCodeId) {
				actorSprite().moveX.set(0, 0, 1, 1);
				if (actorSprite().frameIndex >= walkAnim.turnFirstFrame && actorSprite().frameIndex <= walkAnim.turnLastFrame) {
					actorSprite().anim.set(walkAnim.turnAnimCodeId, 0, 1, 1);
					actorSprite().moveY.set(walkAnim.moveYCodeId, 0, 2, 1);
				} else {
					actorSprite().anim.set(walkAnim.walkAnimCodeId, 0, 1, 1);
					actorSprite().moveY.set(walkAnim.moveYCodeId, 0, 1, 1);
				}
			}
		}
		if (direction != 0 && _spriteResourceType == 20) {
			if (actorSprite().anim.codeId == 0x0A97 || actorSprite().anim.codeId == 0x0A7E)
				actorSprite().moveX.codeId = 0x0ABC;
			else if (actorSprite().anim.codeId == 0x0A8E || actorSprite().anim.codeId == 0x0A82)
				actorSprite().moveX.codeId = 0x0ABA;
			else if (actorSprite().anim.codeId == 0x0AA9 || actorSprite().anim.codeId == 0x0A86)
				actorSprite().moveY.codeId = 0x0AC0;
			else if (actorSprite().anim.codeId == 0x0AA0 || actorSprite().anim.codeId == 0x0A8A)
				actorSprite().moveY.codeId = 0x0ABE;
		}
	}

}

void EnchantiaEngine::loadPalette(const char *filename, byte *palette) {
	Common::File fd;
	if (!fd.open(filename))
		error("EnchantiaEngine::loadPalette() Could not open %s", filename);
	fd.read(_palette, 768);
	fd.close();
}

Graphics::Surface *EnchantiaEngine::loadBitmap(const char *filename, int16 width, int16 height) {
	Common::File fd;
	if (!fd.open(filename))
		error("EnchantiaEngine::loadBitmap() Could not open %s", filename);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	byte *data = new byte[fd.size()];
	fd.read(data, fd.size());
	unpackRnc(data, (byte*)surface->getPixels());
	delete[] data;
	fd.close();
	return surface;
}

void EnchantiaEngine::loadRaw(const char *filename, byte **buffer) {
	Common::File fd;
	if (!fd.open(filename))
		error("EnchantiaEngine::loadRaw() Could not open %s", filename);
	delete[] *buffer;
	*buffer = new byte[fd.size()];
	fd.read(*buffer, fd.size());
	fd.close();
}

void EnchantiaEngine::loadWalkBoxes(uint index) {
	Common::File fd;
	uint16 offset, count;

	_walkBoxes.clear();
	if (!fd.open("boxdet.dat"))
		error("EnchantiaEngine::loadWalkBoxes() Could not open boxdet.dat");
	fd.seek(index * 2);
	offset = fd.readUint16LE();
	fd.seek(offset);
	count = fd.readUint16LE();
	for (uint i = 0; i < count; i++) {
		WalkBox walkBox;
		walkBox.type = fd.readByte();
		walkBox.param = fd.readByte();
		walkBox.x1 = fd.readUint16LE();
		walkBox.y1 = fd.readUint16LE();
		walkBox.x2 = fd.readUint16LE();
		walkBox.y2 = fd.readUint16LE();
		debug(4, "walkBox() type: %02X; param: %02X; (%d, %d, %d, %d)", walkBox.type, walkBox.param, walkBox.x1, walkBox.y1, walkBox.x2, walkBox.y2);
		_walkBoxes.push_back(walkBox);
	}
}

void EnchantiaEngine::checkWalkBoxes(int16 &currActorX, int16 &currActorY) {
	int16 boxActorY = actorSprite().y;
	bool boxActorYFlag = true;
	bool checkAgain = true, afterWalk = true;

	while (checkAgain) {
		int16 x1 = actorSprite().x + _cameraStripX;
		int16 y1 = actorSprite().y + actorSprite().height;
		int16 x2 = x1 + actorSprite().width - 1;
		int16 y2 = y1 - 1;
		int16 boxType, boxParam;
		if (_spriteResourceType == 20) {
			x1 += 3;
			x2 -= 3;
			y1 -= 2;
		} else {
			x1 += 6;
			x2 -= 6;
			y1 -= 4;
		}

		checkAgain = false;
		queryWalkBoxInfo(x1, y1, x2, y2, boxType, boxParam);

		switch (boxType) {
		case kBoxTypeNone:
			break;
		case kBoxTypeAction:
			afterWalk = false;
			debug(4, "walkBox: handleBoxAction (%d)", boxParam);
			handleBoxAction(boxParam);
			_walkIncrY2 = 2;
			break;
		case kBoxTypeEndGame:
			afterWalk = false;
			debug(4, "walkBox: endGame");
			// TODO paletteEffect(_palette, 32, 256);
			// TODO This also ends the main game loop and so the game
			break;
		case kBoxTypeChangeScene:
			afterWalk = false;
			debug(4, "walkBox: loadScene (%d)", boxParam);
			loadScene(boxParam, false);
			break;
		case kBoxTypeBlock:
			if (boxActorYFlag && (boxParam & 1) && !actorSprite().moveY.codeId) {
				boxActorYFlag = false;
				checkAgain = true;
				++boxActorY;
				++currActorY;
				++actorSprite().y;
			} else if (boxActorYFlag && (boxParam & 2) && !actorSprite().moveY.codeId) {
				boxActorYFlag = false;
				checkAgain = true;
				--boxActorY;
				--currActorY;
				--actorSprite().y;
			} else if (currActorY != actorSprite().y) {
				_someY = currActorY;
				_walkErrorY = 0;
				actorSprite().y = currActorY;
				if (currActorX != actorSprite().x)
					checkAgain = true;
			} else if (currActorX != actorSprite().x) {
				_someX = currActorX;
				_walkErrorX = 0;
				actorSprite().x = currActorX;
				_someY = boxActorY;
				_walkErrorY = 0;
				actorSprite().y = boxActorY;
				checkAgain = true;
			}
			break;
		default:
			break;
		}
	}

	if (afterWalk) {
		if (currActorX != actorSprite().x || currActorY != actorSprite().y) {
			_walkIncrY2 = 2;
		} else if (--_walkIncrY2 <= 0) {
			_walkDistance = 0;
			_walkIncrY2 = 2;
		}
	}

}

void EnchantiaEngine::queryWalkBoxInfo(int16 x1, int16 y1, int16 x2, int16 y2, int16 &type, int16 &param) {
	byte queryBoxValue = 255;

	if (x1 <= _queryBoxX2 && y1 <= _queryBoxY2 && x2 >= _queryBoxX1 && y2 >= _queryBoxY1)
		queryBoxValue = _queryBoxDefValue;

	for (uint i = 0; i < _walkBoxes.size(); i++) {
		const WalkBox &walkBox = _walkBoxes[i];
		if (x1 <= walkBox.x2 && y1 <= walkBox.y2 && x2 >= walkBox.x1 && y2 >= walkBox.y1) {
			switch (walkBox.type) {
			case 0:
				queryBoxValue = walkBox.param;
				break;
			case 1:
				// TODO Unused? End of game...
				break;
			case 2:
				// Scene change
				type = kBoxTypeChangeScene;
				param = walkBox.param;
				// If the scene can't be changed this walk box is ignored
				if (handleBoxSceneChange(type, param))
					return;
				break;
			case 3:
				if (walkBox.param == 0 || walkBox.param == 1 || walkBox.param == 2 || walkBox.param == 3 ||
					walkBox.param == 4 || walkBox.param == 9 || walkBox.param == 10 || walkBox.param == 11) {
					type = 8;
					param = walkBox.param;
					return;
				}
				break;
			default:
				// Nothing
				break;
			}
		}
	}

	if (queryBoxValue != 255) {
		type = kBoxTypeBlock;
		param = queryBoxValue;
	} else {
		type = kBoxTypeNone;
		param = 0;
	}

}

void EnchantiaEngine::checkBoxClick() {
	// NOTE Skipped pathfinding code which seems to be unused
	_boxWalkInfos[0].next = NULL;
	_boxWalkInfos[0].x = _mouseX;
	_boxWalkInfos[0].y = _mouseY;
	_currWalkInfo = &_boxWalkInfos[0];
}

int16 EnchantiaEngine::actionThumbsUp() {
	int16 walkCount = 1;
	resetActor();
	if (_actorQueuedAnim.codeId) {
		_currWalkInfo = &_walkInfos[0];
		walkCount = startActorWalk(_currWalkInfo);
	}
	cursorSprite().frameIndex = 1;
	cursorSprite().anim.set(0x9976, 0, 15, 1);
	return walkCount;
}

void EnchantiaEngine::actionThumbsDown() {
	cursorSprite().frameIndex = 2;
	cursorSprite().anim.set(0x9976, 0, 15, 1);
}

void EnchantiaEngine::clearInventory() {
	for (uint i = 0; i < kInventoryTableCount; i++)
		_inventoryTable[i] = -1;
}

bool EnchantiaEngine::hasInventoryItem(int16 id) {
	for (uint i = 0; i < kInventoryTableCount; i++)
		if (_inventoryTable[i] == id)
			return true;
	return false;
}

bool EnchantiaEngine::addInventoryItem(int16 id) {
	for (uint i = 0; i < kInventoryTableCount; i++)
		if (_inventoryTable[i] < 0) {
			_inventoryTable[i] = id;
			return true;
		}
	return false;
}

bool EnchantiaEngine::removeInventoryItem(int16 id) {
	bool found = false;
	for (uint i = 0; i < kInventoryTableCount; i++)
		if (!found && _inventoryTable[i] == id) {
			found = true;
		} else if (found) {
			_inventoryTable[i - 1] = _inventoryTable[i];
		}
	if (found)
		_inventoryTable[kInventoryTableCount - 1] = -1;
	return found;
}

void EnchantiaEngine::resetActor() {
	_flgCanRunMenu = false;
	_flgCanRunBoxClick = false;
	_walkDistance = 0;
	_currWalkInfo = NULL;
	_flgWalkBoxEnabled = false;
	actorSprite().anim.index = 0;
	actorSprite().anim.ticks = 1;
	actorSprite().anim.initialTicks = 2;
	actorSprite().moveX.index = 0;
	actorSprite().moveX.ticks = 1;
	actorSprite().moveX.initialTicks = 2;
	actorSprite().moveY.index = 0;
	actorSprite().moveY.ticks = 1;
	actorSprite().moveY.initialTicks = 2;
}

void EnchantiaEngine::playSound(byte soundNum, Sprite &sprite) {
	if (soundNum != 0xFF &&
		sprite.x <= 319 && sprite.x + sprite.width >= 0 &&
		sprite.y <= 199 && sprite.y + sprite.height >= 0) {
		const SoundItem &soundItem = _dat->getSound(soundNum, _sceneIndex);
		if (!_mixer->isSoundHandleActive(_soundHandle) || soundItem.priority > _currSoundPriority) {
			Audio::AudioStream *audioStream;
			byte *soundData = soundNum < 15 ? _stdSfx : _sceneSfx;
			uint16 soundSize;
			soundData = soundData + READ_LE_UINT16(soundData + soundItem.index * 2);
			soundSize = READ_LE_UINT16(soundData) - soundItem.sizeDecr;
			uint freq = 1000000 / (256 - soundItem.freq);
			_currSoundPriority = soundItem.priority;
			_mixer->stopHandle(_soundHandle);
			audioStream = Audio::makeRawStream(soundData + 2, soundSize,
				freq, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
			_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, audioStream);
		}
	}
}

void EnchantiaEngine::runMenuBar() {
	static const Color menuBarColorTable[] = {
		{7, 0, 0}, {15, 0, 0}, {23, 0, 0}, {31, 0, 0}, {39, 0, 0},
		{47, 0, 0}, {55, 0, 0}, {63, 0, 0}, {0, 7, 0}, {0, 15, 0},
		{0, 23, 0}, {0, 31, 0}, {0, 39, 0}, {0, 47, 0}, {0, 55, 0},
		{0, 63, 0}, {0, 0, 7}, {0, 0, 15}, {0, 0, 23}, {0, 0, 31},
		{0, 0, 39}, {0, 0, 47}, {0, 0, 55}, {0, 0, 63},
	};

	enum { stRunning, stAction, stDone } status = stRunning;
	enum { kMenuTop, kMenuBottom } menuPosition = kMenuBottom;
	uint rectPalCounter = 1, colorIndex = 0, menuCounter = 0;
	int16 cursorX, y;
	uint slotIndex = 0, prevSlotIndex = 0xFFFF;
	MenuSlotAction currMenu = kMenuNone, newMenu = kMenuMain;
	bool needRedraw = true;

	Graphics::Surface *savedScreen = new Graphics::Surface();
	savedScreen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	savedScreen->copyFrom(*_screen);

	_isSaveAllowed = false;

	_walkInfos[0].next = NULL;
	_walkInfos[1].next = NULL;
	_walkInfos[2].next = NULL;

	cursorX = _mouseX;
	uint32 savedMouseY = _mouseY;
	if (_mouseY < 100) {
		menuPosition = kMenuTop;
		y = 0;
	} else {
		menuPosition = kMenuBottom;
		y = 168;
	}

	while (status != stDone && !shouldQuit()) {

		if (currMenu != newMenu) {
			currMenu = newMenu;
			buildMenuBar(newMenu);
			needRedraw = true;
		}

		if (needRedraw) {
			needRedraw = false;
			drawSurface(_screen, _menuSurface, 0, y);
		}
		_screen->frameRect(Common::Rect(slotIndex * 32 + 3, y + 3, slotIndex * 32 + 29, y + 29), 0xFF);
		_system->copyRectToScreen((const byte*)_screen->getBasePtr(0, y), 320, 0, y, 320, 32);
		menuCounter++;

		if (++colorIndex >= 24)
			colorIndex = 0;
		setPaletteColor(0xFF, menuBarColorTable[colorIndex]);

		updateEvents();

		_system->warpMouse(_mouseX, menuPosition == kMenuTop ? 16 : 184);

		cursorX = _mouseX;
		if (_mouseButton == kLeftButton)
			status = stAction;
		else if (_mouseButton == kRightButton)
			status = stDone;
		// TODO Keyboard handling
		slotIndex = cursorX / 32;
		if (slotIndex != prevSlotIndex) {
			needRedraw = true;
			prevSlotIndex = slotIndex;
		}
		_system->updateScreen();
		if (status == stAction) {
			newMenu = runMenuBarAction(slotIndex);
			if (newMenu != kMenuNone) {
				status = stRunning;
				needRedraw = true;
			} else
				status = stDone;
		}
	}

	_system->warpMouse(_mouseX, savedMouseY);

	// Restore command bar background
	_screen->copyFrom(*savedScreen);
	savedScreen->free();
	delete savedScreen;
	_system->copyRectToScreen((const byte*)_screen->getBasePtr(0, 0), 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void EnchantiaEngine::setPaletteColor(uint16 index, Color c) {
	_palette[index * 3] = c.r;
	_palette[index * 3 + 1] = c.g;
	_palette[index * 3 + 2] = c.b;
	setVgaPalette(_palette);
}

void EnchantiaEngine::updateMenuBar() {
	// Draw menu background to _menuSurface
	drawSurface(_menuSurface, _menuSprite, 0, 0);
	for (uint i = 0; i < kMenuBarTableCount; i++) {
		if (_menuSlots[i].id < 0)
			_menuSlots[i].action = kMenuMain;
		else if (_menuSlots[i].id < 23)
			_menuSlots[i].action = kMenuSlotActions[_menuSlots[i].id];
	}
	for (uint i = 0; i < kMenuBarTableCount && _menuSlots[i].id >= 0; i++) {
		uint frameIndex = _menuSlots[i].id;
		if (frameIndex >= 26)
			frameIndex = _dat->getSceneItemInfo(frameIndex - 26).frameIndex;
		drawSprite(_menuSurface, _iconsSpr->getFrame(frameIndex), 4 + 32 * i, 4);
		/* TODO
		if (_menuBarItems[i] >= 26 && _menuBarItems[i] - 26 == _useItemId)
			drawSprite(_menuSurface, _gridSprite, 4 + 32 * i, 4);
		*/
	}
}

void EnchantiaEngine::listMenuBarItems(int16 *table, int16 base, MenuSlotAction action) {
	for (uint i = 0; i < kMenuBarTableCount; i++) {
		_menuSlots[i].id = table[i] >= 0 ? base + table[i] : -1;
		_menuSlots[i].action = action;
	}
}

void EnchantiaEngine::collectSceneItems(uint firstIndex, uint minCount, byte itemFlags, MenuSlotAction action) {
	int16 x1 = actorSprite().x - 5;
	int16 x2 = actorSprite().x + actorSprite().width - 1 + 5;
	int16 y1 = actorSprite().y + actorSprite().height - 4 - 4;
	int16 y2 = actorSprite().y + actorSprite().height - 1 + 4;
	for (uint i = 2; i < kSpriteCount; i++) {
		Sprite &sprite = _sprites[i];
		if (sprite.status && ((sprite.status & 4) || !(sprite.status & 2)) &&
			sprite.id >= 0 && sprite.id < 1024 && (_dat->getSceneItemInfo(sprite.id).flags & itemFlags) &&
			(_collectItemIgnoreActorPosition ||
			(sprite.x <= x2 && sprite.x + sprite.width > x1 &&
			sprite.y - sprite.yAdd <= y2 && sprite.y + sprite.height + sprite.yAdd > y1))) {

			_menuSlots[firstIndex].id = 26 + sprite.id;
			_menuSlots[firstIndex].action = action;
			firstIndex++;
			--minCount;
		}
	}
	// Fill the remaining slots
	while (minCount--)
		_menuSlots[firstIndex++].id = -1;
}

void EnchantiaEngine::buildMenuBar(MenuSlotAction menu) {

	uint firstIndex, minCount;
	bool offerInventory;

	switch (menu) {

	case kMenuMain:
		_useItemId = idNone;
		for (uint i = 0; i < kMenuBarTableCount; i++)
			_menuSlots[i].id = i;
		updateMenuBar();
		if (actorSprite().anim.codeId != 0) {
			SpriteResource *gridSpr = new SpriteResource();
			gridSpr->loadSingle(_dat->getGridSprite());
			drawSprite(_menuSurface, gridSpr->getFrame(0), 36, 4);

			_menuSlots[1].action = kMenuMain;
			_menuSlots[2].action = kMenuMain;
		}
		break;

	case kMenuInventory:
		listMenuBarItems(_inventoryTable, 26, kMenuMain);
		updateMenuBar();
		break;

	case kMenuSelectInventoryItem:
		listMenuBarItems(_inventoryTable, 26, kMenuActionUse);
		updateMenuBar();
		break;

	case kMenuSelectItem:
		firstIndex = 0;
		minCount = 10;
		offerInventory = _currUseCommand != kCmdInsert && _currUseCommand != kCmdGive;
		if (offerInventory) {
			_menuSlots[0].id = 0;
			firstIndex++;
			minCount--;
		}
		collectSceneItems(firstIndex, minCount, 1, kMenuActionUse);
		updateMenuBar();
		if (offerInventory)
			_menuSlots[0].action = kMenuSelectInventoryItem;
		break;

	case kMenuTake:
		collectSceneItems(0, 10, 2, kMenuActionTake);
		updateMenuBar();
		break;

	case kMenuUse:
		for (uint i = 0; i < 8; i++)
			_menuSlots[i].id = 15 + i;
		_menuSlots[8].id = -1;
		_menuSlots[9].id = -1;
		updateMenuBar();
		break;

	case kMenuLook:
		collectSceneItems(0, 10, 1, kMenuActionLook);
		updateMenuBar();
		break;

	case kMenuTalk:
		listMenuBarItems(_talkTable, 23, kMenuActionTalk);
		updateMenuBar();
		break;

	case kMenuFight:
		listMenuBarItems(_inventoryTable, 26, kMenuActionFight);
		updateMenuBar();
		break;

	case kMenuJump:
		collectSceneItems(0, 10, 5, kMenuActionJump);
		updateMenuBar();
		break;

	case kMenuDisk:
		break;

	default:
		break;

	}

}

MenuSlotAction EnchantiaEngine::runMenuBarAction(uint slotIndex) {
	MenuSlotAction menu = kMenuNone;
	int16 id;

	switch (_menuSlots[slotIndex].action) {

	case kMenuActionUseCommand:
		_currUseCommand = slotIndex;
		if (_currUseCommand == kCmdInsert || _currUseCommand == kCmdEat || _currUseCommand == kCmdWear ||
			_currUseCommand == kCmdThrow || _currUseCommand == kCmdGive || _currUseCommand == kCmdCombine)
			// Select an item from the inventory
			menu = kMenuSelectInventoryItem;
		else
			// Select any item
			menu = kMenuSelectItem;
		break;

	case kMenuActionUse:
		id = _menuSlots[slotIndex].id - 26;
		if (_useItemId == idNone &&
			(_currUseCommand == kCmdUnlock || _currUseCommand == kCmdInsert ||
			_currUseCommand == kCmdGive || _currUseCommand == kCmdCombine)) {
			_useItemId = id;
			// Run menu again to select the second item
			menu = kMenuSelectItem;
		} else {
			performCommand(_currUseCommand, _useItemId, id);
		}
		break;

	case kMenuActionTake:
		id = _menuSlots[slotIndex].id - 26;
		if (id != idWishCoin && _dat->getSceneItemInfo(id).frameIndex == 31) {
			// Any money (except the wishing well coin) gets 1 point
			_theScore += 1;
			performCommand(kCmdTake, idNone, id);
		} else if (_dat->getSceneItemInfo(id).frameIndex == 34) {
			// Any jewel (in the long hall after the dungeon) gets 2 points
			_theScore += 2;
			performCommand(kCmdTake, idNone, id);
		} else if (id != idMoneyPouch51 && id != idWoman && !addInventoryItem(id)) {
			// Try to add the inventory item (except the money and woman from the wishing well)
			// Exit if it fails, i.e. the inventory is full
			actionThumbsDown();
			debug(4, "Take failed -> inventory full");
		} else if (!performCommand(kCmdTake, idNone, id))
			removeInventoryItem(id);
		break;

	case kMenuActionTalk:
		id = _menuSlots[slotIndex].id - 23;
		performCommand(kCmdTalk, idNone, id);
		break;

	case kMenuActionLook:
		id = _menuSlots[slotIndex].id - 26;
		performCommand(kCmdLook, idNone, id);
		break;

	case kMenuActionFight:
		id = _menuSlots[slotIndex].id - 26;
		performCommand(kCmdFight, idNone, id);
		break;

	case kMenuActionJump:
		id = _menuSlots[slotIndex].id - 26;
		performCommand(kCmdJump, idNone, id);
		break;

	case kMenuActionInfo:
		showInfo();
		break;

	default:
		menu = _menuSlots[slotIndex].action;

	}

	return menu;
}

void EnchantiaEngine::showInfo() {
	// TODO: extract from exe into dat file?
	byte charTransTable[] = {0x2D, 0x2A, 0x1E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0x2C, 0xFF, 0x1B, 0xFF,
							 0xFF, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
							 0x1A, 0xFF, 0xFF, 0xFF, 0xFF, 0x2B, 0xFF, 0x00, 0x01, 0x02, 0x03,
							 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
							 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1C, 0xFF,
							 0x1D, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
							 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
							 0x56, 0x57, 0x58, 0x59, 0x5A, 0x3A, 0x2D, 0x5B, 0x5D, 0xFF, 0x26,
							 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0xFF,
							 0x3F, 0x2B, 0xFF};

	int score = _theScore;
	int percentage = score * 100 / 478;

	Graphics::Surface *savedScreen = new Graphics::Surface();
	savedScreen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	savedScreen->copyFrom(*_screen);

	byte *creditsTxt = _dat->getScoreCreditsTxt();
	byte *i = creditsTxt;
	uint16 addr = *(i + 1) << 8 | *i;
	while (addr != 0xFFFF) {
		byte *paragraphPtr = creditsTxt + addr;

		uint16 textX = *(paragraphPtr + 1) << 8 | *paragraphPtr;
		uint16 textY = *(paragraphPtr + 3) << 8 | *(paragraphPtr + 2);
		uint16 centerTextX = (~textX + 320) >> 1;
		uint16 centerTextY = (~textY + 200) >> 1;
		paragraphPtr += 4;

		int offset = (112 * centerTextY) + (centerTextX >> 2);

		_screen->fillRect(Common::Rect(centerTextX, centerTextY, centerTextX + textX, centerTextY + textY), 0x01);

		byte *txtPtr = paragraphPtr;
		int16 x = centerTextX + 8;
		int16 y = centerTextY + 5;
		Point startPos = { x, y };
		Point currPos = { startPos.x, startPos.y };

		byte c = *txtPtr++;
		while (true) {
			byte fontIndex = charTransTable[c];
			if (c != 0xFF && c != 0xFE && fontIndex != 0xFF) {
				drawSprite(_screen, _fontSpr->getFrame(fontIndex), currPos.x, currPos.y);
			} else if (c == 0xFF) {
				currPos.x = startPos.x;
				currPos.y += 10;
			} else if (c == 0xFE) {
				break;
			}

			currPos.x += 8;
			c = *(txtPtr++);
		}

		_system->copyRectToScreen((const byte *)_screen->getBasePtr(centerTextX, centerTextY), 320, centerTextX, centerTextY, textX, textY);
		_system->updateScreen();

		while (true) {
			updateEvents();
			if (_mouseButton == kLeftButton)
				break;
		}

		_screen->copyFrom(*savedScreen);
		_system->copyRectToScreen((const byte*)_screen->getBasePtr(0, 0), 320, 0, 0, 320, 200);
		_system->updateScreen();

		i += 2;
		addr = *(i + 1) << 8 | *i;
	}

	savedScreen->free();
	delete savedScreen;
}

void EnchantiaEngine::updateFunc411h() {
	if (_flgCanRunBoxClick) {
		_flgCanRunBoxClick = false;
		_flgCanRunMenu = false;
		_flgWalkBoxEnabled = false;
		_walkDistance = 0;
		_currWalkInfo = NULL;
		actorSprite().setCodeSync(0xAFF7, 0xB015, 0xB021, 1, 1);
		_sprites[7].setPos(actorSprite().x - 2, actorSprite().y + 13);
		_sprites[7].setCodeSync(0xB04E, 0xB054, 0xB058, 10, 2);
		_sprites[13].setPos(actorSprite().x - 10, 210);
		_sprites[13].anim.set(0xB05C, 0, 20, 1);
		_sprites[13].moveX.set(0xB06A, 0, 22, 1);
		_sprites[13].moveY.set(0xB071, 0, 22, 1);
	}
}

SpriteDef *EnchantiaEngine::getSpriteDef(uint16 id) {
	return _dat->getSpriteDef(id);
}

SpriteTemplate *EnchantiaEngine::getSpriteTemplate(uint16 id) {
	return _dat->getSpriteTemplate(id);
}

SceneItemInfo &EnchantiaEngine::getSceneItemInfo(uint index) {
	return _dat->getSceneItemInfo(index);
}

SpriteDef *EnchantiaEngine::getSceneSpriteDef(uint sceneIndex, uint index) {
	return _dat->getSceneSpriteDef(sceneIndex, index);
}

} // End of namespace Enchantia
