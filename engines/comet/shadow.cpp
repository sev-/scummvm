#include "sound/voc.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/font.h"
#include "comet/anim.h"
#include "comet/pak.h"

#include "comet/screen.h"
#include "comet/dialog.h"

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

}

void CometEngine::drawLines() {
	for (int i = 0; i < _linesArray.size(); i++) {
		if (_linesArray[i].directionIndex == 3) {
			_screen->fillRect(_linesArray[i].x, 198, _linesArray[i].x2, 199, 120);
			_screen->hLine(_linesArray[i].x + 1, 199, _linesArray[i].x2 - 2, 127);
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

void CometEngine::initLines(byte *data) {
	byte count = *data++;
	_linesArray.clear();
	while (count--) {
		LineItem lineItem;
		lineItem.directionIndex = *data++;
		lineItem.fileNumber = *data++;
		lineItem.scriptNumber = *data++;
		lineItem.x = (*data++) * 2;
		lineItem.x2 = (*data++) * 2;
		_linesArray.push_back(lineItem);
	}
}

void CometEngine::addBlockingRect(int x, int y, int x2, int y2) {
	_blockingRects.push_back(Common::Rect(x * 2, y, x2 * 2, y2));
}

void CometEngine::loadSceneBackground() {
	loadPakToPtr(DName, _backgroundFileIndex, _sceneBackground);
}

void CometEngine::loadStaticObjects() {
	if (!_staticObjects)
		_staticObjects = new Anim(this);
	_staticObjects->load(DName, _backgroundFileIndex + 1);
}

void CometEngine::drawSceneForeground() {
	byte *sec0 = _staticObjects->getSubSection(0, 0);
	if (sec0[1] != 0) {
		_staticObjects->runSeq1(0, 0, 0);
	}
}

/* Graphics */

void plotProc(int x, int y, int color, void *data) {
	if (x >= 0 && x < 320 && y >= 0 && y < 200)
		((byte*)data)[x + y * 320] = color;
}

void CometEngine::initAndLoadGlobalData() {

	//TODO: cbFileOpenPtr = NULL;

	_screen->loadFont("RES.PAK", 0);

	_bulleVa2 = new Anim(this);
	_bulleVa2->load("RES.PAK", 1);

	_marche0Va2 = new Anim(this);
	_marche0Va2->load("RES.PAK", 2);

	_objectsVa2 = new Anim(this);
	_objectsVa2->load("RES.PAK", 4);

	_ctuPal = loadFromPak("RES.PAK", 5);
	_flashbakPal = loadFromPak("RES.PAK", 6);
	_cdintroPal = loadFromPak("RES.PAK", 7);
	_pali0Pal = loadFromPak("RES.PAK", 8);

	_cursorVa2 = new Anim(this);
	_cursorVa2->load("RES.PAK", 9);

	_iconeVa2 = new Anim(this);
	_iconeVa2->load("RES.PAK", 3);
	
	_screen->setFontColor(0);

	//TODO: seg001:0758 Mouse cursor stuff...
	
	_paletteBuffer = new byte[768];
	memcpy(_paletteBuffer, _ctuPal, 768);
	
	initData();

	loadGlobalTextData();
	
	//TODO...

	setFileAndScriptNumber(_startupFileNumber, _startupScriptNumber);
	
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
	
	_currentFileNumber = 0;
	_scriptNumber = 0;
	
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

void CometEngine::setFileAndScriptNumber(int fileNumber, int scriptNumber) {

	_fileNumber = fileNumber;
	_scriptNumber = scriptNumber;
	
	//debug(4, "CometEngine::setFileAndScriptNumber(%d, %d)", fileNumber, scriptNumber);
	
	//FIXME
	sprintf(AName, "A%d%d.PAK", _fileNumber / 10, _fileNumber % 10);
	sprintf(DName, "D%d%d.PAK", _fileNumber / 10, _fileNumber % 10);
	sprintf(RName, "R%d%d.CC4", _fileNumber / 10, _fileNumber % 10);
	
	//debug(4, "AName = %s; DName = %s; RName = %s", AName, DName, RName);
	
}

void CometEngine::updateGame() {

	_gameLoopCounter++;
	_textColorFlag++;

	if (_fileNumber != _currentFileNumber)
		updateFileNumber();

	if (_scriptNumber != _currentScriptNumber)
		updateScriptNumber();

	memcpy(_screen->getScreen(), _sceneBackground, 64000);
	
	if (_cmdLook)
		updateSub03(true);

	if (_cmdGet)
		updateSub02();

	handleInput();
	runAllScripts();
	
	if (_needToLoadSavegameFlag)
		return;
		
	drawLines();
	
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
	for (int i = 0; i < _blockingRects.size(); i++)
		_screen->fillRect(_blockingRects[i].left, _blockingRects[i].top, _blockingRects[i].right, _blockingRects[i].bottom, 120);
	_screen->fillRect(_sceneObjects[0].x - _sceneObjects[0].deltaX, _sceneObjects[0].y - _sceneObjects[0].deltaY,
		_sceneObjects[0].x + _sceneObjects[0].deltaX, _sceneObjects[0].y, 150);
	for (int index = 0; index < _linesArray.size(); index++) {
		int x3, y3, x4, y4;
		getPortalRect(index, x3, y3, x4, y4);
		//debug(4, "PORTAL: (%d, %d, %d, %d); direction = %d; fileNumber = %d; scriptNumber = %d", x3, y3, x4, y4, _linesArray[index].directionIndex, _linesArray[index].fileNumber, _linesArray[index].scriptNumber);
		_screen->fillRect(x3, y3, x4, y4, 25);
	}
	/* end DEBUG rectangles */


	updateScreen();
	
	updateSceneObjectFlag();
	
	_needToLoadSavegameFlag = false;
	
	_cmdTalk = false;
	_cmdGet = false;
	_cmdLook = false;

}

void CometEngine::updateFileNumber() {
	if (_fileNumber != -1) {
		freeMarche();
		freeMarcheAndStaticObjects();
		setFileAndScriptNumber(_fileNumber, _scriptNumber);
		updateScriptNumber();
	}
}

void CometEngine::updateScriptNumber() {

	//TODO: mouse_4(0, 0x40);

	if ((_sceneObjects[0].walkStatus != 0 && _sceneObjects[0].direction == 2 && _sceneObjects[0].x < 319) ||
		(_sceneObjects[0].direction == 4 && _sceneObjects[0].x > 0)) {
		
		_sceneObjects[0].y = _sceneObjects[0].y2;
		
	} else {

		resetMarcheAndStaticObjects();
		_scriptNumber3 = _currentScriptNumber;
		_currentScriptNumber = _scriptNumber;
		_fileNumber3 = _currentFileNumber;
		_currentFileNumber = _fileNumber;
		
		sceneObjectResetDirectionAdd(&_sceneObjects[0]);
		
		_sceneObjects[0].visible = true;
		_sceneObjects[0].value5 = 0;
		_sceneObjects[0].value6 = 0;
		_sceneObjects[0].x5 = 0;
		_sceneObjects[0].y5 = 0;
		_sceneObjects[0].x6 = 319;
		_sceneObjects[0].y6 = 199;

		// TODO: _palFlag = false;

		loadAndRunScript();
		
		handleSceneChange(_scriptNumber3, _fileNumber3);
		
		//TODO: mouse_4(0, 0);
		
	}
	
}

void CometEngine::updateSub02() {
	//debug(4, "CometEngine::updateSub02()");
	
	Common::Rect rect1;
	calcRect01(rect1, 0, 50);
	
	int sceneItemIndex = rectInSceneItem(rect1);

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
		int sceneItemIndex = rectInSceneItem(rect);
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

	// FIXME
	if (!_staticObjects)
		return;

	byte *sec00 = _staticObjects->getSubSection(0, 0);
	int count = sec00[1];

	sec00 += 8;
	
	for (int i = 0; i < count; i++) {

		SpriteDraw temp;
		temp.y = sec00[0];
		temp.index = 16;
		_spriteArray.push_back(temp);
		
		sec00 += 8;

	}

}

void CometEngine::sceneObjectsUpdate03() {
	for (int i = 0; i < 11; i++) {
		if (_sceneObjects[i].flag != 0 && _sceneObjects[i].visible) {
			sceneObjectUpdate05(_sceneObjects[i].y, i);
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
	
	byte *va2sec00 = _staticObjects->getSubSection(0, 0) + 2;
	byte *va2sec1 = _staticObjects->getSection(1);

	for (int i = 0; i < _spriteArray.size(); i++) {

		if (_spriteArray[i].index < 16) {
			drawSceneAnimsSub(_spriteArray[i].index);
		} else {
			int frameCount = READ_LE_UINT16(va2sec00);
			int index = READ_LE_UINT16(va2sec00 + 2);
			int x = READ_LE_UINT16(va2sec00 + 4);
			int y = READ_LE_UINT16(va2sec00 + 6);
			/* TODO?: Seems to work fine as is
			if (frameCount == 1) {
			} else {
			}
			*/
			_staticObjects->runSeq1(index, x, y);
			va2sec00 += 8;
		}
		
	}

	if (_itemInSight && _sceneObjects[0].direction != 1)
		drawLineOfSight();
	
}

void CometEngine::drawSceneAnimsSub(int objectIndex) {

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	int x = sceneObject->x, y = sceneObject->y;
	int deltaX = sceneObject->deltaX, deltaY = sceneObject->deltaY;
	
	Anim *anim = _marcheItems[sceneObject->marcheIndex].anim;

	byte *sec2 = anim->getSubSection(2, sceneObject->animIndex) + 2;
	
	//debug(4, "setScreenRect(%d, %d, %d, %d)", sceneObject->x5, sceneObject->y5, sceneObject->x6, sceneObject->y6);
	//TODO: setScreenRect(sceneObject->x5, sceneObject->y5, sceneObject->x6, sceneObject->y6);

	if (sceneObject->directionChanged == 2) {
		sceneObject->value4 = drawSceneObject(anim, sec2, sceneObject->animFrameIndex, sceneObject->value4,
			x, y, sceneObject->animFrameCount);
	} else {
		if (objectIndex == 0 && _itemInSight && sceneObject->direction == 1)
			drawLineOfSight();
		int index = READ_LE_UINT16(sec2 + sceneObject->animFrameIndex * 8);
		anim->runSeq1(index, x, y);
	}


}

int CometEngine::drawSceneObject(Anim *anim, byte *sec2, int animFrameIndex, int value4, int x, int y, int animFrameCount) {

	int index = READ_LE_UINT16( sec2 + animFrameIndex * 8 );
	int mulVal = READ_LE_UINT16( sec2 + animFrameIndex * 8 + 2 );
	int gfxMode = mulVal >> 14;
	mulVal &= 0x3FFF;
	int lx = x, ly = y;

	byte *lsec2 = sec2;
	for (int i = 0; i <= animFrameIndex; i++) {
		lx += (int16)READ_LE_UINT16( lsec2 + 4 );
		ly += (int16)READ_LE_UINT16( lsec2 + 6 );
		lsec2 += 8;
	}

	//debug(4, "x = %d; y = %d; lx = %d; ly = %d; gfxMode = %d; mulVal = %d", x, y, lx, ly, gfxMode, mulVal);
	
	if (gfxMode == 0) {
		anim->runSeq1(index, lx, ly);
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

	drawBubble( x - _textMaxTextWidth - 4, y - 4, x + _textMaxTextWidth + 4, y + _textMaxTextHeight );

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

		uint16 temp1, temp2;

		// FIXME: This check is not in the original, find out why it's needed here...
		if (sceneObject->marcheIndex == -1)
			return;

		byte *sec2 = _marcheItems[sceneObject->marcheIndex].anim->getSubSection(2, sceneObject->animIndex) + 2;
		sec2 += (sceneObject->animFrameIndex * 8) + 2;

		temp1 = READ_LE_UINT16(sec2);
		temp2 = temp1 >> 14;
		temp1 &= 0x3FFF;

		if (temp2 == 1) {
			if (temp1 < 1)
				temp1 = 1;
			if (sceneObject->value4 >= temp1 - 1) {
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
			uint16 temp1, temp2;
			
			byte *sec2 = _marcheItems[sceneObject->marcheIndex].anim->getSubSection(2, sceneObject->animIndex) + 2;
			sec2 += (sceneObject->animFrameIndex * 8) + 2;

			temp1 = READ_LE_UINT16(sec2);
			temp2 = temp1 >> 14;
			temp1 &= 0x3FFF;
			
			if (temp2 == 1) {
				if (temp1 < 1)
					temp1 = 1;
				if (sceneObject->value4 >= temp1 - 1) {
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
 	_linesArray.clear();
	_mouseFlag = 0;
	_sceneItems.clear();

}

void CometEngine::sceneObjectDirection1(int objectIndex, SceneObject *sceneObject) {

	//debug(4, "sceneObjectDirection1 %d", objectIndex); fflush(stdout);

	int x = sceneObject->x;
	int y = sceneObject->y;
	
	switch (sceneObject->direction) {
	case 1:
	case 3:
		if (random(2) == 0) {
			x = _blockingTestRect.left - (sceneObject->deltaX + 2);
		} else {
			x = _blockingTestRect.right + (sceneObject->deltaX + 2);
		}
		break;
	case 2:
	case 4:
		if (random(2) == 0) {
			y = _blockingTestRect.top - 2;
		} else {
			y = _blockingTestRect.bottom + (sceneObject->deltaY + 2);
		}
		break;
	}

	sceneObjectUpdateDirection2(objectIndex, x, y);

}

void CometEngine::sceneObjectUpdateDirectionTo(int objectIndex, SceneObject *sceneObject) {

	debug(4, "CometEngine::sceneObjectUpdateDirectionTo()  objectIndex = %d", objectIndex); fflush(stdout);
	debug(4, "CometEngine::sceneObjectUpdateDirectionTo()  sceneObject->value5 = %d", sceneObject->value5); fflush(stdout);

	if (sceneObject->value5 == 1 || sceneObject->value5 == 2) {
		// TODO
		//debug(4, "CometEngine::sceneObjectUpdateDirectionTo()"); fflush(stdout);
		sceneObjectDirection2(objectIndex, sceneObject);
	} else if (sceneObject->value5 == 6 && sceneObject->value6 == 6 && sceneObject->linesIndex == 0) {
		// TODO
		//debug(4, "CometEngine::sceneObjectUpdateDirectionTo()"); fflush(stdout);
		fflush(stdout); _system->delayMillis(5000);
		sceneObject->value6 = 0;
		sceneObjectResetDirectionAdd(sceneObject);
		if (sceneObject->flag2 == 1) {
			sceneObjectUpdateFlag(sceneObject, sceneObject->flag);
		}
	} else {
		sceneObjectDirection1(objectIndex, sceneObject);
	}

}

void CometEngine::sceneObjectUpdate03(SceneObject *sceneObject, int objectIndex, bool flag) {

	//debug(4, "CometEngine::sceneObjectUpdate03()"); fflush(stdout);

	if (!flag)
		sceneObjectUpdateDirectionTo(objectIndex, sceneObject);
	
	int comp = comparePointXY(sceneObject->x, sceneObject->y, sceneObject->x2, sceneObject->y2);
	
	//debug(4, "## %d, %d -> %d, %d; %d; %02X", sceneObject->x, sceneObject->y, sceneObject->x2, sceneObject->y2, comp, sceneObject->walkStatus); fflush(stdout);
	_screen->fillRect(sceneObject->x2 - 6, sceneObject->y2 - 6, sceneObject->x2 + 6, sceneObject->y2 + 6, 220);
	drawDottedLine(sceneObject->x, sceneObject->y, sceneObject->x2, sceneObject->y2, 100);
	
	if (comp == 3 || ((sceneObject->walkStatus & 8) && (comp == 1)) || ((sceneObject->walkStatus & 0x10) && (comp == 2))) {
		debug(4, "--1"); fflush(stdout);
	
		if (sceneObject->walkStatus & 4) {
			sceneObjectUpdateDirection2(objectIndex, sceneObject->x3, sceneObject->y3);
			sceneObject->walkStatus &= 0xFFFB;
		} else {
			sceneObjectResetDirectionAdd(sceneObject);
		}
		
	} else if ((sceneObject->walkStatus & 3) == comp) {
		//debug(4, "--2"); fflush(stdout);
		sceneObject->walkStatus ^= 3;
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

	//debug(4, "CometEngine::sceneObjectUpdate04(%d)  old: %d, %d", objectIndex, x, y);

	Anim *anim = _marcheItems[sceneObject->marcheIndex].anim;

 	byte *sec2 = anim->getSubSection(2,	sceneObject->animIndex) + 2;
 	int16 xAdd = (int16)READ_LE_UINT16(sec2 + sceneObject->animFrameIndex * 8 + 4);
 	int16 yAdd = (int16)READ_LE_UINT16(sec2 + sceneObject->animFrameIndex * 8 + 6);

 	//TODO: SceneObject_sub_8243(sceneObject->direction, &xAdd, &yAdd);

 	x += xAdd;
 	y += yAdd;
 	
 	if (sceneObject->walkStatus & 3) {
		sceneObjectGetXY1(sceneObject, x, y);
		//debug(4, "CometEngine::sceneObjectUpdate04(%d)  target: %d, %d", objectIndex, x, y);
	}

	if (sceneObject->value5 != 8) {

		uint16 collisionType = checkCollision(objectIndex, x, y, sceneObject->deltaX, sceneObject->deltaY, sceneObject->direction);
		
		//debug(4, "collisionType = %04X", collisionType); fflush(stdout);
		
		if (collisionType != 0) {
			collisionType = handleCollision(sceneObject, objectIndex, collisionType);
			if (collisionType == 0)
				return false;
		} else {
			sceneObject->value5 = 0;
		}

	}

	//debug(4, "CometEngine::sceneObjectUpdate04(%d)  new: %d, %d", objectIndex, x, y);

	sceneObject->x = x;
	sceneObject->y = y;

	return true;

}

void CometEngine::sceneObjectUpdate05(int y, int objectIndex) {

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
	int ofs;

	fd.open(RName);
	fd.seek(_currentScriptNumber * 4);
	fd.read(&ofs, 4);
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
	if (_staticObjects) {
		delete _staticObjects;
		_staticObjects = NULL;
	}
}

void CometEngine::resetMarcheAndStaticObjects() {
	resetTextValues();
	freeMarcheAndStaticObjects();
}

void CometEngine::updateScreen() {

	//TODO: seg011:0003 - seg011:004C
	
	if (_currentFileNumber == 9 && _currentScriptNumber == 0 && _paletteValue2 == 0) {
		memcpy(_paletteBuffer, _ctuPal, 768);
		memcpy(_ctuPal, _pali0Pal, 768);
		memcpy(_palette, _pali0Pal, 768);
		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteValue2 = 3;
	} else if (_currentFileNumber == 9 && _currentScriptNumber == 1 && _paletteValue2 == 3) {
		memcpy(_ctuPal, _cdintroPal, 768);
		memcpy(_palette, _cdintroPal, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteValue2 = 2;
	} else if (_currentFileNumber == 5 && _currentScriptNumber == 0 && (_paletteValue2 == 2 || _paletteValue2 == 3)) {
		memcpy(_ctuPal, _paletteBuffer, 768);
		memcpy(_palette, _paletteBuffer, 768);
  		_screen->clearScreen();
		_screen->setFullPalette(_ctuPal);
		_paletteValue2 = 0;
	} else if (_currentFileNumber == 0 && _currentScriptNumber == 0 && _paletteValue2 != 0) {
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
			_bulleVa2->runSeq1(8, xPos, yPos);
		}
	}

	for (xPos = x1 + 8; x2 - 8 > xPos; xPos += 8) {
		_bulleVa2->runSeq1(3, xPos, y1 + 8);
		_bulleVa2->runSeq1(4, xPos, y2);
	}

	for (yPos = y1 + 16; yPos < y2; yPos += 8) {
		_bulleVa2->runSeq1(1, x1, yPos);
		_bulleVa2->runSeq1(6, x2 - 16, yPos);
	}
	
	_bulleVa2->runSeq1(0, x1, y1 + 8);
	_bulleVa2->runSeq1(5, x2 - 16, y1 + 8);
	_bulleVa2->runSeq1(2, x1, y2);
	_bulleVa2->runSeq1(7, x2 - 16, y2);

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
	fd.read(&blockOffset, 4);
	fd.seek(blockOffset);
	fd.read(&subBlockStartOffset, 4);
	fd.seek(blockOffset + subIndex * 4);
	fd.read(&subBlockOffset, 4);
	fd.read(&subBlockNextOffset, 4);
	fd.seek(blockOffset + subBlockOffset);
	subBlockSize = subBlockNextOffset - subBlockOffset + 1;
	fd.read(text, subBlockSize);
	fd.close();
	decodeText((byte*)text, subBlockSize, subBlockOffset - subBlockStartOffset);
	return subBlockSize;
}

void CometEngine::loadTextData(byte *textBuffer, int index, int size) {
	Common::File fd;
	int ofs, chunkSize;
	//TODO: Use the language-specific CC4-file
	fd.open("E.CC4");
	fd.seek(index * 4);
	fd.read(&ofs, 4);
	fd.seek(ofs);
	fd.read(textBuffer, size);
	fd.close();
	chunkSize = READ_LE_UINT32(textBuffer);
	decodeText(textBuffer + chunkSize, size - chunkSize, 0);
}

char *CometEngine::getTextEntry(int index, byte *textBuffer) {
	return (char*)(textBuffer + READ_LE_UINT32(textBuffer + index * 4) + 1);
}

void CometEngine::setText(char *text) {
	
	int counter = 0;

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
		text += strlen(text) + 1;
		if (textWidth != 0)
			_textMaxTextHeight++;
		counter++;
		if (counter == 3 && *text != '*') {
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

	int x1, y1, x2, y2, temp, tempY = 0;
	byte *xb = _xBuffer;

	for (int i = 0; i < _pointsArray.size() - 1; i++) {
		x1 = _pointsArray[i].x;
		y1 = _pointsArray[i].y;
		x2 = _pointsArray[i + 1].x;
		y2 = _pointsArray[i + 1].y;
		x2 -= x1;
		if (x2 != 0) {
			y2 -= y1;
			tempY = y1;
			y1 = 1;
			if (y2 < 0) {
				y2 = -y2;
				y1 = -y1;
			}
			if (x2 > y2) {
				temp = x2 >> 1;
				for (int j = 0; j < x2; j++) {
					*xb++ = tempY;
					temp += y2;
					if (temp >= x2) {
						temp -= x2;
						tempY += y1;
					}
				}
			} else {
				temp = y2 >> 1;
				for (int j = 0; j < y2; j++) {
					tempY += y1;
					temp += x2;
					if (temp >= y2) {
						temp -= y2;
						*xb++ = tempY;
					}
				}
			}
		}
	}

	*xb++ = tempY;

}

int CometEngine::Points_getY_sub_8419(int x, int y) {
	int yp = 0;
	for (int i = 0; i < _pointsArray.size(); i++) {
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

int CometEngine::checkCollisionWithScenePortals(const Common::Rect &rect, int direction) {

	int x, y, x2, x3, y3, x4, y4;
	
	x = rect.left;
	y = rect.top;
	x2 = rect.right;

	for (int index = 0; index < _linesArray.size(); index++) {
		bool flag = false;
		if (_linesArray[index].directionIndex == direction) {
			getPortalRect(index, x3, y3, x4, y4);
			if (direction == 1 || direction == 3) {
				flag = (x >= x3) && (x2 <= x4);
			} else if (direction == 2) {
				flag = (y >= y3) && (y <= y4) && (x2 >= x3);
			} else if (direction == 4) {
				flag = (y >= y3) && (y <= y4) && (x <= x4);
			}
			if (flag)
				return index | 0x400;
		}
	}
	
	return 0;
}

void CometEngine::rect_sub_CC94(int &x, int &y, int deltaX, int deltaY) {

	if (x - deltaX < 0)
		x = deltaX + 1;
	if (x + deltaX > 319)
		x = 319 - deltaX - 1;

	Common::Rect tempRect(x - deltaX, y - deltaY, x + deltaX, y);
	
	if (checkCollisionWithScenePortals(tempRect, 1) ||
		checkCollisionWithScenePortals(tempRect, 2) ||
		checkCollisionWithScenePortals(tempRect, 3) ||
		checkCollisionWithScenePortals(tempRect, 4))
		return;
		
	if (y - deltaY <= _xBuffer[x - deltaX])
		y = _xBuffer[x - deltaX] + deltaY + 2;
		
	if (y - deltaY <= _xBuffer[x + deltaX])
		y = _xBuffer[x + deltaX] + deltaY + 2;

	if (y > 199)
		y = 199;

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

void CometEngine::getPortalRect(int index, int &x, int &y, int &x2, int &y2) {

	LineItem *lineItem = &_linesArray[index];
	
	x = lineItem->x;
	x2 = lineItem->x2;
	
	y = _xBuffer[x];
	
	if (x == x2) {
		y2 = 199;
	} else if (lineItem->directionIndex == 3) {
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
	fflush(stdout);

	/* The VOC header's first byte is '\0' instead of a 'C' so we have to work around it */
	byte *readBuffer = new byte[size];
	_narFile->read(readBuffer, size);
	readBuffer[0] = 'C';

	Common::MemoryReadStream vocStream(readBuffer, size, true);

	byte *buffer = Audio::loadVOCFromStream(vocStream, size, rate);
	_mixer->playRaw(Audio::Mixer::kSpeechSoundType, &_voiceHandle, buffer, size, rate, Audio::Mixer::FLAG_AUTOFREE | Audio::Mixer::FLAG_UNSIGNED);

}

int CometEngine::checkLinesSub(int fileNumber, int scriptNumber) {
	
	if (scriptNumber == -1) {
		_fileNumber = -1;
		return 0;
	}
	
	_scriptNumber = scriptNumber;
	_fileNumber = fileNumber;
	
	SceneObject *sceneObject = getSceneObject(0);
	
	if (sceneObject->direction != 1 && sceneObject->direction != 3) {
		int x, y, x2, y2;

		sceneObject->value6 = 4;

		getPortalRect(sceneObject->linesIndex, x, y, x2, y2);
		if (x2 == 318)
			x2 = 319;
			
		sceneObject->value5 = 8;

		if (sceneObject->direction == 2) {
			sceneObject->x5 = 0;
			sceneObject->x6 = x2;
			sceneObjectUpdateDirection2(0, 319, sceneObject->y);
		} else if (sceneObject->direction == 4) {
			sceneObject->x5 = x;
			sceneObject->x6 = 319;
			sceneObjectUpdateDirection2(0, 0, sceneObject->y);
		}

		sceneObject->walkStatus &= 0xFFFB;

	}
	
	return 1;
	
}

uint16 CometEngine::rectInSceneItem(const Common::Rect &rect) {
	for (int i = 0; i < _sceneItems.size(); i++) {
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

	x = rect.left;
	y = rect.top;
	x2 = rect.right;
	y2 = rect.bottom;

	if (x < 0)
		x = 0;

	if (x2 > 319)
		x2 = 319;

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

	for (int index = 0; index < _blockingRects.size(); index++) {
		_blockingTestRect = _blockingRects[index];
		if (_blockingRects[index].left != _blockingRects[index].right) {
			if (rectCompare(_blockingTestRect, rect)) {
				return index | 0x300;
			}
		}
	}
	
	return 0;

}

int CometEngine::checkCollisionWithActors(int skipIndex, Common::Rect &rect) {

	for (int index = 0; index < 11; index++) {
		SceneObject *sceneObject = getSceneObject(index);
		if (index != skipIndex && sceneObject->flag != 0 && sceneObject->value5 != 8) {
			_blockingTestRect.left = sceneObject->x - sceneObject->deltaX;
			_blockingTestRect.top = sceneObject->y - sceneObject->deltaY;
			_blockingTestRect.right = sceneObject->x + sceneObject->deltaX;
			_blockingTestRect.bottom = sceneObject->y;
			if (rectCompare(rect, _blockingTestRect)) {
				return index | 0x600;
			}
		}
	}
	
	return 0;

}

uint16 CometEngine::checkCollision(int index, int x, int y, int deltaX, int deltaY, int direction) {

	uint16 collisionType = 0;
	Common::Rect rect;
	
	rect.left = x - deltaX;
	rect.top = y - deltaY;
	rect.right = x + deltaX;
	rect.bottom = y;
	
	collisionType = checkCollisionWithRoomBounds(rect, direction);
	if (collisionType != 0) {
		uint16 portalCollision = checkCollisionWithScenePortals(rect, direction);
		if (portalCollision != 0)
			collisionType = portalCollision;
	} else {
		collisionType = checkCollisionWithBlockingRects(rect);
		if (collisionType == 0)
			collisionType = checkCollisionWithActors(index, rect);
	}

	//debug(4, "CometEngine::checkCollision() collisionType = %02X", collisionType); fflush(stdout);

	return collisionType;

}

void CometEngine::initStaticObjectRects() {

	byte *sec00 = _staticObjects->getSubSection(0, 0);
	int rectCount = sec00[1];
	
	sec00 += 2;

	_blockingRects.clear();
	
	for (int index = 0; index < rectCount; index++) {

		int section = sec00[0];
		sec00 += 2;
		
		int subSection = READ_LE_UINT16(sec00);
		sec00 += 2;
		
		int x = READ_LE_UINT16(sec00) / 2;
		sec00 += 2;
		
		int y = READ_LE_UINT16(sec00);
		sec00 += 2;
		
		byte *subSec = _staticObjects->getSubSection(section, subSection & 0x7FFF) - 2;
		
		int subX = subSec[0] / 2;

		if (subX != 0) {
			int blockX, blockY, blockX2, blockY2;
			if (section == 0) {
				blockX = x - subX;
			} else {
				blockX = x;
			}
			blockY = y - ( (((subSec[1] >> 4) & 3) + 1) << 2 ); // FIXME
			blockX2 = x + subX;
			blockY2 = y;
			addBlockingRect(blockX, blockY, blockX2, blockY2);
		}

	}

}

uint16 CometEngine::handleCollision(SceneObject *sceneObject, int index, uint16 collisionType) {

	int result = 0;
	
	sceneObject->value5 = (collisionType >> 8) & 0xFF;
	sceneObject->linesIndex = collisionType & 0xFF;

	/*
	//DEBUG
	if (sceneObject->value5 == 4) {
		debug(4, "sceneObject->value5 = %d!!!", sceneObject->value5); fflush(stdout);
		_system->delayMillis(5000);
	}
	*/

	if (index == 0 && sceneObject->value5 == 4) {
		result = checkLinesSub(_linesArray[sceneObject->linesIndex].fileNumber, _linesArray[sceneObject->linesIndex].scriptNumber);
	}
	
	if (result == 0) {
		sceneObjectSetDirectionAdd(sceneObject, 0);
		sceneObjectUpdate02(sceneObject);
	}

	return result;

}

void CometEngine::handleInventory() {

}

void CometEngine::handleSceneChange(int scriptNumber, int fileNumber) {

	const int directionArray[] = {0, 3, 4, 1, 2};

	int direction = 1;
	int x = 160, x2 = 160, y = 190, y2 = 190;
	SceneObject *sceneObject = getSceneObject(0);
	
	for (int lineIndex = 0; lineIndex < _linesArray.size(); lineIndex++) {
		LineItem *lineItem = &_linesArray[lineIndex];
		if (lineItem->scriptNumber == scriptNumber && lineItem->fileNumber == fileNumber) {
			direction = directionArray[lineItem->directionIndex];
			if (sceneObject->direction == direction) {
				getPortalRect(lineIndex, x, y, x2, y2);
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
	
	sceneObjectUpdateDirection2(index, x, y);

}

} // End of namespace Comet
