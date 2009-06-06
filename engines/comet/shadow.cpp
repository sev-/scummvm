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

void CometEngine::calcRect01(Common::Rect &rect, int delta1, int delta2) {

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

int CometEngine::calcDirection(int x1, int y1, int x2, int y2) {

/*
	int deltaX = (x2 - x1) / 2;
	int deltaY = y2 - y1;
	
	if (ABS(deltaX) <= ABS(deltaY)) {
		if (deltaX <= 0)
			return kDirectionLeft;
		else
			return kDirectionRight;
	} else {
		if (deltaY <= 0)
			return kDirectionUp;
		else
			return kDirectionDown;
	}
*/

	// New code, I think the code above is wrong...

	int deltaX = x2 - x1;
	int deltaY = y2 - y1;

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
	for (uint32 i = 0; i < _sceneExits.size(); i++) {
		if (_sceneExits[i].directionIndex == 3) {
			_screen->fillRect(_sceneExits[i].x, 198, _sceneExits[i].x2, 199, 120);
			_screen->hLine(_sceneExits[i].x + 1, 199, _sceneExits[i].x2 - 2, 127);
		}
	}
}

/* Scene */

void CometEngine::initSceneBackground() {
	
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
	
	if (!_loadingGameFlag)
		initStaticObjectRects();

}

void CometEngine::initPoints(byte *data) {
	byte count = *data++;
	_pointsArray.clear();
	while (count--) {
		int x = (*data++) * 2;
		int y = *data++;
		_pointsArray.push_back(Common::Point(x, y));
	}
	initPointsArray2();
}

void CometEngine::initSceneExits(byte *data) {
	byte count = *data++;
	_sceneExits.clear();
	while (count--) {
		SceneExitItem sceneExitItem;
		sceneExitItem.directionIndex = *data++;
		sceneExitItem.chapterNumber = *data++;
		sceneExitItem.sceneNumber = *data++;
		sceneExitItem.x = (*data++) * 2;
		sceneExitItem.x2 = (*data++) * 2;
		_sceneExits.push_back(sceneExitItem);
	}
}

void CometEngine::addBlockingRect(int x, int y, int x2, int y2) {
	_blockingRects.push_back(Common::Rect(x * 2, y, x2 * 2, y2));
}

void CometEngine::loadSceneBackground() {
	loadPakToPtr(DName, _backgroundFileIndex, _sceneBackground);
}

void CometEngine::loadStaticObjects() {
	delete _sceneObjectsSprite;
	
	debug("CometEngine::loadStaticObjects() DName = %s; index = %d", DName, _backgroundFileIndex + 1);
	
	_sceneObjectsSprite = loadMarcheData(DName, _backgroundFileIndex + 1);
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

	_bubbleSprite = loadMarcheData("RES.PAK", 1);
	_heroSprite = loadMarcheData("RES.PAK", 2);
	_objectsVa2 = loadMarcheData("RES.PAK", 4);

	_ctuPal = loadFromPak("RES.PAK", 5);
	_flashbakPal = loadFromPak("RES.PAK", 6);
	_cdintroPal = loadFromPak("RES.PAK", 7);
	_pali0Pal = loadFromPak("RES.PAK", 8);

	_cursorVa2 = loadMarcheData("RES.PAK", 9);
	_iconeVa2 = loadMarcheData("RES.PAK", 3);
	
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
	_textFlag2 = false;
	_narOkFlag = false;
	loadTextData(_textBuffer2, 0, 1000);
	loadTextData(_textBuffer3, 1, 2200);
}

void CometEngine::initData() {

	_sceneBackground = new byte[72000];
	_scratchBuffer = _sceneBackground + 64000;
	_scriptData = new byte[3000];
	_textBuffer1 = new byte[1000];
	_textBuffer2 = new byte[1000];
	_textBuffer3 = new byte[2200];
	_palette = new byte[768];

	memcpy(_palette, _ctuPal, 768);
	
	memset(_scriptVars2, 0, sizeof(_scriptVars2));
	memset(_scriptVars3, 0, sizeof(_scriptVars3));

	_screen->setFontColor(19);
	resetHeroDirectionChanged();
	
	_currentChapterNumber = 0;
	_sceneNumber = 0;
	
	_pointsArray.clear();
	_pointsArray.push_back(Common::Point(0, 100));
	_pointsArray.push_back(Common::Point(319, 100));
	initPointsArray2();
	
	resetVars();
	
	sceneObjectsResetFlags();

	sceneObjectInit(0, loadMarche(1, 0));

	sceneObjectSetXY(0, 160, 190);
	_sceneObjects[0].flag = 99;

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

	if (_chapterNumber != _currentChapterNumber)
		updateChapterNumber();

	if (_sceneNumber != _currentSceneNumber)
		updateSceneNumber();

	memcpy(_screen->getScreen(), _sceneBackground, 64000);
	
	if (_cmdLook)
		updateSub03(true);

	if (_cmdGet)
		updateSub02();

	handleInput();
	runAllScripts();
	
	if (_needToLoadSavegameFlag)
		return;
		
	drawSceneExits();
	
	sceneObjectsUpdate01();
	sceneObjectsUpdate02();
	
	updateStaticObjects();

	sceneObjectsUpdate03();

	updateSub03(false);

	drawSceneAnims();

	if (_talkieMode == 0)
		updateTextDialog();

	if (_talkieMode == 1 && (_textFlag2 || _flag03))
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
	if (_scriptVars2[11] < 100 && _scriptVars2[10] == 1)
		drawTextIllsmouth();
	*/

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
		_screen->putPixel(x, _xBuffer[x], 0);
	/* end DEBUG rectangles */


	updateScreen();
	
	updateSceneObjectFlag();
	
	_needToLoadSavegameFlag = false;
	
	_cmdTalk = false;
	_cmdGet = false;
	_cmdLook = false;

}

void CometEngine::updateChapterNumber() {
	if (_chapterNumber != -1) {
		freeMarche();
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

		_sceneObjects[0].y = _sceneObjects[0].y2;

	} else {

		resetMarcheAndStaticObjects();
		_prevSceneNumber = _currentSceneNumber;
		_currentSceneNumber = _sceneNumber;
		_prevChapterNumber = _currentChapterNumber;
		_currentChapterNumber = _chapterNumber;
		
		sceneObjectResetDirectionAdd(&_sceneObjects[0]);
		
		_sceneObjects[0].visible = true;
		_sceneObjects[0].collisionType = 0;
		_sceneObjects[0].value6 = 0;
		_sceneObjects[0].x5 = 0;
		_sceneObjects[0].y5 = 0;
		_sceneObjects[0].x6 = 319;
		_sceneObjects[0].y6 = 199;

		// TODO: _palFlag = false;

		loadAndRunScript();
		
		handleSceneChange(_prevSceneNumber, _prevChapterNumber);
		
		//TODO: mouse_4(0, 0);
		
	}
	
}

void CometEngine::updateSub02() {
	//debug(4, "CometEngine::updateSub02()");
	
	Common::Rect rect1;
	calcRect01(rect1, 0, 50);
	
	int sceneItemIndex = findSceneItemAt(rect1);

	if (sceneItemIndex != 0) {
		SceneItem *sceneItem = &_sceneItems[sceneItemIndex & 0xFF];
		if (sceneItem->paramType == 0) {
			_scriptVars3[sceneItem->itemIndex] = 1;
			_inventoryItemIndex = sceneItem->itemIndex;
			sceneItem->active = false;
			setTextEx(sceneItem->itemIndex, _textBuffer3);
		} else {
			setTextEx(4, _textBuffer2);
		}
	}
	
}

void CometEngine::updateSub03(bool flag) {

	//debug(4, "TODO CometEngine::updateSub03(%d)", flag);
	
	_itemInSight = false;
	
	//debug(4, "_mouseFlag = %d", _mouseFlag);
	
	if (_mouseFlag != 15) {
		Common::Rect rect;
		calcRect01(rect, 0, 50);
		int sceneItemIndex = findSceneItemAt(rect);
		if (sceneItemIndex != 0) {
			SceneItem *sceneItem = &_sceneItems[sceneItemIndex & 0xFF];
			_itemInSight = true;
			_itemDirection = _sceneObjects[0].direction;
			_itemX = sceneItem->x;
			_itemY = sceneItem->y - 6;

			if (flag && (!_dialog->isRunning() || !_textFlag2)) {
				byte *textBuffer;
				if (sceneItem->paramType == 0)
					textBuffer = _textBuffer3;
				else
					textBuffer = _textBuffer1;
				setTextEx(sceneItem->itemIndex, textBuffer);
			}
		}
	}
	
	//TODO?
}

void CometEngine::sceneObjectsUpdate01() {
	for (int i = 0; i < 10; i++) {
		if (_sceneObjects[i].flag != 0)
			sceneObjectUpdate02(&_sceneObjects[i]);
	}
	sceneObjectUpdate01(&_sceneObjects[10]);
}

void CometEngine::sceneObjectsUpdate02() {
	for (int i = 0; i < 11; i++) {
		if (_sceneObjects[i].flag != 0) {
			bool flag = sceneObjectUpdate04(i);
			if (_sceneObjects[i].walkStatus & 3)
				sceneObjectUpdate03(&_sceneObjects[i], i, flag);
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

void CometEngine::sceneObjectsUpdate03() {
	for (int i = 0; i < 11; i++) {
		if (_sceneObjects[i].flag != 0 && _sceneObjects[i].visible) {
			sceneObjectEnqueueForDrawing(_sceneObjects[i].y, i);
		}
	}
}

void CometEngine::updateSceneObjectFlag() {
	if (_sceneObjects[0].flag > 0 && _sceneObjects[0].flag < 99 && (_gameLoopCounter & 0x1FF) == 0) {
		_sceneObjects[0].flag++;
	}
}

void CometEngine::drawSceneAnims() {
	//TODO: Real stuff

	//TODO: setScreenRectAll();

	debug(1, "_spriteArray.size() = %d", _spriteArray.size());
	
	int objectCmdIndex = 0;

	for (uint32 i = 0; i < _spriteArray.size(); i++) {
		if (_spriteArray[i].index < 16) {
			drawSceneAnimsSub(_spriteArray[i].index);
		} else {
			AnimationCommand *cmd = _sceneObjectsSprite->_elements[0]->commands[objectCmdIndex];
			_screen->drawAnimationCommand(_sceneObjectsSprite, cmd, 0, 0);
			objectCmdIndex++;
		}
	}

	if (_itemInSight && _sceneObjects[0].direction != 1)
		drawLineOfSight();
	
}

void CometEngine::drawSceneAnimsSub(int objectIndex) {

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	int x = sceneObject->x, y = sceneObject->y;
	int deltaX = sceneObject->deltaX, deltaY = sceneObject->deltaY;

	Animation *animation = _marcheItems[sceneObject->marcheIndex].anim;
	AnimationFrameList *frameList = animation->_anims[sceneObject->animIndex];

	//debug(4, "setScreenRect(%d, %d, %d, %d)", sceneObject->x5, sceneObject->y5, sceneObject->x6, sceneObject->y6);
	//TODO: setScreenRect(sceneObject->x5, sceneObject->y5, sceneObject->x6, sceneObject->y6);

	if (sceneObject->directionChanged == 2) {
		sceneObject->value4 = drawSceneObject(animation, frameList, sceneObject->animFrameIndex, sceneObject->value4,
			x, y, sceneObject->animFrameCount);
	} else {
		if (objectIndex == 0 && _itemInSight && sceneObject->direction == 1)
			drawLineOfSight();
		_screen->drawAnimationElement(animation, frameList->frames[sceneObject->animFrameIndex]->elementIndex, x, y);
	}

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

	debug(0, "x = %d; y = %d; drawX = %d; drawY = %d; gfxMode = %d; mulVal = %d",
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
	
	if (_textFlag2 || _flag03)
		updateText();
		
	if (_dialog->isRunning())
		_dialog->update();
	
}

void CometEngine::updateText() {

	//TODO

	SceneObject *sceneObject = getSceneObject(_sceneObjectIndex);
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
		_textFlag2 = _textFlag1;
		if (_textFlag1) {
			setText(_textNextPos);
		} else {
			resetTextValues();
		}
	}
	
	if (_talkieMode == 1 && _textDuration <= 0) {
		_textFlag2 = _textFlag1;
		if (_textFlag1) {
			setText(_textNextPos);
		} else {
			if (!_mixer->isSoundHandleActive(_voiceHandle)) {
				resetTextValues();
			} else {
				_textDuration = 2;
				_textFlag2 = true;
			}
		}
	}

	if (_talkieMode == 2 && _textDuration <= 0) {
		_textFlag2 = _textFlag1;
		if (_textFlag1) {
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

void CometEngine::sceneObjectUpdate01(SceneObject *sceneObject) {

	if (sceneObject->animSubIndex2 == -1) {

		//uint16 temp1, temp2;

		// FIXME: This check is not in the original, find out why it's needed here...
		if (sceneObject->marcheIndex == -1)
			return;

		AnimationFrame *frame = _marcheItems[sceneObject->marcheIndex].anim->_anims[sceneObject->animIndex]->frames[sceneObject->animFrameIndex];

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

void CometEngine::sceneObjectUpdate02(SceneObject *sceneObject) {

	if (sceneObject->directionChanged == 1) {
		sceneObject->directionChanged = 0;
		sceneObjectSetAnimNumber(sceneObject, sceneObject->direction + sceneObject->directionAdd - 1);
	} else {

		if (sceneObject->animSubIndex2 == -1) {

			AnimationFrame *frame = _marcheItems[sceneObject->marcheIndex].anim->_anims[sceneObject->animIndex]->frames[sceneObject->animFrameIndex];

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
	//TODO: _palValue = 255;
	//TODO: g_sp_byte_1 = 0;
	_cmdGet = false;
	_cmdLook = false;
	_cmdTalk = false;
 	_sceneExits.clear();
	_mouseFlag = 0;
	_sceneItems.clear();

}

void CometEngine::sceneObjectAvoidBlockingRect(int objectIndex, SceneObject *sceneObject) {

	int x = sceneObject->x;
	int y = sceneObject->y;

	debug(4, "CometEngine::sceneObjectAvoidBlockingRect() 1) objectIndex = %d; x = %d; y = %d", objectIndex, x, y);

	switch (sceneObject->direction) {
	case 1:
	case 3:
		if (random(2) == 0) {
			x = _blockingRect.left - (sceneObject->deltaX + 2);
		} else {
			x = _blockingRect.right + (sceneObject->deltaX + 2);
		}
		break;
	case 2:
	case 4:
		if (random(2) == 0) {
			y = _blockingRect.top - 2;
		} else {
			y = _blockingRect.bottom + (sceneObject->deltaY + 2);
		}
		break;
	}

	debug(4, "CometEngine::sceneObjectAvoidBlockingRect() 2) objectIndex = %d; x = %d; y = %d", objectIndex, x, y);

	sceneObjectUpdateDirection2(objectIndex, x, y);

}

void CometEngine::sceneObjectUpdateDirectionTo(int objectIndex, SceneObject *sceneObject) {

	debug(4, "CometEngine::sceneObjectUpdateDirectionTo() objectIndex = %d", objectIndex);
	debug(4, "CometEngine::sceneObjectUpdateDirectionTo() sceneObject->collisionType = %d", sceneObject->collisionType);

	if (sceneObject->collisionType == 1 || sceneObject->collisionType == 2) {
		// TODO
		//debug(4, "CometEngine::sceneObjectUpdateDirectionTo()");
		sceneObjectDirection2(objectIndex, sceneObject);
	} else if (sceneObject->collisionType == 6 && sceneObject->value6 == 6 && sceneObject->linesIndex == 0) {
		// TODO
		//debug(4, "CometEngine::sceneObjectUpdateDirectionTo()");
		 _system->delayMillis(5000);
		sceneObject->value6 = 0;
		sceneObjectResetDirectionAdd(sceneObject);
		if (sceneObject->flag2 == 1) {
			sceneObjectUpdateFlag(sceneObject, sceneObject->flag);
		}
	} else {
		sceneObjectAvoidBlockingRect(objectIndex, sceneObject);
	}

}

void CometEngine::sceneObjectUpdate03(SceneObject *sceneObject, int objectIndex, bool flag) {

	//debug(4, "CometEngine::sceneObjectUpdate03()");

	if (!flag)
		sceneObjectUpdateDirectionTo(objectIndex, sceneObject);

	int comp = comparePointXY(sceneObject->x, sceneObject->y, sceneObject->x2, sceneObject->y2);
	
	debug(4, "WALK FROM (%d, %d) TO (%d, %d); comp = %d; walkStatus = %02X; walkStatus & 3 = %d", sceneObject->x, sceneObject->y, sceneObject->x2, sceneObject->y2, comp, sceneObject->walkStatus, sceneObject->walkStatus & 3);
	_screen->fillRect(sceneObject->x2 - 6, sceneObject->y2 - 6, sceneObject->x2 + 6, sceneObject->y2 + 6, 220);
	drawDottedLine(sceneObject->x, sceneObject->y, sceneObject->x2, sceneObject->y2, 100);
	
	if (comp == 3 || ((sceneObject->walkStatus & 8) && (comp == 1)) || ((sceneObject->walkStatus & 0x10) && (comp == 2))) {

		debug(4, "--1");
	
		if (sceneObject->walkStatus & 4) {
			sceneObjectUpdateDirection2(objectIndex, sceneObject->x3, sceneObject->y3);
			sceneObject->walkStatus &= ~4;
		} else {
			sceneObjectResetDirectionAdd(sceneObject);
		}
		
	} else if ((sceneObject->walkStatus & 3) == comp) {
		//debug(4, "--2");

		debug(4, "Old walkStatus = %02X", sceneObject->walkStatus);

		sceneObject->walkStatus ^= 3;
		
		debug(4, "New walkStatus = %02X", sceneObject->walkStatus);
		
		sceneObjectCalcDirection(sceneObject);
	}

}

bool CometEngine::sceneObjectUpdate04(int objectIndex) {

	//debug(4, "CometEngine::sceneObjectUpdate04(%d)", objectIndex);

	SceneObject *sceneObject = getSceneObject(objectIndex);

	if (sceneObject->directionAdd != 4)
		return false;

	int x = sceneObject->x;
	int y = sceneObject->y;

	debug(4, "CometEngine::sceneObjectUpdate04(%d)  old: %d, %d", objectIndex, x, y);

	//debug(4, "CometEngine::sceneObjectUpdate04(%d)  old: %d, %d", objectIndex, x, y);

	Animation *anim = _marcheItems[sceneObject->marcheIndex].anim;
	AnimationFrame *frame = anim->_anims[sceneObject->animIndex]->frames[sceneObject->animFrameIndex];

 	int16 xAdd = frame->xOffs;
 	int16 yAdd = frame->yOffs;

 	debug(4, "animFrameIndex = %d; animFrameCount = %d", sceneObject->animFrameIndex, sceneObject->animFrameCount);
 	
 	//TODO: SceneObject_sub_8243(sceneObject->direction, &xAdd, &yAdd);

 	debug(4, "xAdd = %d; yAdd = %d", xAdd, yAdd);

 	x += xAdd;
 	y += yAdd;
 	
 	if (sceneObject->walkStatus & 3) {
		debug(4, "WALKING_1 (%d, %d); %d: (%d, %d)", x, y, sceneObject->direction, sceneObject->x2, sceneObject->y2);
		sceneObjectGetXY1(sceneObject, x, y);
		debug(4, "WALKING_2 (%d, %d)", x, y);
		//debug(4, "CometEngine::sceneObjectUpdate04(%d)  target: %d, %d", objectIndex, x, y);
	}

	if (sceneObject->collisionType != 8) {
		uint16 collisionType = checkCollision(objectIndex, x, y, sceneObject->deltaX, sceneObject->deltaY, sceneObject->direction);
		debug(4, "collisionType (checkCollision) = %04X", collisionType);
		//debug(4, "collisionType = %04X", collisionType);
		if (collisionType != 0) {
			collisionType = handleCollision(sceneObject, objectIndex, collisionType);
			debug(4, "collisionType (handleCollision) = %04X", collisionType);
			if (collisionType == 0)
				return false;
		} else {
			sceneObject->collisionType = 0;
		}
	}

	debug(4, "CometEngine::sceneObjectUpdate04(%d)  new: %d, %d", objectIndex, x, y);

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

void CometEngine::loadAndRunScript() {

	Common::File fd;
	uint32 ofs;

	fd.open(RName);
	fd.seek(_currentSceneNumber * 4);
	ofs = fd.readUint32LE();
	fd.seek(ofs);
	fd.read(_scriptData, 3000);
	fd.close();

	if (!_loadingGameFlag) {
		resetVars();
		sceneObjectsResetFlags();
		initializeScript();
	} else {
		//TODO: ScriptInterpreter_initializeAfterLoadGame
	}

}

void CometEngine::freeMarcheAndStaticObjects() {
	freeMarche();
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
	
	if (_currentChapterNumber == 9 && _currentSceneNumber == 0 && _paletteValue2 == 0) {
		memcpy(_paletteBuffer, _ctuPal, 768);
		memcpy(_ctuPal, _pali0Pal, 768);
		memcpy(_palette, _pali0Pal, 768);
		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteValue2 = 3;
	} else if (_currentChapterNumber == 9 && _currentSceneNumber == 1 && _paletteValue2 == 3) {
		memcpy(_ctuPal, _cdintroPal, 768);
		memcpy(_palette, _cdintroPal, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteValue2 = 2;
	} else if (_currentChapterNumber == 5 && _currentSceneNumber == 0 && (_paletteValue2 == 2 || _paletteValue2 == 3)) {
		memcpy(_ctuPal, _paletteBuffer, 768);
		memcpy(_palette, _paletteBuffer, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteValue2 = 0;
	} else if (_currentChapterNumber == 0 && _currentSceneNumber == 0 && _paletteValue2 != 0) {
		memcpy(_ctuPal, _paletteBuffer, 768);
		memcpy(_palette, _paletteBuffer, 768);
		_screen->setFullPalette(_ctuPal);
		_paletteValue2 = 0;
	}

	_screen->update();

}

void CometEngine::resetHeroDirectionChanged() {
	_mouseFlag = false;
	if (_sceneObjects[0].directionChanged == 2)
		_sceneObjects[0].directionChanged = 0;
}

void CometEngine::textProc(int objectIndex, int narSubIndex, int color) {

	_sceneObjectIndex = objectIndex;
	_narSubIndex = narSubIndex;
	
	WRITE_LE_UINT32(_textBuffer1, 4);
	
	loadString(_narFileIndex + 3, _narSubIndex, _textBuffer1 + 4);
	
	if (_talkieMode == 0 || _talkieMode == 1) {
		setText(getTextEntry(0, _textBuffer1));
	}

	if (_talkieMode == 2 || _talkieMode == 1) {
		playVoice(_narSubIndex);
	}

	_textFlag2 = true;
	_textColor = color;

}

void CometEngine::textProc2(int objectIndex, int narSubIndex, int animNumber) {

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	textProc(objectIndex, narSubIndex, sceneObject->color);

	if (animNumber != 0xFF) {
		_animIndex = sceneObject->animIndex;
		_animSubIndex2 = sceneObject->animSubIndex2;
		_animSubIndex = sceneObject->animFrameIndex;
		sceneObjectSetAnimNumber(sceneObject, animNumber);
		sceneObject->directionChanged = 2;
	} else {
		_animIndex = -1;
	}

	_curScript->status |= kScriptTalking;
	_scriptBreakFlag = true;

}

int CometEngine::random(int maxValue) {
	if (maxValue >= 2)
		return _rnd.getRandomNumber(maxValue - 1);
	else
		return 0;
}

void CometEngine::drawBubble(int x1, int y1, int x2, int y2) {

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
	
	int xPos, yPos;

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

void CometEngine::decodeText(byte *text, int size, int key) {
	uint8 tempByte = 0x54;
	for (int curOfs = 0; curOfs < size; curOfs++)
		text[curOfs] = text[curOfs] - tempByte * (key + curOfs + 1);
}

uint32 CometEngine::loadString(int index, int subIndex, byte *text) {
	Common::File fd;
	uint32 blockOffset, subBlockStartOffset, subBlockSize;
	uint32 subBlockOffset, subBlockNextOffset;
	//TODO: Use the language-specific CC4-file
	fd.open("E.CC4");
	fd.seek(index * 4);
	blockOffset = fd.readUint32LE();
	fd.seek(blockOffset);
	subBlockStartOffset = fd.readUint32LE();
	fd.seek(blockOffset + subIndex * 4);
	subBlockOffset = fd.readUint32LE();
	subBlockNextOffset = fd.readUint32LE();
	fd.seek(blockOffset + subBlockOffset);
	subBlockSize = subBlockNextOffset - subBlockOffset + 1;
	fd.read(text, subBlockSize);
	fd.close();
	decodeText((byte*)text, subBlockSize, subBlockOffset - subBlockStartOffset);
	return subBlockSize;
}

void CometEngine::loadTextData(byte *textBuffer, int index, int size) {
	Common::File fd;
	uint32 ofs, chunkSize;
	//TODO: Use the language-specific CC4-file
	fd.open("E.CC4");
	fd.seek(index * 4);
	ofs = fd.readUint32LE();
	fd.seek(ofs);
	fd.read(textBuffer, size);
	fd.close();
	chunkSize = READ_LE_UINT32(textBuffer);
	decodeText(textBuffer + chunkSize, size - chunkSize, 0);
}

byte *CometEngine::getTextEntry(int index, byte *textBuffer) {
	return (byte*)(textBuffer + READ_LE_UINT32(textBuffer + index * 4) + 1);
}

void CometEngine::setText(byte *text) {
	
	int lineCount = 0;

	_currentText = text;

	_textMaxTextHeight = 0;
	_textMaxTextWidth = 0;
	_textFlag1 = false;
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
		lineCount++;
		if (lineCount == 3 && *text != '*') {
			_textFlag1 = true;
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
	_textFlag2 = false;
}

void CometEngine::initPointsArray2() {

	int x1, y1, x2, y2, errorX, errorY = 0;
	byte *xb = _xBuffer;

	for (uint32 i = 0; i < _pointsArray.size() - 1; i++) {
		x1 = _pointsArray[i].x;
		y1 = _pointsArray[i].y;
		x2 = _pointsArray[i + 1].x;
		y2 = _pointsArray[i + 1].y;
		x2 -= x1;
		if (x2 != 0) {
			y2 -= y1;
			errorY = y1;
			y1 = 1;
			if (y2 < 0) {
				y2 = -y2;
				y1 = -y1;
			}
			if (x2 > y2) {
				errorX = x2 >> 1;
				for (int j = 0; j < x2; j++) {
					*xb++ = errorY;
					errorX += y2;
					if (errorX >= x2) {
						errorX -= x2;
						errorY += y1;
					}
				}
			} else {
				errorX = y2 >> 1;
				for (int j = 0; j < y2; j++) {
					errorY += y1;
					errorX += x2;
					if (errorX >= y2) {
						errorX -= y2;
						*xb++ = errorY;
					}
				}
			}
		}
	}

	*xb++ = errorY;

}

int CometEngine::Points_getY_sub_8419(int x, int y) {
	int yp = 0;
	for (uint32 i = 0; i < _pointsArray.size(); i++) {
		yp = _pointsArray[i].y;
		if (_pointsArray[i].x > x && yp >= y)
			break;
	}
	if (yp >= 199)
		return 190;
	else
		return yp;
}

int CometEngine::Points_getY_sub_8477(int x, int y) {
	int yp = 0;
	for (int i = _pointsArray.size() - 1; i >= 0; i--) {
		yp = _pointsArray[i].y;
		if (_pointsArray[i].x < x && yp >= y)
			break;
	}
	if (yp >= 199)
		return 190;
	else
		return yp;
}

int CometEngine::checkCollisionWithSceneExits(const Common::Rect &rect, int direction) {

	int x, y, x2, x3, y3, x4, y4;
	
	x = rect.left;
	y = rect.top;
	x2 = rect.right;

	for (uint32 index = 0; index < _sceneExits.size(); index++) {
		bool flag = false;
		if (_sceneExits[index].directionIndex == direction) {
			getSceneExitRect(index, x3, y3, x4, y4);
			if (direction == 1 || direction == 3) {
				flag = (x >= x3) && (x2 <= x4);
			} else if (direction == 2) {
				flag = (y >= y3) && (y <= y4) && (x2 >= x3);
			} else if (direction == 4) {
				flag = (y >= y3) && (y <= y4) && (x <= x4);
			}
			if (flag)
				return 0x400 | index;
		}
	}
	
	return 0;
}

void CometEngine::rect_sub_CC94(int &x, int &y, int deltaX, int deltaY) {

	debug(1, "rect_sub_CC94() 1) x = %d; y = %d; deltaX = %d; deltaY = %d", x, y, deltaX, deltaY);

	if (x - deltaX < 0)
		x = deltaX + 1;
	if (x + deltaX > 319)
		x = 319 - deltaX - 1;

	Common::Rect tempRect(x - deltaX, y - deltaY, x + deltaX, y);
	
	if (checkCollisionWithSceneExits(tempRect, 1) ||
		checkCollisionWithSceneExits(tempRect, 2) ||
		checkCollisionWithSceneExits(tempRect, 3) ||
		checkCollisionWithSceneExits(tempRect, 4))
		return;
		
	debug(1, "rect_sub_CC94() 1b) x1 = %d; x2 = %d", _xBuffer[x - deltaX], _xBuffer[x + deltaX]);

	if (y - deltaY <= _xBuffer[x - deltaX])
		y = _xBuffer[x - deltaX] + deltaY + 2;
		
	if (y - deltaY <= _xBuffer[x + deltaX])
		y = _xBuffer[x + deltaX] + deltaY + 2;

	if (y > 199)
		y = 199;

	debug(1, "rect_sub_CC94() 2) x = %d; y = %d; deltaX = %d; deltaY = %d", x, y, deltaX, deltaY);

}

bool CometEngine::rectCompare(const Common::Rect &rect1, const Common::Rect &rect2) {

	return ( rect1.left <= rect2.right && rect1.top <= rect2.bottom && rect1.right >= rect2.left && rect1.bottom >= rect2.top );

}

bool CometEngine::rectCompare02(int objectIndex1, int objectIndex2, int x, int y) {

	SceneObject *sceneObject1, *sceneObject2;

	sceneObject1 = getSceneObject(objectIndex1);
	sceneObject2 = getSceneObject(objectIndex2);

	Common::Rect rect1(
		sceneObject1->x - sceneObject1->deltaX,
		sceneObject1->y - sceneObject1->deltaY,
		sceneObject1->x + sceneObject1->deltaX,
		sceneObject1->y);

	Common::Rect rect2(
		sceneObject2->x - x / 2,
		sceneObject2->y - y / 2,
		sceneObject2->x + x / 2,
		sceneObject2->y + y / 2);
	
	return rectCompare(rect1, rect2);

}

bool CometEngine::isPlayerInRect(int x, int y, int x2, int y2) {

	Common::Rect rect1(x, y, x2, y2);
	Common::Rect rect2(
		_sceneObjects[0].x - _sceneObjects[0].deltaX,
		_sceneObjects[0].y - _sceneObjects[0].deltaY,
		_sceneObjects[0].x + _sceneObjects[0].deltaX,
		_sceneObjects[0].y);
	
	return rectCompare(rect1, rect2);

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
				break;

			case Common::EVENT_LBUTTONUP:
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
	while (_keyScancode != Common::KEYCODE_INVALID && _keyDirection != 0) {
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
	
	if ((_mouseFlag & _mouseButtons4) || _dialog->isRunning()) {
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
		sceneObjectResetDirectionAdd(sceneObject);
		return;
	}

	int directionAdd = sceneObject->directionAdd;

	sceneObject->x2 = sceneObject->x;
	sceneObject->y2 = sceneObject->y;
	
	if (directionAdd == 4)
		directionAdd = 0;
		
	if (sceneObject->direction == _mouseCursor2 && !(_mouseFlag & _mouseButtons4))
		directionAdd = 4;
		
	int direction = mouseDirectionTable[sceneObject->direction * 5 + _mouseCursor2];
	
	sceneObjectSetDirection(sceneObject, direction);
	sceneObjectSetDirectionAdd(sceneObject, directionAdd);
	
}

void CometEngine::skipText() {

	_textDuration = 1;
	_textFlag2 = false;
	
	while (_keyScancode != Common::KEYCODE_INVALID && _keyDirection2 != 0)
		handleEvents();

}

void CometEngine::handleKeyInput() {

	//TODO

}

void CometEngine::getSceneExitRect(int index, int &x, int &y, int &x2, int &y2) {

	SceneExitItem *sceneExitItem = &_sceneExits[index];
	
	x = sceneExitItem->x;
	x2 = sceneExitItem->x2;
	
	y = _xBuffer[x];
	
	if (x == x2) {
		y2 = 199;
	} else if (sceneExitItem->directionIndex == 3) {
		y = 199;
		y2 = 199;
	} else {
		y2 = _xBuffer[x2];
	}
	
	if (y > y2)
		SWAP(y, y2);

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
	_narFile->open(narFilename);
	
	if (!_narFile->isOpen()) {
		debug(4, "CometEngine::openVoiceFile()  Could not open %s", narFilename);
		return;
	}
	
	_narCount = _narFile->readUint32LE() / 4;
	
	_narOffsets = new uint32[_narCount + 1];
 	_narFile->seek(0);

	for (int i = 0; i < _narCount; i++)
		_narOffsets[i] = _narFile->readUint32LE();
		
	_narOffsets[_narCount] = _narFile->size();

}
	
void CometEngine::playVoice(int number) {

	if (_mixer->isSoundHandleActive(_voiceHandle))
		_mixer->stopHandle(_voiceHandle);

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

	Common::MemoryReadStream vocStream(readBuffer, size, true);

	byte *buffer = Audio::loadVOCFromStream(vocStream, size, rate);
	_mixer->playRaw(Audio::Mixer::kSpeechSoundType, &_voiceHandle, buffer, size, rate, Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED);

}

int CometEngine::checkLinesSub(int chapterNumber, int sceneNumber) {
	
	if (sceneNumber == -1) {
		_chapterNumber = -1;
		return 0;
	}
	
	_sceneNumber = sceneNumber;
	_chapterNumber = chapterNumber;
	
	SceneObject *sceneObject = getSceneObject(0);
	
	if (sceneObject->direction != 1 && sceneObject->direction != 3) {
		int x, y, x2, y2;

		sceneObject->value6 = 4;

		getSceneExitRect(sceneObject->linesIndex, x, y, x2, y2);
		if (x2 == 318)
			x2 = 319;
			
		sceneObject->collisionType = 8;

		if (sceneObject->direction == 2) {
			sceneObject->x5 = 0;
			sceneObject->x6 = x2;
			sceneObjectUpdateDirection2(0, 319, sceneObject->y);
		} else if (sceneObject->direction == 4) {
			sceneObject->x5 = x;
			sceneObject->x6 = 319;
			sceneObjectUpdateDirection2(0, 0, sceneObject->y);
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

void CometEngine::setTextEx(int index, byte *textBuffer) {

	_sceneObjectIndex = 0;
	_textColor = 21;
	_narSubIndex = index;
	setText(getTextEntry(index, textBuffer));
	_textFlag2 = true;
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

	for (int index = 0; index < 256; index++) {
		if (_scriptVars3[index] == 2)
			_scriptVars3[index] = 1;
	}
	
	if (_invActiveItem != -1) {
		if (_scriptVars3[_invActiveItem] == 1)
			_scriptVars3[_invActiveItem] = 2;
	}

}

int CometEngine::checkCollisionWithRoomBounds(const Common::Rect &rect, int direction) {

	int x, y, x2, y2, y3, y4, result;
	
	result = 0;

	x = CLIP<int>(rect.left, 0, 319);
	x2 = CLIP<int>(rect.right, 0, 319);

	y = rect.top;
	y2 = rect.bottom;

	y3 = _xBuffer[x];
	y4 = _xBuffer[x2];
	
	switch (direction) {
	case 1:
		if (y <= y3 || y <= y4)
			result = 0x100;
		break;
	case 2:
		if (y4 >= y || x2 == 319)
			result = 0x100;
		break;
	case 4:
		if (y3 >= y || x == 0)
			result = 0x100;
		break;
	default:
		// Nothing
		break;
	}
	
	if (y2 > 199)
		result = 0x200;

	return result;

}

int CometEngine::checkCollisionWithBlockingRects(Common::Rect &rect) {

	for (uint32 index = 0; index < _blockingRects.size(); index++) {
		_blockingRect = _blockingRects[index];
		if (_blockingRects[index].left != _blockingRects[index].right) {
			if (rectCompare(_blockingRect, rect)) {
				return 0x300 | index;
			}
		}
	}
	
	return 0;

}

int CometEngine::checkCollisionWithActors(int skipIndex, Common::Rect &rect) {

	for (int index = 0; index < 11; index++) {
		SceneObject *sceneObject = getSceneObject(index);
		if (index != skipIndex && sceneObject->flag != 0 && sceneObject->collisionType != 8) {
			_blockingRect.left = sceneObject->x - sceneObject->deltaX;
			_blockingRect.top = sceneObject->y - sceneObject->deltaY;
			_blockingRect.right = sceneObject->x + sceneObject->deltaX;
			_blockingRect.bottom = sceneObject->y;
			if (rectCompare(rect, _blockingRect)) {
				return 0x600 | index;
			}
		}
	}
	
	return 0;

}

uint16 CometEngine::checkCollision(int index, int x, int y, int deltaX, int deltaY, int direction) {

	uint16 collisionType = 0;

	Common::Rect rect(x - deltaX, y - deltaY, x + deltaX, y);
	
	collisionType = checkCollisionWithRoomBounds(rect, direction);
	if (collisionType != 0) {
		uint16 sceneExitCollision = checkCollisionWithSceneExits(rect, direction);
		if (sceneExitCollision != 0)
			collisionType = sceneExitCollision;
	} else {
		collisionType = checkCollisionWithBlockingRects(rect);
		if (collisionType == 0)
			collisionType = checkCollisionWithActors(index, rect);
	}

	return collisionType;

}

void CometEngine::initStaticObjectRects() {

	_blockingRects.clear();

	for (uint i = 0; i < _sceneObjectsSprite->_elements[0]->commands.size(); i++) {
		AnimationCommand *cmd = _sceneObjectsSprite->_elements[0]->commands[i];
		AnimationElement *objectElement = _sceneObjectsSprite->_elements[((cmd->arg2 << 8) | cmd->arg1) & 0x7FFF];
		debug("%03d: cmd = %d; arg1 = %d; arg2 = %d; x = %d; y = %d; width = %d; height = %d",
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
			addBlockingRect(blockX1, blockY1, blockX2, blockY2);
		}
	}

}

uint16 CometEngine::handleCollision(SceneObject *sceneObject, int index, uint16 collisionType) {

	int result = 0;
	
	sceneObject->collisionType = (collisionType >> 8) & 0xFF;
	sceneObject->linesIndex = collisionType & 0xFF;

	if (index == 0 && sceneObject->collisionType == 4) {
		result = checkLinesSub(_sceneExits[sceneObject->linesIndex].chapterNumber, _sceneExits[sceneObject->linesIndex].sceneNumber);
	}
	
	if (result == 0) {
		sceneObjectSetDirectionAdd(sceneObject, 0);
		sceneObjectUpdate02(sceneObject);
	}

	return result;

}

void CometEngine::handleInventory() {

}

void CometEngine::handleSceneChange(int sceneNumber, int chapterNumber) {

	debug(4, "###### handleSceneChange(%d, %d)", sceneNumber, chapterNumber);

	const int directionArray[] = {0, 3, 4, 1, 2};

	int direction = 1;
	int x = 160, x2 = 160, y = 190, y2 = 190;
	SceneObject *sceneObject = getSceneObject(0);
	
	for (uint sceneExitIndex = 0; sceneExitIndex < _sceneExits.size(); sceneExitIndex++) {
		SceneExitItem *sceneExitItem = &_sceneExits[sceneExitIndex];
		if (sceneExitItem->sceneNumber == sceneNumber && sceneExitItem->chapterNumber == chapterNumber) {
			direction = directionArray[sceneExitItem->directionIndex];
			if (sceneObject->direction == direction) {
				getSceneExitRect(sceneExitIndex, x, y, x2, y2);
				break;
			}
		}
	}

	sceneObject->x = (x2 - x) / 2 + x;
	sceneObject->y = (y2 - y) / 2 + y;
	sceneObject->direction = direction;
	sceneObjectSetAnimNumber(sceneObject, direction - 1);
	
	// TODO: scene change effects
	
	loadSceneBackground();
	
	/* TODO
	if (_paletteMode == 0) {
		buildPalette(_ctuPal, _palette, _palValue);
		_screen->setFullPalette(_palette);
	}
	*/

}

void CometEngine::sceneObjectDirection2(int index, SceneObject *sceneObject) {

	int x = sceneObject->x;
	int y = sceneObject->y;

	debug(1, "sceneObjectDirection2(%d); 1) x = %d; y = %d", index, x, y);
	
	switch (sceneObject->direction) {
	case 1:
	case 3:
		x = sceneObject->x2;
		break;
	case 2:
		if (sceneObject->y2 <= sceneObject->y) {
			y = Points_getY_sub_8419(x + sceneObject->deltaX, y - sceneObject->deltaY) +
				sceneObject->deltaY + 2;
		}
		break;
	case 4:
		if (sceneObject->y2 <= sceneObject->y) {
			y = Points_getY_sub_8477(x - sceneObject->deltaX, y - sceneObject->deltaY) +
				sceneObject->deltaY + 1;
		}
		break;
	}
	
	debug(1, "sceneObjectDirection2(%d); 2) x = %d; y = %d", index, x, y);

	sceneObjectUpdateDirection2(index, x, y);

}

} // End of namespace Comet
