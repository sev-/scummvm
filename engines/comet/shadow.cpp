#include "sound/voc.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/pak.h"

#include "comet/screen.h"
#include "comet/dialog.h"
#include "comet/animation.h"
#include "comet/animationmgr.h"
#include "comet/scene.h"

namespace Comet {

// TODO: Move a lot of stuff to own classes

void drawDottedLinePlotProc(int x, int y, int color, void *data = NULL) {
	// FIXME: Fix this messy stuff
	CometEngine *engine = (CometEngine*)data;
	if (x >= 0 && x < 320 && y >= 0 && y < 200) {
		engine->_dotFlag++;
		if (engine->_dotFlag & 2)
			engine->_screen->getScreen()[x + y * 320] = color;
	}
}

void CometEngine::drawDottedLine(int x1, int y1, int x2, int y2, int color) {
	// FIXME: (see drawDottedLinePlotProc above)
	_dotFlag = 1;
	Graphics::drawLine(x1, y1, x2, y2, color, drawDottedLinePlotProc, (void*)this);
}

int CometEngine::comparePointXY(int x, int y, int x2, int y2) {
	int flags = 0;
	if (x == x2)
		flags |= 1;
	if (y == y2)
		flags |= 2;
	return flags;
}

void CometEngine::calcSightRect(Common::Rect &rect, int delta1, int delta2) {

	int x = _sceneObjects[0].x - _sceneObjects[0].deltaX - 8;
	int y = _sceneObjects[0].y - _sceneObjects[0].deltaY - 8;
	int x2 = _sceneObjects[0].x + _sceneObjects[0].deltaX + 8;
	int y2 = _sceneObjects[0].y + 8;

	switch (_sceneObjects[0].direction) {
	case 1:
		y -= delta2;
		y2 -= 20;
		x -= delta1;
		x2 += delta1;
		break;
	case 2:
		x2 += delta2;
		x += 25;
		y -= 32;
		break;
	case 3:
		y2 += delta2;
		y += 20;
		break;
	case 4:
		x -= delta2;
		x2 -= 25;
		y -= 32;
		break;
	}

	rect.left = MAX(x, 0);
	rect.top = MAX(y, 0);
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
	for (uint32 i = 0; i < _scene->_sceneExits.size(); i++) {
		if (_scene->_sceneExits[i].directionIndex == 3) {
			_screen->fillRect(_scene->_sceneExits[i].x1, 198, _scene->_sceneExits[i].x2, 199, 120);
			_screen->hLine(_scene->_sceneExits[i].x1 + 1, 199, _scene->_sceneExits[i].x2 - 2, 127);
		}
	}
}

/* Scene */

void CometEngine::initSceneBackground(bool loadingGame) {
	
	_screen->clearScreen();
	
	loadSceneBackground();
	loadStaticObjects();

	memcpy(_screen->getScreen(), _sceneBackground, 64000);
	drawSceneForeground();
	memcpy(_sceneBackground, _screen->getScreen(), 64000);

	/*TODO
	if (screenCopyFlag != 0)
		screen_c_1();
	*/
	
	if (!loadingGame)
		initStaticObjectRects();

}

void CometEngine::loadSceneBackground() {
	loadPakToPtr(DName, _backgroundFileIndex, _sceneBackground);
}

void CometEngine::loadStaticObjects() {
	delete _sceneObjectsSprite;
	
	debug(8, "CometEngine::loadStaticObjects() DName = %s; index = %d", DName, _backgroundFileIndex + 1);
	
	_sceneObjectsSprite = _animationMan->loadAnimationResource(DName, _backgroundFileIndex + 1);
}

void CometEngine::drawSceneForeground() {
	if (_sceneObjectsSprite->_elements.size() > 0)
		_screen->drawAnimationElement(_sceneObjectsSprite, 0, 0, 0);
}

/* Graphics */

void plotProc(int x, int y, int color, void *data) {
	if (x >= 0 && x < 320 && y >= 0 && y < 200)
		((byte*)data)[x + y * 320] = color;
}

void CometEngine::initAndLoadGlobalData() {

	_screen->loadFont("RES.PAK", 0);

	_bubbleSprite = _animationMan->loadAnimationResource("RES.PAK", 1);
	_heroSprite = _animationMan->loadAnimationResource("RES.PAK", 2);
	_objectsVa2 = _animationMan->loadAnimationResource("RES.PAK", 4);

	_ctuPal = loadFromPak("RES.PAK", 5);
	_flashbakPal = loadFromPak("RES.PAK", 6);
	_cdintroPal = loadFromPak("RES.PAK", 7);
	_pali0Pal = loadFromPak("RES.PAK", 8);

	_cursorVa2 = _animationMan->loadAnimationResource("RES.PAK", 9);
	_iconeVa2 = _animationMan->loadAnimationResource("RES.PAK", 3);
	
	_screen->setFontColor(0);

	//TODO: seg001:0758 Mouse cursor stuff...
	
	_paletteBuffer = new byte[768];
	memcpy(_paletteBuffer, _ctuPal, 768);
	
	initData();

	loadGlobalTextData();
	
	//TODO...

	setChapterAndScene(_startupChapterNumber, _startupSceneNumber);
	
}

void CometEngine::loadGlobalTextData() {
	_textActive = false;
	_narOkFlag = false;
	_textBuffer2 = _textReader->loadTextStrings(0);
	_textBuffer3 = _textReader->loadTextStrings(1);
}

void CometEngine::initData() {

	_sceneBackground = new byte[72000];
	_scratchBuffer = _sceneBackground + 64000;
	_palette = new byte[768];

	memcpy(_palette, _ctuPal, 768);
	
	memset(_scriptVars, 0, sizeof(_scriptVars));
	memset(_itemStatus, 0, sizeof(_itemStatus));

	_screen->setFontColor(19);
	unblockInput();
	
	_currentChapterNumber = 0;
	_sceneNumber = 0;

/*
	_bounds.clear();
	_bounds.push_back(Common::Point(0, 100));
	_bounds.push_back(Common::Point(319, 100));
	initSceneBoundsMap();
*/
	resetVars();
	
	sceneObjectsResetFlags();

	sceneObjectInit(0, _animationMan->getAnimationResource(1, 0));

	sceneObjectSetPosition(0, 160, 190);
	_sceneObjects[0].life = 99;

}

void CometEngine::setChapterAndScene(int chapterNumber, int sceneNumber) {

	_chapterNumber = chapterNumber;
	_sceneNumber = sceneNumber;
	
	//debug(4, "CometEngine::setChapterAndScene(%d, %d)", chapterNumber, sceneNumber);
	
	//FIXME
	sprintf(AName, "A%d%d.PAK", _chapterNumber / 10, _chapterNumber % 10);
	sprintf(DName, "D%d%d.PAK", _chapterNumber / 10, _chapterNumber % 10);
	sprintf(RName, "R%d%d.CC4", _chapterNumber / 10, _chapterNumber % 10);
	
	//debug(4, "AName = %s; DName = %s; RName = %s", AName, DName, RName);
	
}

void CometEngine::updateGame() {

	_gameLoopCounter++;
	_textColorFlag++;

#if 0
	debug(0, "_chapterNumber = %d; _currentChapterNumber = %d", _chapterNumber, _currentChapterNumber);
	debug(0, "_sceneNumber = %d; _currentSceneNumber = %d", _sceneNumber, _currentSceneNumber);
#endif

	debug(1, "CometEngine::updateGame() #0");

	if (_chapterNumber != _currentChapterNumber)
		updateChapterNumber();

	if (_sceneNumber != _currentSceneNumber)
		updateSceneNumber();

	memcpy(_screen->getScreen(), _sceneBackground, 64000);

	if (_cmdLook)
		lookAtItemInSight(true);

	if (_cmdGet)
		getItemInSight();

	debug(1, "CometEngine::updateGame() #1");

	handleInput();
	
	/*
	// Test for mouse-based walking, it even works somewhat
	if (_mouseLeft) {
		sceneObjectStartWalking(0, _mouseX, _mouseY);
	}
	*/

	debug(1, "CometEngine::updateGame() #2");

	_script->runAllScripts();

	if (_needToLoadSavegameFlag)
		return;

	debug(1, "CometEngine::updateGame() #3.1");
	drawSceneExits();
	debug(1, "CometEngine::updateGame() #3.2");
	sceneObjectsUpdateAnimations();
	debug(1, "CometEngine::updateGame() #3.3");
	sceneObjectsUpdateMovement();
	debug(1, "CometEngine::updateGame() #3.4");
	updateStaticObjects();
	debug(1, "CometEngine::updateGame() #3.5");
	sceneObjectsEnqueueForDrawing();
	debug(1, "CometEngine::updateGame() #3.6");
	lookAtItemInSight(false);

	debug(1, "CometEngine::updateGame() #4");

	drawSprites();

	debug(1, "CometEngine::updateGame() #5");

	if (_talkieMode == 0)
		updateTextDialog();

	if (_talkieMode == 1 && (_textActive || _flag03))
		updateText();

	if (_dialog->isRunning())
		_dialog->update();

	updateTalkAnims();

	if (_talkieMode == 2 || _flag03)
		updateText();

	if (_dialog->isRunning())
		_dialog->update();
		
	updateTalkAnims();
	
	/*TODO:
	if (_scriptVars[11] < 100 && _scriptVars[10] == 1)
		drawTextIllsmouth();
	*/
	if (_debugRectangles) {
	#if 0
		debug(1, "CometEngine::updateGame() #A");
		/* begin DEBUG rectangles */
		for (uint32 i = 0; i < _blockingRects.size(); i++)
			_screen->fillRect(_blockingRects[i].left, _blockingRects[i].top, _blockingRects[i].right, _blockingRects[i].bottom, 120);
		_screen->fillRect(_sceneObjects[0].x - _sceneObjects[0].deltaX, _sceneObjects[0].y - _sceneObjects[0].deltaY,
			_sceneObjects[0].x + _sceneObjects[0].deltaX, _sceneObjects[0].y, 150);
		for (uint32 index = 0; index < _sceneExits.size(); index++) {
			int x3, y3, x4, y4;
			getSceneExitRect(index, x3, y3, x4, y4);
			//debug(4, "SCENE EXIT: (%d, %d, %d, %d); direction = %d; chapterNumber = %d; sceneNumber = %d", x3, y3, x4, y4, _sceneExits[index].directionIndex, _sceneExits[index].chapterNumber, _sceneExits[index].sceneNumber);
			_screen->fillRect(x3, y3, x4, y4, 25);
		}
		for (int x = 0;  x < 320; x++)
			_screen->putPixel(x, _boundsMap[x], 0);
		/* end DEBUG rectangles */
	#endif
	}

	debug(1, "CometEngine::updateGame() #9");

	updateScreen();
	
	updateHeroLife();
	
	_needToLoadSavegameFlag = false;
	
	_cmdTalk = false;
	_cmdGet = false;
	_cmdLook = false;

	debug(1, "CometEngine::updateGame() #################################################################");

}

void CometEngine::updateChapterNumber() {
	if (_chapterNumber != -1) {
		_animationMan->purgeAnimationSlots();
		freeMarcheAndStaticObjects();
		setChapterAndScene(_chapterNumber, _sceneNumber);
		updateSceneNumber();
	}
}

void CometEngine::updateSceneNumber() {

	//TODO: mouse_4(0, 0x40);

	if (_sceneObjects[0].walkStatus != 0 &&
		((_sceneObjects[0].direction == 2 && _sceneObjects[0].x < 319) ||
		(_sceneObjects[0].direction == 4 && _sceneObjects[0].x > 0))) {

		_sceneObjects[0].y = _sceneObjects[0].walkDestY;

	} else {

		resetMarcheAndStaticObjects();
		_prevSceneNumber = _currentSceneNumber;
		_currentSceneNumber = _sceneNumber;
		_prevChapterNumber = _currentChapterNumber;
		_currentChapterNumber = _chapterNumber;
		
		sceneObjectStopWalking(&_sceneObjects[0]);
		
		_sceneObjects[0].visible = true;
		_sceneObjects[0].collisionType = 0;
		_sceneObjects[0].value6 = 0;
		_sceneObjects[0].clipX1 = 0;
		_sceneObjects[0].clipY1 = 0;
		_sceneObjects[0].clipX2 = 319;
		_sceneObjects[0].clipY2 = 199;

		// TODO: _palFlag = false;

		loadAndRunScript();
		
		handleSceneChange(_prevSceneNumber, _prevChapterNumber);
		
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
			_itemStatus[sceneItem->itemIndex] = 1;
			_inventoryItemIndex = sceneItem->itemIndex;
			sceneItem->active = false;
			setTextEx(sceneItem->itemIndex, _textBuffer3->getString(sceneItem->itemIndex));
		} else {
			setTextEx(4, _textBuffer3->getString(4));
		}
	}
	
}

void CometEngine::lookAtItemInSight(bool flag) {

	_itemInSight = false;
	
	if (_blockedInput != 15) {
		Common::Rect sightRect;
		calcSightRect(sightRect, 0, 50);
		//_screen->fillRect(rect.left, rect.top, rect.right, rect.bottom, 150);
		int sceneItemIndex = findSceneItemAt(sightRect);
		if (sceneItemIndex != 0) {
			SceneItem *sceneItem = &_sceneItems[sceneItemIndex & 0xFF];
			_itemInSight = true;
			_itemDirection = _sceneObjects[0].direction;
			_itemX = sceneItem->x;
			_itemY = sceneItem->y - 6;
			if (flag && (!_dialog->isRunning() || !_textActive)) {
				if (sceneItem->paramType == 0) {
					setTextEx(sceneItem->itemIndex, _textBuffer3->getString(sceneItem->itemIndex));
				} else {
					warning("sceneItem->paramType != 0; sceneItem->itemIndex = %d", sceneItem->itemIndex);
					// TODO: Remove this:
					// setTextEx(sceneItem->itemIndex, getTextEntry(sceneItem->itemIndex, textBuffer));
					// NOTE: Looks like this is never used in Comet CD, the resp. opcode is unused there.
				}
			}
		}
	}
	
	//TODO?
}

void CometEngine::sceneObjectsUpdateAnimations() {
	for (int i = 0; i < 10; i++) {
		if (_sceneObjects[i].life != 0)
			sceneObjectUpdateAnimation(&_sceneObjects[i]);
	}
	sceneObjectUpdatePortraitAnimation(&_sceneObjects[10]);
}

void CometEngine::sceneObjectsUpdateMovement() {
	for (int i = 0; i < 11; i++) {
		if (_sceneObjects[i].life != 0) {
			Common::Rect obstacleRect;
			bool flag = sceneObjectUpdatePosition(i, obstacleRect);
			if (_sceneObjects[i].walkStatus & 3)
				sceneObjectUpdateWalking(&_sceneObjects[i], i, flag, obstacleRect);
		}
	}
}

void CometEngine::updateStaticObjects() {

	_spriteArray.clear();

	if (!_sceneObjectsSprite)
		return;

	for (uint i = 0; i < _sceneObjectsSprite->_elements[0]->commands.size(); i++) {
		AnimationCommand *cmd = _sceneObjectsSprite->_elements[0]->commands[i];
		SpriteDraw temp;
		temp.y = cmd->points[0].y;
		temp.index = 16;
		_spriteArray.push_back(temp);
	}

}

void CometEngine::sceneObjectsEnqueueForDrawing() {
	for (int i = 0; i < 11; i++) {
		if (_sceneObjects[i].visible && _sceneObjects[i].life > 0) {
			sceneObjectEnqueueForDrawing(_sceneObjects[i].y, i);
		}
	}
}

void CometEngine::updateHeroLife() {
	if (_sceneObjects[0].life > 0 && _sceneObjects[0].life < 99 && (_gameLoopCounter & 0x1FF) == 0) {
		_sceneObjects[0].life++;
	}
}

void CometEngine::drawSprites() {
	//TODO: Real stuff

	//TODO: setScreenRectAll();

	//debug(1, "_spriteArray.size() = %d", _spriteArray.size());
	
	int objectCmdIndex = 0;

	for (uint32 i = 0; i < _spriteArray.size(); i++) {
		if (_spriteArray[i].index < 16) {
			drawActor(_spriteArray[i].index);
		} else {
			AnimationCommand *cmd = _sceneObjectsSprite->_elements[0]->commands[objectCmdIndex];
			_screen->drawAnimationCommand(_sceneObjectsSprite, cmd, 0, 0);
			objectCmdIndex++;
		}
	}

	if (_itemInSight && _sceneObjects[0].direction != 1)
		drawLineOfSight();
	
}

void CometEngine::drawActor(int objectIndex) {

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	int x = sceneObject->x, y = sceneObject->y;
	//int deltaX = sceneObject->deltaX, deltaY = sceneObject->deltaY;

	Animation *animation = _animationMan->getAnimation(sceneObject->animationSlot);
	
	/* NOTE: Yet another workaround for a crash (see sceneObjectUpdateAnimation). */
	if (sceneObject->animIndex >= (int)animation->_anims.size()) {
		error("here: animIndex = %d; _anims = %d", sceneObject->animIndex, animation->_anims.size());
		return;
	}

	AnimationFrameList *frameList = animation->_anims[sceneObject->animIndex];

	_screen->setClipRect(sceneObject->clipX1, sceneObject->clipY1, sceneObject->clipX2 + 1, sceneObject->clipY2 + 1);

	if (sceneObject->directionChanged == 2) {
		sceneObject->value4 = drawSceneObject(animation, frameList, sceneObject->animFrameIndex, sceneObject->value4,
			x, y, sceneObject->animFrameCount);
	} else {
		if (objectIndex == 0 && _itemInSight && sceneObject->direction == 1)
			drawLineOfSight();
		_screen->drawAnimationElement(animation, frameList->frames[sceneObject->animFrameIndex]->elementIndex, x, y);
	}

	_screen->setClipRect(0, 0, 320, 200);

	// DEBUG: Show object number
	/*
	char temp[16];
	snprintf(temp, 16, "%d", objectIndex);
	x = CLIP(x, 16, 320 - 16);
	y = CLIP(y, 16, 200 - 16);
	_screen->drawText(x, y, temp);
	*/

}

int CometEngine::drawSceneObject(Animation *animation, AnimationFrameList *frameList, int animFrameIndex, int value4, int x, int y, int animFrameCount) {

	AnimationFrame *frame = frameList->frames[animFrameIndex];

	int drawX = x, drawY = y;
	int index = frame->elementIndex;
	int mulVal = frame->flags & 0x3FFF;
	int gfxMode = frame->flags >> 14;

	for (int i = 0; i <= animFrameIndex; i++) {
		drawX += frameList->frames[i]->xOffs;
		drawY += frameList->frames[i]->yOffs;
	}

	debug(8, "x = %d; y = %d; drawX = %d; drawY = %d; gfxMode = %d; mulVal = %d",
		x, y, drawX, drawY, gfxMode, mulVal);

	if (gfxMode == 0) {
		_screen->drawAnimationElement(animation, index, drawX, drawY);
#if 0
	} else if (gfxMode == 1) {
		//TEST ONLY
		//anim->runSeq1(index, lx, ly);
		//anim->runSeq1(index, 0, 100);
#endif
 	} else {
		//debug(4, "CometEngine::drawSceneObject()  gfxMode == %d not yet implemented", gfxMode);
		/*
		// Dump Anim
		FILE *o = fopen("gfxmode2.va2", "wb");
		fwrite(anim->_animData, 2000, 1, o);
		fclose(o);
		exit(0);
		*/
	}

	return 0;
}

void CometEngine::updateTextDialog() {
	
	if (_textActive || _flag03)
		updateText();
		
	if (_dialog->isRunning())
		_dialog->update();
	
}

void CometEngine::updateText() {

	//TODO

	SceneObject *sceneObject = getSceneObject(_talkActorIndex);
	int x, y;

	if (sceneObject->textX != -1) {
		x = sceneObject->textX;
		y = sceneObject->textY;
	} else {
		x = sceneObject->x;
		y = sceneObject->y - _textMaxTextHeight - 50;
	}

	drawBubble(x - _textMaxTextWidth - 4, y - 4, x + _textMaxTextWidth + 4, y + _textMaxTextHeight);

	_screen->drawText3(x + 1, y, _currentText, _textColor, 0);
	
	_textDuration--;
	
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
			if (!_mixer->isSoundHandleActive(_voiceHandle)) {
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
			if (!_mixer->isSoundHandleActive(_voiceHandle)) {
				resetTextValues();
			}
		}
	}

}

void CometEngine::updateTalkAnims() {
	//TODO
}

void CometEngine::sceneObjectUpdatePortraitAnimation(SceneObject *sceneObject) {

	if (sceneObject->animSubIndex2 == -1) {

		// FIXME: This check is not in the original, find out why it's needed here...
		if (sceneObject->animationSlot == -1)
			return;

		AnimationFrame *frame = _animationMan->getAnimation(sceneObject->animationSlot)->_anims[sceneObject->animIndex]->frames[sceneObject->animFrameIndex];

		uint16 value = frame->flags & 0x3FFF;
		uint16 gfxMode = frame->flags >> 14;

		if (gfxMode == 1) {
			if (value < 1)
				value = 1;
			if (sceneObject->value4 >= value - 1) {
				sceneObject->value4 = 0;
				sceneObject->animFrameIndex++;
			}
		} else {
			sceneObject->animFrameIndex++;
		}

		if (sceneObject->animFrameIndex >= sceneObject->animFrameCount) {
			sceneObject->animFrameIndex = 0;
			if (sceneObject->animIndex < 4) {
				if (_portraitTalkCounter == 0) {
					if (_talkieMode == 0) {
						_portraitTalkAnimNumber = random(4);
						if (_portraitTalkAnimNumber == 0)
							_portraitTalkCounter = 1;
					} else {
						_portraitTalkAnimNumber = random(3);
						if (!_narOkFlag)
					  		_portraitTalkAnimNumber = 0;
					}
				} else {
					_portraitTalkCounter++;
					if (((_talkieMode == 1 || _talkieMode == 2) && _portraitTalkCounter == 1) || _portraitTalkCounter == 10)
						_portraitTalkCounter = 0;
				}
				sceneObjectSetAnimNumber(sceneObject, _portraitTalkAnimNumber);
			}
		}

	} else {
		sceneObject->value4 = 0;
	}

}

void CometEngine::sceneObjectUpdateAnimation(SceneObject *sceneObject) {

	if (sceneObject->directionChanged == 1) {
		sceneObject->directionChanged = 0;
		sceneObjectSetAnimNumber(sceneObject, sceneObject->direction + sceneObject->directionAdd - 1);
	} else {

		if (sceneObject->animSubIndex2 == -1) {

			/* NOTE: See note below, but here we bail out. */
			if (sceneObject->animIndex >= (int)_animationMan->getAnimation(sceneObject->animationSlot)->_anims.size())
				return;

			/* NOTE: After watching the ritual the players' frame number is out-of-bounds.
				I don't know yet why this happens, but setting it to 0 at least avoids a crash. */
			if (sceneObject->animFrameIndex >= (int)_animationMan->getAnimation(sceneObject->animationSlot)->_anims[sceneObject->animIndex]->frames.size())
				sceneObject->animFrameIndex = 0;

			AnimationFrame *frame = _animationMan->getAnimation(sceneObject->animationSlot)->_anims[sceneObject->animIndex]->frames[sceneObject->animFrameIndex];

			uint16 value = frame->flags & 0x3FFF;
			uint16 gfxMode = frame->flags >> 14;

			if (gfxMode == 1) {
				if (value < 1)
					value = 1;
				if (sceneObject->value4 >= value - 1) {
					sceneObject->value4 = 0;
					sceneObject->animFrameIndex++;
				}
			} else {
				sceneObject->animFrameIndex++;
			}
			
			if (sceneObject->animFrameIndex >= sceneObject->animFrameCount)
				sceneObject->animFrameIndex = 0;

		} else {
			sceneObject->value4 = 0;
		}
		
	}
}

void CometEngine::resetVars() {

	//TODO: scDisableRectFlag();
	_paletteBrightness = 255;
	//TODO: g_sp_byte_1 = 0;
	_cmdGet = false;
	_cmdLook = false;
	_cmdTalk = false;
 	//_sceneExits.clear();
	_blockedInput = 0;
	_sceneItems.clear();

}

void CometEngine::sceneObjectMoveAroundObstacle(int objectIndex, SceneObject *sceneObject, Common::Rect &obstacleRect) {

	int x = sceneObject->x;
	int y = sceneObject->y;

	debug(4, "CometEngine::sceneObjectMoveAroundObstacle() 1) objectIndex = %d; x = %d; y = %d", objectIndex, x, y);

	switch (sceneObject->direction) {
	case 1:
	case 3:
		if (random(2) == 0) {
			x = obstacleRect.left - (sceneObject->deltaX + 2);
		} else {
			x = obstacleRect.right + (sceneObject->deltaX + 2);
		}
		break;
	case 2:
	case 4:
		if (random(2) == 0) {
			y = obstacleRect.top - 2;
		} else {
			y = obstacleRect.bottom + (sceneObject->deltaY + 2);
		}
		break;
	}

	debug(4, "CometEngine::sceneObjectMoveAroundObstacle() 2) objectIndex = %d; x = %d; y = %d", objectIndex, x, y);

	sceneObjectStartWalking(objectIndex, x, y);

}

void CometEngine::sceneObjectHandleCollision(int objectIndex, SceneObject *sceneObject, Common::Rect &obstacleRect) {

	debug(4, "CometEngine::sceneObjectHandleCollision() objectIndex = %d", objectIndex);
	debug(4, "CometEngine::sceneObjectHandleCollision() sceneObject->collisionType = %d", sceneObject->collisionType);

	if (sceneObject->collisionType == 1 || sceneObject->collisionType == 2) {
		// TODO
		sceneObjectMoveAroundBounds(objectIndex, sceneObject);
	} else if (sceneObject->collisionType == 6 && sceneObject->value6 == 6 && sceneObject->collisionIndex == 0) {
		// TODO
		//debug(4, "CometEngine::sceneObjectHandleCollision()");
		sceneObject->value6 = 0;
		sceneObjectStopWalking(sceneObject);
		if (sceneObject->flag2 == 1) {
			sceneObjectUpdateLife(sceneObject, sceneObject->life);
		}
	} else {
		sceneObjectMoveAroundObstacle(objectIndex, sceneObject, obstacleRect);
	}

}

void CometEngine::sceneObjectUpdateWalking(SceneObject *sceneObject, int objectIndex, bool flag, Common::Rect &obstacleRect) {

	if (!flag)
		sceneObjectHandleCollision(objectIndex, sceneObject, obstacleRect);

	int comp = comparePointXY(sceneObject->x, sceneObject->y, sceneObject->walkDestX, sceneObject->walkDestY);
	
	if (_debugRectangles) {
		_screen->fillRect(sceneObject->walkDestX - 6, sceneObject->walkDestY - 6, sceneObject->walkDestX + 6, sceneObject->walkDestY + 6, 220);
		drawDottedLine(sceneObject->x, sceneObject->y, sceneObject->walkDestX, sceneObject->walkDestY, 100);
	}

	if (comp == 3 || ((sceneObject->walkStatus & 8) && (comp == 1)) || ((sceneObject->walkStatus & 0x10) && (comp == 2))) {
		if (sceneObject->walkStatus & 4) {
			sceneObjectStartWalking(objectIndex, sceneObject->savedWalkDestX, sceneObject->savedWalkDestY);
			sceneObject->walkStatus &= ~4;
		} else {
			sceneObjectStopWalking(sceneObject);
		}
	} else if ((sceneObject->walkStatus & 3) == comp) {
		sceneObject->walkStatus ^= 3;
		sceneObjectCalcDirection(sceneObject);
	}

}

bool CometEngine::sceneObjectUpdatePosition(int objectIndex, Common::Rect &obstacleRect) {

	//debug(4, "CometEngine::sceneObjectUpdatePosition(%d)", objectIndex);

	SceneObject *sceneObject = getSceneObject(objectIndex);

	if (sceneObject->directionAdd != 4)
		return false;

	int x = sceneObject->x;
	int y = sceneObject->y;

	debug(4, "CometEngine::sceneObjectUpdatePosition(%d)  old: %d, %d", objectIndex, x, y);

	Animation *anim = _animationMan->getAnimation(sceneObject->animationSlot);
	AnimationFrame *frame = anim->_anims[sceneObject->animIndex]->frames[sceneObject->animFrameIndex];

 	int16 xAdd = frame->xOffs;
 	int16 yAdd = frame->yOffs;

 	debug(4, "animFrameIndex = %d; animFrameCount = %d", sceneObject->animFrameIndex, sceneObject->animFrameCount);
 	
 	// TODO: SceneObject_sub_8243(sceneObject->direction, &xAdd, &yAdd); (but has no effect in Comet CD)

 	debug(4, "xAdd = %d; yAdd = %d", xAdd, yAdd);

 	x += xAdd;
 	y += yAdd;
 	
 	if (sceneObject->walkStatus & 3) {
		sceneObjectGetNextWalkDestXY(sceneObject, x, y);
	}

	if (sceneObject->collisionType != 8) {
		uint16 collisionType = checkCollision(objectIndex, x, y, sceneObject->deltaX, sceneObject->deltaY, sceneObject->direction, obstacleRect);
		debug(4, "collisionType (checkCollision) = %04X", collisionType);
		if (collisionType != 0) {
			collisionType = updateCollision(sceneObject, objectIndex, collisionType);
			debug(4, "collisionType (updateCollision) = %04X", collisionType);
			if (collisionType == 0)
				return false;
		} else {
			sceneObject->collisionType = 0;
		}
	}

	debug(4, "CometEngine::sceneObjectUpdatePosition(%d)  new: %d, %d", objectIndex, x, y);

	sceneObject->x = x;
	sceneObject->y = y;

	return true;

}

void CometEngine::sceneObjectEnqueueForDrawing(int y, int objectIndex) {

	uint32 index = 0;
	for (index = 0; index < _spriteArray.size(); index++) {
		if (_spriteArray[index].y > y)
			break;
	}

	SpriteDraw temp;
	temp.y = y;
	temp.index = objectIndex;
	_spriteArray.insert_at(index, temp);

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
		sceneObjectsResetFlags();
		_script->initializeScript();
	} else {
		_script->initializeScriptAfterLoadGame();
	}

}

void CometEngine::freeMarcheAndStaticObjects() {
	_animationMan->purgeAnimationSlots();
	if (_sceneObjectsSprite) {
		delete _sceneObjectsSprite;
		_sceneObjectsSprite = NULL;
	}
}

void CometEngine::resetMarcheAndStaticObjects() {
	resetTextValues();
	freeMarcheAndStaticObjects();
}

void CometEngine::updateScreen() {

	//TODO: seg011:0003 - seg011:004C
	
	if (_clearScreenRequest) {
		_screen->clearScreen();
		_clearScreenRequest = false;
	}
	
	if (_currentChapterNumber == 9 && _currentSceneNumber == 0 && _introPaletteState == 0) {
		memcpy(_paletteBuffer, _ctuPal, 768);
		memcpy(_ctuPal, _pali0Pal, 768);
		memcpy(_palette, _pali0Pal, 768);
		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_introPaletteState = 3;
	} else if (_currentChapterNumber == 9 && _currentSceneNumber == 1 && _introPaletteState == 3) {
		memcpy(_ctuPal, _cdintroPal, 768);
		memcpy(_palette, _cdintroPal, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_introPaletteState = 2;
	} else if (_currentChapterNumber == 5 && _currentSceneNumber == 0 && (_introPaletteState == 2 || _introPaletteState == 3)) {
		memcpy(_ctuPal, _paletteBuffer, 768);
		memcpy(_palette, _paletteBuffer, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_introPaletteState = 0;
	} else if (_currentChapterNumber == 0 && _currentSceneNumber == 0 && _introPaletteState != 0) {
		memcpy(_ctuPal, _paletteBuffer, 768);
		memcpy(_palette, _paletteBuffer, 768);
		_screen->setFullPalette(_ctuPal);
		_introPaletteState = 0;
	}

	_screen->update();

}

void CometEngine::unblockInput() {
	_blockedInput = 0;
	if (_sceneObjects[0].directionChanged == 2)
		_sceneObjects[0].directionChanged = 0;
}

void CometEngine::actorTalk(int objectIndex, int talkTextIndex, int color) {

	_talkActorIndex = objectIndex;
	_talkTextIndex = talkTextIndex;
	
	if (_talkieMode == 0 || _talkieMode == 1) {
		setText(_textReader->getString(_narFileIndex + 3, _talkTextIndex));
	}

	if (_talkieMode == 2 || _talkieMode == 1) {
		playVoice(_talkTextIndex);
	}

	_textActive = true;
	_textColor = color;

}

void CometEngine::actorTalkWithAnim(int objectIndex, int talkTextIndex, int animNumber) {

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	actorTalk(objectIndex, talkTextIndex, sceneObject->textColor);

	if (animNumber != 0xFF) {
		_animIndex = sceneObject->animIndex;
		_animSubIndex2 = sceneObject->animSubIndex2;
		_animSubIndex = sceneObject->animFrameIndex;
		sceneObjectSetAnimNumber(sceneObject, animNumber);
		sceneObject->directionChanged = 2;
	} else {
		_animIndex = -1;
	}

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
	_flag03 = false;
	_textActive = false;
}

bool CometEngine::rectCompare(const Common::Rect &rect1, const Common::Rect &rect2) {

	return ( rect1.left <= rect2.right && rect1.top <= rect2.bottom && rect1.right >= rect2.left && rect1.bottom >= rect2.top );

}

bool CometEngine::isActorNearActor(int objectIndex1, int objectIndex2, int x, int y) {

	SceneObject *sceneObject1, *sceneObject2;

	sceneObject1 = getSceneObject(objectIndex1);
	sceneObject2 = getSceneObject(objectIndex2);

	Common::Rect actorRect1(
		sceneObject1->x - sceneObject1->deltaX,
		sceneObject1->y - sceneObject1->deltaY,
		sceneObject1->x + sceneObject1->deltaX,
		sceneObject1->y);

	Common::Rect actorRect2(
		sceneObject2->x - x / 2,
		sceneObject2->y - y / 2,
		sceneObject2->x + x / 2,
		sceneObject2->y + y / 2);

	return rectCompare(actorRect1, actorRect2);

}

bool CometEngine::isPlayerInZone(int x, int y, int x2, int y2) {

	Common::Rect zoneRect(x, y, x2, y2);
	Common::Rect playerRect(
		_sceneObjects[0].x - _sceneObjects[0].deltaX,
		_sceneObjects[0].y - _sceneObjects[0].deltaY,
		_sceneObjects[0].x + _sceneObjects[0].deltaX,
		_sceneObjects[0].y);
	
	return rectCompare(zoneRect, playerRect);

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
				_mouseLeft = true;
				break;

			case Common::EVENT_LBUTTONUP:
				_mouseLeft = false;
				break;

			case Common::EVENT_RBUTTONDOWN:
				break;

			case Common::EVENT_RBUTTONUP:
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
	while (_keyScancode != Common::KEYCODE_INVALID || _keyDirection != 0) {
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

	SceneObject *sceneObject = getSceneObject(0);
	
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

	if (sceneObject->walkStatus & 3)
		return;
		
	if (_dialog->isRunning() && sceneObject->directionAdd != 0) {
		sceneObjectStopWalking(sceneObject);
		return;
	}

	int directionAdd = sceneObject->directionAdd;

	sceneObject->walkDestX = sceneObject->x;
	sceneObject->walkDestY = sceneObject->y;
	
	if (directionAdd == 4)
		directionAdd = 0;
		
	if (sceneObject->direction == _mouseCursor2 && !(_blockedInput & _mouseButtons4))
		directionAdd = 4;
		
	int direction = mouseDirectionTable[sceneObject->direction * 5 + _mouseCursor2];
	
	sceneObjectSetDirection(sceneObject, direction);
	sceneObjectSetDirectionAdd(sceneObject, directionAdd);
	
}

void CometEngine::skipText() {
	_textDuration = 1;
	_textActive = false;
	//waitForKeys();
}

void CometEngine::handleKeyInput() {

	//TODO

}

void CometEngine::openVoiceFile(int index) {

	if (_narFile) {
		_narFile->close();
		delete _narFile;
	}
	
	if (_narOffsets)
		delete[] _narOffsets;
		
	char narFilename[16];
	snprintf(narFilename, 16, "D%02d.NAR", index);
	
	_narFile = new Common::File();

	if (!_narFile->open(narFilename))
		error("CometEngine::openVoiceFile()  Could not open %s", narFilename);

	// TODO: Better don't read the offsets at all, only in playVoice
	_narCount = _narFile->readUint32LE() / 4;
	while (!_narFile->eos() && _narCount == 0) {
		_narCount = _narFile->readUint32LE() / 4;
	}
	
	_narOffsets = new uint32[_narCount + 1];
 	_narFile->seek(0);

	for (int i = 0; i < _narCount; i++)
		_narOffsets[i] = _narFile->readUint32LE();
		
	_narOffsets[_narCount] = _narFile->size();

}
	
void CometEngine::playVoice(int number) {

	stopVoice();

	//debug("playVoice() number = %d; _narCount = %d", number, _narCount);

	if (!_narOffsets || number >= _narCount)
		return;

	//debug(4, "CometEngine::playVoice(): number = %d; count = %d", number, _narCount);
		
	if (number + 1 >= _narCount) {
		debug(4, "CometEngine::playVoice(%d)  Voice number error from debugging rooms", number);
		return;
	}

	if (_narOffsets[number] == 0)
		return;

	//debug(4, "CometEngine::playVoice(): offset = %08X", _narOffsets[number]);

	_narFile->seek(_narOffsets[number]);

	int size, rate;
	
	if (_narOffsets[number + 1] <= _narOffsets[number]) {
		debug(4, "CometEngine::playVoice(%d)  Offset error", number);
		return;
	}
	
	size = _narOffsets[number + 1] - _narOffsets[number];

	//debug(4, "CometEngine::playVoice() size = %d", size);


	/* The VOC header's first byte is '\0' instead of a 'C' so we have to work around it */
	byte *readBuffer = (byte *)malloc(size);
	_narFile->read(readBuffer, size);
	readBuffer[0] = 'C';

	Common::MemoryReadStream vocStream(readBuffer, size, Common::DisposeAfterUse::YES);

	byte *buffer = Audio::loadVOCFromStream(vocStream, size, rate);
	_mixer->playRaw(Audio::Mixer::kSpeechSoundType, &_voiceHandle, buffer, size, rate, Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED);

}

void CometEngine::stopVoice() {
	if (_mixer->isSoundHandleActive(_voiceHandle))
		_mixer->stopHandle(_voiceHandle);
}

int CometEngine::handleLeftRightSceneExitCollision(int chapterNumber, int sceneNumber) {
	
	if (sceneNumber == -1) {
		_chapterNumber = -1;
		return 0;
	}
	
	_sceneNumber = sceneNumber;
	_chapterNumber = chapterNumber;
	
	SceneObject *sceneObject = getSceneObject(0);
	
	if (sceneObject->direction != 1 && sceneObject->direction != 3) {
	
		int x1, y1, x2, y2;

		sceneObject->value6 = 4;

		_scene->getSceneExitRect(sceneObject->collisionIndex, x1, y1, x2, y2);
		if (x2 == 318)
			x2 = 319;

		// Disable collision checks
		sceneObject->collisionType = 8;

		if (sceneObject->direction == 2) {
			sceneObject->clipX1 = 0;
			sceneObject->clipX2 = x2;
			sceneObjectStartWalking(0, 319, sceneObject->y);
		} else if (sceneObject->direction == 4) {
			sceneObject->clipX1 = x1;
			sceneObject->clipX2 = 319;
			sceneObjectStartWalking(0, 0, sceneObject->y);
		}
		
		sceneObject->walkStatus &= ~4;
		
	}
	
	return 1;
	
}

uint16 CometEngine::findSceneItemAt(const Common::Rect &rect) {
	for (uint i = 0; i < _sceneItems.size(); i++) {
		if (_sceneItems[i].active) {
			Common::Rect itemRect(_sceneItems[i].x - 8, _sceneItems[i].y - 8, _sceneItems[i].x + 8, _sceneItems[i].y + 8);
			if (rectCompare(rect, itemRect)) {
				return 0x500 | i;
			}
		}
	}
	return 0;
}

void CometEngine::setTextEx(int index, byte *text) {

	_talkActorIndex = 0;
	_textColor = 21;
	_talkTextIndex = index;
	setText(text);
	_textActive = true;
	_flag03 = true;

}

void CometEngine::drawLineOfSight() {
	if (_itemInSight) {
		int x = _sceneObjects[0].x;
		int y = _sceneObjects[0].y - 35;
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
		drawDottedLine(x, y, _itemX + random(3) - 1, _itemY + random(3) - 1, 7);
	}
}

void CometEngine::invUseItem() {

	for (uint index = 0; index < 256; index++) {
		if (_itemStatus[index] == 2)
			_itemStatus[index] = 1;
	}
	
	if (_invActiveItem != -1) {
		if (_itemStatus[_invActiveItem] == 1)
			_itemStatus[_invActiveItem] = 2;
	}

}

void CometEngine::invCheckActiveItem() {

	/* If the currently selected item was disabled, scan for the preceeding item
		and set it as selected item. */
	if (_invActiveItem >= 0 && _itemStatus[_invActiveItem] == 0) {
		if (_invActiveItem >= 1) {
			for (_invActiveItem = _invActiveItem - 1; _invActiveItem >= 0 && _itemStatus[_invActiveItem] == 0;
				_invActiveItem--) {
			}
		} else {
			_invActiveItem = -1;
		}
	}

	/* Check if the player wants to read the notebook */
	if (_itemStatus[0] == 2) {
		handleReadBook();
		_itemStatus[0] = 1;
	}


}

int CometEngine::checkCollisionWithActors(int skipIndex, Common::Rect &rect, Common::Rect &obstacleRect) {

	for (int index = 0; index < 11; index++) {
		SceneObject *sceneObject = getSceneObject(index);
		if (index != skipIndex && sceneObject->life != 0 && sceneObject->collisionType != 8) {
			obstacleRect.left = sceneObject->x - sceneObject->deltaX;
			obstacleRect.top = sceneObject->y - sceneObject->deltaY;
			obstacleRect.right = sceneObject->x + sceneObject->deltaX;
			obstacleRect.bottom = sceneObject->y;
			if (rectCompare(rect, obstacleRect)) {
				return 0x600 | index;
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

#if 0
	if (collisionType != 0)
		debug(0, "index = %d; x = %d; y = %d; deltaX = %d; deltaY = %d, collisionType = %04X",
			index, x, y, deltaX, deltaY, collisionType);
#endif

	return collisionType;

}

void CometEngine::initStaticObjectRects() {

	_scene->_blockingRects.clear();

	for (uint i = 0; i < _sceneObjectsSprite->_elements[0]->commands.size(); i++) {
		AnimationCommand *cmd = _sceneObjectsSprite->_elements[0]->commands[i];
		AnimationElement *objectElement = _sceneObjectsSprite->_elements[((cmd->arg2 << 8) | cmd->arg1) & 0x7FFF];
		debug(8, "%03d: cmd = %d; arg1 = %d; arg2 = %d; x = %d; y = %d; width = %d; height = %d",
			i, cmd->cmd, cmd->arg1, cmd->arg2, cmd->points[0].x, cmd->points[0].y, objectElement->width, objectElement->height);
		if (objectElement->width / 2 > 0) {
			int16 blockX1, blockY1, blockX2, blockY2;
			if (cmd->cmd == 0)
				blockX1 = (cmd->points[0].x - objectElement->width) / 2;
			else
				blockX1 = cmd->points[0].x / 2;
			blockY1 = cmd->points[0].y - ( (((objectElement->height >> 4) & 3) + 1) << 2 ); // FIXME
			blockX2 = (cmd->points[0].x + objectElement->width) / 2;
			blockY2 = cmd->points[0].y;
			_scene->addBlockingRect(blockX1, blockY1, blockX2, blockY2);
		}
	}

}

uint16 CometEngine::updateCollision(SceneObject *sceneObject, int objectIndex, uint16 collisionType) {

	int result = 0;
	
	sceneObject->collisionType = (collisionType >> 8) & 0xFF;
	sceneObject->collisionIndex = collisionType & 0xFF;

	if (objectIndex == 0 && sceneObject->collisionType == 4) {
		int chapterNumber, sceneNumber;
		_scene->getSceneExitLink(sceneObject->collisionIndex, chapterNumber, sceneNumber);
		result = handleLeftRightSceneExitCollision(chapterNumber, sceneNumber);
	}
	
	if (result == 0) {
		sceneObjectSetDirectionAdd(sceneObject, 0);
		sceneObjectUpdateAnimation(sceneObject);
	}

	return result;

}

int CometEngine::handleInventory() {

	Common::Array<uint16> items;
	uint firstItem = 0, currentItem = 0, maxItemsOnScreen = 10, animFrameCounter = 0;
	int result = 0;

	/*
	// DEBUG
	for (uint16 i = 1; i < 20; i++)
		items.push_back(i);
	*/
		
	waitForKeys();
		
	// Build items array and set up variables
	for (int i = 0; i < 256; i++) {
		if (_itemStatus[i] >= 1) {
			items.push_back(i);
			if (i == _invActiveItem) {
				firstItem = items.size() < 5 ? 0 : items.size() - 5;
				currentItem = items.size() - 1;
			}
		}
	}

	while (!result) {
	
		// TODO: Check mouse rectangles
	
		drawInventory(items, firstItem, currentItem, animFrameCounter++);

		// TODO: Handle mouse rectangles
		
		switch (_keyScancode) {
		case Common::KEYCODE_DOWN:
		{
			if ((currentItem - firstItem + 1 < maxItemsOnScreen) && (currentItem + 1 < items.size())) {
				// TODO: Check mouse rectangle
				currentItem++;
			} else if (firstItem + maxItemsOnScreen < items.size()) {
				firstItem++;
				currentItem++;
			}
			break;
		}
		case Common::KEYCODE_UP:
		{
			if (currentItem > firstItem) {
				// TODO: Check mouse rectangle
				currentItem--;
			} else if (firstItem > 0) {
				firstItem--;
				currentItem--;
			}
			break;
		}
		case Common::KEYCODE_ESCAPE:
			result = 2;
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_u:
			for (uint i = 0; i < 255; i++) {
				if (_itemStatus[i] == 2)
					_itemStatus[i] = 1;
			}
			_invActiveItem = items[currentItem];
			// Return just selects, U actually uses the item
			if (_keyScancode == Common::KEYCODE_u) {
				//debug("Use item #%d", _invActiveItem);
				_itemStatus[_invActiveItem] = 2;
			}
			result = 1;
			break;
		default:
			break;
		}
		
  		waitForKeys();
		handleEvents();
		_system->delayMillis(20); // TODO: Adjust or use fps counter
	}
	
	result = 2 - result;

	// TODO...

	return result;
}

void CometEngine::drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter) {

	uint xadd = 74, yadd = 64, itemHeight = 12, maxItemsOnScreen = 10;

	_screen->drawAnimationElement(_iconeVa2, 16, 0, 0);

	if (firstItem > 0)
		_screen->drawAnimationElement(_iconeVa2, 52, 0, 0);

	if (firstItem + maxItemsOnScreen < items.size())
		_screen->drawAnimationElement(_iconeVa2, 53, 0, 0);

	for (uint i = 0; (i < maxItemsOnScreen) && (firstItem + i < items.size()); i++) {
		byte *itemName = _textBuffer3->getString(items[firstItem + i]);
		int x = xadd + 21, y = yadd + itemHeight * i;
		_screen->setFontColor(120);
		_screen->drawText(x, y, itemName);
		_screen->setFontColor(119);
		_screen->drawText(x + 1, y + 1, itemName);
		x = xadd;
		y = yadd +  + itemHeight * i - 3;
		// TODO: Implement and use drawIcon instead
		_screen->drawAnimationElement(_objectsVa2, _objectsVa2->_anims[items[firstItem + i]]->frames[0]->elementIndex, x, y);
	}
	
	if (items.size() > 0) {
		int x = xadd + 16, y = yadd + (currentItem - firstItem) * itemHeight - 1;
		_screen->frameRect(x, y, 253, y + itemHeight - 1, _invSelectionColor);
		_invSelectionColor++;
		if (_invSelectionColor >= 96)
			_invSelectionColor = 80;
	}

	_screen->update();

}

int CometEngine::handleReadBook() {

	int pageNumber, pageCount, talkPageNumber = -1, result = 0;

	// TODO: Use values from script
	pageNumber = 4;
	pageCount = 14;

	bookTurnPageTextEffect(false, pageNumber, pageCount);

	// Set speech file
	openVoiceFile(7);

	while (!result) {

		drawBookPage(pageNumber, pageCount, 64);

		do {
			// Play page speech
			if (talkPageNumber != pageNumber) {
				if (pageNumber > 0) {
					playVoice(pageNumber);
				} else {
					stopVoice();
				}
				talkPageNumber = pageNumber;
			}
			// TODO: Check mouse rectangles
			handleEvents();
			_system->delayMillis(20); // TODO: Adjust or use fps counter
		} while (_keyScancode == Common::KEYCODE_INVALID && _keyDirection == 0);
		
		// TODO: Handle mouse rectangles
		
		switch (_keyScancode) {
		case Common::KEYCODE_RETURN:
			result = 1;
			break;
		case Common::KEYCODE_ESCAPE:
			result = 2;
			break;
		case Common::KEYCODE_LEFT:
			if (pageNumber > 0) {
				bookTurnPageTextEffect(true, pageNumber, pageCount);
				bookTurnPage(false);
				pageNumber--;
				bookTurnPageTextEffect(false, pageNumber, pageCount);
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (pageNumber < pageCount) {
				bookTurnPageTextEffect(true, pageNumber, pageCount);
				bookTurnPage(true);
				pageNumber++;
				bookTurnPageTextEffect(false, pageNumber, pageCount);
			}
			break;
		default:
			break;
		}

  		waitForKeys();

	}

	waitForKeys();
	stopVoice();
 	_textActive = false;

	openVoiceFile(_narFileIndex);

	result = 2 - result;
	
	return result;

}

void CometEngine::drawBookPage(int pageTextIndex, int pageTextMaxIndex, byte fontColor) {

	int xadd = 58, yadd = 48, x = 0, lineNumber = 0;
	char pageNumberString[10];
	int pageNumberStringWidth;

	byte *pageText = _textReader->getString(2, pageTextIndex);
	
	_screen->drawAnimationElement(_iconeVa2, 30, 0, 0);
	if (pageTextIndex < pageTextMaxIndex)
		_screen->drawAnimationElement(_iconeVa2, 37, 0, 0);
		
	_screen->setFontColor(58);

	snprintf(pageNumberString, 10, "- %d -", pageTextIndex * 2 + 1);
	pageNumberStringWidth = _screen->_font->getTextWidth((byte*)pageNumberString);
	_screen->drawText(xadd + (106 - pageNumberStringWidth) / 2, 180, (byte*)pageNumberString);
	
 	snprintf(pageNumberString, 10, "- %d -", pageTextIndex * 2 + 2);
	pageNumberStringWidth = _screen->_font->getTextWidth((byte*)pageNumberString);
	_screen->drawText(xadd + 115 + (106 - pageNumberStringWidth) / 2, 180, (byte*)pageNumberString);
	
	_screen->setFontColor(fontColor);
	
	while (*pageText != 0 && *pageText != '*') {
		x = xadd + (106 - _screen->_font->getTextWidth(pageText)) / 2;
		if (x < 0)
			x = 0;
		_screen->drawText(x, yadd + lineNumber * 10, pageText);
		if (++lineNumber == 13) {
			xadd += 115;
			yadd -= 130;
		}
		while (*pageText != 0 && *pageText != '*')
			pageText++;
		pageText++;
	}

}

void CometEngine::bookTurnPage(bool turnDirection) {
	if (turnDirection) {
		for (uint i = 38; i < 49; i++) {
			_screen->drawAnimationElement(_iconeVa2, 30, 0, 0);
			_screen->drawAnimationElement(_iconeVa2, i, 0, 0);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	} else {
		for (uint i = 49; i > 38; i--) {
			_screen->drawAnimationElement(_iconeVa2, 30, 0, 0);
			_screen->drawAnimationElement(_iconeVa2, i, 0, 0);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	}
}

void CometEngine::bookTurnPageTextEffect(bool turnDirection, int pageTextIndex, int pageTextMaxIndex) {
	if (turnDirection) {
		for (byte fontColor = 64; fontColor < 72; fontColor++) {
			drawBookPage(pageTextIndex, pageTextMaxIndex, fontColor);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	} else {
		for (byte fontColor = 72; fontColor > 64; fontColor--) {
			drawBookPage(pageTextIndex, pageTextMaxIndex, fontColor);
			_screen->update();
			_system->delayMillis(40); // TODO
		}
	}
}

void CometEngine::handleSceneChange(int sceneNumber, int chapterNumber) {

	debug(4, "###### handleSceneChange(%d, %d)", sceneNumber, chapterNumber);

	const int directionArray[] = {0, 3, 4, 1, 2};

	int direction = 1;
	int x1 = 160, x2 = 160, y1 = 190, y2 = 190;
	SceneObject *sceneObject = getSceneObject(0);
	
	for (uint sceneExitIndex = 0; sceneExitIndex < _scene->_sceneExits.size(); sceneExitIndex++) {
		SceneExitItem *sceneExitItem = &_scene->_sceneExits[sceneExitIndex];
		if (sceneExitItem->sceneNumber == sceneNumber && sceneExitItem->chapterNumber == chapterNumber) {
			direction = directionArray[sceneExitItem->directionIndex];
			if (sceneObject->direction == direction) {
				_scene->getSceneExitRect(sceneExitIndex, x1, y1, x2, y2);
				break;
			}
		}
	}

	sceneObject->x = (x2 - x1) / 2 + x1;
	sceneObject->y = (y2 - y1) / 2 + y1;
	sceneObject->direction = direction;
	sceneObjectSetAnimNumber(sceneObject, direction - 1);
	
	// TODO: scene change effects
	
	loadSceneBackground();
	
	if (_screen->getFadeType() == kFadeNone) {
		_screen->buildPalette(_ctuPal, _palette, _paletteBrightness);
		_screen->setFullPalette(_palette);
	}

}

void CometEngine::sceneObjectMoveAroundBounds(int index, SceneObject *sceneObject) {

	int x = sceneObject->x;
	int y = sceneObject->walkDestY;

	debug(1, "sceneObjectMoveAroundBounds(%d); 1) x = %d; y = %d", index, x, y);
	
	switch (sceneObject->direction) {
	case 1:
	case 3:
		x = sceneObject->walkDestX;
		break;
	case 2:
		if (sceneObject->walkDestY <= sceneObject->y) {
			y = _scene->findBoundsRight(x + sceneObject->deltaX, y - sceneObject->deltaY) +
				sceneObject->deltaY + 2;
		}
		break;
	case 4:
		if (sceneObject->walkDestY <= sceneObject->y) {
			y = _scene->findBoundsLeft(x - sceneObject->deltaX, y - sceneObject->deltaY) +
				sceneObject->deltaY + 1;
		}
		break;
	}
	
	debug(1, "sceneObjectMoveAroundBounds(%d); 2) x = %d; y = %d", index, x, y);

	sceneObjectStartWalking(index, x, y);

}

} // End of namespace Comet
