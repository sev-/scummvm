/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ENCHANTIA_H
#define ENCHANTIA_H

#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/util.h"
#include "common/debug.h"
#include "common/debug-channels.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/surface.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

namespace Enchantia {

class EnchantiaEngine;
class DatFile;
class AdlibMusicPlayer;
class MidiPlayer;

enum {
	kNoButton		= 0,
	kLeftButton		= 1,
	kRightButton	= 2
};

enum ItemId {
	idShackles			= 0,
	idKey				= 1,
	idCellWall			= 2,
	idPaperClip			= 3,
	idKeyHole4			= 4,
	idCoin5				= 5,
	idFishBowl			= 6,
	idGoldfish			= 7,
	idKeyHole8			= 8,
	idCoin9				= 9,
	idJewel10			= 10,
	idJewel11			= 11,
	idJewel12			= 12,
	idJewel13			= 13,
	idJewel14			= 14,
	idStuckFish			= 15,
	idMud16				= 16,
	idWorm				= 17,
	idShell				= 18,
	idSeaWeed19			= 19,
	idMrFish			= 20,
	idAirTank			= 21,
	idTurtle			= 22,
	idCattleProd		= 23,
	idClam				= 24,
	idPlug				= 25,
	idCoin26			= 26,
	idRock27			= 27,
	idRock28			= 28,
	idRock29			= 29,
	idRock30			= 30,
	idRock31			= 31,
	idRock32			= 32,
	idRock33			= 33,
	idRock34			= 34,
	idRock35			= 35,
	idRings				= 36,
	idMud				= 37,
	idRock38			= 38,
	idRock39			= 39,
	idRock40			= 40,
	idRock41			= 41,
	idRock42			= 42,
	idRock43			= 43,
	idRock44			= 44,
	idRock45			= 45,
	idRock46			= 46,
	idWishCoin			= 47,
	idButton48			= 48,
	idSeaWeed			= 49,
	idBoulder50			= 50,
	idMoneyPouch51		= 51,
	idWoman				= 52,
	idHelmet			= 53,
	idRockBasher		= 54,
	idString			= 55,
	idBoard				= 56,
	idMagnet			= 57,
	idBoulder58			= 58,
	idDown59			= 59,
	idBucket			= 60,
	idComputer			= 61,
	idHole62			= 62,
	idThread			= 63,
	idBranch			= 64,
	idHole65			= 65,
	idHole66			= 66,
	idHole67			= 67,
	idHole68			= 68,
	idLittleCreature	= 69,
	idMagnetOnString	= 70,
	idMudBranch			= 71,
	idMudSeaWeed		= 72,
	idBranchSeaWeed		= 73,
	idMudBranchSeaWeed	= 74,
	idKnife				= 75,
	idMoneyPouch		= 76,
	idMage				= 77,
	idBenn				= 78,
	idDress				= 79,
	idDoor80			= 80,
	idSally				= 81,
	idMaitreD			= 82,
	idFrank				= 83,
	idMeat				= 84,
	idCliffCreature		= 85,
	idGloves			= 86,
	idItem87			= 87,
	idButton88			= 88,
	idButton89			= 89,
	idButton90			= 90,
	idButton91			= 91,
	idCliffRock			= 92,
	idRock93			= 93,
	idRock94			= 94,
	idRope				= 95,
	idItem96			= 96,
	idWriting			= 97,
	idBoomBoxGuy		= 98,
	idKeyCard			= 99,
	idDirtySock			= 100,
	idPen				= 101,
	idBigNose			= 102,
	idRobot				= 103,
	idSockWithCoins		= 104,
	idCassette			= 105,
	idCassetteMusic		= 106,
	idGoldCoins			= 107,
	idRemote			= 108,
	idStamp				= 109,
	idHair				= 110,
	idLetter			= 111,
	idLetterStamp		= 112,
	idConsole			= 113,
	idPostBox			= 114,
	idTray				= 115,
	idKeyCardLock		= 116,
	idPlank117			= 117,
	idPlank118			= 118,
	idGoldFoil			= 119,
	idItem120			= 120,
	idMarblesBag		= 121,
	idButton122			= 122,
	idKeyHole123		= 123,
	idFan				= 124,
	idWoodenPlank		= 125,
	idEskimo			= 126,
	idFishingRod		= 127,
	idJumpingFish		= 128,
	idYeti				= 129,
	idCreature130		= 130,
	idItem131			= 131,
	idSprayCan			= 132,
	idWalrus			= 133,
	idSnowball			= 134,
	idBoat				= 135,
	idIcicle136			= 136,
	idIcicle137			= 137,
	idIcicle138			= 138,
	idIcicle139			= 139,
	idDice				= 140,
	idBroom				= 141,
	idHolster			= 142,
	idBowl				= 143,
	idSunTanOil			= 144,
	idGlass				= 145,
	idIceWindow			= 146,
	idWhistle			= 147,
	idWhistleMegaphone	= 148,
	idCarJack			= 149,
	idMagnifier			= 150,
	idGun				= 151,
	idMegaphone			= 152,
	idIceCube			= 153,
	idIcicle			= 154,
	idBeamHole1			= 155,
	idBeamHole2			= 156,
	idBeamHole3			= 157,
	idBeamHole4			= 158,
	idDoor159			= 159,
	idUp160				= 160,
	idMatchBox			= 161,
	idMatch				= 162,
	idBabyFoot			= 163,
	idFireExtinguisher	= 164,
	idBone				= 165,
	idVampire			= 166,
	idPlate				= 167,
	idShovel			= 168,
	idCymbals			= 169,
	idGarlic			= 170,
	idVacuumCleaner		= 171,
	idGraveStone		= 172,
	idCross				= 173,
	idGate				= 174,
	idRing				= 175,
	idBook				= 176,
	idEvilQueen			= 177,
	idNone				= 255
};

// NOTE Only scenes which are directly referenced in the code are listed here.

enum SceneId {
	kSceneDungeon			= 0,
	kSceneDungeonHall		= 1,
	kSceneUnderwater		= 3,
	kSceneCaveMonster		= 5,
	kSceneCaveRocks			= 6,
	kSceneCaveLake			= 8,
	kSceneWishingWell		= 9,
	kSceneCaveHole			= 13,
	kSceneBandit			= 16,
	kSceneFountain			= 17,
	kSceneTown20			= 20,
	kSceneTown21			= 21,
	kSceneMageShop			= 22,
	kSceneCostumeShop		= 24,
	kSceneChangingRoom		= 25,
	kSceneSallySeeAll		= 26,
	kSceneFranksFood		= 29,
	kSceneCliff				= 30,
	kSceneBigBird			= 31,
	kSceneCars				= 32,
	kSceneShipWreck			= 33,
	kSceneRemotesDump		= 34,
	kSceneBand				= 35,
	kSceneInsideWreck		= 36,
	kSceneCaveWall			= 37,
	kSceneCloud				= 38,
	kSceneElectro			= 39,
	kSceneIceDesert40		= 40,
	kSceneIceDesert41		= 41,
	kSceneIceDesert42		= 42,
	kSceneIceDesert43		= 43,
	kSceneBoat				= 44,
	kSceneIceCastle			= 45,
	kSceneIceCastleHall		= 46,
	kSceneIceCastleGun		= 52,
	kSceneIceCastleIcicle	= 55,
	kSceneIceCastleBeam		= 56,
	kSceneIceCastleWitch	= 57,
	kSceneIceCastle58		= 58,
	kSceneTrollBaby			= 63,
	kSceneGraveyard65		= 65,
	kSceneEvilQueen			= 72,
	kSceneTheEnd			= 73,
	kSceneGraveyard77		= 77
};

enum TalkItem {
	idTalkHelp			= 0,
	idTalkHi			= 1,
	idTalkOpenSesame	= 2
};

enum Command {
	kCmdUnlock			= 0,
	kCmdInsert			= 1,
	kCmdMove			= 2,
	kCmdEat				= 3,
	kCmdWear			= 4,
	kCmdThrow			= 5,
	kCmdGive			= 6,
	kCmdCombine			= 7,
	kCmdTake			= 8,
	kCmdTalk			= 9,
	kCmdLook			= 10,
	kCmdFight			= 11,
	kCmdJump			= 12
};

enum MenuSlotAction {
	kMenuNone,
	kMenuMain,
	kMenuInventory,
	kMenuSelectInventoryItem,
	kMenuSelectItem,
	kMenuTake,
	kMenuUse,
	kMenuLook,
	kMenuTalk,
	kMenuFight,
	kMenuJump,
	kMenuDisk,
	kMenuActionSound = 0x8000,
	kMenuActionInfo,
	kMenuActionDiskLoad,
	kMenuActionDiskSave,
	kMenuActionDiskDelete,
	kMenuActionDiskNothing,
	kMenuActionUseCommand,
	kMenuActionTake,
	kMenuActionTalk,
	kMenuActionLook,
	kMenuActionFight,
	kMenuActionJump,
	kMenuActionUse
};

enum BoxType {
	kBoxTypeNone		= 0,
	kBoxTypeBlock		= 1,
	kBoxTypeChangeScene = 2,
	kBoxTypeEndGame		= 4,
	kBoxTypeAction		= 8
};

enum ScrollDirection {
	kScrollLeft,
	kScrollRight
};

const uint16 kNone = 0xFFFF;

class SpriteResource;

struct SpriteScript {
	uint16 codeId;
	uint index;
	uint ticks;
	uint initialTicks;
	SpriteScript() : codeId(0), index(0) {}
	void set(uint16 ncodeId, uint nindex, uint nticks, uint ninitialTicks) {
		codeId = ncodeId;
		index = nindex;
		ticks = nticks;
		initialTicks = ninitialTicks;
	}
	void set2(uint16 ncodeId, uint nindex, uint nticks) {
		codeId = ncodeId;
		index = nindex;
		ticks = nticks;
	}
};

struct Sprite {
	byte status;
	byte frameIndex;
	int16 x, y;
	int16 width, height;
	int16 heightAdd, yAdd;
	int16 id;
	SpriteScript anim;
	SpriteScript moveX;
	SpriteScript moveY;
	SpriteResource *spriteResource;
	Sprite() : status(0), spriteResource(NULL) {}
	void setPos(int16 newX, int16 newY) {
		x = newX;
		y = newY;
	}
	void setCodeSync(uint16 animCodeId, uint16 moveXCodeId, uint16 moveYCodeId,
		byte ticks, byte initialTicks) {
		if (animCodeId != kNone)
			anim.set(animCodeId, 0, ticks, initialTicks);
		if (moveXCodeId != kNone)
			moveX.set(moveXCodeId, 0, ticks, initialTicks);
		if (moveYCodeId != kNone)
			moveY.set(moveYCodeId, 0, ticks, initialTicks);
	}
	void setCodeSync2(uint16 animCodeId, uint16 moveXCodeId, uint16 moveYCodeId,
		byte ticks) {
		if (animCodeId != kNone)
			anim.set2(animCodeId, 0, ticks);
		if (moveXCodeId != kNone)
			moveX.set2(moveXCodeId, 0, ticks);
		if (moveYCodeId != kNone)
			moveY.set2(moveYCodeId, 0, ticks);
	}
	void changeCodeSync(uint16 animCodeId, uint16 moveXCodeId, uint16 moveYCodeId) {
		if (animCodeId != kNone)
			anim.codeId = animCodeId;
		if (moveXCodeId != kNone)
			moveX.codeId = moveXCodeId;
		if (moveYCodeId != kNone)
			moveY.codeId = moveYCodeId;
	}
	void setTicksSync(byte ticks) {
		anim.ticks = ticks;
		moveX.ticks = ticks;
		moveY.ticks = ticks;
	}
	int16 x2() const { return x + width; }
	int16 y2() const { return y + height; }
};

struct SpriteDef {
	uint16 selfId;
	byte type;
	byte status;
	byte frameIndex;
	int16 x, y;
	uint16 templateId;
	void setPos(int16 newX, int16 newY) {
		x = newX;
		y = newY;
	}
};

struct SpriteTemplate {
	uint16 selfId;
	int16 heightAdd;
	int16 yAdd;
	uint16 id;
	uint16 animListId;
	byte animListTicks;
	byte animListInitialTicks;
	uint16 moveXListId;
	byte moveXListTicks;
	byte moveXListInitialTicks;
	uint16 moveYListId;
	byte moveYListTicks;
	byte moveYListInitialTicks;
};

struct SoundItem {
	byte priority;
	// Frequency value, same as e.g. in VOC files
	byte freq;
	uint16 sizeDecr;
	byte index;
};

struct DrawItem {
	Sprite *sprite;
	int16 priority;
};

struct SceneDecoration {
	int16 x, y;
	//int16 width, height;
	byte frameIndex;
	SpriteResource *spriteResource;
	bool redraw;
};

struct SceneItemInfo {
	byte flags;
	byte frameIndex;
};

struct SceneFilenames {
	Common::String mapFilename;
	Common::String sprFilename;
	Common::String sfxFilename;
};

struct SceneInitItem {
	byte status;
	byte frameIndex;
	int16 x, y;
	uint16 animListId;
	uint16 moveXListId;
	uint16 moveYListId;
	byte cameraStripNum;
	byte spriteResourceType;
};

struct WalkBox {
	byte type, param;
	int16 x1, y1, x2, y2;
};

struct WalkInfo {
	WalkInfo *next;
	int16 x, y;
	WalkInfo() : next(NULL), x(0), y(0) {
	}
};

struct AnimQueueItem {
	Sprite *sprite;
	byte ticks;
	byte initialTicks;
	uint16 animListId;
	uint16 moveXListId;
	uint16 moveYListId;
	AnimQueueItem() : sprite(NULL), animListId(0), moveXListId(0), moveYListId(0) {
	}
};

struct MenuSlot {
	MenuSlotAction action;
	int16 id;
};

struct SpriteListItem {
	int16 x, y;
	uint16 moveXListId;
	uint16 moveYListId;
};

struct Point {
	int16 x, y;
};

struct Color {
	uint8 r, g, b;
};

const uint kSpriteCount = 70;
const uint kSceneDecorationCount = 10;
const uint kMenuBarTableCount = 10;
const uint kInventoryTableCount = 10;
const uint kTalkTableCount = 10;
const uint kBoxWalkInfosCount = 10;
const uint kWalkInfosCount = 4;
const uint kAnimQueueCount = 70;

typedef Common::Array<Common::Rect> RectArray;

class EnchantiaEngine : public Engine {
private:

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

public:
	EnchantiaEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~EnchantiaEngine();

	void updateEvents();

	void quit();

private:
	const ADGameDescription	*_gameDescription;
	Common::RandomSource _rnd;

	DatFile *_dat;
	AdlibMusicPlayer *_adlibMusic;
	MidiPlayer *_midiPlayer;
	int32 _currRolandMusicFilenameIndex;

	int16 _mouseX, _mouseY;
	int _mouseButton;
	bool _buttonDown;

	Graphics::Surface *_screen;
	Graphics::Surface *_background;
	byte _palette[768];
	byte *_sceneMap;

	RectArray *_dirtyRects1, *_dirtyRects2;

	Sprite _sprites[kSpriteCount];
	SceneDecoration _sceneDecorations[kSceneDecorationCount];
	DrawItem _drawItems[kSpriteCount - 1]; // One less because sprite 0 is the mouse cursor

	SpriteScript _actorQueuedAnim;
	SpriteScript _actorQueuedMoveX;
	SpriteScript _actorQueuedMoveY;
	AnimQueueItem _animQueue[kAnimQueueCount];
	uint _animQueueCount;

	Common::Array<WalkBox> _walkBoxes;

	bool _flgCanRunMenu;
	bool _flgCanRunBoxClick;
	bool _flgWalkBoxEnabled;
	int _theScore;

	SpriteResource *_stdBrd;
	SpriteResource *_currBrd;
	SpriteResource *_iconsSpr;
	SpriteResource *_mouseSpr;
	SpriteResource *_sceneSpr;

	byte *_stdSfx, *_sceneSfx;
	byte _currSoundPriority;
	Audio::SoundHandle _soundHandle;

	int16 _currBrdIndex;
	int16 _currMouseSpriteIndex;
	bool _currMouseCursorVisible;

	int16 _sceneStripTableCount;
	const byte *_sceneStripTable;

	int16 _sceneIndex, _currSceneLink, _nextSceneLink;
	int _screenShakeStatus;
	byte _spriteResourceType;
	int16 _cameraStripNum, _cameraStripX;
	int16 _scrollBorderLeft, _scrollBorderRight;
	uint _sceneSpritesCount, _sceneDecorationCount;

	int16 _actorXTable[10];

	bool _collectItemIgnoreActorPosition;

	int16 _walkSlopeX, _someX;
	int16 _walkSlopeY, _someY;
	int32 _walkSlopeErrX, _walkSlopeErrY;
	uint32 _walkErrorX, _walkErrorY;
	int16 _walkIncrY2;
	int _walkDistance;

	WalkInfo *_currWalkInfo;
	WalkInfo _boxWalkInfos[kBoxWalkInfosCount];
	WalkInfo _walkInfos[kWalkInfosCount];
	int16 _queryBoxX1, _queryBoxY1, _queryBoxX2, _queryBoxY2;
	byte _queryBoxDefValue;

	int _sceneCountdown1;
	int _scene63BabySleepCounter;
	int _rockBasherCounters[3];

	int16 _useItemId;
	uint _currUseCommand;

	Graphics::Surface *_menuSurface;
	Graphics::Surface *_menuSprite;

	MenuSlot _menuSlots[kMenuBarTableCount];

	int16 _inventoryTable[kInventoryTableCount];
	int16 _talkTable[kTalkTableCount];

	byte _flags1, _flags2;
	byte _scene25Value;

	void setVgaPalette(byte *palette);
	void palFadeOut();

	int getRandom(int max);
	uint32 getMillis();

	void gameLoop();

	SpriteResource *loadBrd(int16 index);

	void initializeSprite(Sprite *sprite, SpriteDef *spriteDef, int16 cameraX = 0);
	Sprite *insertSprite(SpriteDef *spriteDef);
	void insertSpriteList(const SpriteListItem *spriteList, uint count, uint16 spriteDefId, uint spriteIndex, byte ticks);

	void loadScene(int16 sceneLink, bool isLoadGame);
	void loadSceneMap(const char *filename);

	void initSceneSprites();

	void buildDrawItems();
	void drawSprites();

	void drawSurface(Graphics::Surface *destSurface, Graphics::Surface *surface, int16 x, int16 y);
	void copySurfaceRect(Graphics::Surface *destSurface, Graphics::Surface *surface,
		Common::Rect &r);
	void drawSprite(Graphics::Surface *destSurface, Graphics::Surface *sprite, int16 x, int16 y, byte flags = 0,
		Common::Rect *clipRect = NULL);
	void drawStrip(int16 stripNum, int16 x);

	void drawMouseCursor();

	void queueWalk(int16 x, int16 y);
	void queueWalkToSprite(uint spriteIndex, int16 deltaX, int16 deltaY);
	void queueActor(uint16 animListId, uint16 moveXListId, uint16 moveYListId);
	void queueAnim(uint spriteIndex, byte ticks, byte initialTicks,
		uint16 animListId, uint16 moveXListId, uint16 moveYListId);

	void updateSpriteAnim(Sprite &sprite);
	void updateSpriteMoveX(Sprite &sprite);
	void updateSpriteMoveY(Sprite &sprite);
	void updateAnimations();
	void updateSceneBackground();
	void debugDrawWalkBoxes();
	void debugDrawSceneItems();
	void scrollScene(ScrollDirection direction);

	int16 startActorWalk(WalkInfo *walkInfo);
	void updateActorWalk();
	void setActorWalkDirection(byte direction);

	void loadPalette(const char *filename, byte *palette);
	Graphics::Surface *loadBitmap(const char *filename, int16 width, int16 height);
	void loadRaw(const char *filename, byte **buffer);
	void loadWalkBoxes(uint index);
	void checkWalkBoxes(int16 &currActorX, int16 &currActorY);
	void queryWalkBoxInfo(int16 x1, int16 y1, int16 x2, int16 y2, int16 &type, int16 &param);
	void checkBoxClick();

	int16 actionThumbsUp();
	void actionThumbsDown();

	void clearInventory();
	bool hasInventoryItem(int16 id);
	bool addInventoryItem(int16 id);
	bool removeInventoryItem(int16 id);

	void resetActor();

	void playSound(byte soundNum, Sprite &sprite);

	void runMenuBar();
	void updateMenuBar();
	void listMenuBarItems(int16 *table, int16 base, MenuSlotAction action);
	void collectSceneItems(uint firstIndex, uint minCount, byte itemFlags, MenuSlotAction action);
	void buildMenuBar(MenuSlotAction menu);
	MenuSlotAction runMenuBarAction(uint slotIndex);

	void updateFunc411h();

	SpriteDef *getSpriteDef(uint16 id);
	SpriteTemplate *getSpriteTemplate(uint16 id);
	SceneItemInfo &getSceneItemInfo(uint index);
	SpriteDef *getSceneSpriteDef(uint sceneIndex, uint index);

	Sprite& cursorSprite() { return _sprites[0]; }
	Sprite& actorSprite() { return _sprites[1]; }

	void setPaletteColor(uint16 index, Color c);

	// Logic functions
	void handleSceneInit(SceneInitItem &sceneInitItem, int16 sceneLink);
	void updateScene();
	void updateSprites();
	void handleBoxAction(int16 action);
	bool handleBoxSceneChange(int16 &type, int16 &param);
	bool performCommand(int cmd, int item1, int item2);
	void talkCommandHelp();
	void talkCommandHi();
	void talkCommandOpenSesame();
	void defaultTakeItem(uint type, int16 x, int16 y, uint spriteIndex);
	void playIntro();

public:

	/* Save/load */

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &description, bool isAutosave = false);
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header);

};

} // End of namespace Enchantia

#endif
