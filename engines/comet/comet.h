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
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/config-manager.h"
#include "sound/mixer.h"

#include "engines/engine.h"

//#include "comet/music.h"

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
struct AnimationFrameList;
class ScriptInterpreter;

enum {
	kDirectionUp		= 3,
	kDirectionLeft		= 2,
	kDirectionDown		= 1,
	kDirectionRight		= 4
};

struct SceneObject {
	int x, y;
	int directionAdd, directionChanged, direction;
	int flag2;
	int marcheIndex;
	int animIndex;
	int animFrameIndex;
	int value4;
	int animFrameCount;
	int animSubIndex2;
	int deltaX, deltaY;
	uint16 collisionType;
	int linesIndex;
	int value6;
	int flag;
	int color;
	int value7;
	int textX, textY;
	uint16 walkStatus;
	int x2, y2;
	int x3, y3;
	int x5, x6, y5, y6;
	bool visible;
	int value8;
};

struct MarcheItem {
	int marcheNumber;
	int fileIndex;
	Animation *anim;
};

struct SceneExitItem {
	int directionIndex;
	int chapterNumber;
	int sceneNumber;
	//int unused;
	int x, x2;
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
	Common::RandomSource _rnd;

	Screen *_screen;
	Dialog *_dialog;
	ScriptInterpreter *_script;

	byte *_sceneBackground, *_scratchBuffer;
	byte *_textBuffer1, *_textBuffer2, *_textBuffer3;
	byte *_palette;

	MarcheItem _marcheItems[20];
	int _marcheNumber;

	SceneObject _sceneObjects[11];

	Common::Array<SceneExitItem> _sceneExits;
	Common::Array<SceneItem> _sceneItems;
	int _itemX, _itemY, _itemDirection, _inventoryItemIndex;

	int _paletteValue2;
	byte *_paletteBuffer;

	int _backgroundFileIndex;

	int _narFileIndex;
	bool _narOkFlag;

	byte *_currentText, *_textNextPos;
	int _textSpeed;
	int _textMaxTextHeight, _textMaxTextWidth, _textDuration;

	/* Input related */
	Common::KeyCode _keyScancode;
	int _keyDirection, _keyDirection2;
	int _mouseButtons4, _mouseButtons5;
	int _mouseX, _mouseY;
	int _mouseCursor2;
	int _mouseFlag;
	int _scriptMouseFlag;

	bool _needToLoadSavegameFlag, _loadingGameFlag;
	
	int _startupChapterNumber, _startupSceneNumber;
	int _chapterNumber, _sceneNumber;
	int _currentChapterNumber, _currentSceneNumber;
	int _prevChapterNumber, _prevSceneNumber;

	Animation *_bubbleSprite, *_heroSprite, *_objectsVa2, *_cursorVa2, *_iconeVa2;
	Animation *_sceneObjectsSprite;
	byte *_ctuPal, *_flashbakPal, *_cdintroPal, *_pali0Pal;

	int _gameLoopCounter;
	
	bool _cmdLook, _cmdGet, _cmdTalk;
	int _invActiveItem;

	int _textColorFlag;
	bool _flag03, _itemInSight;

	int _portraitTalkCounter, _portraitTalkAnimNumber;

	Common::Array<Common::Rect> _blockingRects;

	int _talkieMode;
	bool _moreText, _textActive;
	int _textColor;
	
	bool _endLoopFlag;
	
	int _scriptRandomValue;
	
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

	void initAndLoadGlobalData();
	void loadGlobalTextData();
	void initData();

	void handleEvents();
	void waitForKeys();
	void handleInput();
	void handleKeyInput();
	
	void handleInventory();
	
	void invUseItem();
	
	void skipText();

	void setChapterAndScene(int chapterNumber, int sceneNumber);
	void updateGame();
	void updateChapterNumber();
	void updateSceneNumber();
	void updateSub02();
	void updateSub03(bool flag);
	void sceneObjectsUpdate01();
	void sceneObjectsUpdate02();
	void updateStaticObjects();
	void sceneObjectsUpdate03();
	void updateSceneObjectFlag();
	void sceneObjectUpdateDirectionTo(int objectIndex, SceneObject *sceneObject, Common::Rect &obstacleRect);
	void sceneObjectMoveAroundObstacle(int objectIndex, SceneObject *sceneObject, Common::Rect &obstacleRect);
	void resetVars();
	
	void drawSceneAnims();
	void drawSceneAnimsSub(int objectIndex);
	int drawSceneObject(Animation *animation, AnimationFrameList *frameList, int animFrameIndex, int value4, int x, int y, int animFrameCount);
	
	void updateTextDialog();
	void updateText();
	void updateTalkAnims();
	void sceneObjectUpdatePortraitAnimation(SceneObject *sceneObject);
	void sceneObjectUpdateAnimation(SceneObject *sceneObject);
	void sceneObjectUpdate03(SceneObject *sceneObject, int objectIndex, bool flag, Common::Rect &obstacleRect);
	bool sceneObjectUpdate04(int objectIndex, Common::Rect &obstacleRect);
	void sceneObjectEnqueueForDrawing(int y, int objectIndex);
	void freeMarcheAndStaticObjects();
	void resetMarcheAndStaticObjects();

	void updateScreen();

	int random(int maxValue);
	
	/* Graphics */
	void drawDottedLine(int x1, int y1, int x2, int y2, int color);

	void drawBubble(int x1, int y1, int x2, int y2);
	void decodeText(byte *text, int size, int key);
	uint32 loadString(int index, int subIndex, byte *text);
	void loadTextData(byte *textBuffer, int index, int size);
	byte *getTextEntry(int index, byte *textBuffer);
	void setText(byte *text);
	void resetTextValues();
	void drawDialogTextBubbles();
	void setTextEx(int index, byte *textBuffer);

	/* Scene */
	void initSceneBackground();
	void initStaticObjectRects();
	void initPoints(byte *data);
	void initSceneExits(byte *data);
	void addBlockingRect(int x, int y, int x2, int y2);
	void loadSceneBackground();
	void loadStaticObjects();
	void drawSceneForeground();

	int checkLinesSub(int chapterNumber, int sceneNumber);

	int checkCollisionWithRoomBounds(const Common::Rect &rect, int direction);
	int checkCollisionWithBlockingRects(Common::Rect &rect, Common::Rect &obstacleRect);
	int checkCollisionWithActors(int skipIndex, Common::Rect &rect, Common::Rect &obstacleRect);
	uint16 checkCollision(int index, int x, int y, int deltaX, int deltaY, int direction, Common::Rect &obstacleRect);
	
	void handleSceneChange(int sceneNumber, int chapterNumber);

	uint16 findSceneItemAt(const Common::Rect &rect);
	void drawLineOfSight();
	
	PointArray _bounds;
	byte _boundsMap[320];
	
	void initSceneBoundsMap();
	int Points_getY_sub_8419(int x, int y);
	int Points_getY_sub_8477(int x, int y);
	void rect_sub_CC94(int &x, int &y, int deltaX, int deltaY);
	
	void sceneObjectDirection2(int index, SceneObject *sceneObject);
	
	int checkCollisionWithSceneExits(const Common::Rect &rect, int direction);
	uint16 handleCollision(SceneObject *sceneObject, int index, uint16 collisionType);
	
	void getSceneExitRect(int index, int &x, int &y, int &x2, int &y2);

	void resetHeroDirectionChanged();

	/* Marche */
	int findMarcheItem(int marcheNumber, int fileIndex);
	int findFreeMarcheSlot();
	bool isMarcheLoaded(int marcheIndex);
	void clearMarcheByIndex(int marcheIndex);
	Animation *loadMarcheData(const char *pakFilename, int fileIndex);
	Animation *getMarcheAnim(int marcheNumber);
	void freeAllMarche();
	void freeMarche();
	int loadMarche(int marcheNumber, int fileIndex);
	void freeMarcheAnims();
	void loadAllMarche();
	void unloadSceneObjectSprite(SceneObject *sceneObject);

	/* SceneObject */
	void sceneObjectInit(int itemIndex, int marcheIndex);
	void sceneObjectSetDirection(SceneObject *sceneObject, int direction);
	void sceneObjectSetDirectionAdd(SceneObject *sceneObject, int directionAdd);
	void sceneObjectSetAnimNumber(SceneObject *sceneObject, int index);
	void sceneObjectResetDirectionAdd(SceneObject *sceneObject);
	void sceneObjectCalcDirection(SceneObject *sceneObject);
	void sceneObjectGetXY1(SceneObject *sceneObject, int &x, int &y);
	void sceneObjectSetXY(int index, int x, int y);
	void sceneObjectUpdateFlag(SceneObject *sceneObject, int flag);
	void sceneObjectUpdateXYFlags(SceneObject *sceneObject);
	bool sceneObjectUpdateDirection2(int objectIndex, int x, int y);
	SceneObject *getSceneObject(int index);

	/* Text */
	int _talkActorIndex, _animIndex, _animSubIndex2, _animSubIndex, _narSubIndex;
	void actorSay(int objectIndex, int narSubIndex, int color);
	void actorSayWithAnim(int objectIndex, int narSubIndex, int animNumber);

	/* SceneObjects */
	void sceneObjectsResetFlags();

	/* Misc */
	int comparePointXY(int x, int y, int x2, int y2);
	void calcSightRect(Common::Rect &rect, int delta1, int delta2);

	/* Script */
	int *_scriptVars1[256];
	int _scriptVars2[256], _itemStatus[256];
	void loadAndRunScript();

	bool rectCompare(const Common::Rect &rect1, const Common::Rect &rect2);
	bool rectCompare02(int objectIndex1, int objectIndex2, int x, int y);
	bool isPlayerInRect(int x, int y, int x2, int y2);

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

// FIXME: remove global
//extern CometEngine *_vm;


} // End of namespace Comet

#endif
