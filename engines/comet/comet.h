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

enum {
	kScriptWalking			= 0x01,
	kScriptSleeping			= 0x02,
	kScriptAnimPlaying		= 0x04,
	kScriptDialogRunning	= 0x10,
	kScriptPaused			= 0x20,
	kScriptTalking			= 0x40
};

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

struct RectItem {
	int x, y, x2, y2, id;
};

struct SceneItem {
	int itemIndex;
	bool active;
	int paramType;
	int x, y;
};

class Script {
public:
	byte *code;
	byte *ip;
	int16 objectIndex;
	uint16 status;
	int scriptNumber;
	int counter;
	int x, y, x2, y2;
	Script(CometEngine *vm) : _vm(vm) {
	}
	byte loadByte();
	int16 loadInt16();
	void jump();
	uint16 loadVarValue();
	uint16 loadValue();
	bool evalBoolOp(int value1, int value2, int boolOp);
	SceneObject *object() const;
private:
	CometEngine *_vm;
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
	bool _textFlag1, _textFlag2;
	int _textColor;
	
	bool _endLoopFlag;
	
	int _scriptRandomValue;
	
	Common::Rect _blockingRect;

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
	void sceneObjectUpdateDirectionTo(int objectIndex, SceneObject *sceneObject);
	void sceneObjectAvoidBlockingRect(int objectIndex, SceneObject *sceneObject);
	void resetVars();
	
	void drawSceneAnims();
	void drawSceneAnimsSub(int objectIndex);
	int drawSceneObject(Animation *animation, AnimationFrameList *frameList, int animFrameIndex, int value4, int x, int y, int animFrameCount);
	
	void updateTextDialog();
	void updateText();
	void updateTalkAnims();
	void sceneObjectUpdate01(SceneObject *sceneObject);
	void sceneObjectUpdate02(SceneObject *sceneObject);
	void sceneObjectUpdate03(SceneObject *sceneObject, int objectIndex, bool flag);
	bool sceneObjectUpdate04(int objectIndex);
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
	int checkCollisionWithBlockingRects(Common::Rect &rect);
	int checkCollisionWithActors(int skipIndex, Common::Rect &rect);
	uint16 checkCollision(int index, int x, int y, int deltaX, int deltaY, int direction);
	
	void handleSceneChange(int sceneNumber, int chapterNumber);

	uint16 findSceneItemAt(const Common::Rect &rect);
	void drawLineOfSight();
	
	PointArray _pointsArray;
	byte _xBuffer[320];
	
	void initPointsArray2();
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
	int _sceneObjectIndex, _animIndex, _animSubIndex2, _animSubIndex, _narSubIndex;
	void textProc(int objectIndex, int narSubIndex, int color);
	void textProc2(int objectIndex, int narSubIndex, int animNumber);

	/* SceneObjects */
	void sceneObjectsResetFlags();

	/* Misc */
	int comparePointXY(int x, int y, int x2, int y2);
	void calcRect01(Common::Rect &rect, int delta1, int delta2);

	/* Script */
	byte *_scriptData;
	int *_scriptVars1[256];
	int _scriptVars2[256], _scriptVars3[256];
	int _scriptCount;
	Script *_scripts[17];
	int _curScriptNumber;
	Script *_curScript;
	bool _scriptBreakFlag;
	void loadAndRunScript();
	void initializeScript();
	void prepareScript(int scriptNumber);
	void runScript(int scriptNumber);
	void runAllScripts();
	//TODO: Use something like getGlobalVar(index) and setGlobalVar(index, value) instead?
	int *getVarPointer(int varIndex);
	SceneObject *getScriptSceneObject();

	void processScriptStatus8();
	void processScriptSleep();
	void processScriptWalk();
	void processScriptAnim();
	void processScriptDialog();
	void processScriptTalk();
	
	bool rectCompare(const Common::Rect &rect1, const Common::Rect &rect2);
	bool rectCompare02(int objectIndex1, int objectIndex2, int x, int y);
	bool isPlayerInRect(int x, int y, int x2, int y2);

	void objectWalkToXYAbs(Script *script, bool xyFlag);
	void objectWalkToXYRel(Script *script, bool xyFlag);

	bool o1_Sub_rectCompare01(Script *script);
	
	void o1_addSceneItem(Script *script, int paramType);

	/* Script functions */
	void o1_sceneObjectSetDirection(Script *script);
	void o1_jump(Script *script);
	void o1_objectWalkToXAbs(Script *script);
	void o1_objectWalkToYAbs(Script *script);
	void o1_loop(Script *script);
	void o1_objectSetPosition(Script *script);
	void o1_sleep(Script *script);
	void o1_if(Script *script);
	void o1_condJump2(Script *script);
	void o1_objectWalkToXRel(Script *script);
	void o1_objectWalkToYRel(Script *script);
	void o1_setMouseFlags(Script *script);
	void o1_resetHeroDirectionChanged(Script *script);
	void o1_sceneObjectSetDirectionTo(Script *script);
	void o1_selectObject(Script *script);
	void o1_initPoints(Script *script);
	void o1_initSceneExits(Script *script);
	void o1_addSceneItem1(Script *script);
	void o1_startScript(Script *script);
	void o1_pauseScript(Script *script);
	void o1_playCutscene(Script *script);
	void o1_setVar(Script *script);
	void o1_incVar(Script *script);
	void o1_subVar(Script *script);
	void o1_setSceneObjectCollisionTypeTo8(Script *script);
	void o1_setSceneObjectCollisionTypeTo0(Script *script);
	void o1_updateDirection2(Script *script);
	void o1_setSceneNumber(Script *script);
	void o1_setAnimValues(Script *script);
	void o1_setMarcheNumber(Script *script);
	void o1_setZoomByItem(Script *script);
	void o1_startDialog(Script *script);
	void o1_waitWhilePlayerIsInRect(Script *script);
	void o1_waitUntilPlayerIsInRect(Script *script);
	void o1_unloadSceneObjectSprite(Script *script);
	void o1_setObjectClipX(Script *script);
	void o1_setObjectClipY(Script *script);
	void o1_orVar(Script *script);
	void o1_loadScene(Script *script);
	void o1_sceneObjectSetAnimNumber(Script *script);
	void o1_addBlockingRect(Script *script);
	void o1_sub_A67F(Script *script);
	void o1_sub_A64B(Script *script);
	void o1_sub_A735(Script *script);
	void o1_removeBlockingRect(Script *script);
	void o1_setSceneObjectColor(Script *script);
	void o1_setTextXY(Script *script);
	void o1_playMusic(Script *script);
	void o1_setRandomValue(Script *script);
	void o1_setChapterNumber(Script *script);
	void o1_dialog(Script *script);
	void o1_addSceneItem2(Script *script);
	void o1_playAnim(Script *script);
	void o1_sub_AD04(Script *script);
	void o1_initSceneObject(Script *script);
	void o1_loadSceneObjectSprite(Script *script);
	void o1_setObjectVisible(Script *script);
	void o1_paletteFadeIn(Script *script);
	void o1_paletteFadeOut(Script *script);
	void o1_setNarFileIndex(Script *script);
	void o1_deactivateSceneItem(Script *script);
	void o1_sample_2(Script *script);
	void o1_sample_1(Script *script);

public:
	/* Misc */
	//ALL FIXME
	int _dotFlag;
	int calcDirection(int x1, int y1, int x2, int y2);
	void drawSceneExits();

protected:

private:
};

void plotProc(int x, int y, int color, void *data = NULL);

// FIXME: remove global
//extern CometEngine *_vm;


} // End of namespace Comet

#endif
