#include "comet/comet.h"
#include "comet/music.h"
#include "comet/screen.h"
#include "comet/script.h"
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

	debug(3, "    jump: %d (%04X)", ofs, ofs);
	
	ip += ofs;
	
}

uint16 Script::loadVarValue() {
	int varIndex = loadInt16();
	int value = *_inter->getVarPointer(varIndex);
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

SceneObject *Script::object() const {
	assert( objectIndex >= 0 && objectIndex < 11 );
	return _inter->getSceneObject(objectIndex);
}


/* Script interpreter */

ScriptInterpreter::ScriptInterpreter(CometEngine *vm) : _vm(vm) {

	_scriptData = new byte[3000];
	_scriptCount = 0;
	_curScriptNumber = -1;
	_curScript = NULL;

	for (int i = 0; i < 17; i++)
		_scripts[i] = new Script(this);

	setupOpcodes();
	
}

ScriptInterpreter::~ScriptInterpreter() {
	delete[] _scriptData;
	for (int i = 0; i < 17; i++)
		delete _scripts[i];
}

typedef Common::Functor1Mem<Script*, void, ScriptInterpreter> ScriptOpcodeF;
#define RegisterOpcode(x) \
	_opcodes.push_back(new ScriptOpcodeF(this, &ScriptInterpreter::x));  \
	_opcodeNames.push_back(#x);
void ScriptInterpreter::setupOpcodes() {
	// TODO
	
	// 0
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_sceneObjectSetDirection);
	RegisterOpcode(o1_break);
	RegisterOpcode(o1_jump);
	RegisterOpcode(o1_objectWalkToXAbs);
	// 5
	RegisterOpcode(o1_objectWalkToYAbs);
	RegisterOpcode(o1_loop);
	RegisterOpcode(o1_objectSetPosition);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_sleep);
	// 10
	RegisterOpcode(o1_if);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_condJump2);
	// 15
	RegisterOpcode(o1_objectWalkToXRel);
	RegisterOpcode(o1_objectWalkToYRel);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_setMouseFlags);
	RegisterOpcode(o1_resetHeroDirectionChanged);
	// 20
	RegisterOpcode(o1_sceneObjectSetDirectionTo);
	RegisterOpcode(o1_selectObject);
	RegisterOpcode(o1_initPoints);
	RegisterOpcode(o1_initSceneExits);
	RegisterOpcode(o1_nop);//TODO
	// 25
	RegisterOpcode(o1_addSceneItem1);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_startScript);
	RegisterOpcode(o1_pauseScript);
	RegisterOpcode(o1_nop);//TODO
	// 30
	RegisterOpcode(o1_playCutscene);
	RegisterOpcode(o1_setVar);
	RegisterOpcode(o1_incVar);
	RegisterOpcode(o1_subVar);
	RegisterOpcode(o1_setSceneObjectCollisionTypeTo8);
	// 35
	RegisterOpcode(o1_setSceneObjectCollisionTypeTo0);
	RegisterOpcode(o1_updateDirection2);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_waitWhilePlayerIsInRect);
	// 40
	RegisterOpcode(o1_waitUntilPlayerIsInRect);
	RegisterOpcode(o1_unloadSceneObjectSprite);
	RegisterOpcode(o1_setObjectClipX);
	RegisterOpcode(o1_setObjectClipY);
	RegisterOpcode(o1_setSceneNumber);
	// 45
	RegisterOpcode(o1_setAnimValues);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_setMarcheNumber);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	// 50
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_setZoomByItem);
	RegisterOpcode(o1_startDialog);
	RegisterOpcode(o1_nop);//TODO
	// 55
	RegisterOpcode(o1_nop);//TODO o1_setNeedToFillScreenFlag(_curScript);
	RegisterOpcode(o1_orVar);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_loadScene);
	RegisterOpcode(o1_nop);//TODO
	// 60
	RegisterOpcode(o1_addBlockingRect);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_sub_A67F);
	RegisterOpcode(o1_sub_A64B);
	RegisterOpcode(o1_nop);//TODO
	// 65
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_sub_A735);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_removeBlockingRect);
	RegisterOpcode(o1_setSceneObjectColor);
	// 70
	RegisterOpcode(o1_setTextXY);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	// 75
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_playMusic);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_setRandomValue);
	// 80
	RegisterOpcode(o1_setChapterNumber);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_dialog);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_addSceneItem2);
	// 85
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);// TODO: op_waitForKey();
	RegisterOpcode(o1_playAnim);
	RegisterOpcode(o1_sceneObjectSetAnimNumber);
	RegisterOpcode(o1_sub_AD04);
	// 90
	RegisterOpcode(o1_initSceneObject);
	RegisterOpcode(o1_loadSceneObjectSprite);
	RegisterOpcode(o1_setObjectVisible);
	RegisterOpcode(o1_paletteFadeIn);
	RegisterOpcode(o1_paletteFadeOut);
	// 95
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_setNarFileIndex);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_deactivateSceneItem);
	RegisterOpcode(o1_sample_2);
	// 100
	RegisterOpcode(o1_sample_1);
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO

}
#undef RegisterOpcode

void ScriptInterpreter::initializeScript() {

	_scriptCount = READ_LE_UINT16(_scriptData) / 2;
	
	debug(2, "CometEngine::initializeScript()  _scriptCount = %d", _scriptCount);
	
	for (int scriptNumber = 0; scriptNumber < _scriptCount; scriptNumber++)
		prepareScript(scriptNumber);

	_scripts[0]->status = 0;
	runScript(0);

}

void ScriptInterpreter::prepareScript(int scriptNumber) {

	Script *script = _scripts[scriptNumber];

	uint16 ofs = READ_LE_UINT16(_scriptData + scriptNumber * 2);

	script->ip = _scriptData + ofs;
	script->code = _scriptData + ofs;
	script->objectIndex = 0;
	script->status = kScriptPaused;
	script->scriptNumber = 0;
	script->counter = 0;

}

void ScriptInterpreter::processScriptStatus8() {

	//debug(3, "######## processScriptStatus8()");

	int scriptNumber = _curScript->scriptNumber;
	Script *script = _scripts[scriptNumber];
	
	if ((script->status & 8) && script->scriptNumber == _curScriptNumber) {
		script->status &= ~8;
		script->scriptNumber = 0;
		_curScript->status &= ~8;
		_curScript->scriptNumber = 0;
	} else {
		_scriptBreakFlag = true;
	}

}

void ScriptInterpreter::processScriptSleep() {

	//debug(3, "######## processScriptSleep()");

	if (_curScript->scriptNumber > 0)
		_curScript->scriptNumber--;

	if (_curScript->scriptNumber == 0) {
		_curScript->status &= ~kScriptSleeping;
		debug(4, "*** sleeping finished");
	} else {
		_scriptBreakFlag = true;
	}

}

void ScriptInterpreter::processScriptWalk() {

	debug(3, "######## processScriptWalk()  objectIndex = %d", _curScript->objectIndex);

	debug(2, "CometEngine::processScriptWalk() walkStatus = %d; flag = %d",
		_curScript->object()->walkStatus, _curScript->object()->flag);

	if ((_curScript->object()->walkStatus & 3) == 0 || _curScript->object()->flag == 0) {
		_curScript->status &= ~kScriptWalking;
		_vm->sceneObjectSetAnimNumber(_curScript->object(), 0);
		debug(4, "*** walking finished");
	} else {
		_scriptBreakFlag = true;
	}

}

void ScriptInterpreter::processScriptAnim() {

	if (_curScript->object()->animFrameIndex + 1 == _curScript->object()->animFrameCount) {
		_curScript->status &= ~kScriptAnimPlaying;
		debug(4, "*** anim playing finished");
	} else {
		_scriptBreakFlag = true;
	}

}

void ScriptInterpreter::processScriptDialog() {

	if (!_vm->_dialog->isRunning()) {
		_curScript->status &= ~kScriptDialogRunning;
		// FIXME: I don't like that getChoiceScriptIp directly returns a pointer
		// should be encapsulated in either Script or Dialog
		_curScript->ip = _vm->_dialog->getChoiceScriptIp();
		_curScript->jump();
		debug(4, "*** dialog finished");
	} else {
 		_scriptBreakFlag = true;
 	}

}

void ScriptInterpreter::processScriptTalk() {

	if (_vm->_textActive == 0) {
		_curScript->status &= ~kScriptTalking;
		if (_vm->_sceneObjectIndex == 10) {
			if (_vm->_animIndex != -1)
				_vm->_sceneObjects[_vm->_animIndex].visible = true;
			_vm->_sceneObjects[10].flag = 0;
			_vm->_screen->enableTransitionEffect();
		} else if (_vm->_animIndex != -1) {
			SceneObject *sceneObject = _vm->getSceneObject(_vm->_sceneObjectIndex);
			_vm->sceneObjectSetAnimNumber(sceneObject, _vm->_animIndex);
			sceneObject->animSubIndex2 = _vm->_animSubIndex2;
			sceneObject->animFrameIndex = _vm->_animSubIndex;
			_vm->_animIndex = -1;
		}
		debug(4, "*** talking finished");
	} else {
		_scriptBreakFlag = true;
	}
	
}

void ScriptInterpreter::runScript(int scriptNumber) {

	//debug(3, "CometEngine::runScript(%d)", scriptNumber);

	_curScriptNumber = scriptNumber;
	_curScript = _scripts[scriptNumber];

	_scriptBreakFlag = false;

	if (_curScript->status & kScriptWalking)
		debug(2, "kScriptWalking %d", scriptNumber);
	if (_curScript->status & kScriptSleeping)
		debug(2, "kScriptSleeping %d", scriptNumber);
	if (_curScript->status & kScriptAnimPlaying)
		debug(2, "kScriptAnimPlaying %d", scriptNumber);
	if (_curScript->status & kScriptDialogRunning)
		debug(2, "kScriptDialogRunning %d", scriptNumber);
	if (_curScript->status & kScriptPaused)
		debug(2, "kScriptPaused %d", scriptNumber);
	if (_curScript->status & kScriptTalking)
		debug(2, "kScriptTalking %d", scriptNumber);

	if (_curScript->status & kScriptPaused)
		return;
		
	if (_curScript->status & 8)
		processScriptStatus8();

	if (_curScript->status & kScriptSleeping)
		processScriptSleep();

	if (_curScript->status & kScriptWalking)
		processScriptWalk();

	if (_curScript->status & kScriptAnimPlaying)
		processScriptAnim();

	if (_curScript->status & kScriptDialogRunning)
		processScriptDialog();

	if (_curScript->status & kScriptTalking)
		processScriptTalk();

	while (!_scriptBreakFlag) {
		byte opcode = *_curScript->ip++;

		debug(2, "[%02d:%08X] %d", _curScriptNumber, (uint32)(_curScript->ip - _curScript->code), opcode);

		if (opcode >= _opcodes.size())
			error("CometEngine::runScript() Unknown opcode %d", opcode);
			
		(*_opcodes[opcode])(_curScript);

	}

}

void ScriptInterpreter::runAllScripts() {
	// Run all scripts except the main script
	for (int scriptNumber = 1; scriptNumber < _scriptCount; scriptNumber++) {
		runScript(scriptNumber);
	}
}

int *ScriptInterpreter::getVarPointer(int varIndex) {

	if (varIndex < 1000) {
		assert(_vm->_scriptVars1[varIndex]);
		return _vm->_scriptVars1[varIndex];
	} else if (varIndex < 2000) {
		return &_vm->_scriptVars2[varIndex - 1000];
	} else {
		return &_vm->_itemStatus[varIndex - 2000];
	}
	
}

bool ScriptInterpreter::evalBoolOp(int value1, int value2, int boolOp) {
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
		error("ScriptInterpreter::evalBoolOp() Unknown bool operation code %d", boolOp);
	}
	return false;
}

SceneObject *ScriptInterpreter::getScriptSceneObject() {
	return _vm->getSceneObject(_curScript->objectIndex);
}

SceneObject *ScriptInterpreter::getSceneObject(int index) {
	return _vm->getSceneObject(index);
}

void ScriptInterpreter::objectWalkToXYAbs(Script *script, bool xyFlag) {
	
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

	debug(3, "objectWalkToXYAbs()  object: %d; old: %d, %d; new: %d, %d", script->objectIndex, script->object()->x, script->object()->y, x, y);

	if (_vm->sceneObjectUpdateDirection2(script->objectIndex, x, y)) {
		if (!xyFlag) {
			script->object()->walkStatus |= 8;
		} else {
			script->object()->walkStatus |= 0x10;
		}
		script->status |= kScriptWalking;
		_scriptBreakFlag = true;
	}
	
}

void ScriptInterpreter::objectWalkToXYRel(Script *script, bool xyFlag) {

	int x, y;
	int delta = script->loadByte();
	
	script->object()->directionChanged = 0;

	if (!xyFlag) {
		x = _vm->_sceneObjects[0].x + delta;
		y = script->object()->y;
	} else {
		x = script->object()->x;
		y = _vm->_sceneObjects[0].y + delta;
	}

	debug(3, "objectWalkToXYRel()  object: %d; old: %d, %d; new: %d, %d", script->objectIndex, script->object()->x, script->object()->y, x, y);

	if (_vm->sceneObjectUpdateDirection2(script->objectIndex, x, y)) {
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
void ScriptInterpreter::o1_addSceneItem(Script *script, int paramType) {

	int itemIndex, x, y;
	
	if (paramType == 0) {
		itemIndex = script->loadByte();
	} else {
		itemIndex = script->loadInt16();
	}
	
	x = script->loadByte() * 2;
	y = script->loadByte();
	
	debug(2, "o1_addSceneItem(%d, %d, %d)", itemIndex, x, y);
	
	SceneItem sceneItem;
	sceneItem.itemIndex = itemIndex;
	sceneItem.active = true;
	sceneItem.paramType = paramType;
	sceneItem.x = x;
	sceneItem.y = y;
	_vm->_sceneItems.push_back(sceneItem);

}

/* Script functions */

void ScriptInterpreter::o1_nop(Script *script) {
}

void ScriptInterpreter::o1_sceneObjectSetDirection(Script *script) {
	debug(2, "o1_sceneObjectSetDirection");

	int direction = script->loadByte();
	script->object()->directionChanged = 0;
	_vm->sceneObjectSetDirection(script->object(), direction);

}

void ScriptInterpreter::o1_break(Script *script) {
	_scriptBreakFlag = true;
}

void ScriptInterpreter::o1_jump(Script *script) {
	debug(2, "o1_jump()");
	
	script->jump();
}

void ScriptInterpreter::o1_objectWalkToXAbs(Script *script) {
	debug(2, "o1_objectWalkToXAbs()");

	objectWalkToXYAbs(script, false);
}

void ScriptInterpreter::o1_objectWalkToYAbs(Script *script) {
	debug(2, "o1_objectWalkToYAbs()");

	objectWalkToXYAbs(script, true);
}

void ScriptInterpreter::o1_loop(Script *script) {

	byte loopCount = script->ip[2];
	
	debug(2, "o1_loop(%d)", loopCount);
	
	script->counter++;
	
	if (script->counter < loopCount) {
		script->jump();
	} else {
		script->counter = 0;
		script->ip += 3;
	}
	
}

void ScriptInterpreter::o1_objectSetPosition(Script *script) {

	int x = script->loadByte() * 2;
	int y = script->loadByte();

	debug(2, "o1_objectSetPosition(%d, %d)", x, y);

	_vm->sceneObjectSetXY(script->objectIndex, x, y);

}

void ScriptInterpreter::o1_sleep(Script *script) {

	int sleepCount = script->loadByte();

	debug(2, "o1_sleep(%d)", sleepCount);

	script->scriptNumber = sleepCount;
	script->status |= kScriptSleeping;
	_scriptBreakFlag = true;
	
}

void ScriptInterpreter::o1_if(Script *script) {

	uint16 value1 = script->loadVarValue();
	byte boolOp = script->loadByte();
	uint16 value2 = script->loadValue();

	const char* boolOps[] = {"==", "!=", ">", ">=", "<", "<=", "&", "|"};
	debug(2, "o1_if %d %s %d", value1, boolOps[boolOp], value2);

	if (evalBoolOp(value1, value2, boolOp))
		script->ip += 2;
	else
		script->jump();

}

void ScriptInterpreter::o1_condJump2(Script *script) {
	if (!o1_Sub_rectCompare01(script)) {
		script->jump();
	} else {
		script->ip += 2;
	}
}

void ScriptInterpreter::o1_objectWalkToXRel(Script *script) {
	debug(2, "o1_objectWalkToXRel()");

	objectWalkToXYRel(script, false);
}

void ScriptInterpreter::o1_objectWalkToYRel(Script *script) {
	debug(2, "o1_objectWalkToYRel()");

	objectWalkToXYRel(script, true);
}

void ScriptInterpreter::o1_setMouseFlags(Script *script) {

	int flagIndex = script->loadByte();
	
	debug(2, "o1_setMouseFlags(%d)", flagIndex);

	if (flagIndex == 0) {
		_vm->_mouseCursor2 = 0;
		_vm->_mouseFlag = 15;
		_vm->sceneObjectResetDirectionAdd(getSceneObject(0));
	} else {
		const int constFlagsArray[5] = {0, 1, 8, 2, 4};
		_vm->_mouseFlag |= constFlagsArray[flagIndex];
	}
	
}

void ScriptInterpreter::o1_resetHeroDirectionChanged(Script *script) {
	debug(2, "o1_resetHeroDirectionChanged()");
	
	_vm->resetHeroDirectionChanged();
}

void ScriptInterpreter::o1_sceneObjectSetDirectionTo(Script *script) {

	SceneObject *playerObject = getSceneObject(0);
	
	int direction = _vm->calcDirection( script->object()->x, script->object()->y, playerObject->x, playerObject->y );
	script->object()->directionChanged = 0;
	_vm->sceneObjectSetDirection(script->object(), direction);
	
}

void ScriptInterpreter::o1_selectObject(Script *script) {

	int objectIndex = script->loadByte();
	
	debug(2, "o1_selectObject(%d)", objectIndex);
	
	script->objectIndex = objectIndex;
	
}

void ScriptInterpreter::o1_initPoints(Script *script) {
	debug(2, "o1_initPoints");

	_vm->initPoints(script->ip);
	script->ip += *script->ip * 2 + 1;

}

void ScriptInterpreter::o1_initSceneExits(Script *script) {
	debug(2, "o1_initSceneExits");

	_vm->initSceneExits(script->ip);
	script->ip += *script->ip * 5 + 1;

}

void ScriptInterpreter::o1_addSceneItem1(Script *script) {
	o1_addSceneItem(script, 0);
}

void ScriptInterpreter::o1_startScript(Script *script) {

	int scriptNumber = script->loadByte();

	debug(2, "o1_startScript(%d)", scriptNumber);

	if (scriptNumber < _scriptCount) {
		prepareScript(scriptNumber);
		_scripts[scriptNumber]->status &= ~kScriptPaused;
	}

}

void ScriptInterpreter::o1_pauseScript(Script *script) {

	int scriptNumber = script->loadByte();

	debug(2, "o1_pauseScript(%d)", scriptNumber);

	_scripts[scriptNumber]->status |= kScriptPaused;

}

void ScriptInterpreter::o1_playCutscene(Script *script) {

	int fileIndex = script->loadByte();
	int indexSubStart = script->loadByte();
	int index = script->loadInt16();
	int counterMax = script->loadByte();
	int cutsceneCounter2 = script->loadByte();
	
	//TODO
	//playCutscene( fileIndex, indexSubStart, index, counterMax, cutsceneCounter2, script->ip );
	
	script->ip += cutsceneCounter2 * 3;

}

void ScriptInterpreter::o1_setVar(Script *script) {

	int varIndex = script->loadInt16();
	int value = script->loadValue();

	debug(2, "o1_setVar(%d, %d)", varIndex, value);

	int *varPtr = getVarPointer(varIndex);
	*varPtr = value;
	
}

void ScriptInterpreter::o1_incVar(Script *script) {

	int varIndex = script->loadInt16();
	int value = script->loadValue();

	debug(2, "o1_incVar(%d, %d)", varIndex, value);

	int *varPtr = getVarPointer(varIndex);
	*varPtr += value;

}

void ScriptInterpreter::o1_subVar(Script *script) {

	int varIndex = script->loadInt16();
	int value = script->loadValue();

	debug(2, "o1_subVar(%d, %d)", varIndex, value);

	int *varPtr = getVarPointer(varIndex);
	*varPtr -= value;

}

void ScriptInterpreter::o1_setSceneObjectCollisionTypeTo8(Script *script) {

	debug(2, "o1_setSceneObjectCollisionTypeTo8()");

	script->object()->collisionType = 8;
}

void ScriptInterpreter::o1_setSceneObjectCollisionTypeTo0(Script *script) {

	debug(2, "o1_setSceneObjectCollisionTypeTo0()");

	script->object()->collisionType = 0;
}

void ScriptInterpreter::o1_updateDirection2(Script *script) {

	int x = script->loadByte() * 2;
	int y = script->loadByte();

	debug(2, "o1_updateDirection2(%d, %d)", x, y);
	
	script->object()->directionChanged = 0;
	
	if (_vm->sceneObjectUpdateDirection2(script->objectIndex, x, y)) {
		script->status |= kScriptWalking;
		_scriptBreakFlag = true;
	}

}

void ScriptInterpreter::o1_setSceneNumber(Script *script) {

	int sceneNumber = script->loadByte();

	debug(2, "o1_setSceneNumber(%d)", sceneNumber);

	if (sceneNumber == 0xFF) {
		_vm->_sceneNumber = -1;
	} else {
		_vm->_sceneNumber = sceneNumber;
	}

}

void ScriptInterpreter::o1_setAnimValues(Script *script) {

	int animIndex = script->loadByte();
	int animFrameIndex = script->loadByte();

	debug(2, "o1_setAnimValues(%d, %d)", animIndex, animFrameIndex);

	script->object()->animIndex = animIndex;
	script->object()->animFrameIndex = animFrameIndex;
	script->object()->animSubIndex2 = animFrameIndex;
	script->object()->directionChanged = 2;
	
}

void ScriptInterpreter::o1_setMarcheNumber(Script *script) {
	_vm->_marcheNumber = script->loadByte();
}

void ScriptInterpreter::o1_setZoomByItem(Script *script) {

	int objectIndex = script->loadByte();
	int zoomFactor = script->loadByte();

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	_vm->_screen->setZoom(zoomFactor, sceneObject->x, sceneObject->y);
	
}

void ScriptInterpreter::o1_startDialog(Script *script) {

	_vm->_dialog->run(script);

	script->status |= kScriptDialogRunning;
	_scriptBreakFlag = true;

	//TODO: waitForKey();
	
}

void ScriptInterpreter::o1_waitWhilePlayerIsInRect(Script *script) {

	debug(2, "o1_waitWhilePlayerIsInRect(%d, %d, %d, %d)", script->x, script->y, script->x2, script->y2);

	//DEBUG
	_vm->_screen->fillRect(script->x, script->y, script->x2, script->y2, 60);

	if (_vm->isPlayerInRect(script->x, script->y, script->x2, script->y2)) {
		script->ip--;
		_scriptBreakFlag = true;
	}

}

void ScriptInterpreter::o1_waitUntilPlayerIsInRect(Script *script) {

	script->x = script->loadByte() * 2;
	script->y = script->loadByte();
	script->x2 = script->loadByte() * 2;
	script->y2 = script->loadByte();

	debug(2, "o1_waitUntilPlayerIsInRect(%d, %d, %d, %d)", script->x, script->y, script->x2, script->y2);

	//DEBUG
	_vm->_screen->fillRect(script->x, script->y, script->x2, script->y2, 70);

	if (!_vm->isPlayerInRect(script->x, script->y, script->x2, script->y2)) {
		script->ip -= 5;
		_scriptBreakFlag = true;
	}

}

void ScriptInterpreter::o1_unloadSceneObjectSprite(Script *script) {
	debug(2, "o1_unloadSceneObjectSprite");

	if (script->objectIndex != 0) {
		script->object()->flag = 0;
		if (script->object()->marcheIndex != -1)
			_vm->unloadSceneObjectSprite(script->object());
	}

}

void ScriptInterpreter::o1_setObjectClipX(Script *script) {
	
	int x5 = script->loadByte() * 2;
	int x6 = script->loadByte() * 2;

	debug(2, "o1_setObjectClipX(%d, %d)", x5, x6);
	
	script->object()->x5 = x5;
	script->object()->x6 = x6;

}

void ScriptInterpreter::o1_setObjectClipY(Script *script) {

	int y5 = script->loadByte();
	int y6 = script->loadByte();

	debug(2, "o1_setObjectClipY(%d, %d)", y5, y6);

	script->object()->y5 = y5;
	script->object()->y6 = y6;

}

void ScriptInterpreter::o1_orVar(Script *script) {
	debug(2, "o1_orVar");

	int varIndex = script->loadInt16();
	int value = script->loadValue();
	int *varPtr = getVarPointer(varIndex);
	*varPtr |= value;
	
}

void ScriptInterpreter::o1_loadScene(Script *script) {

	_vm->_backgroundFileIndex = script->loadByte() * 2;

	debug(2, "o1_loadScene(%d)", _vm->_backgroundFileIndex);

	_vm->initSceneBackground();

}

void ScriptInterpreter::o1_addBlockingRect(Script *script) {

	int x = script->loadByte();
	int y = script->loadByte();
	int x2 = script->loadByte();
	int y2 = script->loadByte();

	debug(2, "o1_addBlockingRect(%d,%d,%d,%d)", x, y, x2, y2);

	_vm->addBlockingRect(x, y, x2, y2);

}

void ScriptInterpreter::o1_sub_A67F(Script *script) {

	int objectIndex = script->loadByte();

	if (_vm->_cmdTalk && _vm->rectCompare02(0, objectIndex, 40, 40)) {
		script->ip += 2;
		_vm->_cmdTalk = false;
	} else {
		script->jump();
	}

}

void ScriptInterpreter::o1_sub_A64B(Script *script) {

	debug(2, "o1_sub_A64B()");

	if (_vm->_cmdTalk) {
		if (o1_Sub_rectCompare01(script)) {
			script->ip += 2;
			_vm->_cmdTalk = false;
		} else {
			script->jump();
		}
	} else {
		script->ip += 4;
		script->jump();
	}

}

bool ScriptInterpreter::o1_Sub_rectCompare01(Script *script) {

	SceneObject *sceneObject = getSceneObject(0);

	script->x = script->loadByte() * 2;
	script->y = script->loadByte();
	script->x2 = script->loadByte() * 2;
	script->y2 = script->loadByte();
	
	//DEBUG
	_vm->_screen->fillRect(script->x, script->y, script->x2, script->y2, 50);
	
	Common::Rect rect1(script->x, script->y, script->x2, script->y2);
	Common::Rect rect2(
		sceneObject->x - sceneObject->deltaX,
		sceneObject->y - sceneObject->deltaY,
		sceneObject->x + sceneObject->deltaX,
		sceneObject->y);
	
	return _vm->rectCompare(rect1, rect2);

}

void ScriptInterpreter::o1_sub_A735(Script *script) {
	// LOOK AT handler

	debug(2, "o1_sub_A735()");

	if (_vm->_cmdLook) {
		if (o1_Sub_rectCompare01(script)) {
			script->ip += 2;
			_vm->_cmdLook = false;
		} else {
			script->jump();
		}
	} else {
		script->ip += 4;
		script->jump();
	}

}

void ScriptInterpreter::o1_removeBlockingRect(Script *script) {

	int x = script->loadByte() * 2;
	int y = script->loadByte();
	
	for (int i = _vm->_blockingRects.size() - 1; i >= 0; i--) {
		if (_vm->_blockingRects[i].left == x && _vm->_blockingRects[i].top == y) {
			_vm->_blockingRects.remove_at(i);
			break;
		}
	}

}

void ScriptInterpreter::o1_setSceneObjectColor(Script *script) {
	debug(2, "o1_setSceneObjectColor");

	script->object()->color = script->loadByte();
	
}

void ScriptInterpreter::o1_setTextXY(Script *script) {

	int textX = script->loadByte() * 2;
	int textY = script->loadByte();

	debug(2, "o1_setTextXY(%d, %d)", textX, textY);

	script->object()->textX = textX;
	script->object()->textY = textY;
	
}

void ScriptInterpreter::o1_playMusic(Script *script) {

	int fileIndex = script->loadByte();

	debug(2, "o1_playMusic(%d)", fileIndex);

	if (fileIndex != 0xFF) {
		_vm->_music->playMusic(fileIndex);
	} else {
		//TODO: musicFadeDown();
		_vm->_music->stopMusic();
	}

}

void ScriptInterpreter::o1_setRandomValue(Script *script) {
	debug(2, "o1_setRandomValue()");
	_vm->_scriptRandomValue = _vm->random(script->loadByte());
}

void ScriptInterpreter::o1_setChapterNumber(Script *script) {
	_vm->_chapterNumber = script->loadByte();
	debug(2, "o1_setChapterNumber(%d)", _vm->_chapterNumber);
	_scriptBreakFlag = true;
}

void ScriptInterpreter::o1_dialog(Script *script) {

	int objectIndex = script->loadByte();
	int narSubIndex = script->loadInt16();
	int animNumber = script->loadByte();

	debug(2, "o1_dialog(%d, %d, %d)", objectIndex, narSubIndex, animNumber);

	_vm->textProc2(objectIndex, narSubIndex, animNumber);

	_curScript->status |= kScriptTalking;
	_scriptBreakFlag = true;

}

void ScriptInterpreter::o1_addSceneItem2(Script *script) {
	o1_addSceneItem(script, 1);
}

void ScriptInterpreter::o1_playAnim(Script *script) {
	debug(2, "o1_playAnim");

	o1_sceneObjectSetAnimNumber(script);
	script->status |= kScriptAnimPlaying;
	_scriptBreakFlag = true;

}

void ScriptInterpreter::o1_sceneObjectSetAnimNumber(Script *script) {

	int animIndex = script->loadByte();

	debug(2, "o1_sceneObjectSetAnimNumber(%d)", animIndex);

	_vm->sceneObjectSetAnimNumber(script->object(), animIndex);
	script->object()->directionChanged = 2;

}

void ScriptInterpreter::o1_sub_AD04(Script *script) {

	int objectIndex = script->loadByte();
	int narSubIndex = script->loadInt16();
	int animNumber = script->loadByte();
	int fileIndex = script->loadByte();
	
	debug(2, "o1_sub_AD04(%d, %d, %d, %d)", objectIndex, narSubIndex, animNumber, fileIndex);
	//_system->delayMillis(5000);

	int marcheIndex = _vm->loadMarche(_vm->_marcheNumber, fileIndex);
	_vm->sceneObjectInit(10, marcheIndex);
	
	if (objectIndex != -1) {
		_vm->_sceneObjects[10].textX = 0;
		_vm->_sceneObjects[10].textY = 160;
		_vm->_sceneObjects[10].color = getSceneObject(objectIndex)->color;
	}
	
	_vm->_marcheNumber = 0;
	_vm->sceneObjectSetXY(10, 0, 199);
	_vm->textProc2(10, narSubIndex, animNumber);
	_vm->_animIndex = objectIndex;
	_vm->_screen->enableTransitionEffect();

	_curScript->status |= kScriptTalking;
	_scriptBreakFlag = true;

}

void ScriptInterpreter::o1_initSceneObject(Script *script) {
	debug(2, "o1_initSceneObject");

	o1_selectObject(script);

	if (script->objectIndex != 0) {
		_vm->sceneObjectInit(script->objectIndex, -1);
	}

}

void ScriptInterpreter::o1_loadSceneObjectSprite(Script *script) {

	int fileIndex = script->loadByte();
	
	debug(2, "o1_loadSceneObjectSprite(%d)", fileIndex);

	_vm->unloadSceneObjectSprite(script->object());
	
	script->object()->marcheIndex = _vm->loadMarche(_vm->_marcheNumber, fileIndex);
	_vm->_marcheNumber = 0;
	
}

void ScriptInterpreter::o1_setObjectVisible(Script *script) {
	int visible = script->loadByte();
	debug(2, "o1_setObjectVisible(%d)", visible);
	script->object()->visible = visible;
}

void ScriptInterpreter::o1_paletteFadeIn(Script *script) {
	int paletteValue = script->loadByte();
	debug(2, "o1_paletteFadeIn(%d)", paletteValue);
	_vm->_screen->setFadeType(kFadeIn);
	_vm->_screen->setFadeValue(paletteValue);
}

void ScriptInterpreter::o1_paletteFadeOut(Script *script) {
	int paletteValue = script->loadByte();
	debug(2, "o1_paletteFadeOut(%d)", paletteValue);
	_vm->_screen->setFadeType(kFadeOut);
	_vm->_screen->setFadeValue(paletteValue);
}

void ScriptInterpreter::o1_setNarFileIndex(Script *script) {

	_vm->_narFileIndex = script->loadByte();

	debug(2, "o1_setNarFileIndex(%d)", _vm->_narFileIndex);

	_vm->openVoiceFile(_vm->_narFileIndex);
	
}

void ScriptInterpreter::o1_deactivateSceneItem(Script *script) {

	int itemIndex = script->loadByte();

	uint index = 0;
	while (index < _vm->_sceneItems.size()) {
		if (_vm->_sceneItems[index].itemIndex == itemIndex) {
			_vm->_sceneItems.remove_at(index);
		} else {
			index++;
		}
	}

}

void ScriptInterpreter::o1_sample_2(Script *script) {
	debug(2, "o1_sample_2");

	int narFileIndex = script->loadByte();
	
	//TODO: seg010:074A
	
}

void ScriptInterpreter::o1_sample_1(Script *script) {
	debug(2, "o1_sample_1");

	int narFileIndex = script->loadByte();
	int value = script->loadByte();
	
	//TODO


}

} // End of namespace Comet
