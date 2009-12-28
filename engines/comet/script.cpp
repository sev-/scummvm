#include "comet/comet.h"
#include "comet/music.h"
#include "comet/screen.h"
#include "comet/script.h"
#include "comet/dialog.h"
#include "comet/scene.h"

namespace Comet {

/* Script */

byte Script::readByte() {
	return *ip++;
}

int16 Script::readInt16() {
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
	int varIndex = readInt16();
	return *_inter->getVarPointer(varIndex);
}

uint16 Script::loadValue() {
	byte type = readByte();
	switch (type) {
	case 11:
		return loadVarValue();
	case 12:
		return readByte();
	default:
		return readInt16();
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
	// 0
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_sceneObjectSetDirection);
	RegisterOpcode(o1_break);
	RegisterOpcode(o1_jump);
	RegisterOpcode(o1_objectWalkToX);
	// 5
	RegisterOpcode(o1_objectWalkToY);
	RegisterOpcode(o1_loop);
	RegisterOpcode(o1_objectSetPosition);
	RegisterOpcode(o1_synchronize);
	RegisterOpcode(o1_sleep);
	// 10
	RegisterOpcode(o1_if);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_ifHeroInZone);
	// 15
	RegisterOpcode(o1_objectWalkToXRel);
	RegisterOpcode(o1_objectWalkToYRel);
	RegisterOpcode(o1_objectWalkToXYRel);
	RegisterOpcode(o1_blockInput);
	RegisterOpcode(o1_unblockInput);
	// 20
	RegisterOpcode(o1_sceneObjectSetDirectionToHero);
	RegisterOpcode(o1_selectObject);
	RegisterOpcode(o1_initSceneBounds);
	RegisterOpcode(o1_initSceneExits);
	RegisterOpcode(o1_nop);
	// 25
	RegisterOpcode(o1_addSceneObject);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_startScript);
	RegisterOpcode(o1_stopScript);
	RegisterOpcode(o1_startMultipleScripts);
	// 30
	RegisterOpcode(o1_playCutscene);
	RegisterOpcode(o1_setVar);
	RegisterOpcode(o1_incVar);
	RegisterOpcode(o1_subVar);
	RegisterOpcode(o1_sceneObjectDisableCollisions);
	// 35
	RegisterOpcode(o1_sceneObjectEnableCollisions);
	RegisterOpcode(o1_objectWalkTo);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_setPaletteBrightness);
	RegisterOpcode(o1_waitUntilHeroExitZone);
	// 40
	RegisterOpcode(o1_waitUntilHeroEnterZone);
	RegisterOpcode(o1_sceneObjectDelete);
	RegisterOpcode(o1_setObjectClipX);
	RegisterOpcode(o1_setObjectClipY);
	RegisterOpcode(o1_setSceneNumber);
	// 45
	RegisterOpcode(o1_setAnimValues);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_setMarcheNumber);
	RegisterOpcode(o1_heroIncPositionY);
	RegisterOpcode(o1_nop);
	// 50
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_setZoom);
	RegisterOpcode(o1_setZoomByItem);
	RegisterOpcode(o1_startDialog);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	// 55
	RegisterOpcode(o1_clearScreen);
	RegisterOpcode(o1_orVar);
	RegisterOpcode(o1_andVar);
	RegisterOpcode(o1_loadScene);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	// 60
	RegisterOpcode(o1_addBlockingRect);
	RegisterOpcode(o1_ifSpeak);
	RegisterOpcode(o1_ifSpeakTo);
	RegisterOpcode(o1_ifSpeakZone);
	RegisterOpcode(o1_ifLook);
	// 65
	RegisterOpcode(o1_ifLookAt);
	RegisterOpcode(o1_ifLookZone);
	RegisterOpcode(o1_addBeam);//TODO
	RegisterOpcode(o1_removeBlockingRect);
	RegisterOpcode(o1_objectSetTextColor);
	// 70
	RegisterOpcode(o1_setTextXY);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_breakLoop);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_nop); // Unused in Comet CD
	// 75
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_playMusic);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_setRandomValue);
	// 80
	RegisterOpcode(o1_setChapterNumber);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_actorTalk);
	RegisterOpcode(o1_loadSavegame);
	RegisterOpcode(o1_addSceneItem2); // Unused in Comet CD
	// 85
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_nop);// TODO: o1_waitForKey();
	RegisterOpcode(o1_playAnim);
	RegisterOpcode(o1_sceneObjectSetAnimNumber);
	RegisterOpcode(o1_actorTalkPortrait);
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

void ScriptInterpreter::initializeScriptAfterLoadGame() {

	debug(2, "CometEngine::initializeScriptAfterLoadGame()  _scriptCount = %d", _scriptCount);

	for (int scriptNumber = 0; scriptNumber < _scriptCount; scriptNumber++) {
		Script *script = _scripts[scriptNumber];
		uint16 ofs = READ_LE_UINT16(_scriptData + scriptNumber * 2);
		script->code = _scriptData + ofs;
		script->ip = script->code + script->resumeIp;
	}

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

void ScriptInterpreter::processScriptSynchronize() {

	//debug(3, "######## processScriptSynchronize()");

	int scriptNumber = _curScript->scriptNumber;
	Script *script = _scripts[scriptNumber];
	
	if ((script->status & kScriptSynchronize) && script->scriptNumber == _curScriptNumber) {
		script->status &= ~kScriptSynchronize;
		script->scriptNumber = 0;
		_curScript->status &= ~kScriptSynchronize;
		_curScript->scriptNumber = 0;
	} else {
		_yield = true;
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
		_yield = true;
	}

}

void ScriptInterpreter::processScriptWalk() {

	debug(3, "######## processScriptWalk()  objectIndex = %d", _curScript->objectIndex);

	debug(2, "CometEngine::processScriptWalk() walkStatus = %d; life = %d",
		_curScript->object()->walkStatus, _curScript->object()->life);

	if ((_curScript->object()->walkStatus & 3) == 0 || _curScript->object()->life == 0) {
		_curScript->status &= ~kScriptWalking;
		_vm->sceneObjectSetAnimNumber(_curScript->object(), 0);
		debug(4, "*** walking finished");
	} else {
		_yield = true;
	}

}

void ScriptInterpreter::processScriptAnim() {

	if (_curScript->object()->animFrameIndex + 1 == _curScript->object()->animFrameCount) {
		_curScript->status &= ~kScriptAnimPlaying;
		debug(4, "*** anim playing finished");
	} else {
		_yield = true;
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
 		_yield = true;
 	}

}

void ScriptInterpreter::processScriptTalk() {

	if (_vm->_textActive == 0) {
		_curScript->status &= ~kScriptTalking;
		if (_vm->_talkActorIndex == 10) {
			if (_vm->_animIndex != -1)
				_vm->_sceneObjects[_vm->_animIndex].visible = true;
			_vm->_sceneObjects[10].life = 0;
			_vm->_screen->enableTransitionEffect();
		} else if (_vm->_animIndex != -1) {
			SceneObject *sceneObject = _vm->getSceneObject(_vm->_talkActorIndex);
			_vm->sceneObjectSetAnimNumber(sceneObject, _vm->_animIndex);
			sceneObject->animSubIndex2 = _vm->_animSubIndex2;
			sceneObject->animFrameIndex = _vm->_animSubIndex;
			_vm->_animIndex = -1;
		}
		debug(4, "*** talking finished");
	} else {
		_yield = true;
	}
	
}

void ScriptInterpreter::runScript(int scriptNumber) {

	//debug(3, "CometEngine::runScript(%d)", scriptNumber);

	_curScriptNumber = scriptNumber;
	_curScript = _scripts[scriptNumber];

	_yield = false;

	if (_curScript->status & kScriptWalking)
		debug(2, "kScriptWalking %d", scriptNumber);
	if (_curScript->status & kScriptSleeping)
		debug(2, "kScriptSleeping %d", scriptNumber);
	if (_curScript->status & kScriptAnimPlaying)
		debug(2, "kScriptAnimPlaying %d", scriptNumber);
	if (_curScript->status & kScriptSynchronize)
		debug(2, "kScriptSynchronize %d", scriptNumber);
	if (_curScript->status & kScriptDialogRunning)
		debug(2, "kScriptDialogRunning %d", scriptNumber);
	if (_curScript->status & kScriptPaused)
		debug(2, "kScriptPaused %d", scriptNumber);
	if (_curScript->status & kScriptTalking)
		debug(2, "kScriptTalking %d", scriptNumber);

	if (_curScript->status & kScriptPaused)
		return;
		
	if (_curScript->status & kScriptSynchronize)
		processScriptSynchronize();

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

	while (!_yield) {
		byte opcode = *_curScript->ip++;

		/* DEBUG: So that o1_nop can print the opcode
			This will be removed again once all opcodes are implemented.
		*/
		_curScript->debugOpcode = opcode;

		debug(2, "[%02d:%08X] %d", _curScriptNumber, (uint32)(_curScript->ip - _curScript->code), opcode);

		if (opcode >= _opcodes.size())
			error("CometEngine::runScript() Invalid opcode %d", opcode);
			
		(*_opcodes[opcode])(_curScript);

	}

}

void ScriptInterpreter::runAllScripts() {
	debug(2, "ScriptInterpreter::runAllScripts()");
	// Run all scripts except the main script
	for (int scriptNumber = 1; scriptNumber < _scriptCount; scriptNumber++) {
		runScript(scriptNumber);
	}
}

int16 *ScriptInterpreter::getVarPointer(int varIndex) {

	if (varIndex < 1000) {
		assert(_vm->_systemVars[varIndex]);
		return _vm->_systemVars[varIndex];
	} else if (varIndex < 2000) {
		return &_vm->_scriptVars[varIndex - 1000];
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

void ScriptInterpreter::objectWalkToXY(Script *script, bool xyFlag) {
	
	int x, y;
	int newValue = script->readByte();
	
	script->object()->directionChanged = 0;

	if (!xyFlag) {
		x = newValue * 2;
		y = script->object()->y;
	} else {
		x = script->object()->x;
		y = newValue;
	}

	debug(3, "objectWalkToXY()  object: %d; old: %d, %d; new: %d, %d", script->objectIndex, script->object()->x, script->object()->y, x, y);

	if (_vm->sceneObjectStartWalking(script->objectIndex, x, y)) {
		if (!xyFlag) {
			script->object()->walkStatus |= 8;
		} else {
			script->object()->walkStatus |= 0x10;
		}
		script->status |= kScriptWalking;
		_yield = true;
	}
	
}

void ScriptInterpreter::objectWalkToXYRel(Script *script, bool xyFlag) {

	int x, y;
	int delta = script->readByte();
	
	script->object()->directionChanged = 0;

	if (!xyFlag) {
		x = _vm->_sceneObjects[0].x + delta;
		y = script->object()->y;
	} else {
		x = script->object()->x;
		y = _vm->_sceneObjects[0].y + delta;
	}

	debug(3, "objectWalkToXYRel()  object: %d; old: %d, %d; new: %d, %d", script->objectIndex, script->object()->x, script->object()->y, x, y);

	if (_vm->sceneObjectStartWalking(script->objectIndex, x, y)) {
		if (!xyFlag) {
			script->object()->walkStatus |= 8;
		} else {
			script->object()->walkStatus |= 0x10;
		}
		script->status |= kScriptWalking;
		_yield = true;
	}

}

// TODO: Decouple from Script
void ScriptInterpreter::o1_addSceneItem(Script *script, int paramType) {

	int itemIndex, x, y;
	
	if (paramType == 0) {
		itemIndex = script->readByte();
	} else {
		itemIndex = script->readInt16();
	}
	
	x = script->readByte() * 2;
	y = script->readByte();
	
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
	debug("Unimplemented opcode %02X", script->debugOpcode);
}

void ScriptInterpreter::o1_sceneObjectSetDirection(Script *script) {
	debug(2, "o1_sceneObjectSetDirection");

	int direction = script->readByte();
	script->object()->directionChanged = 0;
	_vm->sceneObjectSetDirection(script->object(), direction);

}

void ScriptInterpreter::o1_break(Script *script) {
	script->status |= kScriptPaused;
	_yield = true;
}

void ScriptInterpreter::o1_jump(Script *script) {
	debug(2, "o1_jump()");
	
	script->jump();
}

void ScriptInterpreter::o1_objectWalkToX(Script *script) {
	debug(2, "o1_objectWalkToX()");

	objectWalkToXY(script, false);
}

void ScriptInterpreter::o1_objectWalkToY(Script *script) {
	debug(2, "o1_objectWalkToY()");

	objectWalkToXY(script, true);
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

	int x = script->readByte() * 2;
	int y = script->readByte();

	debug(2, "o1_objectSetPosition(%d, %d)", x, y);

	_vm->sceneObjectSetPosition(script->objectIndex, x, y);

}

void ScriptInterpreter::o1_synchronize(Script *script) {
	script->scriptNumber = script->readByte();
	script->status |= kScriptSynchronize;
	_yield = true;
}

void ScriptInterpreter::o1_sleep(Script *script) {

	int sleepCount = script->readByte();

	debug(2, "o1_sleep(%d)", sleepCount);

	script->scriptNumber = sleepCount;
	script->status |= kScriptSleeping;
	_yield = true;
	
}

void ScriptInterpreter::o1_if(Script *script) {

	uint16 value1 = script->loadVarValue();
	byte boolOp = script->readByte();
	uint16 value2 = script->loadValue();

	const char* boolOps[] = {"==", "!=", ">", ">=", "<", "<=", "&", "|"};
	debug(2, "o1_if %d %s %d", value1, boolOps[boolOp], value2);

	if (evalBoolOp(value1, value2, boolOp))
		script->ip += 2;
	else
		script->jump();

}

void ScriptInterpreter::o1_ifHeroInZone(Script *script) {
	if (!isHeroInZone(script)) {
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

void ScriptInterpreter::o1_objectWalkToXYRel(Script *script) {
	debug(2, "o1_objectWalkToXYRel()");

	SceneObject *playerObject = getSceneObject(0);
	SceneObject *sceneObject = script->object();
	int deltaX = script->readByte();
	int deltaY = script->readByte();
	int x = playerObject->x + deltaX;
	int y = playerObject->y + deltaY;

	sceneObject->directionChanged = 0;
	_vm->_scene->superFilterWalkDestXY(x, y, sceneObject->deltaX, sceneObject->deltaY);
	sceneObject->walkStatus = 0;
	if (_vm->sceneObjectStartWalking(script->objectIndex, x, y)) {
		script->status |= kScriptWalking;
		_yield = true;
	}

}

void ScriptInterpreter::o1_blockInput(Script *script) {

	int flagIndex = script->readByte();
	
	debug(2, "o1_blockInput(%d)", flagIndex);

	if (flagIndex == 0) {
		_vm->_mouseCursor2 = 0;
		_vm->_mouseFlag = 15;
		_vm->sceneObjectStopWalking(getSceneObject(0));
	} else {
		const int constFlagsArray[5] = {0, 1, 8, 2, 4};
		_vm->_mouseFlag |= constFlagsArray[flagIndex];
	}
	
}

void ScriptInterpreter::o1_unblockInput(Script *script) {
	debug(2, "o1_unblockInput()");
	
	_vm->resetHeroDirectionChanged();
}

void ScriptInterpreter::o1_sceneObjectSetDirectionToHero(Script *script) {

	SceneObject *playerObject = getSceneObject(0);
	
	int direction = _vm->calcDirection( script->object()->x, script->object()->y, playerObject->x, playerObject->y );
	script->object()->directionChanged = 0;
	_vm->sceneObjectSetDirection(script->object(), direction);
	
}

void ScriptInterpreter::o1_selectObject(Script *script) {

	int objectIndex = script->readByte();
	
	debug(2, "o1_selectObject(%d)", objectIndex);
	
	script->objectIndex = objectIndex;
	
}

void ScriptInterpreter::o1_initSceneBounds(Script *script) {
	debug(2, "o1_initSceneBounds");

	_vm->_scene->initPoints(script->ip);
	script->ip += *script->ip * 2 + 1;

}

void ScriptInterpreter::o1_initSceneExits(Script *script) {
	debug(2, "o1_initSceneExits");

	_vm->_scene->initSceneExits(script->ip);
	script->ip += *script->ip * 5 + 1;

}

void ScriptInterpreter::o1_addSceneObject(Script *script) {
	o1_addSceneItem(script, 0);
}

void ScriptInterpreter::o1_startScript(Script *script) {

	int scriptNumber = script->readByte();

	debug(2, "o1_startScript(%d)", scriptNumber);

	if (scriptNumber < _scriptCount) {
		prepareScript(scriptNumber);
		_scripts[scriptNumber]->status &= ~kScriptPaused;
	}

}

void ScriptInterpreter::o1_stopScript(Script *script) {

	int scriptNumber = script->readByte();

	debug(2, "o1_stopScript(%d)", scriptNumber);

	_scripts[scriptNumber]->status |= kScriptPaused;

}

void ScriptInterpreter::o1_startMultipleScripts(Script *script) {
	int scriptNumber;
	while ((scriptNumber = script->readByte()) != 0xFF) {
		prepareScript(scriptNumber);
		_scripts[scriptNumber]->status &= ~kScriptPaused;
	}
}

void ScriptInterpreter::o1_playCutscene(Script *script) {

	int fileIndex = script->readByte();
	int indexSubStart = script->readByte();
	int index = script->readInt16();
	int counterMax = script->readByte();
	int cutsceneCounter2 = script->readByte();
	
	//TODO
	//playCutscene( fileIndex, indexSubStart, index, counterMax, cutsceneCounter2, script->ip );
	
	script->ip += cutsceneCounter2 * 3;

}

void ScriptInterpreter::o1_setVar(Script *script) {

	int varIndex = script->readInt16();
	int value = script->loadValue();

	debug(2, "o1_setVar(%d, %d)", varIndex, value);

	int16 *varPtr = getVarPointer(varIndex);
	*varPtr = value;
	
}

void ScriptInterpreter::o1_incVar(Script *script) {

	int varIndex = script->readInt16();
	int value = script->loadValue();

	debug(2, "o1_incVar(%d, %d)", varIndex, value);

	int16 *varPtr = getVarPointer(varIndex);
	*varPtr += value;

}

void ScriptInterpreter::o1_subVar(Script *script) {

	int varIndex = script->readInt16();
	int value = script->loadValue();

	debug(2, "o1_subVar(%d, %d)", varIndex, value);

	int16 *varPtr = getVarPointer(varIndex);
	*varPtr -= value;

}

void ScriptInterpreter::o1_sceneObjectDisableCollisions(Script *script) {

	debug(2, "o1_sceneObjectDisableCollisions()");

	script->object()->collisionType = 8;
}

void ScriptInterpreter::o1_sceneObjectEnableCollisions(Script *script) {

	debug(2, "o1_sceneObjectEnableCollisions()");

	script->object()->collisionType = 0;
}

void ScriptInterpreter::o1_objectWalkTo(Script *script) {

	int x = script->readByte() * 2;
	int y = script->readByte();

	debug(2, "o1_walkTo(%d, %d)", x, y);
	
	script->object()->directionChanged = 0;
	
	if (_vm->sceneObjectStartWalking(script->objectIndex, x, y)) {
		script->status |= kScriptWalking;
		_yield = true;
	}

}

void ScriptInterpreter::o1_setPaletteBrightness(Script *script) {
	_vm->_paletteBrightness = script->readByte();
	_vm->_screen->buildPalette(_vm->_ctuPal, _vm->_palette, _vm->_paletteBrightness);
	_vm->_screen->setFullPalette(_vm->_palette);
}

void ScriptInterpreter::o1_setSceneNumber(Script *script) {

	int sceneNumber = script->readByte();

	debug(2, "o1_setSceneNumber(%d)", sceneNumber);

	if (sceneNumber == 0xFF) {
		_vm->_sceneNumber = -1;
	} else {
		_vm->_sceneNumber = sceneNumber;
	}

}

void ScriptInterpreter::o1_setAnimValues(Script *script) {

	int animIndex = script->readByte();
	int animFrameIndex = script->readByte();

	debug(2, "o1_setAnimValues(%d, %d)", animIndex, animFrameIndex);

	script->object()->animIndex = animIndex;
	script->object()->animFrameIndex = animFrameIndex;
	script->object()->animSubIndex2 = animFrameIndex;
	script->object()->directionChanged = 2;
	
}

void ScriptInterpreter::o1_setMarcheNumber(Script *script) {
	_vm->_marcheNumber = script->readByte();
	
	debug(2, "o1_setMarcheNumber(%d)", _vm->_marcheNumber);
	
}

void ScriptInterpreter::o1_heroIncPositionY(Script *script) {
	SceneObject *playerObject = getSceneObject(0);
	_vm->sceneObjectSetPosition(script->objectIndex, playerObject->x, playerObject->y + 1);
}

void ScriptInterpreter::o1_setZoom(Script *script) {

	int zoomX = script->readByte() * 2;
	int zoomY = script->readByte();
	int zoomFactor = script->readByte();

	_vm->_screen->setZoom(zoomFactor, zoomX, zoomY);

}

void ScriptInterpreter::o1_setZoomByItem(Script *script) {

	int objectIndex = script->readByte();
	int zoomFactor = script->readByte();

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	_vm->_screen->setZoom(zoomFactor, sceneObject->x, sceneObject->y);
	
}

void ScriptInterpreter::o1_startDialog(Script *script) {
	_vm->_dialog->run(script);
	_vm->waitForKeys();
	script->status |= kScriptDialogRunning;
	_yield = true;
}

void ScriptInterpreter::o1_waitUntilHeroExitZone(Script *script) {

	debug(2, "o1_waitUntilHeroExitZone(%d, %d, %d, %d)", script->x, script->y, script->x2, script->y2);

	if (_vm->_debugRectangles)
		_vm->_screen->fillRect(script->x, script->y, script->x2, script->y2, 60);

	if (_vm->isPlayerInZone(script->x, script->y, script->x2, script->y2)) {
		script->ip--;
		_yield = true;
	}

}

void ScriptInterpreter::o1_waitUntilHeroEnterZone(Script *script) {

	script->x = script->readByte() * 2;
	script->y = script->readByte();
	script->x2 = script->readByte() * 2;
	script->y2 = script->readByte();

	debug(2, "o1_waitUntilHeroEnterZone(%d, %d, %d, %d)", script->x, script->y, script->x2, script->y2);

	if (_vm->_debugRectangles)
		_vm->_screen->fillRect(script->x, script->y, script->x2, script->y2, 70);

	if (!_vm->isPlayerInZone(script->x, script->y, script->x2, script->y2)) {
		script->ip -= 5;
		_yield = true;
	}

}

void ScriptInterpreter::o1_sceneObjectDelete(Script *script) {
	debug(2, "o1_sceneObjectDelete");

	if (script->objectIndex != 0) {
		script->object()->life = 0;
		if (script->object()->marcheIndex != -1)
			_vm->unloadSceneObjectSprite(script->object());
	}

}

void ScriptInterpreter::o1_setObjectClipX(Script *script) {
	
	int16 clipX1 = script->readByte() * 2;
	int16 clipX2 = script->readByte() * 2;

	debug(2, "o1_setObjectClipX(%d, %d)", clipX1, clipX2);
	
	script->object()->clipX1 = clipX1;
	script->object()->clipX2 = clipX2;

}

void ScriptInterpreter::o1_setObjectClipY(Script *script) {

	int16 clipY1 = script->readByte();
	int16 clipY2 = script->readByte();

	debug(2, "o1_setObjectClipY(%d, %d)", clipY1, clipY2);

	script->object()->clipY1 = clipY1;
	script->object()->clipY2 = clipY2;

}

void ScriptInterpreter::o1_clearScreen(Script *script) {
	_vm->_clearScreenRequest = true;
}

void ScriptInterpreter::o1_orVar(Script *script) {
	int varIndex = script->readInt16();
	int value = script->loadValue();
	int16 *varPtr = getVarPointer(varIndex);
	*varPtr |= value;
}

void ScriptInterpreter::o1_andVar(Script *script) {
	int varIndex = script->readInt16();
	int value = script->loadValue();
	int16 *varPtr = getVarPointer(varIndex);
	*varPtr &= value;
}

void ScriptInterpreter::o1_loadScene(Script *script) {

	_vm->_backgroundFileIndex = script->readByte() * 2;

	debug(2, "o1_loadScene(%d)", _vm->_backgroundFileIndex);

	_vm->initSceneBackground();

}

void ScriptInterpreter::o1_addBlockingRect(Script *script) {

	int x1 = script->readByte();
	int y1 = script->readByte();
	int x2 = script->readByte();
	int y2 = script->readByte();

	debug(2, "o1_addBlockingRect(%d,%d,%d,%d)", x1, y1, x2, y2);

	_vm->_scene->addBlockingRect(x1, y1, x2, y2);

}

void ScriptInterpreter::o1_ifSpeak(Script *script) {
	if (_vm->_cmdTalk) {
		script->ip += 2;
		_vm->_cmdTalk = false;
	} else {
		script->jump();
	}
}

void ScriptInterpreter::o1_ifSpeakTo(Script *script) {
	int objectIndex = script->readByte();
	if (_vm->_cmdTalk && _vm->isActorNearActor(0, objectIndex, 40, 40)) {
		script->ip += 2;
		_vm->_cmdTalk = false;
	} else {
		script->jump();
	}
}

void ScriptInterpreter::o1_ifSpeakZone(Script *script) {
	if (_vm->_cmdTalk) {
		if (isHeroInZone(script)) {
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

bool ScriptInterpreter::isHeroInZone(Script *script) {

	SceneObject *sceneObject = getSceneObject(0);

	script->x = script->readByte() * 2;
	script->y = script->readByte();
	script->x2 = script->readByte() * 2;
	script->y2 = script->readByte();

	if (_vm->_debugRectangles)
		_vm->_screen->fillRect(script->x, script->y, script->x2, script->y2, 50);
	
	Common::Rect rect1(script->x, script->y, script->x2, script->y2);
	Common::Rect rect2(
		sceneObject->x - sceneObject->deltaX,
		sceneObject->y - sceneObject->deltaY,
		sceneObject->x + sceneObject->deltaX,
		sceneObject->y);
	
	return _vm->rectCompare(rect1, rect2);

}

void ScriptInterpreter::o1_ifLook(Script *script) {
	if (_vm->_cmdLook) {
		script->ip += 2;
		_vm->_cmdLook = false;
	} else {
		script->jump();
	}
}

void ScriptInterpreter::o1_ifLookAt(Script *script) {
	int objectIndex = script->readByte();
	if (_vm->_cmdLook && _vm->isActorNearActor(0, objectIndex, 40, 40)) {
		script->ip += 2;
		_vm->_cmdLook = false;
	} else {
		script->jump();
	}
}

void ScriptInterpreter::o1_ifLookZone(Script *script) {
	// LOOK AT handler
	if (_vm->_cmdLook) {
		if (isHeroInZone(script)) {
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

void ScriptInterpreter::o1_addBeam(Script *script) {
	int x1 = script->readByte() * 2;
	int y1 = script->readByte();
	int x2 = script->readByte() * 2;
	int y2 = script->readByte();
	// TODO: Actually add the line
}

void ScriptInterpreter::o1_removeBlockingRect(Script *script) {
	int x = script->readByte() * 2;
	int y = script->readByte();
	_vm->_scene->removeBlockingRect(x, y);
}

void ScriptInterpreter::o1_objectSetTextColor(Script *script) {
	debug(2, "o1_objectSetTextColor");
	script->object()->textColor = script->readByte();
}

void ScriptInterpreter::o1_setTextXY(Script *script) {
	int textX = script->readByte() * 2;
	int textY = script->readByte();
	debug(2, "o1_setTextXY(%d, %d)", textX, textY);
	script->object()->textX = textX;
	script->object()->textY = textY;
}

void ScriptInterpreter::o1_breakLoop(Script *script) {
	script->counter = 0;
	script->jump();
}

void ScriptInterpreter::o1_playMusic(Script *script) {
	int fileIndex = script->readByte();
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
	_vm->_scriptRandomValue = _vm->random(script->readByte());
}

void ScriptInterpreter::o1_setChapterNumber(Script *script) {
	_vm->_chapterNumber = script->readByte();
	debug(2, "o1_setChapterNumber(%d)", _vm->_chapterNumber);
	_yield = true;
}

void ScriptInterpreter::o1_actorTalk(Script *script) {

	int objectIndex = script->readByte();
	int talkTextIndex = script->readInt16();
	int animNumber = script->readByte();

	debug(2, "o1_actorTalk(%d, %d, %d)", objectIndex, talkTextIndex, animNumber);

	_vm->actorTalkWithAnim(objectIndex, talkTextIndex, animNumber);

	_curScript->status |= kScriptTalking;
	_yield = true;

}

void ScriptInterpreter::o1_loadSavegame(Script *script) {
	_vm->_needToLoadSavegameFlag = true;
	_yield = true;
}

void ScriptInterpreter::o1_addSceneItem2(Script *script) {
	o1_addSceneItem(script, 1);
}

void ScriptInterpreter::o1_playAnim(Script *script) {
	debug(2, "o1_playAnim");

	o1_sceneObjectSetAnimNumber(script);
	script->status |= kScriptAnimPlaying;
	_yield = true;

}

void ScriptInterpreter::o1_sceneObjectSetAnimNumber(Script *script) {

	int animIndex = script->readByte();

	debug(2, "o1_sceneObjectSetAnimNumber(%d)", animIndex);

	_vm->sceneObjectSetAnimNumber(script->object(), animIndex);
	script->object()->directionChanged = 2;

}

void ScriptInterpreter::o1_actorTalkPortrait(Script *script) {

	int objectIndex = script->readByte();
	int talkTextIndex = script->readInt16();
	int animNumber = script->readByte();
	int fileIndex = script->readByte();
	
	debug(2, "o1_actorTalkPortrait(%d, %d, %d, %d)", objectIndex, talkTextIndex, animNumber, fileIndex);
	//_system->delayMillis(5000);

	int marcheIndex = _vm->loadMarche(_vm->_marcheNumber, fileIndex);
	_vm->sceneObjectInit(10, marcheIndex);
	
	if (objectIndex != -1) {
		_vm->_sceneObjects[10].textX = 0;
		_vm->_sceneObjects[10].textY = 160;
		_vm->_sceneObjects[10].textColor = getSceneObject(objectIndex)->textColor;
	}
	
	_vm->_marcheNumber = 0;
	_vm->sceneObjectSetPosition(10, 0, 199);
	_vm->actorTalkWithAnim(10, talkTextIndex, animNumber);
	_vm->_animIndex = objectIndex;
	_vm->_screen->enableTransitionEffect();

	_curScript->status |= kScriptTalking;
	_yield = true;

}

void ScriptInterpreter::o1_initSceneObject(Script *script) {
	debug(2, "o1_initSceneObject");

	o1_selectObject(script);

	if (script->objectIndex != 0) {
		_vm->sceneObjectInit(script->objectIndex, -1);
	}

}

void ScriptInterpreter::o1_loadSceneObjectSprite(Script *script) {

	int fileIndex = script->readByte();
	
	debug(2, "o1_loadSceneObjectSprite(%d)", fileIndex);

	_vm->unloadSceneObjectSprite(script->object());
	
	script->object()->marcheIndex = _vm->loadMarche(_vm->_marcheNumber, fileIndex);
	_vm->_marcheNumber = 0;
	
}

void ScriptInterpreter::o1_setObjectVisible(Script *script) {
	int visible = script->readByte();
	debug(2, "o1_setObjectVisible(%d)", visible);
	script->object()->visible = visible;
}

void ScriptInterpreter::o1_paletteFadeIn(Script *script) {
	int fadeStep = script->readByte();
	debug(2, "o1_paletteFadeIn(%d)", fadeStep);
	_vm->_screen->setFadeType(kFadeIn);
	_vm->_screen->setFadeStep(fadeStep);
}

void ScriptInterpreter::o1_paletteFadeOut(Script *script) {
	int fadeStep = script->readByte();
	debug(2, "o1_paletteFadeOut(%d)", fadeStep);
	_vm->_screen->setFadeType(kFadeOut);
	_vm->_screen->setFadeStep(fadeStep);
}

void ScriptInterpreter::o1_setNarFileIndex(Script *script) {

	_vm->_narFileIndex = script->readByte();

	debug(2, "o1_setNarFileIndex(%d)", _vm->_narFileIndex);

	_vm->openVoiceFile(_vm->_narFileIndex);
	
}

void ScriptInterpreter::o1_deactivateSceneItem(Script *script) {

	int itemIndex = script->readByte();

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

	int narFileIndex = script->readByte();
	
	//TODO: seg010:074A
	
}

void ScriptInterpreter::o1_sample_1(Script *script) {
	debug(2, "o1_sample_1");

	int narFileIndex = script->readByte();
	int value = script->readByte();
	
	//TODO


}

} // End of namespace Comet
