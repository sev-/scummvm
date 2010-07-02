#include "common/stream.h"
#include "sound/audiostream.h"
#include "sound/decoders/raw.h"
#include "sound/decoders/voc.h"
#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/pak.h"

#include "comet/animation.h"
#include "comet/animationmgr.h"
#include "comet/dialog.h"
#include "comet/music.h"
#include "comet/scene.h"
#include "comet/screen.h"

namespace Comet {

// TODO: Move a lot of stuff to own classes

int CometEngine::comparePointXY(int x, int y, int x2, int y2) {
	int flags = 0;
	if (x == x2)
		flags |= 1;
	if (y == y2)
		flags |= 2;
	return flags;
}

void CometEngine::calcSightRect(Common::Rect &rect, int delta1, int delta2) {

	int x1 = _actors[0].x - _actors[0].deltaX - 8;
	int y1 = _actors[0].y - _actors[0].deltaY - 8;
	int x2 = _actors[0].x + _actors[0].deltaX + 8;
	int y2 = _actors[0].y + 8;

	switch (_actors[0].direction) {
	case 1:
		y1 -= delta2;
		y2 -= 20;
		x1 -= delta1;
		x2 += delta1;
		break;
	case 2:
		x2 += delta2;
		x1 += 25;
		y1 -= 32;
		break;
	case 3:
		y2 += delta2;
		y1 += 20;
		break;
	case 4:
		x1 -= delta2;
		x2 -= 25;
		y1 -= 32;
		break;
	}

	rect.left = MAX(x1, 0);
	rect.top = MAX(y1, 0);
	rect.right = MIN(x2, 319);
	rect.bottom = MIN(y2, 199);

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

void CometEngine::drawSceneExits() {
	for (uint32 i = 0; i < _scene->_exits.size(); i++) {
		if (_scene->_exits[i].directionIndex == 3) {
			_screen->fillRect(_scene->_exits[i].x1, 198, _scene->_exits[i].x2, 199, 120);
			_screen->hLine(_scene->_exits[i].x1 + 1, 199, _scene->_exits[i].x2 - 2, 127);
		}
	}
}

/* Scene */

void CometEngine::initSceneBackground(bool loadingGame) {
	
	_screen->clearScreen();
	
	loadSceneBackground();
	loadSceneDecoration();

	memcpy(_screen->getScreen(), _sceneBackground, 64000);
	drawSceneDecoration();
	memcpy(_sceneBackground, _screen->getScreen(), 64000);

	/* TODO: Unused in Comet CD
	if (screenCopyFlag != 0)
		screen_c_1();
	*/
	
	if (!loadingGame)
		initSceneDecorationBlockingRects();

}

void CometEngine::loadSceneBackground() {
	loadPakToPtr(DName, _backgroundFileIndex, _sceneBackground);
}

void CometEngine::loadSceneDecoration() {
	delete _sceneDecorationSprite;
	_sceneDecorationSprite = _animationMan->loadAnimationResource(DName, _backgroundFileIndex + 1);
}

void CometEngine::drawSceneDecoration() {
	if (_sceneDecorationSprite->_elements.size() > 0)
		_screen->drawAnimationElement(_sceneDecorationSprite, 0, 0, 0);
}

/* Graphics */

void CometEngine::initAndLoadGlobalData() {

	_screen->loadFont("RES.PAK", 0);

	_bubbleSprite = _animationMan->loadAnimationResource("RES.PAK", 1);
	_heroSprite = _animationMan->loadAnimationResource("RES.PAK", 2);
	_inventoryItemSprites = _animationMan->loadAnimationResource("RES.PAK", 4);

	_ctuPal = loadFromPak("RES.PAK", 5);
	_flashbakPal = loadFromPak("RES.PAK", 6);
	_cdintroPal = loadFromPak("RES.PAK", 7);
	_pali0Pal = loadFromPak("RES.PAK", 8);

	_cursorSprite = _animationMan->loadAnimationResource("RES.PAK", 9);
	_iconSprite = _animationMan->loadAnimationResource("RES.PAK", 3);
	
	_screen->setFontColor(0);

	//TODO: seg001:0758 Mouse cursor stuff...
	
	_paletteBuffer = new byte[768];
	memcpy(_paletteBuffer, _ctuPal, 768);
	
	initData();

	loadGlobalTextData();
	
	//TODO...

	setModuleAndScene(_startupModuleNumber, _startupSceneNumber);
	
}

void CometEngine::loadGlobalTextData() {
	_textActive = false;
	_narOkFlag = false;
	_globalStrings = _textReader->loadTextStrings(0);
	_inventoryItemNames = _textReader->loadTextStrings(1);
}

void CometEngine::initData() {

	_sceneBackground = new byte[72000];
	_palette = new byte[768];

	memcpy(_palette, _ctuPal, 768);
	
	memset(_scriptVars, 0, sizeof(_scriptVars));
	memset(_inventoryItemStatus, 0, sizeof(_inventoryItemStatus));

	_screen->setFontColor(19);
	unblockInput();
	
	_currentModuleNumber = 0;
	_sceneNumber = 0;

/*
	_bounds.clear();
	_bounds.push_back(Common::Point(0, 100));
	_bounds.push_back(Common::Point(319, 100));
	initSceneBoundsMap();
*/
	resetVars();
	
	resetActorsLife();

	actorInit(0, _animationMan->getAnimationResource(1, 0));

	actorSetPosition(0, 160, 190);
	_actors[0].life = 99;

}

void CometEngine::setModuleAndScene(int moduleNumber, int sceneNumber) {

	_moduleNumber = moduleNumber;
	_sceneNumber = sceneNumber;
	
	//debug(4, "CometEngine::setModuleAndScene(%d, %d)", moduleNumber, sceneNumber);
	
	//FIXME
	sprintf(AName, "A%d%d.PAK", _moduleNumber / 10, _moduleNumber % 10);
	sprintf(DName, "D%d%d.PAK", _moduleNumber / 10, _moduleNumber % 10);
	sprintf(RName, "R%d%d.CC4", _moduleNumber / 10, _moduleNumber % 10);
	
	//debug(4, "AName = %s; DName = %s; RName = %s", AName, DName, RName);
	
}

void CometEngine::updateGame() {

	_gameLoopCounter++;
	_textColorFlag++;

	if (_moduleNumber != _currentModuleNumber)
		updateModuleNumber();

	if (_sceneNumber != _currentSceneNumber)
		updateSceneNumber();

	memcpy(_screen->getScreen(), _sceneBackground, 64000);

	if (_cmdLook)
		lookAtItemInSight(true);

	if (_cmdGet)
		getItemInSight();

	handleInput();
	
	_script->runAllScripts();

	if (_loadgameRequested)
		return;

	drawSceneExits();
	updateActorAnimations();
	updateActorMovement();

	buildSpriteDrawQueue();
	
	lookAtItemInSight(false);

	drawSpriteQueue();

	if (_talkieMode == 0)
		updateTextDialog();

	if ((_talkieMode == 1 && (_textActive || _textBubbleActive)) || (_talkieMode == 2 || _textBubbleActive))
		updateText();

	if (_dialog->isRunning())
		_dialog->update();

	updateTalkAnims();

	if (_scriptVars[11] < 100 && _scriptVars[10] == 1)
		drawTextIllsmouth();

	updateScreen();
	
	updateHeroLife();
	
	_loadgameRequested = false;
	
	_cmdTalk = false;
	_cmdGet = false;
	_cmdLook = false;

}

void CometEngine::updateModuleNumber() {
	if (_moduleNumber != -1) {
		_animationMan->purgeAnimationSlots();
		freeMarcheAndStaticObjects();
		setModuleAndScene(_moduleNumber, _sceneNumber);
		updateSceneNumber();
	}
}

void CometEngine::updateSceneNumber() {

	//TODO: mouse_4(0, 0x40);
	Actor *mainActor = getActor(0);

	if (mainActor->walkStatus != 0 &&
		((mainActor->direction == 2 && mainActor->x < 319) ||
		(mainActor->direction == 4 && mainActor->x > 0))) {

		mainActor->y = mainActor->walkDestY;

	} else {

		resetMarcheAndStaticObjects();
		_prevSceneNumber = _currentSceneNumber;
		_currentSceneNumber = _sceneNumber;
		_prevModuleNumber = _currentModuleNumber;
		_currentModuleNumber = _moduleNumber;
		
		actorStopWalking(mainActor);
		
		mainActor->visible = true;
		mainActor->collisionType = kCollisionNone;
		mainActor->value6 = 0;
		mainActor->clipX1 = 0;
		mainActor->clipY1 = 0;
		mainActor->clipX2 = 319;
		mainActor->clipY2 = 199;

		_screen->_palFlag = false;

		loadAndRunScript();
		
		handleSceneChange(_prevSceneNumber, _prevModuleNumber);
		
		//TODO: mouse_4(0, 0);
		
	}
	
}

void CometEngine::getItemInSight() {
	//debug(4, "CometEngine::getItemInSight()");
	
	Common::Rect sightRect;
	calcSightRect(sightRect, 0, 50);
	
	int sceneItemIndex = findSceneItemAt(sightRect);

	if (sceneItemIndex != 0) {
		SceneItem *sceneItem = &_sceneItems[sceneItemIndex & 0xFF];
		if (sceneItem->paramType == 0) {
			_inventoryItemStatus[sceneItem->itemIndex] = 1;
			_inventoryItemIndex = sceneItem->itemIndex;
			sceneItem->active = false;
			showTextBubble(sceneItem->itemIndex, _inventoryItemNames->getString(sceneItem->itemIndex));
		} else {
			showTextBubble(4, _inventoryItemNames->getString(4));
		}
	}
	
}

void CometEngine::lookAtItemInSight(bool showText) {

	_itemInSight = false;
	
	if (_blockedInput != 15) {
		Common::Rect sightRect;
		calcSightRect(sightRect, 0, 50);
		//_screen->fillRect(rect.left, rect.top, rect.right, rect.bottom, 150);
		int sceneItemIndex = findSceneItemAt(sightRect);
		if (sceneItemIndex != 0) {
			SceneItem *sceneItem = &_sceneItems[sceneItemIndex & 0xFF];
			_itemInSight = true;
			_itemDirection = _actors[0].direction;
			_itemX = sceneItem->x;
			_itemY = sceneItem->y - 6;
			if (showText && (!_dialog->isRunning() || !_textActive)) {
				if (sceneItem->paramType == 0) {
					showTextBubble(sceneItem->itemIndex, _inventoryItemNames->getString(sceneItem->itemIndex));
				} else {
					warning("sceneItem->paramType != 0; sceneItem->itemIndex = %d", sceneItem->itemIndex);
					// TODO: Remove this:
					// showTextBubble(sceneItem->itemIndex, getTextEntry(sceneItem->itemIndex, textBuffer));
					// NOTE: Looks like this is never used in Comet CD, the resp. opcode is unused there.
				}
			}
		}
	}
	
	//TODO?
}

void CometEngine::updateActorAnimations() {
	for (int i = 0; i < 10; i++) {
		if (_actors[i].life != 0)
			updateActorAnimation(&_actors[i]);
	}
	updatePortraitAnimation(&_actors[10]);
}

void CometEngine::updateActorMovement() {
	for (int i = 0; i < 11; i++) {
		if (_actors[i].life != 0) {
			Common::Rect obstacleRect;
			bool flag = updateActorPosition(i, obstacleRect);
			if (_actors[i].walkStatus & 3)
				actorUpdateWalking(&_actors[i], i, flag, obstacleRect);
		}
	}
}

void CometEngine::buildSpriteDrawQueue() {
	_spriteDrawQueue.clear();
	_spriteDrawQueue.reserve(16);
	enqueueSceneDecorationForDrawing();
	enqueueActorsForDrawing();
}

void CometEngine::addToSpriteDrawQueue(int y, int actorIndex, int insertIndex) {
	SpriteDraw spriteDraw;
	spriteDraw.y = y;
	spriteDraw.index = actorIndex;
	if (insertIndex >= 0)
		_spriteDrawQueue.insert_at(insertIndex, spriteDraw);
	else
		_spriteDrawQueue.push_back(spriteDraw);
}

void CometEngine::enqueueSceneDecorationForDrawing() {
	if (_sceneDecorationSprite) {
		AnimationElement *elem = _sceneDecorationSprite->_elements[0];
		for (uint i = 0; i < elem->commands.size(); i++) {
			addToSpriteDrawQueue(elem->commands[i]->points[0].y, 16, -1);
		}
	}
}

void CometEngine::enqueueActorsForDrawing() {
	for (int i = 0; i < 11; i++) {
		if (_actors[i].visible && _actors[i].life > 0) {
			enqueueActorForDrawing(_actors[i].y, i);
		}
	}
}

void CometEngine::enqueueActorForDrawing(int y, int actorIndex) {
	uint insertIndex = 0;
	for (insertIndex = 0; insertIndex < _spriteDrawQueue.size(); insertIndex++) {
		if (_spriteDrawQueue[insertIndex].y > y)
			break;
	}
	addToSpriteDrawQueue(y, actorIndex, insertIndex);
}

void CometEngine::updateHeroLife() {
	if (_actors[0].life > 0 && _actors[0].life < 99 && (_gameLoopCounter & 0x1FF) == 0) {
		_actors[0].life++;
	}
}

void CometEngine::drawSpriteQueue() {
	//TODO: Real stuff

	//TODO: setScreenRectAll();

	int objectCmdIndex = 0;

	for (uint32 i = 0; i < _spriteDrawQueue.size(); i++) {
		if (_spriteDrawQueue[i].index < 16) {
			drawActor(_spriteDrawQueue[i].index);
		} else {
			AnimationCommand *cmd = _sceneDecorationSprite->_elements[0]->commands[objectCmdIndex];
			_screen->drawAnimationCommand(_sceneDecorationSprite, cmd, 0, 0);
			objectCmdIndex++;
		}
	}

	if (_itemInSight && _actors[0].direction != 1)
		drawLineOfSight();
	
}

void CometEngine::drawActor(int actorIndex) {

	Actor *actor = getActor(actorIndex);
	
	int x = actor->x, y = actor->y;
	//int deltaX = actor->deltaX, deltaY = actor->deltaY;

	Animation *animation = _animationMan->getAnimation(actor->animationSlot);
	
	/* NOTE: Yet another workaround for a crash (see updateActorAnimation). */
	if (actor->animIndex >= (int)animation->_anims.size()) {
		actor->animIndex = 0;
		actor->animFrameIndex = 0;
		actor->animFrameCount = animation->_anims[0]->frames.size();
	}

	AnimationFrameList *frameList = animation->_anims[actor->animIndex];
	
	_screen->setClipRect(actor->clipX1, actor->clipY1, actor->clipX2 + 1, actor->clipY2 + 1);

	if (actor->status == 2) {
		actor->interpolationStep = _screen->drawAnimation(animation, frameList, actor->animFrameIndex, actor->interpolationStep,
			x, y, actor->animFrameCount);
	} else {
		if (actorIndex == 0 && _itemInSight && actor->direction == 1)
			drawLineOfSight();
		_screen->drawAnimationElement(animation, frameList->frames[actor->animFrameIndex]->elementIndex, x, y);
	}

	_screen->setClipRect(0, 0, 320, 200);

#if 0
	// DEBUG: Show actor number
	char temp[16];
	snprintf(temp, 16, "%d", actorIndex);
	_screen->drawText(CLIP(x, 16, 320 - 16), CLIP(y, 16, 200 - 16), (byte*)temp);
#endif

}

void CometEngine::drawAnimatedIcon(Animation *animation, uint frameListIndex, int x, int y, uint animFrameCounter) {
	AnimationFrameList *frameList = animation->_anims[frameListIndex];
	uint frameIndex = 0;
	if (frameList->frames.size() > 1) {
		frameIndex = animFrameCounter % frameList->frames.size();
		for (uint i = 0; i <= frameIndex; i++) {
			x += frameList->frames[i]->xOffs;
			y += frameList->frames[i]->yOffs;
		}
	}
	_screen->drawAnimationElement(animation, frameList->frames[frameIndex]->elementIndex, x, y);
}

void CometEngine::updateTextDialog() {
	
	if (_textActive || _textBubbleActive)
		updateText();
		
	if (_dialog->isRunning())
		_dialog->update();
	
}

void CometEngine::updateText() {

	//TODO

	Actor *actor = getActor(_talkActorIndex);
	int textX, textY;

	if (actor->textX != -1) {
		textX = actor->textX;
		textY = actor->textY;
	} else {
		textX = actor->x;
		textY = actor->y - _textMaxTextHeight - 50;
	}

	drawBubble(textX - _textMaxTextWidth - 4, textY - 4, textX + _textMaxTextWidth + 4, textY + _textMaxTextHeight);

	_screen->drawText3(textX + 1, textY, _currentText, _talkTextColor, 0);
	
	_textDuration--;

	// TODO: Merge _talkieMode handling code
	
	if (_talkieMode == 0 && _textDuration <= 0) {
		_textActive = _moreText;
		if (_moreText) {
			setText(_textNextPos);
		} else {
			resetTextValues();
		}
	}
	
	if (_talkieMode == 1 && _textDuration <= 0) {
		_textActive = _moreText;
		if (_moreText) {
			setText(_textNextPos);
		} else {
			if (!_mixer->isSoundHandleActive(_sampleHandle)) {
				resetTextValues();
			} else {
				_textDuration = 2;
				_textActive = true;
			}
		}
	}

	if (_talkieMode == 2 && _textDuration <= 0) {
		_textActive = _moreText;
		if (_moreText) {
			setText(_textNextPos);
		} else {
			if (!_mixer->isSoundHandleActive(_sampleHandle)) {
				resetTextValues();
			}
		}
	}

}

void CometEngine::updateTalkAnims() {
	//TODO
}

void CometEngine::resetVars() {

	//TODO: scDisableRectFlag();
	_paletteBrightness = 255;
	//TODO: g_sp_byte_1 = 0;
	_cmdGet = false;
	_cmdLook = false;
	_cmdTalk = false;
 	_scene->clearExits();
	_blockedInput = 0;
	_sceneItems.clear();

}

void CometEngine::loadAndRunScript(bool loadingGame) {

	Common::File fd;
	uint32 ofs;

	fd.open(RName);
	fd.seek(_currentSceneNumber * 4);
	ofs = fd.readUint32LE();
	fd.seek(ofs);
	fd.read(_script->_scriptData, 3000);
	fd.close();

	if (!loadingGame) {
		resetVars();
		resetActorsLife();
		_script->initializeScript();
	} else {
		_script->initializeScriptAfterLoadGame();
	}

}

void CometEngine::freeMarcheAndStaticObjects() {
	_animationMan->purgeAnimationSlots();
	if (_sceneDecorationSprite) {
		delete _sceneDecorationSprite;
		_sceneDecorationSprite = NULL;
	}
}

void CometEngine::resetMarcheAndStaticObjects() {
	resetTextValues();
	freeMarcheAndStaticObjects();
}

void CometEngine::updateScreen() {

	//TODO: seg011:0003 - seg011:004C
	
	if (_beams.size() > 0)
		drawBeams();
	
	if (_clearScreenRequest) {
		_screen->clearScreen();
		_clearScreenRequest = false;
	}
	
	if (_currentModuleNumber == 9 && _currentSceneNumber == 0 && _paletteStatus == 0) {
		memcpy(_paletteBuffer, _ctuPal, 768);
		memcpy(_ctuPal, _pali0Pal, 768);
		memcpy(_palette, _pali0Pal, 768);
		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteStatus = 3;
	} else if (_currentModuleNumber == 9 && _currentSceneNumber == 1 && _paletteStatus == 3) {
		memcpy(_ctuPal, _cdintroPal, 768);
		memcpy(_palette, _cdintroPal, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteStatus = 2;
	} else if (_currentModuleNumber == 5 && _currentSceneNumber == 0 && (_paletteStatus == 2 || _paletteStatus == 3)) {
		memcpy(_ctuPal, _paletteBuffer, 768);
		memcpy(_palette, _paletteBuffer, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteStatus = 0;
	} else if (_currentModuleNumber == 0 && _currentSceneNumber == 0 && _paletteStatus != 0) {
		memcpy(_ctuPal, _paletteBuffer, 768);
		memcpy(_palette, _paletteBuffer, 768);
		_screen->setFullPalette(_ctuPal);
		_paletteStatus = 0;
	}

	_screen->update();

}

void CometEngine::blockInput(int flagIndex) {
	if (flagIndex == 0) {
		_mouseCursor2 = 0;
		_blockedInput = 15;
		actorStopWalking(getActor(0));
	} else {
		static const int constFlagsArray[5] = {0, 1, 8, 2, 4};
		_blockedInput |= constFlagsArray[flagIndex];
	}
}

void CometEngine::unblockInput() {
	_blockedInput = 0;
	if (_actors[0].status == 2)
		_actors[0].status = 0;
}

int16 CometEngine::random(int maxValue) {
	if (maxValue >= 2)
		return _rnd->getRandomNumber(maxValue - 1);
	else
		return 0;
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

void CometEngine::setText(byte *text) {
	
	int lineCount = 0;

	_currentText = text;
	
	_textMaxTextHeight = 0;
	_textMaxTextWidth = 0;
	_moreText = false;
	_textDuration = 0;
	
	while (*text != '*') {
		int textWidth = _screen->_font->getTextWidth(text);
		_textDuration += textWidth / 4;
		if (_textDuration < 100)
			_textDuration = 100;
		if (textWidth > _textMaxTextWidth)
			_textMaxTextWidth = textWidth;
		text += strlen((char*)text) + 1;
		if (textWidth != 0)
			_textMaxTextHeight++;
		if (++lineCount == 3 && *text != '*') {
			_moreText = true;
			break;
		}
	}

	_textNextPos = text;
	_textMaxTextWidth /= 2;
	_textMaxTextHeight *= 8;
	
	if (_textSpeed == 0) {
		_textDuration /= 2;
	} else if (_textSpeed == 2) {
		_textDuration = _textDuration / 2 + _textDuration;
	}
	
}

void CometEngine::resetTextValues() {
	_dialog->stop();
	_textBubbleActive = false;
	_textActive = false;
}

bool CometEngine::rectCompare(const Common::Rect &rect1, const Common::Rect &rect2) {

	return ( rect1.left <= rect2.right && rect1.top <= rect2.bottom && rect1.right >= rect2.left && rect1.bottom >= rect2.top );

}

int CometEngine::findRect(const GuiRectangle *rects, int x, int y, int count, int defaultId) {
	for (int i = 0; i < count; i++) {
		if (x > rects[i].x && x < rects[i].x2 && y > rects[i].y && y < rects[i].y2)
			return rects[i].id;
	}
	return defaultId;
}

void CometEngine::handleEvents() {

	Common::Event event;
	
	_keyScancode = Common::KEYCODE_INVALID;

	// FIXME: There must be a better way
	bool waitForKeyRelease = false;

	do {

		while (g_system->getEventManager()->pollEvent(event)) {

		switch (event.type) {

			case Common::EVENT_KEYDOWN:
				waitForKeyRelease = false;
				switch (event.kbd.keycode) {
				case Common::KEYCODE_UP:
					_keyDirection = 1;
					break;
				case Common::KEYCODE_DOWN:
					_keyDirection = 2;
					break;
				case Common::KEYCODE_LEFT:
					_keyDirection = 4;
					break;
				case Common::KEYCODE_RIGHT:
					_keyDirection = 8;
					break;
				default:
					waitForKeyRelease = true;
					break;
				}
				_keyScancode = event.kbd.keycode;
				break;

			case Common::EVENT_KEYUP:
				waitForKeyRelease = false;
				switch (event.kbd.keycode) {
				case Common::KEYCODE_UP:
					_keyDirection &= ~1;
					break;
				case Common::KEYCODE_DOWN:
					_keyDirection &= ~2;
					break;
				case Common::KEYCODE_LEFT:
					_keyDirection &= ~4;
					break;
				case Common::KEYCODE_RIGHT:
					_keyDirection &= ~8;
					break;
				default:
					break;
				}
				_keyScancode = event.kbd.keycode;
				break;

			case Common::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;

			case Common::EVENT_LBUTTONDOWN:
				_leftButton = true;
				break;

			case Common::EVENT_LBUTTONUP:
				_leftButton = false;
				break;

			case Common::EVENT_RBUTTONDOWN:
				_rightButton = true;
				break;

			case Common::EVENT_RBUTTONUP:
				_rightButton = false;
				break;

			case Common::EVENT_QUIT:
				_endLoopFlag = true;
				return;

			default:
				break;

			}

		}

	} while (waitForKeyRelease);
	
}

void CometEngine::waitForKeys() {
	while (_keyScancode != Common::KEYCODE_INVALID || _keyDirection != 0 || _leftButton || _rightButton) {
		handleEvents();
	}
}

void CometEngine::handleInput() {

	static const byte mouseCursorArray[] = {
		0, 1, 3, 0, 4, 4, 4, 0, 2, 2, 2, 0, 0, 0, 0, 0
	};
	
	static const byte mouseDirectionTable[] = {
		0, 0, 0, 0, 0, 0, 1, 2, 2, 4, 0, 1, 2, 3, 3, 0, 4, 2, 3, 4, 0, 1, 3, 3, 4, 0
	};

	Actor *actor = getActor(0);
	
	_mouseButtons4 = _keyDirection;
	_mouseCursor2 = mouseCursorArray[_mouseButtons4 & 0x0F];
	
	// TODO: seg009:212C...skip_mouse
	
	if ((_blockedInput & _mouseButtons4) || _dialog->isRunning()) {
		_mouseCursor2 = 0;
		_mouseButtons5 = 0;
	} else {
		_mouseButtons5 = _mouseButtons4 & 0x80;
	}
	
	//FIXME
	_scriptMouseFlag = (_keyScancode == Common::KEYCODE_RETURN) || (_mouseButtons5 & 0x80) || (_keyDirection2 != 0);

	if (actor->walkStatus & 3)
		return;
		
	if (_dialog->isRunning() && actor->directionAdd != 0) {
		actorStopWalking(actor);
		return;
	}

	int directionAdd = actor->directionAdd;

	actor->walkDestX = actor->x;
	actor->walkDestY = actor->y;
	
	if (directionAdd == 4)
		directionAdd = 0;
		
	if (actor->direction == _mouseCursor2 && !(_blockedInput & _mouseButtons4))
		directionAdd = 4;
		
	int direction = mouseDirectionTable[actor->direction * 5 + _mouseCursor2];
	
	actorSetDirection(actor, direction);
	actorSetDirectionAdd(actor, directionAdd);
	
}

void CometEngine::skipText() {
	_textDuration = 1;
	_textActive = false;
	//waitForKeys();
	stopVoice();
}

void CometEngine::handleKeyInput() {

	switch (_keyScancode) {
	case Common::KEYCODE_t:
		_cmdTalk = true;
		waitForKeys();
		break;
	case Common::KEYCODE_g:
		_cmdGet = true;
		waitForKeys();
		break;
	case Common::KEYCODE_l:
		_cmdLook = true;
		waitForKeys();
		break;
	case Common::KEYCODE_o:
		handleInventory();
		waitForKeys();
		break;
	case Common::KEYCODE_u:
		useCurrentInventoryItem();
		waitForKeys();
		break;
	case Common::KEYCODE_d:
		// TODO: handleDiskMenu();
		waitForKeys();
		break;
	case Common::KEYCODE_m:
		handleMap();
		waitForKeys();
		break;
	case Common::KEYCODE_i:
		handleReadBook();
		waitForKeys();
		break;
	case Common::KEYCODE_p:
		// TODO: checkForPauseGame();
		waitForKeys();
		break;
	case Common::KEYCODE_RETURN:
		skipText();
		waitForKeys();
		break;
	default:
		if (Common::KEYCODE_TAB == _keyScancode || _rightButton) {
			handleCommandBar();
		}
		break;			
	}

}

int CometEngine::handleLeftRightSceneExitCollision(int moduleNumber, int sceneNumber) {
	
	if (sceneNumber == -1) {
		_moduleNumber = -1;
		return 0;
	}
	
	_sceneNumber = sceneNumber;
	_moduleNumber = moduleNumber;
	
	Actor *mainActor = getActor(0);
	
	if (mainActor->direction != 1 && mainActor->direction != 3) {
	
		int x1, y1, x2, y2;

		mainActor->value6 = 4;

		_scene->getExitRect(mainActor->collisionIndex, x1, y1, x2, y2);
		if (x2 == 318)
			x2 = 319;

		// Disable collision checks
		mainActor->collisionType = kCollisionDisabled;

		if (mainActor->direction == 2) {
			mainActor->clipX1 = 0;
			mainActor->clipX2 = x2;
			actorStartWalking(0, 319, mainActor->y);
		} else if (mainActor->direction == 4) {
			mainActor->clipX1 = x1;
			mainActor->clipX2 = 319;
			actorStartWalking(0, 0, mainActor->y);
		}
		
		mainActor->walkStatus &= ~4;
		
	}
	
	return 1;
	
}

void CometEngine::addSceneItem(int itemIndex, int x, int y, int paramType) {
	SceneItem sceneItem;
	sceneItem.itemIndex = itemIndex;
	sceneItem.active = true;
	sceneItem.paramType = paramType;
	sceneItem.x = x;
	sceneItem.y = y;
	_sceneItems.push_back(sceneItem);
}

void CometEngine::removeSceneItem(int itemIndex) {
	uint index = 0;
	while (index < _sceneItems.size()) {
		if (_sceneItems[index].itemIndex == itemIndex) {
			_sceneItems.remove_at(index);
		} else {
			index++;
		}
	}
}

uint16 CometEngine::findSceneItemAt(const Common::Rect &rect) {
	for (uint i = 0; i < _sceneItems.size(); i++) {
		if (_sceneItems[i].active) {
			Common::Rect itemRect(_sceneItems[i].x - 8, _sceneItems[i].y - 8, _sceneItems[i].x + 8, _sceneItems[i].y + 8);
			if (rectCompare(rect, itemRect)) {
				return COLLISION(kCollisionSceneItem, i);
			}
		}
	}
	return 0;
}

void CometEngine::showTextBubble(int index, byte *text) {

	_talkActorIndex = 0;
	_talkTextColor = 21;
	_talkTextIndex = index;
	setText(text);
	_textActive = true;
	_textBubbleActive = true;

}

void CometEngine::drawLineOfSight() {
	if (_itemInSight) {
		int x = _actors[0].x;
		int y = _actors[0].y - 35;
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
		_screen->drawDottedLine(x, y, _itemX + random(3) - 1, _itemY + random(3) - 1, 7);
	}
}

uint16 CometEngine::checkCollisionWithActors(int selfActorIndex, Common::Rect &rect, Common::Rect &obstacleRect) {
	for (int index = 0; index < 11; index++) {
		Actor *actor = getActor(index);
		if (index != selfActorIndex && actor->life != 0 && actor->collisionType != kCollisionDisabled) {
			obstacleRect.left = actor->x - actor->deltaX;
			obstacleRect.top = actor->y - actor->deltaY;
			obstacleRect.right = actor->x + actor->deltaX;
			obstacleRect.bottom = actor->y;
			if (rectCompare(rect, obstacleRect)) {
				return COLLISION(kCollisionActor, index);
			}
		}
	}
	return 0;
}

uint16 CometEngine::checkCollision(int index, int x, int y, int deltaX, int deltaY, int direction, Common::Rect &obstacleRect) {

	uint16 collisionType = 0;

	Common::Rect collisionRect(x - deltaX, y - deltaY, x + deltaX, y);
	
	collisionType = _scene->checkCollisionWithBounds(collisionRect, direction);
	if (collisionType != 0) {
		uint16 sceneExitCollision = _scene->checkCollisionWithExits(collisionRect, direction);
		if (sceneExitCollision != 0)
			collisionType = sceneExitCollision;
	} else {
		collisionType = _scene->checkCollisionWithBlockingRects(collisionRect, obstacleRect);
		if (collisionType == 0)
			collisionType = checkCollisionWithActors(index, collisionRect, obstacleRect);
	}

	return collisionType;

}

void CometEngine::initSceneDecorationBlockingRects() {

	_scene->_blockingRects.clear();

	for (uint i = 0; i < _sceneDecorationSprite->_elements[0]->commands.size(); i++) {
		AnimationCommand *cmd = _sceneDecorationSprite->_elements[0]->commands[i];
		AnimationElement *objectElement = _sceneDecorationSprite->_elements[((cmd->arg2 << 8) | cmd->arg1) & 0x7FFF];
		debug(8, "%03d: cmd = %d; arg1 = %d; arg2 = %d; x = %d; y = %d; width = %d; height = %d",
			i, cmd->cmd, cmd->arg1, cmd->arg2, cmd->points[0].x, cmd->points[0].y, objectElement->width, objectElement->height);
		if (objectElement->width / 2 > 0) {
			int16 blockX1, blockY1, blockX2, blockY2;
			if (cmd->cmd == 0)
				blockX1 = (cmd->points[0].x - objectElement->width) / 2;
			else
				blockX1 = cmd->points[0].x / 2;
			blockY1 = cmd->points[0].y - (objectElement->height / 16 % 4 * 4 + 4);
			blockX2 = (cmd->points[0].x + objectElement->width) / 2;
			blockY2 = cmd->points[0].y;
			_scene->addBlockingRect(blockX1, blockY1, blockX2, blockY2);
		}
	}

}

uint16 CometEngine::updateCollision(Actor *actor, int actorIndex, uint16 collisionType) {

	int result = 0;
	
	actor->collisionType = COLLISION_TYPE(collisionType);
	actor->collisionIndex = COLLISION_INDEX(collisionType);

	if (actorIndex == 0 && actor->collisionType == kCollisionSceneExit) {
		int moduleNumber, sceneNumber;
		_scene->getExitLink(actor->collisionIndex, moduleNumber, sceneNumber);
		result = handleLeftRightSceneExitCollision(moduleNumber, sceneNumber);
	}
	
	if (result == 0) {
		actorSetDirectionAdd(actor, 0);
		updateActorAnimation(actor);
	}

	return result;

}

void CometEngine::handleSceneChange(int sceneNumber, int moduleNumber) {

	debug(4, "###### handleSceneChange(%d, %d)", sceneNumber, moduleNumber);

	Actor *actor = getActor(0);
	int direction, x1, x2, y1, y2;

	_scene->findExitRect(sceneNumber, moduleNumber, actor->direction, x1, y1, x2, y2, direction);

	actor->x = (x2 - x1) / 2 + x1;
	actor->y = (y2 - y1) / 2 + y1;
	actor->direction = direction;
	actorSetAnimNumber(actor, direction - 1);
	
	// Scene change effects
	if (_screen->getZoomFactor() == 0) {
		if (direction == 1 || direction == 3) {
			_screen->enableTransitionEffect();
		} else {
			memcpy(_screen->getScreen(), _sceneBackground, 320 * 200);
			buildSpriteDrawQueue();		
			drawSpriteQueue();
			memcpy(_sceneBackground, _screen->getScreen(), 320 * 200);
			if (direction == 2) {
				_screen->screenScrollEffect(_sceneBackground, -1);
			} else if (direction == 4) {
				_screen->screenScrollEffect(_sceneBackground, 1);
			} 		
			loadSceneBackground();
		}
	}

	if (_screen->getFadeType() == kFadeNone) {
		_screen->buildPalette(_ctuPal, _palette, _paletteBrightness);
		_screen->setFullPalette(_palette);
	}

}

void CometEngine::playMusic(int musicIndex) {
	if (musicIndex != 255) {
		//TODO_vm->_music->playMusic(musicIndex);
	} else {
		//TODO: musicFadeDown();
		_music->stopMusic();
	}
}

void CometEngine::playSample(int sampleIndex, int loopCount) {
	// The sample files are plain Voc files without a modified header
	debug(2, "playSample(%d, %d)", sampleIndex, loopCount);
	if (sampleIndex == 255) {
		if (_mixer->isSoundHandleActive(_sampleHandle))
			_mixer->stopHandle(_sampleHandle);
	} else if (!_mixer->isSoundHandleActive(_sampleHandle)) {
		int sampleSize = getPakSize("SMP.PAK", sampleIndex);
		byte *sampleBuffer = (byte*)malloc(sampleSize);
		loadPakToPtr("SMP.PAK", sampleIndex, sampleBuffer);
		Common::MemoryReadStream vocReadStream(sampleBuffer, sampleSize, DisposeAfterUse::YES);
		Audio::AudioStream *audioStream;
		if (loopCount > 1) {
			audioStream = makeLoopingAudioStream(Audio::makeVOCStream(&vocReadStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), loopCount);
		} else {
			audioStream = Audio::makeVOCStream(&vocReadStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		}
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_sampleHandle, audioStream);
	}
}

void CometEngine::drawTextIllsmouth() {
	byte *text = _textReader->getString(2, 36);
	_screen->drawTextOutlined((320 - _screen->_font->getTextWidth(text)) / 2, 180, text, 7, 0); 
	_scriptVars[11]++;
}

void CometEngine::playCutscene(int fileIndex, int frameListIndex, int backgroundIndex, int loopCount, int soundFramesCount, byte *soundFramesData) {
	
	debug("playCutscene(%d, %d, %d, %d, %d)", fileIndex, frameListIndex, backgroundIndex, loopCount, soundFramesCount);

	int palStatus = 0;
	Animation *cutsceneSprite;
	AnimationFrameList *frameList;
	int animFrameCount;
	
	// TODO: __snd_stopSample
	// TODO: narStopSpeech
	skipText();
	
	cutsceneSprite = _animationMan->loadAnimationResource(AName, fileIndex);
	frameList = cutsceneSprite->_anims[frameListIndex];
	animFrameCount = frameList->frames.size();
	
	if (backgroundIndex == 0) {
		_screen->clearScreen();
	} else if (backgroundIndex < 0) {
		_screen->drawAnimationElement(cutsceneSprite, -backgroundIndex, 0, 0);
	} else if (backgroundIndex < 32000) {
		_screen->drawAnimationElement(_sceneDecorationSprite, backgroundIndex, 0, 0);
	} else if (backgroundIndex == 32000) {
		// TODO: Grab vga screen to work screen
	}
	
	memcpy(_sceneBackground, _screen->getScreen(), 64000);

	if (soundFramesCount > 0) {
		int sampleIndex = soundFramesData[0];
		debug("  sampleIndex = %d", sampleIndex);
		// TODO: Load the sample
	}
	
	for (int loopIndex = 0; loopIndex < loopCount; loopIndex++) {
	
		byte *workSoundFramesData = soundFramesData;
		int workSoundFramesCount = soundFramesCount;
		int animFrameIndex, animSoundFrameIndex, interpolationStep = 0;
		
		if (soundFramesCount > 0) {
			workSoundFramesData++;
			animSoundFrameIndex = *workSoundFramesData++;
			workSoundFramesData++;
		}
		
		animFrameIndex = 0;
		while (animFrameIndex < animFrameCount) {
		
			handleEvents();
		
			memcpy(_screen->getScreen(), _sceneBackground, 64000);

			interpolationStep = _screen->drawAnimation(cutsceneSprite, frameList, animFrameIndex, interpolationStep, 0, 0, animFrameCount);
						
			updateTextDialog();
			
			if (palStatus == 1) {
				// TODO: Set the anim palette
				palStatus = 2;
			}										

			_screen->update();
			_system->delayMillis(40); // TODO
						
			if (workSoundFramesCount > 0 && animFrameIndex == animSoundFrameIndex) {
				// TODO: Play the anim sound
				workSoundFramesCount--;
				if (workSoundFramesCount > 0) {
					// TODO: Load the next sample; unused in Comet CD (only max. one sample per cutscene)
				}
			}						

			// TODO: checkForPauseGame();
			if (_keyScancode == Common::KEYCODE_ESCAPE) {
				// TODO: yesNoDialog();
			} else if (_keyScancode == Common::KEYCODE_RETURN) {
				animFrameIndex = animFrameCount;
				loopIndex = loopCount;
				if (_textActive)
					skipText();
			}
			
			if (interpolationStep == 0)		
				animFrameIndex++;
		}
			
	}
	
	delete cutsceneSprite;

	if (palStatus > 0) {
		_screen->setFullPalette(_palette);
	}
	
	if (_textActive)
		resetTextValues();

	loadSceneBackground();
	memcpy(_screen->getScreen(), _sceneBackground, 64000);
	
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
		byte color = random(7) + 144;
		currX2 = random(8) + (x2 - x1) * i / 10 + x1 - 2;
		currY2 = random(8) + (y2 - y1) * i / 10 + y1 - 2;
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
	// TODO: beamColor stuff, unused in Comet CD
}

/* Control bar */

void CometEngine::drawCommandBar(int selectedItem, int animFrameCounter) {

	const int x = 196;
	const int y = 14;

	_screen->drawAnimationElement(_iconSprite, 0, 0, 0);
	_screen->drawAnimationElement(_iconSprite, selectedItem + 1, 0, 0);

	if (_currentInventoryItem >= 0 && _inventoryItemStatus[_currentInventoryItem] == 0) {
		_currentInventoryItem = -1;
		for (int inventoryItem = 0; inventoryItem <= 255 && _currentInventoryItem == -1; inventoryItem++) {
			if (_inventoryItemStatus[inventoryItem] > 0)
				_currentInventoryItem = inventoryItem;
		}
	}	

	if (_currentInventoryItem >= 0)
		drawAnimatedIcon(_inventoryItemSprites, _currentInventoryItem, x, y, animFrameCounter);
	
}

void CometEngine::handleCommandBar() {

	const int kCBANone		= -1;
	const int kCBAExit		= -2;
	const int kCBAVerbTalk	= 0;
	const int kCBAVerbGet	= 1;
	const int kCBAUseItem	= 2;
	const int kCBAVerbLook	= 3;
	const int kCBAInventory	= 4;
	const int kCBAMap		= 5;
	const int kCBAMenu		= 6;

	static const GuiRectangle commandBarRects[] = {
		{  6, 4,  41, 34, kCBAVerbTalk}, 
		{ 51, 4,  86, 34, kCBAVerbGet}, 
		{ 96, 4, 131, 34, kCBAUseItem},
		{141, 4, 176, 34, kCBAVerbLook}, 
		{186, 4, 221, 34, kCBAInventory}, 
		{231, 4, 266, 34, kCBAMap},
		{276, 4, 311, 34, kCBAMenu}};
	const int commandBarItemCount = 6; // Intentionally doesn't match actual count!

	int commandBarStatus = 0;
	int animFrameCounter = 0;
	
	_menuStatus++;
	
	waitForKeys();

	// TODO: copyScreens(vgaScreen, _sceneBackground);
	// TODO: copyScreens(vgaScreen, _workScreen);
	// TODO: setMouseCursor(1, 0);

	_commandBarSelectedItem = kCBANone;

	while (commandBarStatus == 0) {
		int mouseSelectedItem, commandBarAction = kCBANone;
	
		mouseSelectedItem = findRect(commandBarRects, _mouseX, _mouseY, commandBarItemCount + 1, kCBANone);
		if (mouseSelectedItem != kCBANone)
			_commandBarSelectedItem = mouseSelectedItem;
			
		drawCommandBar(_commandBarSelectedItem,	animFrameCounter++);		
		_screen->update();
		_system->delayMillis(40); // TODO

		handleEvents();

		if (_keyScancode == Common::KEYCODE_INVALID && !_leftButton && !_rightButton)
			continue;

		if (_rightButton) {
			commandBarAction = kCBAExit;
		} else if (_leftButton && _commandBarSelectedItem != kCBANone) {
			commandBarAction = _commandBarSelectedItem;
		}
		
		switch (_keyScancode) {
		case Common::KEYCODE_RIGHT:
			if (_commandBarSelectedItem == commandBarItemCount) {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem = 0;
			} else {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem++;
			}
			break;
		case Common::KEYCODE_LEFT:
			if (_commandBarSelectedItem == 0) {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem = commandBarItemCount;
			} else {
				if (mouseSelectedItem == _commandBarSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_commandBarSelectedItem--;
			}
			break;
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_TAB:
			commandBarAction = kCBAExit;
			break;
		case Common::KEYCODE_RETURN:
			commandBarAction = _commandBarSelectedItem;
			break;			
		case Common::KEYCODE_t:
			commandBarAction = kCBAVerbTalk;
			break;
		case Common::KEYCODE_g:
			commandBarAction = kCBAVerbGet;
			break;
		case Common::KEYCODE_l:
			commandBarAction = kCBAVerbLook;
			break;
		case Common::KEYCODE_o:
			commandBarAction = kCBAInventory;
			break;
		case Common::KEYCODE_u:
			commandBarAction = kCBAUseItem;
			break;
		case Common::KEYCODE_d:
			commandBarAction = kCBAMenu;
			break;
		case Common::KEYCODE_m:
			commandBarAction = kCBAMap;
			break;
		default:
			break;			
		}
		
		if (commandBarAction >= 0) {
			drawCommandBar(commandBarAction, animFrameCounter);		
			_screen->update();
		}
		
		switch (commandBarAction) {
		case kCBANone:
			break;
		case kCBAExit:
			commandBarStatus = 2;
			break;
		case kCBAVerbTalk:
			_cmdTalk = true;
			commandBarStatus = 1;
			break;
		case kCBAVerbGet:
			_cmdGet = true;
			commandBarStatus = 1;
			break;
		case kCBAVerbLook:
			_cmdLook = true;
			commandBarStatus = 1;
			break;
		case kCBAUseItem:
			useCurrentInventoryItem();
			commandBarStatus = 1;
			break;
		case kCBAInventory:
			commandBarStatus = handleInventory();
			break;
		case kCBAMap:
			commandBarStatus = handleMap();
			break;
		case kCBAMenu:
			// TODO: Disk menu
			break;
		}								

		waitForKeys();
	
	}

	waitForKeys();

	_menuStatus--;

	loadSceneBackground();

}
	
/* Disk menu */

void CometEngine::drawDiskMenu(int selectedItem) {

	const int x = 137;
	const int y = 65;
	const int itemHeight = 23;

	_screen->drawAnimationElement(_iconSprite, 10, 0, 0);
	_screen->drawAnimationElement(_iconSprite, 11, x, y + selectedItem * itemHeight);

}

int CometEngine::handleDiskMenu() {
	
	const int kDMANone		= -1;
	const int kDMAExit		= -2;
	const int kDMASave		= 0;
	const int kDMALoad		= 1;
	const int kDMAOptions	= 2;
	const int kDMAQuit		= 3;

	static const GuiRectangle diskMenuRects[] = {
		{136,  64, 184,  80, kDMASave},
		{136,  87, 184, 103, kDMALoad},
		{136, 110, 184, 126, kDMAOptions},
		{136, 133, 184, 149, kDMAQuit}};		

	int diskMenuStatus = 0;
	
	_menuStatus++;
	
	waitForKeys();

	// TODO

	_diskMenuSelectedItem = kDMASave;

	while (diskMenuStatus == 0) {
		int mouseSelectedItem, diskMenuAction = kDMANone;
	
		mouseSelectedItem = findRect(diskMenuRects, _mouseX, _mouseY, 4, kDMANone);
		if (mouseSelectedItem != kDMANone)
			_diskMenuSelectedItem = mouseSelectedItem;
			
		drawDiskMenu(_diskMenuSelectedItem);		
		_screen->update();
		_system->delayMillis(40); // TODO

		handleEvents();

		if (_keyScancode == Common::KEYCODE_INVALID && !_leftButton && !_rightButton)
			continue;

		if (_rightButton) {
			diskMenuAction = kDMAExit;
		} else if (_leftButton && _diskMenuSelectedItem != kDMANone) {
			diskMenuAction = _diskMenuSelectedItem;
		}
		
		switch (_keyScancode) {
		case Common::KEYCODE_DOWN:
			if (_diskMenuSelectedItem == 3) {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem = 0;
			} else {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem++;
			}
			break;
		case Common::KEYCODE_UP:
			if (_diskMenuSelectedItem == 0) {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem = 3;
			} else {
				if (mouseSelectedItem == _diskMenuSelectedItem) {
					// TODO: Warp mouse cursor
				}
				_diskMenuSelectedItem--;
			}
			break;
		case Common::KEYCODE_ESCAPE:
			diskMenuAction = kDMAExit;
			break;
		case Common::KEYCODE_RETURN:
			diskMenuAction = _diskMenuSelectedItem;
			break;			
		case Common::KEYCODE_s:
			diskMenuAction = kDMASave;
			break;
		case Common::KEYCODE_l:
			diskMenuAction = kDMALoad;
			break;
		case Common::KEYCODE_t:
			diskMenuAction = kDMAOptions;
			break;
		case Common::KEYCODE_x:
			diskMenuAction = kDMAQuit;
			break;
		default:
			break;			
		}
		
		if (diskMenuAction >= 0) {
			drawDiskMenu(_diskMenuSelectedItem);		
			_screen->update();
		}
		
		switch (diskMenuAction) {
		case kDMANone:
			break;
		case kDMAExit:
			diskMenuStatus = 2;
			break;
		case kDMASave:
			// TODO
			debug("disk menu: save game");
			diskMenuStatus = 0;
			break;
		case kDMALoad:
			// TODO
			debug("disk menu: load game");
			diskMenuStatus = 0;
			break;
		case kDMAOptions:
			// TODO
			debug("disk menu: options");
			diskMenuStatus = 0;
			break;
		case kDMAQuit:
			// TODO
			debug("disk menu: quit");
			diskMenuStatus = 0;
			break;
		}								

		waitForKeys();
	
	}

	waitForKeys();

	_menuStatus--;

	loadSceneBackground();

	return 0;
}

void CometEngine::initSystemVars() {
	_systemVars[0] = &_prevSceneNumber;
	for (int i = 0; i < 10; i++) {
		_systemVars[1 + i * 3] = &_actors[i].life;
		_systemVars[2 + i * 3] = &_actors[i].x;
		_systemVars[3 + i * 3] = &_actors[i].y;
	}
	_systemVars[31] = &_mouseButtons4;
	_systemVars[32] = &_scriptMouseFlag;
	_systemVars[33] = &_scriptRandomValue;
	_systemVars[34] = &_prevModuleNumber;
}

void CometEngine::introMainLoop() {

	_endIntroLoop = false;

	while (!_endIntroLoop /*TODO:Check for quit*/) {
		handleEvents();
		
		switch (_keyScancode) {
		case Common::KEYCODE_ESCAPE:
			_endIntroLoop = true;
			break;
		case Common::KEYCODE_RETURN	:
			skipText();
			break;
		case Common::KEYCODE_p:
			// TODO: checkForPauseGame();
			break;
		default:			
			break;
		}

		if (_currentSceneNumber == 0 && _currentModuleNumber == 0)
			_endIntroLoop = true;

		updateGame();
		_system->delayMillis(40);//TODO

	}

}

void CometEngine::gameMainLoop() {

	/* Hacked together main loop */
	_endLoopFlag = false;
	while (!_endLoopFlag) {
		handleEvents();

#if 0
		// Test the "beam-room"
		_scriptVars[116] = 1;
		_scriptVars[139] = 1;
		_moduleNumber = 7;
		_sceneNumber = 4;
#endif			

		if (_currentModuleNumber == 7 && _currentSceneNumber == 1 && _paletteStatus == 0) {
			memcpy(_paletteBuffer, _ctuPal, 768);
			memcpy(_ctuPal, _flashbakPal, 768);
			memcpy(_palette, _flashbakPal, 768);
			_screen->clearScreen();
			_screen->setFullPalette(_ctuPal);
			_paletteStatus = 1;
		} else if (_currentModuleNumber == 2 && _currentSceneNumber == 22 && _paletteStatus == 1) {
			memcpy(_ctuPal, _paletteBuffer, 768);
			memcpy(_palette, _paletteBuffer, 768);
			_screen->clearScreen();
			_screen->setFullPalette(_ctuPal);
			_paletteStatus = 0;
		}
		
		if (_scriptVars[9] == 1) {
			// TODO: Copy vga screen to scene background
			_scriptVars[9] = runPuzzle();
			loadSceneBackground();
		}
		
		if (!_dialog->isRunning() && _currentModuleNumber != 3 && _actors[0].value6 != 4 && !_screen->_palFlag && !_textActive) {
			handleKeyInput();
		} else if (_keyScancode == Common::KEYCODE_RETURN || (_rightButton && _textActive)) {
			skipText();
		}

		// Debugging keys
		switch (_keyScancode) {
		case Common::KEYCODE_r:
			_debugRectangles = !_debugRectangles;
			break;
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
		_system->delayMillis(40);//TODO

		if (_loadgameRequested) {
			/* TODO:
				while (savegame_load() == 0);
				savegame_load
			*/
			_loadgameRequested = false;
		}

		checkCurrentInventoryItem();

	}
	
}

} // End of namespace Comet
