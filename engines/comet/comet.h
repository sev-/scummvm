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

#ifndef COMET_H
#define COMET_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/array.h"
#include "common/list.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/config-manager.h"
#include "audio/mixer.h"

#include "engines/engine.h"

#include "comet/console.h"
#include "comet/inventory.h"

namespace Comet {

struct CometGameDescription;

class Dialog;
class Screen;

class Anim;
class MusicPlayer;
class CometEngine;
class CometConsole;

class Actor;
class Actors;
class AnimationResource;
struct AnimationElement;
struct AnimationCel;
struct AnimationFrame;
class AnimationManager;
struct AnimationFrameList;
struct InterpolatedAnimationElement;
class Input;
class Gui;
class ResourceManager;
class ScriptInterpreter;
class Scene;
class ScreenResource;
class SoundResource;
class TalkText;
class TextReader;
class TextResource;
class FontResource;
class SystemMouseCursor;

enum CometGameID {
	GID_COMET	= 0,	// Shadow of the Comet
	GID_MUSEUM	= 1		// Lovecraft Museum
};

enum CometGameFeatures {
	GF_DEMO				= 1 << 0,
	GF_CD				= 1 << 1,
	GF_CD_COMPRESSED	= 1 << 2,
	GF_FLOPPY			= 1 << 3
};

enum {
	kDirectionUp		= 3,
	kDirectionLeft		= 2,
	kDirectionDown		= 1,
	kDirectionRight		= 4
};

enum {
	kCollisionNone		= 0,
	kCollisionBounds	= 1,
	kCollisionBoundsOff	= 2,
	kCollisionBlocking	= 3,
	kCollisionSceneExit	= 4,
	kCollisionSceneItem	= 5,
	kCollisionActor		= 6,
	kCollisionDisabled	= 8
};

enum {
	kActorPortrait	=	10
};

#define COLLISION(type, index) (((type) << 8) | (index))
#define COLLISION_TYPE(collision) (((collision) >> 8) & 0xFF)
#define COLLISION_INDEX(collision) ((collision) & 0xFF)

struct SpriteDraw {
	byte y;
	byte index;
};

struct GuiRectangle {
	int x, y, x2, y2, id;
};

struct Beam {
	int x1, y1, x2, y2;
};

typedef Common::Array<Common::Point> PointArray;

// Engine Debug Flags
enum {
	kDebugResource	= (1 << 0),
	kDebugAnimation	= (1 << 1),
	kDebugSaveLoad	= (1 << 2),
	kDebugScript	= (1 << 3),
	kDebugText		= (1 << 4),
	kDebugCollision	= (1 << 5),
	kDebugScreen	= (1 << 6)
};

class Verbs {
public:
	Verbs() {
		clear();
	}
	void clear() {
		_look = false;
		_get = false;
		_talk = false;
	}
	bool isLookRequested() const { return _look; }
	bool isGetRequested() const { return _get; }
	bool isTalkRequested() const { return _talk; }
	void clearLookRequested() { _look = false; }
	void clearGetRequested() { _get = false; }
	void clearTalkRequested() { _talk = false; }
	void requestLook() { _look = true; }
	void requestGet() { _get = true; }
	void requestTalk() { _talk = true; }
protected:
	bool _look, _get, _talk;
};

class CometEngine : public Engine {
protected:
	Common::Error run();
	void shutdown();

	bool initGame();

public:
	CometEngine(OSystem *syst, const CometGameDescription *gameDesc);
	virtual ~CometEngine();

	virtual bool hasFeature(EngineFeature f) const;

	int getGameType() const;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	const Common::String& getTargetName() const { return _targetName; }
	bool isFloppy() const { return getFeatures() & GF_FLOPPY; }

	const char *getGameFile(int fileType);

	const CometGameDescription *_gameDescription;

	GUI::Debugger *getDebugger() { return _console; }
	
	virtual void syncSoundSettings();

private:
	bool detectGame();

//protected:
// Everything is public during the transition phase to more object-oriented design
public:
	Common::RandomSource *_rnd;

	Screen *_screen;
	Dialog *_dialog;
	ScriptInterpreter *_script;
	Scene *_scene;
	AnimationManager *_animationMan;
	ResourceManager *_res;
	Input *_input;

	ScreenResource *_sceneBackgroundResource;
	TalkText *_talkText;

	byte *_tempScreen;

	TextReader *_textReader;
	TextResource *_globalStrings, *_inventoryItemNames;

	Gui *_gui;
	AnimationCel *_mouseCursors[7];

	int16 _animationType;

	Actors *_actors;

	int _itemX, _itemY, _itemDirection;

	int _paletteStatus;
	byte _paletteBrightness, _paletteRedness;
	bool _clearScreenRequest;

	int _backgroundFileIndex;

	// TODO Game speed is currently not yet implemented
	int _gameSpeed;
	uint32 _nextTick;

	bool _loadgameRequested;

	int16 _startupModuleNumber, _startupSceneNumber;
	int16 _moduleNumber, _sceneNumber;
	int16 _currentModuleNumber, _currentSceneNumber;
	int16 _prevModuleNumber, _prevSceneNumber;

	AnimationResource *_bubbleSprite, *_heroSprite, *_inventoryItemSprites, *_cursorSprite, *_iconSprite;
	AnimationResource *_sceneDecorationSprite;
	const byte *_currCursorSprite;
	SystemMouseCursor *_systemMouseCursor;

	byte *_screenPalette, *_backupPalette;

	byte *_gamePalette, *_flashbakPal;
	byte *_introPalette1, *_introPalette2;

	int _gameLoopCounter;
	bool _endIntroLoop;

	int _textColorFlag;
	bool _itemInSight;

	int _portraitTalkCounter, _portraitTalkAnimNumber;

	int16 _scriptRandomValue;

	Verbs _verbs;
	Inventory _inventory;

	// Audio
	MusicPlayer *_music;

	// Sprite array
	Common::Array<SpriteDraw> _spriteDrawQueue;

	// Filenames
	Common::String _animPakName, _scenePakName, _scriptFileName;

	Audio::SoundHandle _sampleHandle;
	SoundResource *_soundResource;
	int _currSoundResourceIndex;

	void initAndLoadGlobalData();
	void loadGlobalTextData();
	void initData();

	void handleKeyInput();
	void syncUpdate(bool screenUpdate = true);

	void drawTextIllsmouth();

	void useCurrentInventoryItem();
	void checkCurrentInventoryItem();

	void stopText();

	void setModuleAndScene(int moduleNumber, int sceneNumber);
	void updateGame();
	void updateModuleNumber();
	void updateSceneNumber();
	void getItemInSight();
	void lookAtItemInSight(bool showText);
	void buildSpriteDrawQueue();
	void addToSpriteDrawQueue(int y, int actorIndex, int insertIndex);
	void enqueueSceneDecorationForDrawing();
	void enqueueActorForDrawing(int y, int actorIndex);
	void updateHeroLife();
	void handleActorCollision(int actorIndex, Actor *actor, Common::Rect &obstacleRect);
	void actorMoveAroundObstacle(int actorIndex, Actor *actor, Common::Rect &obstacleRect);
	void resetVars();

	void drawSpriteQueue();
	void drawActor(int actorIndex);

	void updateTextDialog();
	void updateText();
	void updateTalkAnims();
	void updatePortraitAnimation(Actor *actor);
	void updateActorAnimation(Actor *actor);
	void actorUpdateWalking(Actor *actor, int actorIndex, bool skipCollision, Common::Rect &obstacleRect);
	bool updateActorPosition(int actorIndex, Common::Rect &obstacleRect);
	void freeAnimationsAndSceneDecoration();

	int getPortraitTalkAnimNumber();
	AnimationFrame *getAnimationFrame(int animationSlot, int animIndex, int animFrameIndex);

	void updateScreen();

	// cursorSprite = NULL uses the engine's system cursor
	void setMouseCursor(int cursorNum);
	void setMouseCursorSprite(const AnimationCel *cursorSprite);

	int16 randomValue(int maxValue);
	
	void drawBubble(int x1, int y1, int x2, int y2);
	void setText(byte *text);
	void resetTextValues();
	void drawDialogTextBubbles();
	void showTextBubble(int index, byte *text, int textDuration);

	// Scene
	void initSceneBackground(bool loadingGame = false);
	void initSceneDecorationBlockingRects();
	void loadSceneBackground();
	void loadSceneDecoration();
	void drawSceneDecoration();

    int handleSceneExitCollision(int sceneExitIndex);
	int handleLeftRightSceneExitCollision(int newModuleNumber, int newSceneNumber);

	void handleSceneChange(int sceneNumber, int moduleNumber);

	void drawLineOfSight();
	
	void moveActorAroundBounds(int index, Actor *actor);
	
	void blockInput(int flagIndex);
	void unblockInput();

	int mouseCalcCursorDirection(int fromX, int fromY, int toX, int toY);

	// Marche
	bool isAnimationSlotUsed(int16 animationSlot);
	void clearAnimationSlotByIndex(int16 animationSlot);
	AnimationResource *getGlobalAnimationResource(int16 animationType);
	void unloadActorSprite(Actor *actor);

	// Actor
	void actorInit(int itemIndex, int16 animationSlot);
	void actorSetDirection(Actor *actor, int direction);
	void actorSetDirectionAdd(Actor *actor, int directionAdd);
	void actorSetAnimNumber(Actor *actor, int index);
	void actorStopWalking(Actor *actor);
	void actorCalcDirection(Actor *actor);
	void actorGetNextWalkDestXY(Actor *actor, int &x, int &y);
	void actorSetPosition(int index, int x, int y);
	void actorUpdateLife(Actor *actor, int flag);
	void actorSaveWalkDestXY(Actor *actor);
	bool actorStartWalking(int actorIndex, int x, int y);
	Actor *getActor(int index);

	// Misc
	int comparePointXY(int x, int y, int x2, int y2);

	// Script
	int16 *_systemVars[256];
	int16 _scriptVars[256];
	void loadAndRunScript(bool loadingGame = false);

	bool rectCompare(const Common::Rect &rect1, const Common::Rect &rect2);
	
	int findRect(const GuiRectangle *rects, int x, int y, int count, int defaultId);
	void warpMouseToRect(const GuiRectangle &rect);
	
	bool isActorNearActor(int actorIndex1, int actorIndex2, int x, int y);
	bool isPlayerInZone(int x1, int y1, int x2, int y2);

	void openConsole();

	// Save/load

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
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;
	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &description);
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header);

	// TODO Subclass Serializer and move those two there
	void syncAsPoint(Common::Serializer &s, Common::Point &point);
	void syncAsRect(Common::Serializer &s, Common::Rect &rect);

	void syncScriptVars(Common::Serializer &s);

	int handleMap();

	void playMusic(int musicNumber);
	void playSample(int sampleNumber, int loopCount);
	
	void playCutscene(int fileIndex, int frameListIndex, int backgroundIndex, int loopCount, int soundFramesCount, byte *soundFramesData);

	// Beams
	Common::Array<Beam> _beams;
	
	// Unused in Comet CD
	//byte _beamColor;
	//int _beamColorIncr;

	void addBeam(int x1, int y1, int x2, int y2);
	void drawBeam(int x1, int y1, int x2, int y2);
	void drawBeams();
	
	void initSystemVars();

	void introMainLoop();
	void cometMainLoop();
	void museumMainLoop();
	void checkPauseGame();

public:
	// Misc
	//ALL FIXME
	int calcDirection(int fromX, int fromY, int toX, int toY);
	void drawSceneExits();

protected:

private:
	CometConsole *_console;
};

void plotProc(int x, int y, int color, void *data = NULL);

} // End of namespace Comet

#endif
