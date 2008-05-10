#include "comet/comet.h"
#include "comet/music.h"

#include "comet/screen.h"
#include "comet/dialog.h"

namespace Comet {

/* Script */

byte Script::loadByte() {
	return *ip++;
}

int16 Script::loadInt16() {
	int16 value = (int16)READ_LE_UINT16(ip);
	ip += 2;
	return value;
}

void Script::jump() {

	int16 ofs = (int16)READ_LE_UINT16(ip);

	printf("    jump: %d (%04X)\n", ofs, ofs);
	
	ip += ofs;
	
}

uint16 Script::loadVarValue() {
	int varIndex = loadInt16();
	int value = *_vm->getVarPointer(varIndex);
	return value;
}

uint16 Script::loadValue() {
	byte type = loadByte();
	switch (type) {
	case 11:
		return loadVarValue();
	case 12:
		return loadByte();
	default:
		return loadInt16();
	}
}

bool Script::evalBoolOp(int value1, int value2, int boolOp) {
	switch (boolOp) {
	case 0: // EQ
		return value1 == value2;
	case 1: // NEQ
		return value2 != value2;
	case 2: // GT
		return value1 > value2;
	case 3: // GTE
		return value1 >= value2;
	case 4: // LT
		return value1 < value2;
	case 5: // LTE
		return value1 <= value2;
	case 6: // AND
		return (value1 & value2) != 0;
	case 7: // OR
		return (value1 | value2) != 0;
	default:
		error("Script::evalBoolOp() Unknown bool operation code %d", boolOp);
	}
	return false;
}

SceneObject *Script::object() const {
	assert( objectIndex >= 0 && objectIndex < 11 );
	return _vm->getSceneObject(objectIndex);
}


/* Script interpreter */

void CometEngine::initializeScript() {

	_scriptCount = READ_LE_UINT16(_scriptData) / 2;
	
	printf("CometEngine::initializeScript()  _scriptCount = %d\n", _scriptCount);
	
	for (int scriptNumber = 0; scriptNumber < _scriptCount; scriptNumber++)
		prepareScript(scriptNumber);

	_scripts[0]->status = 0;
	runScript(0);

}

void CometEngine::prepareScript(int scriptNumber) {

	Script *script = _scripts[scriptNumber];

	uint16 ofs = READ_LE_UINT16(_scriptData + scriptNumber * 2);

	script->ip = _scriptData + ofs;
	script->code = _scriptData + ofs;
	script->objectIndex = 0;
	script->status = kScriptPaused;
	script->scriptNumber = 0;
	script->counter = 0;

}

void CometEngine::runScriptSub1() {

	//printf("######## runScriptSub1()\n");

	int scriptNumber = _curScript->scriptNumber;
	Script *script = _scripts[scriptNumber];
	
	if ((script->status & 8) && script->scriptNumber == _curScriptNumber) {
		script->status &= 0xF7;
		script->scriptNumber = 0;
		_curScript->status &= 0xF7;
		_curScript->scriptNumber = 0;
	} else {
		_scriptBreakFlag = true;
	}

}

void CometEngine::runScriptSub2() {

	//printf("######## runScriptSub2()\n");

	if (_curScript->scriptNumber > 0)
		_curScript->scriptNumber--;

	if (_curScript->scriptNumber == 0) {
		_curScript->status &= ~kScriptSleeping;
	} else {
		_scriptBreakFlag = true;
	}

}

void CometEngine::runScriptSub3() {

	printf("######## runScriptSub3()  objectIndex = %d\n", _curScript->objectIndex);

	if (!(_curScript->object()->walkStatus & 3) || _curScript->object()->flag == 0) {
		_curScript->status &= ~kScriptWalking;
		sceneObjectSetAnimNumber(_curScript->object(), 0);
	} else {
		_scriptBreakFlag = true;
	}

}

void CometEngine::runScriptSub4() {

	if (_curScript->object()->animFrameIndex + 1 == _curScript->object()->animFrameCount) {
		_curScript->status &= ~kScriptAnimPlaying;
	} else {
		_scriptBreakFlag = true;
	}

}

void CometEngine::runScriptSub5() {

	if (!_dialog->isRunning()) {
		_curScript->status &= ~kScriptDialogRunning;
		// FIXME: I don't like that getChoiceScriptIp directly returns a pointer
		// should be encapsulated in either Script or Dialog
		_curScript->ip = _dialog->getChoiceScriptIp();
		_curScript->jump();
	} else {
 		_scriptBreakFlag = true;
 	}

}

void CometEngine::runScriptSub6() {

	if (_textFlag2 == 0) {
		_curScript->status &= ~kScriptTalking;
		if (_sceneObjectIndex == 10) {
			if (_animIndex != -1)
				_sceneObjects[_animIndex].visible = true;
			_sceneObjects[10].flag = 0;
			//_screenTransitionEffectFlag = true;
			_screen->enableTransitionEffect();
		} else if (_animIndex != -1) {
			SceneObject *sceneObject = getSceneObject(_sceneObjectIndex);
			sceneObjectSetAnimNumber(sceneObject, _animIndex);
			sceneObject->animSubIndex2 = _animSubIndex2;
			sceneObject->animFrameIndex = _animSubIndex;
			_animIndex = -1;
		}
	} else {
		_scriptBreakFlag = true;
	}
	
}

void CometEngine::runScript(int scriptNumber) {

	//printf("CometEngine::runScript(%d)\n", scriptNumber); fflush(stdout);

	_curScriptNumber = scriptNumber;
	_curScript = _scripts[scriptNumber];

	_scriptBreakFlag = false;

	/*
	printf("%02d ", scriptNumber);
	if (_curScript->status & kScriptWalking)
		printf("kScriptWalking ", scriptNumber);
	if (_curScript->status & kScriptSleeping)
		printf("kScriptSleeping ", scriptNumber);
	if (_curScript->status & kScriptAnimPlaying)
		printf("kScriptAnimPlaying ", scriptNumber);
	if (_curScript->status & kScriptDialogRunning)
		printf("kScriptDialogRunning ", scriptNumber);
	if (_curScript->status & kScriptPaused)
		printf("kScriptPaused ", scriptNumber);
	if (_curScript->status & kScriptTalking)
		printf("kScriptTalking ", scriptNumber);
	printf("\n");
	fflush(stdout);
	*/

	if (_curScript->status & kScriptPaused)
		return;
		
	if (_curScript->status & 8)
		runScriptSub1();

	if (_curScript->status & kScriptSleeping)
		runScriptSub2();

	if (_curScript->status & kScriptWalking)
		runScriptSub3();

	if (_curScript->status & kScriptAnimPlaying)
		runScriptSub4();

	if (_curScript->status & kScriptDialogRunning)
		runScriptSub5();

	if (_curScript->status & kScriptTalking)
		runScriptSub6();

	//printf("%02d: _scriptBreakFlag = %d\n", scriptNumber, _scriptBreakFlag); fflush(stdout);

	// TODO: Put script functions into a nice array

	while (!_scriptBreakFlag) {
		byte funcIndex = *_curScript->ip++;

		printf("-----------------------------------------------   op %d\n", funcIndex);
		printf("%02d ", scriptNumber); fflush(stdout);

		switch (funcIndex) {
		case 0:
			// NOPs
			break;
		case 1:
			o1_sceneObjectSetDirection(_curScript);
			break;
		case 2:
			_scriptBreakFlag = true;
			break;
		case 3:
			o1_jump(_curScript);
			break;
		case 4:
			o1_objectWalkToXAbs(_curScript);
			break;
		case 5:
			o1_objectWalkToYAbs(_curScript);
			break;
		case 6:
			o1_loop(_curScript);
			break;
		case 7:
			o1_objectSetPosition(_curScript);
			break;
		case 9:
			o1_sleep(_curScript);
			break;
		case 10:
			o1_if(_curScript);
			break;
		case 14:
			o1_condJump2(_curScript);
			break;
		case 15:
			o1_objectWalkToXRel(_curScript);
			break;
		case 16:
			o1_objectWalkToYRel(_curScript);
			break;
		case 18:
			o1_setMouseFlags(_curScript);
			break;
		case 19:
			o1_resetHeroDirectionChanged(_curScript);
			break;
  		case 20:
  			o1_sceneObjectSetDirectionTo(_curScript);
  			break;
		case 21:
			o1_selectObject(_curScript);
			break;
		case 22:
			o1_initPoints(_curScript);
			break;
		case 23:
			o1_initLines(_curScript);
			break;
		case 25:
			o1_addSceneItem1(_curScript);
			break;
		case 27:
			o1_startScript(_curScript);
			break;
		case 28:
			o1_pauseScript(_curScript);
			break;
		case 30:
			o1_playCutscene(_curScript);
			break;
		case 31:
			o1_setVar(_curScript);
			break;
		case 32:
			o1_incVar(_curScript);
			break;
		case 33:
			o1_subVar(_curScript);
			break;
  		case 34:
  			o1_setItemValue5To8(_curScript);
  			break;
  		case 35:
  			o1_setItemValue5To0(_curScript);
  			break;
		case 36:
			o1_updateDirection2(_curScript);
			break;
		case 39:
			o1_waitWhilePlayerIsInRect(_curScript);
			break;
		case 40:
			o1_waitUntilPlayerIsInRect(_curScript);
			break;
		case 41:
			o1_freeMarcheEx(_curScript);
			break;
		case 42:
			o1_setObjectClipX(_curScript);
			break;
		case 43:
			o1_setObjectClipY(_curScript);
			break;
		case 44:
			o1_setGlobalScriptNumber(_curScript);
			break;
		case 45:
			o1_setAnimValues(_curScript);
			break;
		case 47:
			o1_setMarcheNumber(_curScript);
			break;
		case 52:
			o1_setZoomByItem(_curScript);
			break;
		case 53:
			o1_startDialog(_curScript);
			break;
		case 56:
			o1_orVar(_curScript);
			break;
		case 58:
			o1_loadScene(_curScript);
			break;
		case 60:
			o1_addBlockingRect(_curScript);
			break;
		case 62:
			o1_sub_A67F(_curScript);
			break;
		case 63:
			o1_sub_A64B(_curScript);
			break;
		case 66:
			o1_sub_A735(_curScript);
			break;
		case 68:
			o1_removeBlockingRect(_curScript);
			break;
		case 69:
			o1_setSceneObjectColor(_curScript);
			break;
		case 70:
			o1_setTextXY(_curScript);
			break;
		case 77:
			o1_playMusic(_curScript);
			break;
		case 79:
			o1_setRandomValue(_curScript);
			break;
		case 80:
			o1_setFileNumber(_curScript);
			break;
		case 82:
			o1_dialog(_curScript);
			break;
		case 84:
			o1_addSceneItem2(_curScript);
			break;
		case 87:
			o1_playAnim(_curScript);
			break;
		case 88:
			o1_sceneObjectSetAnimNumber(_curScript);
			break;
		case 89:
			o1_sub_AD04(_curScript);
			break;
		case 90:
			o1_initSceneObject(_curScript);
			break;
		case 91:
			o1_loadMarche(_curScript);
			break;
		case 92:
			o1_setObjectVisible(_curScript);
			break;
		case 93:
			o1_paletteFadeIn(_curScript);
			break;
		case 94:
			o1_paletteFadeOut(_curScript);
			break;
		case 96:
			o1_setNarFileIndex(_curScript);
			break;
		case 98:
			o1_deactivateSceneItem(_curScript);
			break;
		case 99:
			o1_sample_2(_curScript);
			break;
		case 100:
			o1_sample_1(_curScript);
			break;
		default:
			error("CometEngine::runScript() Unknown opcode %d", funcIndex);
			break;
		}

	}

}

void CometEngine::runAllScripts() {
	// Run all scripts except the main script
	for (int scriptNumber = 1; scriptNumber < _scriptCount; scriptNumber++) {
		runScript(scriptNumber);
	}
}

int *CometEngine::getVarPointer(int varIndex) {

	if (varIndex < 1000) {
		assert(_scriptVars1[varIndex]);
		return _scriptVars1[varIndex];
	} else if (varIndex < 2000) {
		return &_scriptVars2[varIndex - 1000];
	} else {
		return &_scriptVars3[varIndex - 2000];
	}
	
}

SceneObject *CometEngine::getScriptSceneObject() {
	return getSceneObject(_curScript->objectIndex);
}

void CometEngine::objectWalkToXYAbs(Script *script, bool xyFlag) {
	
	int x, y;
	int newValue = script->loadByte();
	
	script->object()->directionChanged = 0;

	if (!xyFlag) {
		x = newValue * 2;
		y = script->object()->y;
	} else {
		x = script->object()->x;
		y = newValue;
	}

	printf("objectWalkToXYAbs()  object: %d; old: %d, %d; new: %d, %d\n", script->objectIndex, script->object()->x, script->object()->y, x, y); fflush(stdout);

	if (sceneObjectUpdateDirection2(script->objectIndex, x, y)) {
		if (!xyFlag) {
			script->object()->walkStatus |= 8;
		} else {
			script->object()->walkStatus |= 0x10;
		}
		script->status |= kScriptWalking;
		_scriptBreakFlag = true;
	}
	
}

void CometEngine::objectWalkToXYRel(Script *script, bool xyFlag) {

	int x, y;
	int delta = script->loadByte();
	
	script->object()->directionChanged = 0;

	if (!xyFlag) {
		x = _sceneObjects[0].x + delta;
		y = script->object()->y;
	} else {
		x = script->object()->x;
		y = _sceneObjects[0].y + delta;
	}

	printf("objectWalkToXYRel()  object: %d; old: %d, %d; new: %d, %d\n", script->objectIndex, script->object()->x, script->object()->y, x, y); fflush(stdout);

	if (sceneObjectUpdateDirection2(script->objectIndex, x, y)) {
		if (!xyFlag) {
			script->object()->walkStatus |= 8;
		} else {
			script->object()->walkStatus |= 0x10;
		}
		script->status |= kScriptWalking;
		_scriptBreakFlag = true;
	}

}

// TODO: Decouple from Script
void CometEngine::o1_addSceneItem(Script *script, int paramType) {

	int itemIndex, x, y;
	
	if (paramType == 0) {
		itemIndex = script->loadByte();
	} else {
		itemIndex = script->loadInt16();
	}
	
	x = script->loadByte() * 2;
	y = script->loadByte();
	
	printf("o1_addSceneItem(%d, %d, %d)", itemIndex, x, y);
	
	SceneItem sceneItem;
	sceneItem.itemIndex = itemIndex;
	sceneItem.active = true;
	sceneItem.paramType = paramType;
	sceneItem.x = x;
	sceneItem.y = y;
	_sceneItems.push_back(sceneItem);

}

/* Script functions */

void CometEngine::o1_sceneObjectSetDirection(Script *script) {
	printf("o1_sceneObjectSetDirection\n");

	int direction = script->loadByte();
	script->object()->directionChanged = 0;
	sceneObjectSetDirection(script->object(), direction);

}

void CometEngine::o1_jump(Script *script) {
	printf("o1_jump()\n");
	
	script->jump();
}

void CometEngine::o1_objectWalkToXAbs(Script *script) {
	printf("o1_objectWalkToXAbs()\n");

	objectWalkToXYAbs(script, false);
}

void CometEngine::o1_objectWalkToYAbs(Script *script) {
	printf("o1_objectWalkToYAbs()\n");

	objectWalkToXYAbs(script, true);
}

void CometEngine::o1_loop(Script *script) {

	byte loopCount = script->ip[2];
	
	printf("o1_loop(%d)\n", loopCount);
	
	script->counter++;
	
	if (script->counter < loopCount) {
		script->jump();
	} else {
		script->counter = 0;
		script->ip += 3;
	}
	
}

void CometEngine::o1_objectSetPosition(Script *script) {

	int x = script->loadByte() * 2;
	int y = script->loadByte();

	printf("o1_objectSetPosition(%d, %d)\n", x, y);

	sceneObjectSetXY(script->objectIndex, x, y);

}

void CometEngine::o1_sleep(Script *script) {

	int sleepCount = script->loadByte();

	printf("o1_sleep(%d)\n", sleepCount);

	script->scriptNumber = sleepCount;
	script->status |= kScriptSleeping;
	_scriptBreakFlag = true;
	
}

void CometEngine::o1_if(Script *script) {

	uint16 value1 = script->loadVarValue();
	byte boolOp = script->loadByte();
	uint16 value2 = script->loadValue();

	const char* boolOps[] = {"==", "!=", ">", ">=", "<", "<=", "&", "|"};
	printf("o1_if %d %s %d\n", value1, boolOps[boolOp], value2);

	if (script->evalBoolOp(value1, value2, boolOp))
		script->ip += 2;
	else
		script->jump();

}

void CometEngine::o1_condJump2(Script *script) {
	if (!o1_Sub_rectCompare01(script)) {
		script->jump();
	} else {
		script->ip += 2;
	}
}

void CometEngine::o1_objectWalkToXRel(Script *script) {
	printf("o1_objectWalkToXRel()\n");

	objectWalkToXYRel(script, false);
}

void CometEngine::o1_objectWalkToYRel(Script *script) {
	printf("o1_objectWalkToYRel()\n");

	objectWalkToXYRel(script, true);
}

void CometEngine::o1_setMouseFlags(Script *script) {

	int flagIndex = script->loadByte();
	
	printf("o1_setMouseFlags(%d)\n", flagIndex);

	if (flagIndex == 0) {
		_mouseCursor2 = 0;
		_mouseFlag = 15;
		sceneObjectResetDirectionAdd(getSceneObject(0));
	} else {
		const int constFlagsArray[5] = {0, 1, 8, 2, 4};
		_mouseFlag |= constFlagsArray[flagIndex];
	}
	
}

void CometEngine::o1_resetHeroDirectionChanged(Script *script) {
	printf("o1_resetHeroDirectionChanged()\n");
	
	resetHeroDirectionChanged();
}

void CometEngine::o1_sceneObjectSetDirectionTo(Script *script) {

	SceneObject *playerObject = getSceneObject(0);
	
	int direction = calcDirection( script->object()->x, script->object()->y, playerObject->x, playerObject->y );
	script->object()->directionChanged = 0;
	sceneObjectSetDirection(script->object(), direction);
	
}

void CometEngine::o1_selectObject(Script *script) {

	int objectIndex = script->loadByte();
	
	printf("o1_selectObject(%d)\n", objectIndex);
	
	script->objectIndex = objectIndex;
	
}

void CometEngine::o1_initPoints(Script *script) {
	printf("o1_initPoints\n");

	initPoints(script->ip);
	script->ip += *script->ip * 2 + 1;

}

void CometEngine::o1_initLines(Script *script) {
	printf("o1_initLines\n");

	initLines(script->ip);
	script->ip += *script->ip * 5 + 1;

}

void CometEngine::o1_addSceneItem1(Script *script) {
	o1_addSceneItem(script, 0);
}

void CometEngine::o1_startScript(Script *script) {

	int scriptNumber = script->loadByte();

	printf("o1_startScript(%d)\n", scriptNumber);

	if (scriptNumber < _scriptCount) {
		prepareScript(scriptNumber);
		_scripts[scriptNumber]->status &= ~kScriptPaused;
	}

}

void CometEngine::o1_pauseScript(Script *script) {

	int scriptNumber = script->loadByte();

	printf("o1_pauseScript(%d)\n", scriptNumber);

	_scripts[scriptNumber]->status |= kScriptPaused;

}

void CometEngine::o1_playCutscene(Script *script) {

	int fileIndex = script->loadByte();
	int indexSubStart = script->loadByte();
	int index = script->loadInt16();
	int counterMax = script->loadByte();
	int cutsceneCounter2 = script->loadByte();
	
	//TODO
	//playCutscene( fileIndex, indexSubStart, index, counterMax, cutsceneCounter2, script->ip );
	
	script->ip += cutsceneCounter2 * 3;

}

void CometEngine::o1_setVar(Script *script) {

	int varIndex = script->loadInt16();
	int value = script->loadValue();

	printf("o1_setVar(%d, %d)\n", varIndex, value);

	int *varPtr = getVarPointer(varIndex);
	*varPtr = value;
	
}

void CometEngine::o1_incVar(Script *script) {

	int varIndex = script->loadInt16();
	int value = script->loadValue();

	printf("o1_incVar(%d, %d)\n", varIndex, value);

	int *varPtr = getVarPointer(varIndex);
	*varPtr += value;

}

void CometEngine::o1_subVar(Script *script) {

	int varIndex = script->loadInt16();
	int value = script->loadValue();

	printf("o1_subVar(%d, %d)\n", varIndex, value);

	int *varPtr = getVarPointer(varIndex);
	*varPtr -= value;

}

void CometEngine::o1_setItemValue5To8(Script *script) {

	printf("o1_setItemValue5To8()\n");

	script->object()->value5 = 8;
}

void CometEngine::o1_setItemValue5To0(Script *script) {

	printf("o1_setItemValue5To0()\n");

	script->object()->value5 = 0;
}

void CometEngine::o1_updateDirection2(Script *script) {

	int x = script->loadByte() * 2;
	int y = script->loadByte();

	debug(2, "o1_updateDirection2(%d, %d)\n", x, y);
	
	script->object()->directionChanged = 0;
	
	if (sceneObjectUpdateDirection2(script->objectIndex, x, y)) {
		script->status |= kScriptWalking;
		_scriptBreakFlag = true;
	}

}

void CometEngine::o1_setGlobalScriptNumber(Script *script) {

	int scriptNumber = script->loadByte();

	printf("o1_setGlobalScriptNumber(%d)\n", scriptNumber);

	if (scriptNumber == 0xFF) {
		_scriptNumber = -1;
	} else {
		_scriptNumber = scriptNumber;
	}

}

void CometEngine::o1_setAnimValues(Script *script) {

	int animIndex = script->loadByte();
	int animFrameIndex = script->loadByte();

	printf("o1_setAnimValues(%d, %d)\n", animIndex, animFrameIndex);

	script->object()->animIndex = animIndex;
	script->object()->animFrameIndex = animFrameIndex;
	script->object()->animSubIndex2 = animFrameIndex;
	script->object()->directionChanged = 2;
	
}

void CometEngine::o1_setMarcheNumber(Script *script) {
	_marcheNumber = script->loadByte();
}

void CometEngine::o1_setZoomByItem(Script *script) {

	int objectIndex = script->loadByte();
	int zoomFactor = script->loadByte();

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	_screen->setZoom(zoomFactor, sceneObject->x, sceneObject->y);
	
}

void CometEngine::o1_startDialog(Script *script) {

	_dialog->run(script);

	script->status |= kScriptDialogRunning;
	_scriptBreakFlag = true;

	//TODO: waitForKey();
	
}

void CometEngine::o1_waitWhilePlayerIsInRect(Script *script) {

	printf("o1_waitWhilePlayerIsInRect(%d, %d, %d, %d)\n", script->x, script->y, script->x2, script->y2);

	//DEBUG
	_screen->fillRect(script->x, script->y, script->x2, script->y2, 60);

	if (isPlayerInRect(script->x, script->y, script->x2, script->y2)) {
		script->ip--;
		_scriptBreakFlag = true;
	}

}

void CometEngine::o1_waitUntilPlayerIsInRect(Script *script) {

	script->x = script->loadByte() * 2;
	script->y = script->loadByte();
	script->x2 = script->loadByte() * 2;
	script->y2 = script->loadByte();

	printf("o1_waitUntilPlayerIsInRect(%d, %d, %d, %d)\n", script->x, script->y, script->x2, script->y2);

	//DEBUG
	_screen->fillRect(script->x, script->y, script->x2, script->y2, 70);


	if (!isPlayerInRect(script->x, script->y, script->x2, script->y2)) {
		script->ip -= 5;
		_scriptBreakFlag = true;
	}

}

void CometEngine::o1_freeMarcheEx(Script *script) {
	printf("o1_freeMarcheEx\n");

	if (script->objectIndex != 0) {
		script->object()->flag = 0;
		if (script->object()->marcheIndex != -1)
			freeMarcheEx(script->object());
	}

}

void CometEngine::o1_setObjectClipX(Script *script) {
	
	int x5 = script->loadByte() * 2;
	int x6 = script->loadByte() * 2;

	printf("o1_setObjectClipX(%d, %d)\n", x5, x6);
	
	script->object()->x5 = x5;
	script->object()->x6 = x6;

}

void CometEngine::o1_setObjectClipY(Script *script) {

	int y5 = script->loadByte();
	int y6 = script->loadByte();

	printf("o1_setObjectClipY(%d, %d)\n", y5, y6);

	script->object()->y5 = y5;
	script->object()->y6 = y6;

}

void CometEngine::o1_orVar(Script *script) {
	printf("o1_orVar\n");

	int varIndex = script->loadInt16();
	int value = script->loadValue();
	int *varPtr = getVarPointer(varIndex);
	*varPtr |= value;
	
}

void CometEngine::o1_loadScene(Script *script) {

	_backgroundFileIndex = script->loadByte() * 2;

	printf("o1_loadScene(%d)\n", _backgroundFileIndex);

	initSceneBackground();

}

void CometEngine::o1_addBlockingRect(Script *script) {

	int x = script->loadByte();
	int y = script->loadByte();
	int x2 = script->loadByte();
	int y2 = script->loadByte();

	printf("o1_addBlockingRect(%d,%d,%d,%d)\n", x, y, x2, y2);

	addBlockingRect(x, y, x2, y2);

}

void CometEngine::o1_sub_A67F(Script *script) {

	int objectIndex = script->loadByte();

	if (_cmdTalk && rectCompare02(0, objectIndex, 40, 40)) {
		script->ip += 2;
		_cmdTalk = false;
	} else {
		script->jump();
	}

}

void CometEngine::o1_sub_A64B(Script *script) {

	printf("o1_sub_A64B()\n");

	if (_cmdTalk) {
		if (o1_Sub_rectCompare01(script)) {
			script->ip += 2;
			_cmdTalk = false;
		} else {
			script->jump();
		}
	} else {
		script->ip += 4;
		script->jump();
	}

}

bool CometEngine::o1_Sub_rectCompare01(Script *script) {

	SceneObject *sceneObject = getSceneObject(0);

	script->x = script->loadByte() * 2;
	script->y = script->loadByte();
	script->x2 = script->loadByte() * 2;
	script->y2 = script->loadByte();
	
	//DEBUG
	_screen->fillRect(script->x, script->y, script->x2, script->y2, 50);
	
	
	Common::Rect rect1(script->x, script->y, script->x2, script->y2);
	Common::Rect rect2(
		sceneObject->x - sceneObject->deltaX,
		sceneObject->y - sceneObject->deltaY,
		sceneObject->x + sceneObject->deltaX,
		sceneObject->y);
	
	return rectCompare(rect1, rect2);

}

void CometEngine::o1_sub_A735(Script *script) {
	// LOOK AT handler

	printf("o1_sub_A735()\n");

	if (_cmdLook) {
		if (o1_Sub_rectCompare01(script)) {
			script->ip += 2;
			_cmdLook = false;
		} else {
			script->jump();
		}
	} else {
		script->ip += 4;
		script->jump();
	}

}

void CometEngine::o1_removeBlockingRect(Script *script) {

	int x = script->loadByte() * 2;
	int y = script->loadByte();
	
	for (int i = _blockingRects.size() - 1; i >= 0; i--) {
		if (_blockingRects[i].left == x && _blockingRects[i].top == y) {
			_blockingRects.remove_at(i);
			break;
		}
	}

}

void CometEngine::o1_setSceneObjectColor(Script *script) {
	printf("o1_setSceneObjectColor\n");

	script->object()->color = script->loadByte();
	
}

void CometEngine::o1_setTextXY(Script *script) {

	int textX = script->loadByte() * 2;
	int textY = script->loadByte();

	printf("o1_setTextXY(%d, %d)\n", textX, textY);

	script->object()->textX = textX;
	script->object()->textY = textY;
	
}

void CometEngine::o1_playMusic(Script *script) {

	int fileIndex = script->loadByte();

	printf("o1_playMusic(%d)\n", fileIndex);

	if (fileIndex != 0xFF) {
		//TODO: playMusic(fileIndex);
		_music->playMusic(fileIndex);
	} else {
		//TODO: musicFadeDown();
		_music->stopMusic();
	}

}

void CometEngine::o1_setRandomValue(Script *script) {
	printf("o1_setRandomValue()\n");
	_scriptRandomValue = random(script->loadByte());
}

void CometEngine::o1_setFileNumber(Script *script) {
	_fileNumber = script->loadByte();
	printf("o1_setFileNumber(%d)\n", _fileNumber);
	_scriptBreakFlag = true;
}

void CometEngine::o1_dialog(Script *script) {

	int objectIndex = script->loadByte();
	int narSubIndex = script->loadInt16();
	int animNumber = script->loadByte();

	printf("o1_dialog(%d, %d, %d)\n", objectIndex, narSubIndex, animNumber);

	textProc2(objectIndex, narSubIndex, animNumber);

}

void CometEngine::o1_addSceneItem2(Script *script) {
	o1_addSceneItem(script, 1);
}

void CometEngine::o1_playAnim(Script *script) {
	printf("o1_playAnim\n");

	o1_sceneObjectSetAnimNumber(script);
	script->status |= kScriptAnimPlaying;
	_scriptBreakFlag = true;

}

void CometEngine::o1_sceneObjectSetAnimNumber(Script *script) {

	int animIndex = script->loadByte();

	printf("o1_sceneObjectSetAnimNumber(%d)\n", animIndex);

	sceneObjectSetAnimNumber(script->object(), animIndex);
	script->object()->directionChanged = 2;

}

void CometEngine::o1_sub_AD04(Script *script) {

	int objectIndex = script->loadByte();
	int narSubIndex = script->loadInt16();
	int animNumber = script->loadByte();
	int fileIndex = script->loadByte();
	
	printf("o1_sub_AD04(%d, %d, %d, %d)\n", objectIndex, narSubIndex, animNumber, fileIndex);
	//_system->delayMillis(5000);

	int marcheIndex = loadMarche(_marcheNumber, fileIndex);
	sceneObjectInit(10, marcheIndex);
	
	if (objectIndex != -1) {
		_sceneObjects[10].textX = 0;
		_sceneObjects[10].textY = 160;
		_sceneObjects[10].color = getSceneObject(objectIndex)->color;
	}
	
	_marcheNumber = 0;
	sceneObjectSetXY(10, 0, 199);
	textProc2(10, narSubIndex, animNumber);
	_animIndex = objectIndex;
	//_screenTransitionEffectFlag = true;
	_screen->enableTransitionEffect();

}

void CometEngine::o1_initSceneObject(Script *script) {
	printf("o1_initSceneObject\n");

	o1_selectObject(script);

	if (script->objectIndex != 0) {
		sceneObjectInit(script->objectIndex, -1);
	}

}

void CometEngine::o1_loadMarche(Script *script) {

	int fileIndex = script->loadByte();
	
	printf("o1_loadMarche(%d)\n", fileIndex);

	freeMarcheEx(script->object());
	
	script->object()->marcheIndex = loadMarche(_marcheNumber, fileIndex);
	_marcheNumber = 0;
	
}

void CometEngine::o1_setObjectVisible(Script *script) {
	int visible = script->loadByte();
	printf("o1_setObjectVisible(%d)\n", visible);
	script->object()->visible = visible;
}

void CometEngine::o1_paletteFadeIn(Script *script) {
	int paletteValue = script->loadByte();
	printf("o1_paletteFadeIn(%d)\n", paletteValue);
	_screen->setFadeType(kFadeIn);
	_screen->setFadeValue(paletteValue);
}

void CometEngine::o1_paletteFadeOut(Script *script) {
	int paletteValue = script->loadByte();
	printf("o1_paletteFadeOut(%d)\n", paletteValue);
	_screen->setFadeType(kFadeOut);
	_screen->setFadeValue(paletteValue);
}

void CometEngine::o1_setNarFileIndex(Script *script) {

	_narFileIndex = script->loadByte();

	printf("o1_setNarFileIndex(%d)\n", _narFileIndex);

	openVoiceFile(_narFileIndex);
	
}

void CometEngine::o1_deactivateSceneItem(Script *script) {

	int itemIndex = script->loadByte();

	int index = 0;
	while (index < _sceneItems.size()) {
		if (_sceneItems[index].itemIndex == itemIndex) {
			_sceneItems.remove_at(index);
		} else {
			index++;
		}
	}

}

void CometEngine::o1_sample_2(Script *script) {
	printf("o1_sample_2\n");

	int narFileIndex = script->loadByte();
	
	//TODO: seg010:074A
	
}

void CometEngine::o1_sample_1(Script *script) {
	printf("o1_sample_1\n");

	int narFileIndex = script->loadByte();
	int value = script->loadByte();
	
	//TODO


}

} // End of namespace Comet
