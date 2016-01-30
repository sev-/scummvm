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

#include "common/stream.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"
#include "graphics/surface.h"

#include "comet/comet.h"
#include "comet/actor.h"
#include "comet/animationmgr.h"
#include "comet/comet_gui.h"
#include "comet/dialog.h"
#include "comet/input.h"
#include "comet/music.h"
#include "comet/resource.h"
#include "comet/resourcemgr.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/talktext.h"

namespace Comet {

// CometEngine

int CometEngine::comparePointXY(int x, int y, int x2, int y2) {
	int flags = 0;
	if (x == x2)
		flags |= 1;
	if (y == y2)
		flags |= 2;
	return flags;
}

int CometEngine::calcDirection(int fromX, int fromY, int toX, int toY) {

	int deltaX = toX - fromX;
	int deltaY = toY - fromY;

	if (ABS(deltaX) / 2 > ABS(deltaY)) {
		if (deltaX > 0)
			return 2;
		else
			return 4;
	} else {
		if (deltaY > 0)
			return 3;
		else
			return 1;
	}

}

// Scene

void CometEngine::initSceneBackground(bool loadingGame) {
	_screen->clear();

	loadSceneBackground();
	loadSceneDecoration();

	// NOTE: Unused in Comet CD
	//if (screenCopyFlag != 0)
	//	screen_c_1();
	
	if (!loadingGame)
		_scene->initBlockingRectsFromAnimation(_sceneDecorationSprite);
}

void CometEngine::loadSceneBackground() {
	_res->loadFromPak(_sceneBackgroundResource, _scenePakName.c_str(), _backgroundFileIndex);
}

void CometEngine::loadSceneDecoration() {
	delete _sceneDecorationSprite;
	_sceneDecorationSprite = _animationMan->loadAnimationResource(_scenePakName.c_str(), _backgroundFileIndex + 1);
}

void CometEngine::drawSceneDecoration() {
	if (_sceneDecorationSprite->getElementCount() > 0)
		_screen->drawAnimationElement(_sceneDecorationSprite, 0, 0, 0);
}

// Graphics

void CometEngine::initAndLoadGlobalData() {

	if (isFloppy()) {
		uint32 resDataSize;
		byte *resData = _res->loadRawFromPak("RES.PAK", 0, &resDataSize);
		_screen->loadFontFromRaw(resData, resDataSize, 6, 0);
		_bubbleSprite = _animationMan->loadAnimationResourceFromRaw(resData, resDataSize, 6, 1);
		_heroSprite = _animationMan->loadAnimationResourceFromRaw(resData, resDataSize, 6, 2);
		_iconSprite = _animationMan->loadAnimationResourceFromRaw(resData, resDataSize, 6, 3);
		_inventoryItemSprites = _animationMan->loadAnimationResourceFromRaw(resData, resDataSize, 6, 4);
		_gamePalette = _res->loadRawFromRaw(resData, resDataSize, 6, 5);
		_flashbakPal = _res->loadRawFromRaw(resData, resDataSize, 6, 6);
		free(resData);
	} else {
		_screen->loadFont("RES.PAK", 0);
		_bubbleSprite = _animationMan->loadAnimationResource("RES.PAK", 1);
		_heroSprite = _animationMan->loadAnimationResource("RES.PAK", 2);
		_iconSprite = _animationMan->loadAnimationResource("RES.PAK", 3);
		_inventoryItemSprites = _animationMan->loadAnimationResource("RES.PAK", 4);
		_gamePalette = _res->loadRawFromPak("RES.PAK", 5);
		_flashbakPal = _res->loadRawFromPak("RES.PAK", 6);
		_introPalette1 = _res->loadRawFromPak("RES.PAK", 7);
		_introPalette2 = _res->loadRawFromPak("RES.PAK", 8);
		// Initialize mouse cursors
		_cursorSprite = _animationMan->loadAnimationResource("RES.PAK", 9);
		_mouseCursors[0] = _cursorSprite->getCel(1);
		_mouseCursors[1] = _cursorSprite->getCel(0);
		_mouseCursors[2] = _cursorSprite->getCel(4);
		_mouseCursors[3] = _cursorSprite->getCel(3);
		_mouseCursors[4] = _cursorSprite->getCel(2);
		_mouseCursors[5] = _cursorSprite->getCel(5);
		_mouseCursors[6] = _cursorSprite->getCel(6);
	}

	_currCursorSprite = 0;

	_screen->setFontColor(0);

	_backupPalette = new byte[768];
	memcpy(_backupPalette, _gamePalette, 768);

	initData();
	loadGlobalTextData();

	setModuleAndScene(_startupModuleNumber, _startupSceneNumber);
	
}

void CometEngine::loadGlobalTextData() {
	_talkText->deactivate();
	_globalStrings = _textReader->loadTextResource(0);
	_inventoryItemNames = _textReader->loadTextResource(1);
}

void CometEngine::initData() {
	_tempScreen = new byte[64000];
	_screenPalette = new byte[768];

	_sceneBackgroundResource = new ScreenResource();

	memcpy(_screenPalette, _gamePalette, 768);

	memset(_scriptVars, 0, sizeof(_scriptVars));

	_screen->setFontColor(19);
	_input->unblockInput();

	_currentModuleNumber = 0;
	_sceneNumber = 0;

	resetVars();

	_actors->resetHealth();

	Actor *mainActor = _actors->getActor(0);
	mainActor->init(_animationMan->getAnimationResource(1, 0));
	mainActor->setPosition(160, 190);
	mainActor->_life = 99;
}

void CometEngine::setModuleAndScene(int moduleNumber, int sceneNumber) {
	_moduleNumber = moduleNumber;
	_sceneNumber = sceneNumber;
	_animPakName = Common::String::format("a%02d.pak", _moduleNumber);
	_scenePakName = Common::String::format("d%02d.pak", _moduleNumber);
	_scriptFileName = Common::String::format("r%02d.cc4", _moduleNumber);
}

void CometEngine::updateGame() {
	_gameLoopCounter++;
	_textColorFlag++;

	if (_moduleNumber != _currentModuleNumber)
		updateModuleNumber();

	if (_sceneNumber != _currentSceneNumber)
		updateSceneNumber();

	_screen->copyFromScreenResource(_sceneBackgroundResource);

	if (_verbs.isLookRequested())
		lookAtItemInSight(true);

	if (_verbs.isGetRequested())
		getItemInSight();

	_input->handleInput();

	_script->runAllScripts();

	if (_loadgameRequested)
		return;

	_scene->drawExits();
	_actors->updateAnimations();
	_actors->updateMovement();
	buildSpriteDrawQueue();
	lookAtItemInSight(false);
	drawSpriteQueue();

	_talkText->update();

	// TODO: Make vars for indices
	if (_scriptVars[11] < 100 && _scriptVars[10] == 1)
		drawTextIllsmouth();

	updateScreen();

	updateHeroLife();

	_loadgameRequested = false;

	_verbs.clear();
}

void CometEngine::updateHeroLife() {
	if ((_gameLoopCounter & 0x1FF) == 0) {
		Actor *mainActor = _actors->getActor(0);
		mainActor->updateHealth();
	}
}

void CometEngine::updateModuleNumber() {
	if (_moduleNumber != -1) {
		_animationMan->purgeAnimationSlots();
		freeAnimationsAndSceneDecoration();
		setModuleAndScene(_moduleNumber, _sceneNumber);
		updateSceneNumber();
	}
}

void CometEngine::updateSceneNumber() {
	Actor *mainActor = _actors->getActor(0);

	if (mainActor->_walkStatus != 0 &&
		((mainActor->_direction == 2 && mainActor->_x < 319) ||
		(mainActor->_direction == 4 && mainActor->_x > 0))) {

		mainActor->_y = mainActor->_walkDestY;

	} else {

		_talkText->resetTextValues();
		freeAnimationsAndSceneDecoration();
		_prevSceneNumber = _currentSceneNumber;
		_currentSceneNumber = _sceneNumber;
		_prevModuleNumber = _currentModuleNumber;
		_currentModuleNumber = _moduleNumber;

		mainActor->stopWalking();
		mainActor->setVisible(true);
		mainActor->enableCollisions();
		mainActor->_value6 = 0;
		mainActor->setClipX(0, 319);
		mainActor->setClipY(0, 199);

		_screen->_palFlag = false;

		loadAndRunScript();

		handleSceneChange(_prevSceneNumber, _prevModuleNumber);

	}
}

void CometEngine::getItemInSight() {
	debug(4, "CometEngine::getItemInSight()");

	Common::Rect sightRect = _actors->getActor(0)->calcSightRect(0, 50);
	SceneItem *sceneItem = _scene->getSceneItemAt(sightRect);
	if (sceneItem) {
		if (sceneItem->paramType == 0) {
			_inventory.requestGetItem(sceneItem->itemIndex);
			sceneItem->active = false;
			_talkText->showTextBubble(sceneItem->itemIndex, _inventoryItemNames->getString(sceneItem->itemIndex), 10);
		} else {
			_talkText->showTextBubble(4, _inventoryItemNames->getString(4), 10);
		}
	}
}

void CometEngine::lookAtItemInSight(bool showText) {
	_itemInSight = false;
	if (_input->getBlockedInput() != 15) {
		Common::Rect sightRect = _actors->getActor(0)->calcSightRect(0, 50);
		SceneItem *sceneItem = _scene->getSceneItemAt(sightRect);
		if (sceneItem) {
			_itemInSight = true;
			_itemDirection = _actors->getActor(0)->_direction;
			_itemX = sceneItem->x;
			_itemY = sceneItem->y - 6;
			if (showText && (!_dialog->isRunning() || !_talkText->isActive())) {
				if (sceneItem->paramType == 0) {
					_talkText->showTextBubble(sceneItem->itemIndex, _inventoryItemNames->getString(sceneItem->itemIndex), 10);
				} else {
					// NOTE: Looks like this is never used in Comet CD, the resp. opcode is unused there.
					warning("CometEngine::lookAtItemInSight() sceneItem.paramType != 0; sceneItem.itemIndex = %d", sceneItem->itemIndex);
					// NOTE: Probably only used in Eternam
					// _talkText->showTextBubble(sceneItem->itemIndex, getTextEntry(sceneItem->itemIndex, textBuffer));
				}
			}
		}
	}
}

void CometEngine::buildSpriteDrawQueue() {
	_spriteDrawQueue.clear();
	_spriteDrawQueue.reserve(16);
	enqueueSceneDecorationForDrawing();
	_actors->enqueueActorsForDrawing();
}

void CometEngine::addToSpriteDrawQueue(int y, int actorIndex, int insertIndex) {
	SpriteDraw spriteDraw;
	spriteDraw.y = y;
	spriteDraw.index = actorIndex;
	if (insertIndex >= 0) {
		_spriteDrawQueue.insert_at(insertIndex, spriteDraw);
	} else {
		_spriteDrawQueue.push_back(spriteDraw);
	}
}

void CometEngine::enqueueSceneDecorationForDrawing() {
	if (_sceneDecorationSprite) {
		AnimationElement *elem = _sceneDecorationSprite->getElement(0);
		for (uint i = 0; i < elem->getCommandCount(); ++i) {
			addToSpriteDrawQueue(elem->getCommand(i)->getPoint(0).y, 16, -1);
		}
	}
}

void CometEngine::enqueueActorForDrawing(int y, int actorIndex) {
	uint insertIndex = 0;
	for (insertIndex = 0; insertIndex < _spriteDrawQueue.size(); insertIndex++)
		if (_spriteDrawQueue[insertIndex].y > y)
			break;
	addToSpriteDrawQueue(y, actorIndex, insertIndex);
}

void CometEngine::drawSpriteQueue() {
	int objectCmdIndex = 0;
	_screen->setClipRect(0, 0, 320, 200);
	for (uint32 i = 0; i < _spriteDrawQueue.size(); i++) {
		if (_spriteDrawQueue[i].index < 16) {
			drawActor(_spriteDrawQueue[i].index);
		} else {
			AnimationCommand *cmd = _sceneDecorationSprite->getElementCommand(0, objectCmdIndex);
			cmd->draw(_screen, _sceneDecorationSprite, 0, 0, 0);
			objectCmdIndex++;
		}
	}
	if (_itemInSight && _actors->getActor(0)->_direction != 1)
		drawLineOfSight();
}

void CometEngine::drawActor(int actorIndex) {
	Actor *actor = _actors->getActor(actorIndex);
	actor->draw();
}

void CometEngine::resetVars() {
	// NOTE: scDisableRectFlag(); // Unused in Comet
	_paletteBrightness = 255;
	// NOTE: g_sp_byte_1 = 0; // Unused in Comet
	_verbs.clear();
 	_scene->clear();
	_input->unblockInput();
}

void CometEngine::loadAndRunScript(bool loadingGame) {
	_script->loadScript(_scriptFileName.c_str(), _currentSceneNumber);
	if (!loadingGame) {
		resetVars();
		_actors->resetHealth();
		_script->initializeScript();
	} else {
		_script->initializeScriptAfterLoadGame();
	}
}

void CometEngine::freeAnimationsAndSceneDecoration() {
	_animationMan->purgeAnimationSlots();
	delete _sceneDecorationSprite;
	_sceneDecorationSprite = NULL;
}

AnimationFrame *CometEngine::getAnimationFrame(int animationSlot, int animIndex, int animFrameIndex) {
	return _animationMan->getAnimation(animationSlot)->getFrameListFrame(animIndex, animFrameIndex);
}

void CometEngine::updateScreen() {

	// NOTE: Draw unknown stuff -> Unused in Comet CD, check Comet floppy

	if (_beams.size() > 0)
		drawBeams();

	// NOTE: Draw pixels -> Unused in Comet CD, check Comet floppy

	if (_clearScreenRequest) {
		_screen->clear();
		_clearScreenRequest = false;
	}

	if (!isFloppy()) {
		if (_currentModuleNumber == 9 && _currentSceneNumber == 0 && _paletteStatus == 0) {
			memcpy(_backupPalette, _gamePalette, 768);
			memcpy(_gamePalette, _introPalette2, 768);
			memcpy(_screenPalette, _introPalette2, 768);
			_screen->clear();
			_screen->setFullPalette(_gamePalette);
			_paletteStatus = 3;
		} else if (_currentModuleNumber == 9 && _currentSceneNumber == 1 && _paletteStatus == 3) {
			memcpy(_gamePalette, _introPalette1, 768);
			memcpy(_screenPalette, _introPalette1, 768);
				_screen->clear();
			_screen->setFullPalette(_gamePalette);
			_paletteStatus = 2;
		} else if (_currentModuleNumber == 5 && _currentSceneNumber == 0 && (_paletteStatus == 2 || _paletteStatus == 3)) {
			memcpy(_gamePalette, _backupPalette, 768);
			memcpy(_screenPalette, _backupPalette, 768);
				_screen->clear();
			_screen->setFullPalette(_gamePalette);
			_paletteStatus = 0;
		} else if (_currentModuleNumber == 0 && _currentSceneNumber == 0 && _paletteStatus != 0) {
			memcpy(_gamePalette, _backupPalette, 768);
			memcpy(_screenPalette, _backupPalette, 768);
			_screen->setFullPalette(_gamePalette);
			_paletteStatus = 0;
		}
	}

	_screen->update();
}

void CometEngine::setMouseCursor(int cursorNum) {
	setMouseCursorSprite(cursorNum < 0 ? 0 : _mouseCursors[cursorNum]);
}

void CometEngine::setMouseCursorSprite(const AnimationCel *cursorSprite) {
	if (isFloppy()) {
		warning("setMouseCursorSprite() called in floppy version");
		return;
	}
	if (!cursorSprite) {
		_systemMouseCursor->setCursor(&_currCursorSprite);
	} else {
		AnimationCelMouseCursor celMouseCursor(cursorSprite);
		celMouseCursor.setCursor(&_currCursorSprite);
	}
}

int16 CometEngine::randomValue(int maxValue) {
	return maxValue >= 2 ? _rnd->getRandomNumber(maxValue - 1) : 0;
}

void CometEngine::drawBubble(int x1, int y1, int x2, int y2) {
	int xPos, yPos;
	int height = (y2 - y1 + 8) / 8 * 8 - (y2 - y1);

	x2 += (x2 - x1 + 8) / 8 * 8 - (x2 - x1);
	y2 += height;

	if (x1 < 0) {
		x2 -= x1;
		x1 = 0;
	}

	if (x2 > 319) {
		x1 -= x2 - 319;
		x2 = 319;
	}

	if (y1 < 0) {
		height = y2 - y1;
		y1 = 0;
		y2 = height + y1;
	}

	if (y2 > 199) {
		height = y2 - y1;
		y2 = 199;
		y1 = y2 - height;
	}

	for (yPos = y1 + 8; y2 - 8 >= yPos; yPos += 8) {
		for (xPos = x1 + 8; x2 - 8 > xPos; xPos += 8) {
			_screen->drawAnimationElement(_bubbleSprite, 8, xPos, yPos);
		}
	}

	for (xPos = x1 + 8; x2 - 8 > xPos; xPos += 8) {
		_screen->drawAnimationElement(_bubbleSprite, 3, xPos, y1 + 8);
		_screen->drawAnimationElement(_bubbleSprite, 4, xPos, y2);
	}

	for (yPos = y1 + 16; yPos < y2; yPos += 8) {
		_screen->drawAnimationElement(_bubbleSprite, 1, x1, yPos);
		_screen->drawAnimationElement(_bubbleSprite, 6, x2 - 16, yPos);
	}

	_screen->drawAnimationElement(_bubbleSprite, 0, x1, y1 + 8);
	_screen->drawAnimationElement(_bubbleSprite, 5, x2 - 16, y1 + 8);
	_screen->drawAnimationElement(_bubbleSprite, 2, x1, y2);
	_screen->drawAnimationElement(_bubbleSprite, 7, x2 - 16, y2);
}

bool CometEngine::rectCompare(const Common::Rect &rect1, const Common::Rect &rect2) {
	return (rect1.left <= rect2.right && rect1.top <= rect2.bottom && rect1.right >= rect2.left && rect1.bottom >= rect2.top);
}

int CometEngine::findRect(const GuiRectangle *rects, int x, int y, int count, int defaultId) {
	for (int i = 0; i < count; i++) {
		if (x > rects[i].x && x < rects[i].x2 && y > rects[i].y && y < rects[i].y2)
			return rects[i].id;
	}
	return defaultId;
}

void CometEngine::warpMouseToRect(const GuiRectangle &rect) {
	_system->warpMouse(rect.x + (rect.x2 - rect.x) / 2,
		rect.y + (rect.y2 - rect.y) / 2);
}

void CometEngine::handleKeyInput() {
	switch (_input->getKeyCode()) {
	case Common::KEYCODE_t:
		_verbs.requestTalk();
		_input->waitForKeys();
		break;
	case Common::KEYCODE_g:
		_verbs.requestGet();
		_input->waitForKeys();
		break;
	case Common::KEYCODE_l:
		_verbs.requestLook();
		_input->waitForKeys();
		break;
	case Common::KEYCODE_o:
		_gui->run(kGuiInventory);
		_input->waitForKeys();
		break;
	case Common::KEYCODE_u:
		useCurrentInventoryItem();
		_input->waitForKeys();
		break;
	case Common::KEYCODE_d:
		_gui->run(kGuiMainMenu);
		_input->waitForKeys();
		break;
	case Common::KEYCODE_m:
		handleMap();
		_input->waitForKeys();
		break;
	case Common::KEYCODE_i:
		_gui->run(kGuiJournal);
		_input->waitForKeys();
		break;
	case Common::KEYCODE_p:
		checkPauseGame();
		_input->waitForKeys();
		break;
	case Common::KEYCODE_RETURN:
		_talkText->stopText();
		break;
	default:
		if (_input->getKeyCode() == Common::KEYCODE_TAB || _input->rightButton()) {
			_gui->run(kGuiCommandBar);
		}
		break;
	}
}

void CometEngine::syncUpdate(bool screenUpdate) {
	const uint32 kTargetFramerate = 10;
	const uint32 kMillisPerFrame = 1000 / kTargetFramerate;
	const uint32 kMinimumTimerResolution = 10;
	const uint32 kUpdateScreenMillis = 20;
	if (screenUpdate)
		_screen->update();
	// Try to keep the framerate as demanded by kMillisPerFrame
	uint32 currTick = _system->getMillis();
	if (_nextTick > currTick && _nextTick - currTick >= kMinimumTimerResolution) {
		uint32 totalWaitMillis = _nextTick - currTick;
		while (totalWaitMillis > 0) {
			uint32 waitMillis = totalWaitMillis > kUpdateScreenMillis
				? kUpdateScreenMillis : totalWaitMillis;
			_system->delayMillis(waitMillis);
			_input->handleEvents();
			_system->updateScreen();
			totalWaitMillis -= waitMillis;
		}
	}
	// TODO Take gamespeed into account later
	_nextTick = currTick + kMillisPerFrame;
}

int CometEngine::handleSceneExitCollision(int sceneExitIndex) {
	int newModuleNumber, newSceneNumber;
	_scene->getExitLink(sceneExitIndex, newModuleNumber, newSceneNumber);
	return handleLeftRightSceneExitCollision(newModuleNumber, newSceneNumber);
}

int CometEngine::handleLeftRightSceneExitCollision(int newModuleNumber, int newSceneNumber) {
	if (newSceneNumber == -1) {
		_moduleNumber = -1;
		return 0;
	}

	_sceneNumber = newSceneNumber;
	_moduleNumber = newModuleNumber;

	Actor *mainActor = _actors->getActor(0);

	if (mainActor->_direction != 1 && mainActor->_direction != 3) {
		int x1, y1, x2, y2;

		mainActor->_value6 = 4;

		_scene->getExitRect(mainActor->_collisionIndex, x1, y1, x2, y2);
		if (x2 == 318)
			x2 = 319;

		mainActor->disableCollisions();

		if (mainActor->_direction == 2) {
			mainActor->setClipX(0, x2);
			mainActor->startWalking(319, mainActor->_y);
		} else if (mainActor->_direction == 4) {
			mainActor->setClipX(x1, 319);
			mainActor->startWalking(0, mainActor->_y);
		}

		mainActor->_walkStatus &= ~4;
	}

	return 1;
}

void CometEngine::drawLineOfSight() {
	if (_itemInSight) {
		Actor *mainActor = _actors->getActor(0);
		int x = mainActor->_x;
		int y = mainActor->_y - 35;
		switch (_itemDirection) {
		case 1:
			y--;
			break;
	 	case 2:
	 		x += 4;
	 		break;
		case 3:
			y++;
			break;
		case 4:
			x -= 5;
			break;
		}
		_screen->drawDottedLine(x, y, _itemX + randomValue(3) - 1, _itemY + randomValue(3) - 1, 7);
	}
}

void CometEngine::handleSceneChange(int sceneNumber, int moduleNumber) {
	Actor *mainActor = _actors->getActor(0);
	int direction, x1, x2, y1, y2;

	_scene->findExitRect(sceneNumber, moduleNumber, mainActor->_direction, x1, y1, x2, y2, direction);

	mainActor->_x = (x2 - x1) / 2 + x1;
	mainActor->_y = (y2 - y1) / 2 + y1;
	mainActor->_direction = direction;
	mainActor->setAnimationIndex(direction - 1);

	// Scene change effects
	if (_screen->getZoomFactor() == 0) {
		if (direction == 1 || direction == 3) {
			_screen->enableTransitionEffect();
		} else {
			// First draw the current scene incl. sprites the last time to
			// a temporary buffer, then actually scroll it.
			_screen->copyFromScreenResource(_sceneBackgroundResource);
			buildSpriteDrawQueue();		
			drawSpriteQueue();
			_screen->copyToScreen(_tempScreen);
			if (direction == 2) {
				_screen->screenScrollEffect(_tempScreen, -1);
			} else if (direction == 4) {
				_screen->screenScrollEffect(_tempScreen, 1);
			}
		}
	}

	if (_screen->getFadeType() == kFadeNone) {
		_screen->buildPalette(_gamePalette, _screenPalette, _paletteBrightness);
		_screen->setFullPalette(_screenPalette);
	}
}

void CometEngine::playMusic(int musicIndex) {
	if (_music) {
		if (musicIndex != 255)
			_music->playMusic(musicIndex);
		else
			_music->stopMusic();
	}
}

void CometEngine::playSample(int sampleIndex, int loopCount) {
	debug(2, "playSample(%d, %d)", sampleIndex, loopCount);
	if (sampleIndex == 255) {
		if (_mixer->isSoundHandleActive(_sampleHandle))
			_mixer->stopHandle(_sampleHandle);
	} else if (!_talkText->isSpeechPlaying() && !_mixer->isSoundHandleActive(_sampleHandle)) {
		if (sampleIndex != _currSoundResourceIndex) {
			// Only load the sample when neccessary
			_res->loadFromPak(_soundResource, "SMP.PAK", sampleIndex);
			_currSoundResourceIndex = sampleIndex;
		}
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sampleHandle, loopCount > 1
			? makeLoopingAudioStream(_soundResource->makeAudioStream(), loopCount)
			: _soundResource->makeAudioStream());
	}
}

void CometEngine::drawTextIllsmouth() {
	byte *text = _textReader->getString(2, 36);
	_screen->drawTextOutlined((320 - _screen->getTextWidth(text)) / 2, 180, text, 7, 0); 
	_scriptVars[11]++;
}

void CometEngine::playCutscene(int fileIndex, int frameListIndex, int backgroundIndex, int loopCount, int soundFramesCount, byte *soundFramesData) {
	debug("playCutscene(%d, %d, %d, %d, %d)", fileIndex, frameListIndex, backgroundIndex, loopCount, soundFramesCount);

	int palStatus = 0;
	AnimationResource *cutsceneSprite;
	AnimationFrameList *frameList;
	int animFrameCount;

	_talkText->stopVoice();
	_talkText->stopText();

	cutsceneSprite = _animationMan->loadAnimationResource(_animPakName.c_str(), fileIndex);
	frameList = cutsceneSprite->getFrameList(frameListIndex);
	animFrameCount = frameList->getFrameCount();

	if (backgroundIndex == 0) {
		_screen->clear();
	} else if (backgroundIndex < 0) {
		_screen->drawAnimationElement(cutsceneSprite, -backgroundIndex, 0, 0);
	} else if (backgroundIndex < 32000) {
		_screen->drawAnimationElement(_sceneDecorationSprite, backgroundIndex, 0, 0);
	} else if (backgroundIndex == 32000) {
		// TODO: Grab vga screen to work screen
	}

	_screen->copyToScreen(_tempScreen);

	if (soundFramesCount > 0) {
		int sampleIndex = soundFramesData[0];
		debug("  sampleIndex = %d", sampleIndex);
		// TODO: Load the sample
	}

	for (int loopIndex = 0; loopIndex < loopCount && !shouldQuit(); loopIndex++) {
		byte *workSoundFramesData = soundFramesData;
		int workSoundFramesCount = soundFramesCount;
		int animFrameIndex, animSoundFrameIndex = 0, interpolationStep = 0;

		if (soundFramesCount > 0) {
			workSoundFramesData++;
			animSoundFrameIndex = *workSoundFramesData++;
			workSoundFramesData++;
		}

		animFrameIndex = 0;
		while (animFrameIndex < animFrameCount) {
			_input->handleEvents();
			if (shouldQuit())
				break;

			_screen->copyFromScreen(_tempScreen);

			interpolationStep = _screen->drawAnimation(cutsceneSprite, frameList, animFrameIndex, interpolationStep, 0, 0, animFrameCount);

			_talkText->updateTextDialog();

			if (palStatus == 1) {
				// TODO: Set the anim palette
				palStatus = 2;
			}

			syncUpdate();

			if (workSoundFramesCount > 0 && animFrameIndex == animSoundFrameIndex) {
				// TODO: Play the anim sound
				workSoundFramesCount--;
				if (workSoundFramesCount > 0) {
					// NOTE: Load the next sample; unused in Comet (only max. one sample per cutscene)
				}
			}

			checkPauseGame();

			if (_input->getKeyCode() == Common::KEYCODE_ESCAPE) {
				// TODO: yesNoDialog();
			} else if (_input->getKeyCode() == Common::KEYCODE_RETURN) {
				animFrameIndex = animFrameCount;
				loopIndex = loopCount;
				if (_talkText->isActive())
					_talkText->stopText();
			}

			if (interpolationStep == 0)
				animFrameIndex++;
		}
	}

	delete cutsceneSprite;

	if (palStatus > 0)
		_screen->setFullPalette(_screenPalette);

	if (_talkText->isActive())
		_talkText->resetTextValues();

	// CHECKME: If this is neccessary (screen is copied in main loop anyways)
	_screen->copyFromScreenResource(_sceneBackgroundResource);
}

void CometEngine::addBeam(int x1, int y1, int x2, int y2) {
	Beam beam;
	beam.x1 = x1;
	beam.y1 = y1;
	beam.x2 = x2;
	beam.y2 = y2;
	_beams.push_back(beam);
}

void CometEngine::drawBeam(int x1, int y1, int x2, int y2) {
	int currX1 = x1, currY1 = y1, currX2, currY2;
	for (int i = 1; i <= 10; i++) {
		byte color = randomValue(7) + 144;
		currX2 = randomValue(8) + (x2 - x1) * i / 10 + x1 - 2;
		currY2 = randomValue(8) + (y2 - y1) * i / 10 + y1 - 2;
		_screen->drawLine(currX1, currY1, currX2, currY2, color);
		currX1 = currX2;
		currY1 = currY2;
	}
}

void CometEngine::drawBeams() {
	for (uint i = 0; i < _beams.size(); i++) {
		Beam *beam = &_beams[i];
		drawBeam(beam->x1, beam->y1, beam->x2, beam->y2);
	}
	_beams.clear();
	// NOTE: beamColor stuff, unused in Comet CD
}

void CometEngine::initSystemVars() {
	_systemVars[0] = &_prevSceneNumber;
	for (int i = 0; i < 10; i++) {
		_systemVars[1 + i * 3] = &_actors->getActor(i)->_life;
		_systemVars[2 + i * 3] = &_actors->getActor(i)->_x;
		_systemVars[3 + i * 3] = &_actors->getActor(i)->_y;
	}
	_systemVars[31] = &_input->_cursorDirection;
	_systemVars[32] = &_input->_scriptKeybFlag;
	_systemVars[33] = &_scriptRandomValue;
	_systemVars[34] = &_prevModuleNumber;
}

void CometEngine::useCurrentInventoryItem() {
	_inventory.resetStatus();
	_inventory.requestUseSelectedItem();
}

void CometEngine::checkCurrentInventoryItem() {
	_inventory.testSelectedItemRemoved();
	// Check if the player wants to read the notebook
	if (_inventory.getStatus(0) == 2) {
		_gui->run(kGuiJournal);
		_inventory.setStatus(0, 1);
	}
}

void CometEngine::introMainLoop() {
	_endIntroLoop = false;

	while (!_endIntroLoop && !shouldQuit()) {
		_input->handleEvents();

		switch (_input->getKeyCode()) {
		case Common::KEYCODE_ESCAPE:
			_endIntroLoop = true;
			break;
		case Common::KEYCODE_RETURN:
			_talkText->stopText();
			break;
		case Common::KEYCODE_p:
			checkPauseGame();
			break;
		default:
			break;
		}

		if (_currentSceneNumber == 0 && _currentModuleNumber == 0)
			_endIntroLoop = true;

		updateGame();
		syncUpdate(false);
	}
}

void CometEngine::cometMainLoop() {
	while (!shouldQuit()) {
		_input->handleEvents();

		if (_currentModuleNumber == 7 && _currentSceneNumber == 1 && _paletteStatus == 0) {
			memcpy(_backupPalette, _gamePalette, 768);
			memcpy(_gamePalette, _flashbakPal, 768);
			memcpy(_screenPalette, _flashbakPal, 768);
			_screen->clear();
			_screen->setFullPalette(_gamePalette);
			_paletteStatus = 1;
		} else if (_currentModuleNumber == 2 && _currentSceneNumber == 22 && _paletteStatus == 1) {
			memcpy(_gamePalette, _backupPalette, 768);
			memcpy(_screenPalette, _backupPalette, 768);
			_screen->clear();
			_screen->setFullPalette(_gamePalette);
			_paletteStatus = 0;
		}

		if (_scriptVars[9] == 1) {
			_scriptVars[9] = _gui->run(kGuiPuzzle);
			loadSceneBackground();
		}

		if (!_dialog->isRunning() && _currentModuleNumber != 3 && _actors->getActor(0)->_value6 != 4 && !_screen->_palFlag && !_talkText->isActive()) {
			handleKeyInput();
		// Original behavior: } else if (_input->getKeyCode() == Common::KEYCODE_RETURN || (_input->rightButton() && _talkText->isActive())) {
		} else if ((_input->getKeyCode() == Common::KEYCODE_RETURN || _input->rightButton()) && _talkText->isActive()) {
			_talkText->stopText();
		}

		if (shouldQuit())
			return;

		if (debugTestPuzzle) {
			_gui->run(kGuiPuzzle);
			debugTestPuzzle = false;
		}

		// Debugging keys
		switch (_input->getKeyCode()) {
		case Common::KEYCODE_F7:
			savegame("comet.000", "Quicksave");
			break;
		case Common::KEYCODE_F9:
			loadgame("comet.000");
			break;
		case Common::KEYCODE_KP_PLUS:
			_sceneNumber++;
			debug("## New _sceneNumber = %d", _sceneNumber);
			break;
		case Common::KEYCODE_KP_MINUS:
			if (_sceneNumber > 0) {
				debug("## New _sceneNumber = %d", _sceneNumber);
				_sceneNumber--;
			}
			break;
		case Common::KEYCODE_KP_MULTIPLY:
			_moduleNumber++;
			_sceneNumber = 0;
			debug("## New _moduleNumber = %d", _moduleNumber);
			break;
		case Common::KEYCODE_KP_DIVIDE:
			if (_moduleNumber > 0) {
				_moduleNumber--;
				_sceneNumber = 0;
				debug("## New _moduleNumber = %d", _moduleNumber);
			}
			break;
		default:
			break;
		}

		updateGame();
		syncUpdate(false);

		if (_loadgameRequested) {
			// TODO:
			//	while (savegame_load() == 0);
			//	savegame_load

			_loadgameRequested = false;
		}

		checkCurrentInventoryItem();
	}
}

void CometEngine::museumMainLoop() {
	while (!shouldQuit()) {
		_input->handleEvents();
		if (!_dialog->isRunning() && _currentModuleNumber != 3 && _actors->getActor(0)->_value6 != 4 && !_screen->_palFlag && !_talkText->isActive()) {
			handleKeyInput();
		} else if ((_input->getKeyCode() == Common::KEYCODE_RETURN || _input->rightButton()) && _talkText->isActive()) {
			_talkText->stopText();
		}
		if (shouldQuit())
			return;
		updateGame();
		syncUpdate(false);
		if (_loadgameRequested) {
			// TODO:
			//	while (savegame_load() == 0);
			//	savegame_load
			_loadgameRequested = false;
		}
	}
}

void CometEngine::checkPauseGame() {
	static const byte *pauseText = (const byte*)"Game Paused";
	if (_input->getKeyCode() == Common::KEYCODE_p) {
		int x = (320 - _screen->getTextWidth(pauseText)) / 2;
		int y = 180;
		_input->waitForKeys();
		_screen->setFontColor(80);
		_screen->drawText(x + 1, y + 1, pauseText);
		_screen->drawText(x + 1, y - 1, pauseText);
		_screen->drawText(x + 1, y, pauseText);
		_screen->drawText(x - 1, y, pauseText);
		_screen->drawText(x, y + 1, pauseText);
		_screen->drawText(x, y - 1, pauseText);
		_screen->drawText(x - 1, y + 1, pauseText);
		_screen->drawText(x - 1, y - 1, pauseText);
		_screen->setFontColor(95);
		_screen->drawText(x, y, pauseText);
		_screen->update();
		_input->clearKeyDirection();
		_talkText->stopVoice();
		do {
			_input->handleEvents();
			syncUpdate(false);
		} while (_input->getKeyCode() == Common::KEYCODE_INVALID && !_input->leftButton() && !_input->rightButton() && !shouldQuit());
	}
}

int CometEngine::handleMap() {
	int mapResult = 0;

	_talkText->stopVoice();

	if (_input->getBlockedInput() == 0 && !_talkText->isActive() && !_dialog->isRunning() && !_talkText->isSpeechPlaying() &&
		_scriptVars[7] != 1 && _scriptVars[8] != 1 && _scriptVars[8] != 2) {

		setMouseCursor(-1);

		if (_currentModuleNumber == 0 && 
			((_currentSceneNumber >= 0 && _currentSceneNumber <= 22) ||
			(_currentSceneNumber >= 30 || _currentSceneNumber <= 52) ||
			(_currentSceneNumber >= 60 || _currentSceneNumber <= 82))) {
			mapResult = _gui->run(kGuiTownMap);
			_inventory.setStatus(0, 1);
		}

		if (_currentModuleNumber == 6 && _currentSceneNumber >= 0 && _currentSceneNumber <= 22) {
			mapResult = _gui->run(kGuiTownMap);
			_inventory.setStatus(0, 1);
		}
		
	}

	return mapResult;
}

void CometEngine::openConsole() {
	_console->attach();
	_console->onFrame();
}

} // End of namespace Comet
