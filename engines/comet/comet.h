/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/config-manager.h"
#include "sound/mixer.h"

#include "engines/engine.h"

//#include "comet/music.h"
#include "comet/text.h"

namespace Comet {

enum {
	kFileTypeHash
};

enum {
	kDebugScript = 1 << 0
};

struct CometGameDescription;

class Dialog;
class Screen;

class Font;
class Anim;
class MusicPlayer;
class CometEngine;

class Animation;
class AnimationElement;
class AnimationManager;
struct AnimationFrameList;
class InterpolatedAnimationElement;
class ScriptInterpreter;
class Scene;

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

struct Actor {
	int16 x, y;
	int16 directionAdd, directionChanged, direction;
	byte flag2;
	int16 animationSlot;
	int16 animIndex;
	int16 animFrameIndex;
	int16 value4;
	int16 animFrameCount;
	int16 animSubIndex2;
	int16 deltaX, deltaY;
	uint16 collisionType;
	int16 collisionIndex;
	byte value6;
	int16 life;
	byte textColor;
	byte value7;
	int16 textX, textY;
	uint16 walkStatus;
	int16 walkDestX, walkDestY;
	int16 savedWalkDestX, savedWalkDestY;
	int16 clipX1, clipY1, clipX2, clipY2;
	bool visible;
};

struct AnimationSlot {
	int16 animationType;
	int16 fileIndex;
	Animation *anim;
};

struct SceneExitItem {
	int directionIndex;
	int moduleNumber;
	int sceneNumber;
	int x1, x2;
};

struct SpriteDraw {
	byte y;
	byte index;
};

struct SceneItem {
	int itemIndex;
	bool active;
	int paramType;
	int x, y;
};

struct RectItem {
	int x, y, x2, y2, id;
};

typedef Common::Array<Common::Point> PointArray;

class CometEngine : public Engine {
	// FIXME: I don't need no friends
	friend class Script;
	friend class Animation;
protected:
	Common::Error run();
	void shutdown();
	
	bool initGame();

public:
	CometEngine(OSystem *syst, const CometGameDescription *gameDesc);
	virtual ~CometEngine();

	int getGameType() const;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;

	const char *getGameFile(int fileType);

	const CometGameDescription *_gameDescription;

private:
	bool detectGame();

//protected:
// Everything is public during the transition phase to more object-oriented design
public:
	Common::RandomSource *_rnd;

	bool _debugRectangles;

	Screen *_screen;
	Dialog *_dialog;
	ScriptInterpreter *_script;
	Scene *_scene;
	AnimationManager *_animationMan;

	byte *_sceneBackground;

	TextReader *_textReader;
	TextStrings *_textBuffer2, *_textBuffer3; // TODO: Better names

	byte *_palette;

	int16 _animationType;

	Actor _actors[11];

	Common::Array<SceneItem> _sceneItems;
	int _itemX, _itemY, _itemDirection, _inventoryItemIndex;

	int _introPaletteState;
	byte *_paletteBuffer;
	int _paletteBrightness;
	bool _clearScreenRequest;

	int _backgroundFileIndex;

	int _narFileIndex;
	bool _narOkFlag;

	byte *_currentText, *_textNextPos;
	int _textSpeed;
	int _textMaxTextHeight, _textMaxTextWidth, _textDuration;

	/* Input related */
	Common::KeyCode _keyScancode;
	int _keyDirection, _keyDirection2;
	int16 _mouseButtons4, _mouseButtons5;
	int _mouseX, _mouseY;
	bool _leftButton, _rightButton;
	int _mouseCursor2;
	int _blockedInput;
	int16 _scriptMouseFlag;

	bool _needToLoadSavegameFlag;
	
	int16 _startupModuleNumber, _startupSceneNumber;
	int16 _moduleNumber, _sceneNumber;
	int16 _currentModuleNumber, _currentSceneNumber;
	int16 _prevModuleNumber, _prevSceneNumber;

	Animation *_bubbleSprite, *_heroSprite, *_objectsVa2, *_cursorVa2, *_iconSprite;
	Animation *_sceneObjectsSprite;
	byte *_ctuPal, *_flashbakPal, *_cdintroPal, *_pali0Pal;

	int _gameLoopCounter;
	
	bool _cmdLook, _cmdGet, _cmdTalk;
	int _invActiveItem;
	byte _invSelectionColor;

	int _textColorFlag;
	bool _textBubbleActive, _itemInSight;

	int _portraitTalkCounter, _portraitTalkAnimNumber;

	int _talkieMode;
	bool _moreText, _textActive;
	byte _talkTextColor;
	
	bool _endLoopFlag;
	
	int16 _scriptRandomValue;
	
	/* Audio */
	MusicPlayer *_music;

	/* Sprite array */
	Common::Array<SpriteDraw> _spriteArray;

	/* Filenames */
	char AName[16], DName[16], RName[16];

	Common::File *_narFile;
	int _narCount;
	uint32 *_narOffsets;
	Audio::SoundHandle _voiceHandle;

	void openVoiceFile(int index);
	void playVoice(int number);
	void stopVoice();

	void initAndLoadGlobalData();
	void loadGlobalTextData();
	void initData();

	void handleEvents();
	void waitForKeys();
	void handleInput();
	void handleKeyInput();
	
	int handleInventory();
	void drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter);
	void invUseItem();
	void invCheckActiveItem();
	int handleReadBook();
	void drawBookPage(int pageTextIndex, int pageTextMaxIndex, byte fontColor);
	void bookTurnPage(bool turnDirection);
	void bookTurnPageTextEffect(bool turnDirection, int pageTextIndex, int pageTextMaxIndex);
	
	void skipText();

	void setModuleAndScene(int moduleNumber, int sceneNumber);
	void updateGame();
	void updateModuleNumber();
	void updateSceneNumber();
	void getItemInSight();
	void lookAtItemInSight(bool flag);
	void updateActorAnimations();
	void updateActorMovement();
	void updateStaticObjects();
	void enqueueActorsForDrawing();
	void updateHeroLife();
	void handleActorCollision(int actorIndex, Actor *actor, Common::Rect &obstacleRect);
	void actorMoveAroundObstacle(int actorIndex, Actor *actor, Common::Rect &obstacleRect);
	void resetVars();
	
	void drawSprites();
	void drawActor(int actorIndex);
	int drawActorAnimation(Animation *animation, AnimationFrameList *frameList, int animFrameIndex, int value4, int x, int y, int animFrameCount);
	void drawAnimatedIcon(Animation *animation, uint frameListIndex, int x, int y, uint animFrameCounter);
	
	void updateTextDialog();
	void updateText();
	void updateTalkAnims();
	void updatePortraitAnimation(Actor *actor);
	void updateActorAnimation(Actor *actor);
	void actorUpdateWalking(Actor *actor, int actorIndex, bool flag, Common::Rect &obstacleRect);
	bool updateActorPosition(int actorIndex, Common::Rect &obstacleRect);
	void enqueueActorForDrawing(int y, int actorIndex);
	void freeMarcheAndStaticObjects();
	void resetMarcheAndStaticObjects();

	void updateScreen();

	int16 random(int maxValue);
	
	/* Graphics */
	void drawDottedLine(int x1, int y1, int x2, int y2, int color);

	void drawBubble(int x1, int y1, int x2, int y2);
	void setText(byte *text);
	void resetTextValues();
	void drawDialogTextBubbles();
	void showTextBubble(int index, byte *text);

	/* Scene */
	void initSceneBackground(bool loadingGame = false);
	void initStaticObjectRects();
	void loadSceneBackground();
	void loadStaticObjects();
	void drawSceneForeground();

	int handleLeftRightSceneExitCollision(int moduleNumber, int sceneNumber);

	uint16 checkCollisionWithActors(int selfActorIndex, Common::Rect &rect, Common::Rect &obstacleRect);
	uint16 checkCollision(int index, int x, int y, int deltaX, int deltaY, int direction, Common::Rect &obstacleRect);
	
	void handleSceneChange(int sceneNumber, int moduleNumber);

	uint16 findSceneItemAt(const Common::Rect &rect);
	void drawLineOfSight();
	
	void moveActorAroundBounds(int index, Actor *actor);
	
	uint16 updateCollision(Actor *actor, int actorIndex, uint16 collisionType);
	
	void unblockInput();

	/* Marche */
	bool isAnimationSlotUsed(int16 animationSlot);
	void clearAnimationSlotByIndex(int16 animationSlot);
	Animation *getGlobalAnimationResource(int16 animationType);
	void unloadActorSprite(Actor *actor);

	/* Actor */
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

	/* Text */
	int _talkActorIndex, _animIndex, _animSubIndex2, _animSubIndex, _talkTextIndex;
	void actorTalk(int actorIndex, int talkTextIndex, int color);
	void actorTalkWithAnim(int actorIndex, int talkTextIndex, int animNumber);

	/* SceneObjects */
	void resetActorsLife();

	/* Misc */
	int comparePointXY(int x, int y, int x2, int y2);
	void calcSightRect(Common::Rect &rect, int delta1, int delta2);

	/* Script */
	int16 *_systemVars[256];
	int16 _scriptVars[256], _itemStatus[256];
	void loadAndRunScript(bool loadingGame = false);

	bool rectCompare(const Common::Rect &rect1, const Common::Rect &rect2);
	
	int findRect(const RectItem *rects, int x, int y, int count, int defaultId);
	
	bool isActorNearActor(int actorIndex1, int actorIndex2, int x, int y);
	bool isPlayerInZone(int x1, int y1, int x2, int y2);

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
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;
	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *description);
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header);

	/* Puzzle */
	uint16 _puzzleTiles[6][6];
	Animation *_puzzleSprite;
	int _puzzleTableRow, _puzzleTableColumn;
	int _puzzleCursorX, _puzzleCursorY;
	 
	int runPuzzle();
	void puzzleDrawFinger();
	void puzzleDrawField();
	void puzzleDrawTile(int columnIndex, int rowIndex, int xOffs, int yOffs);
	void puzzleMoveTileColumn(int columnIndex, int direction);
	void puzzleMoveTileRow(int rowIndex, int direction);
	bool puzzleTestIsSolved();

	void updateMap();
	void handleMap();

public:
	/* Misc */
	//ALL FIXME
	int _dotFlag;
	int calcDirection(int fromX, int fromY, int toX, int toY);
	void drawSceneExits();

protected:

private:
};

void plotProc(int x, int y, int color, void *data = NULL);

} // End of namespace Comet

#endif
