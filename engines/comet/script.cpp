#include "comet/comet.h"
#include "comet/screen.h"
#include "comet/script.h"
#include "comet/dialog.h"
#include "comet/scene.h"
#include "comet/animationmgr.h"
#include "comet/resource.h"

namespace Comet {

/* Script */

byte Script::readByte() {
	return code[ip++];
}

int16 Script::readInt16() {
	int16 value = (int16)READ_LE_UINT16(code + ip);
	ip += 2;
	return value;
}

void Script::jump() {
	int16 ofs = (int16)READ_LE_UINT16(code + ip);
	debug(3, "  jump: %d (%04X)", ofs, ofs);
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

Actor *Script::actor() const {
	assert( actorIndex >= 0 && actorIndex < 11 );
	return _inter->getActor(actorIndex);
}

/* ScriptInterpreter */

ScriptInterpreter::ScriptInterpreter(CometEngine *vm) : _vm(vm) {

	_scriptCount = 0;
	_curScriptNumber = -1;

    _scriptResource = new ScriptResource();

	for (int i = 0; i < kMaxScriptCount; i++)
		_scripts[i] = new Script(this);

	setupOpcodes();
	
}

ScriptInterpreter::~ScriptInterpreter() {

	delete _scriptResource;

	for (int i = 0; i < kMaxScriptCount; i++)
		delete _scripts[i];

}

typedef Common::Functor1Mem<Script*, void, ScriptInterpreter> ScriptOpcodeF;
#define RegisterOpcode(x) \
	_opcodes.push_back(new ScriptOpcodeF(this, &ScriptInterpreter::x));  \
	_opcodeNames.push_back(#x);
void ScriptInterpreter::setupOpcodes() {
	// 0
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_actorSetDirection);
	RegisterOpcode(o1_break);
	RegisterOpcode(o1_jump);
	RegisterOpcode(o1_actorWalkToX);
	// 5
	RegisterOpcode(o1_actorWalkToY);
	RegisterOpcode(o1_loop);
	RegisterOpcode(o1_actorSetPosition);
	RegisterOpcode(o1_synchronize);
	RegisterOpcode(o1_sleep);
	// 10
	RegisterOpcode(o1_if);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_nop);
	RegisterOpcode(o1_ifHeroInZone);
	// 15
	RegisterOpcode(o1_actorWalkToMainActorX);
	RegisterOpcode(o1_actorWalkToMainActorY);
	RegisterOpcode(o1_actorWalkToMainActorXY);
	RegisterOpcode(o1_blockInput);
	RegisterOpcode(o1_unblockInput);
	// 20
	RegisterOpcode(o1_actorSetDirectionToHero);
	RegisterOpcode(o1_selectActor);
	RegisterOpcode(o1_initSceneBounds);
	RegisterOpcode(o1_initSceneExits);
	RegisterOpcode(o1_nop);
	// 25
	RegisterOpcode(o1_addSceneObject);
	RegisterOpcode(o1_endIntroLoop);
	RegisterOpcode(o1_startScript);
	RegisterOpcode(o1_stopScript);
	RegisterOpcode(o1_startMultipleScripts);
	// 30
	RegisterOpcode(o1_playCutscene);
	RegisterOpcode(o1_setVar);
	RegisterOpcode(o1_incVar);
	RegisterOpcode(o1_subVar);
	RegisterOpcode(o1_actorDisableCollisions);
	// 35
	RegisterOpcode(o1_actorEnableCollisions);
	RegisterOpcode(o1_actorWalkTo);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_setPaletteBrightness);
	RegisterOpcode(o1_waitUntilHeroExitZone);
	// 40
	RegisterOpcode(o1_waitUntilHeroEnterZone);
	RegisterOpcode(o1_actorDelete);
	RegisterOpcode(o1_actorSetClipX);
	RegisterOpcode(o1_actorSetClipY);
	RegisterOpcode(o1_setSceneNumber);
	// 45
	RegisterOpcode(o1_setupActorAnim);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_setAnimationType);
	RegisterOpcode(o1_heroIncPositionY);
	RegisterOpcode(o1_nop);
	// 50
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_setZoom);
	RegisterOpcode(o1_setZoomByActor);
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
	RegisterOpcode(o1_actorSetTextColor);
	// 70
	RegisterOpcode(o1_actorSetTextPosition);
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
	RegisterOpcode(o1_gotoModule);
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_actorTalk);
	RegisterOpcode(o1_loadSavegame);
	RegisterOpcode(o1_addSceneItem2); // Unused in Comet CD
	// 85
	RegisterOpcode(o1_nop); // Unused in Comet CD
	RegisterOpcode(o1_nop);// TODO: o1_waitForKey();
	RegisterOpcode(o1_playActorAnim);
	RegisterOpcode(o1_actorSetAnimNumber);
	RegisterOpcode(o1_actorTalkPortrait);
	// 90
	RegisterOpcode(o1_initActor);
	RegisterOpcode(o1_loadActorSprite);
	RegisterOpcode(o1_setActorVisible);
	RegisterOpcode(o1_paletteFadeIn);
	RegisterOpcode(o1_paletteFadeOut);
	// 95
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_setNarFileIndex);
	RegisterOpcode(o1_ifNearActor);//TODO
	RegisterOpcode(o1_removeSceneItem);
	RegisterOpcode(o1_playSample);
	// 100
	RegisterOpcode(o1_playSampleLooping);
	RegisterOpcode(o1_setRedPalette);
	RegisterOpcode(o1_setWhitePalette);//TODO
	RegisterOpcode(o1_nop);//TODO
	RegisterOpcode(o1_nop);//TODO

}
#undef RegisterOpcode

void ScriptInterpreter::loadScript(const char *filename, int index) {
	_vm->_res->loadFromCC4(_scriptResource, filename, index);
}

void ScriptInterpreter::initializeScript() {
	_scriptCount = _scriptResource->getCount();
	for (int scriptNumber = 0; scriptNumber < _scriptCount; scriptNumber++)
		initScript(scriptNumber);
	_scripts[0]->status = 0;
	runScript(0);
}

void ScriptInterpreter::initializeScriptAfterLoadGame() {
	debug(2, "CometEngine::initializeScriptAfterLoadGame()  _scriptCount = %d", _scriptCount);
	for (int scriptNumber = 0; scriptNumber < _scriptCount; scriptNumber++)
		_scripts[scriptNumber]->code = _scriptResource->getScript(scriptNumber);
}

void ScriptInterpreter::initScript(int scriptNumber) {
	Script *script = _scripts[scriptNumber];
	script->code = _scriptResource->getScript(scriptNumber);
	script->ip = 0;
	script->actorIndex = 0;
	script->status = kScriptPaused;
	script->scriptNumber = 0;
	script->loopCounter = 0;
}

void ScriptInterpreter::processScriptSynchronize(Script *script) {
	Script *otherScript = _scripts[script->scriptNumber];
	if ((otherScript->status & kScriptSynchronize) && otherScript->scriptNumber == _curScriptNumber) {
		otherScript->status &= ~kScriptSynchronize;
		otherScript->scriptNumber = 0;
		script->status &= ~kScriptSynchronize;
		script->scriptNumber = 0;
	} else {
		_yield = true;
	}
}

void ScriptInterpreter::processScriptSleep(Script *script) {
	if (script->scriptNumber > 0)
		script->scriptNumber--;
	if (script->scriptNumber == 0) {
		script->status &= ~kScriptSleeping;
	} else {
		_yield = true;
	}
}

void ScriptInterpreter::processScriptWalk(Script *script) {
	if ((script->actor()->walkStatus & 3) == 0 || script->actor()->life == 0) {
		script->status &= ~kScriptWalking;
		_vm->actorSetAnimNumber(script->actor(), 0);
	} else {
		_yield = true;
	}
}

void ScriptInterpreter::processScriptAnim(Script *script) {
	if (script->actor()->animFrameIndex + 1 == script->actor()->animFrameCount) {
		script->status &= ~kScriptAnimPlaying;
	} else {
		_yield = true;
	}
}

void ScriptInterpreter::processScriptDialog(Script *script) {
	if (!_vm->_dialog->isRunning()) {
		script->status &= ~kScriptDialogRunning;
		script->ip = _vm->_dialog->getChoiceScriptIp();
		script->jump();
	} else {
 		_yield = true;
 	}
}

void ScriptInterpreter::processScriptTalk(Script *script) {
	if (!_vm->_textActive) {
		script->status &= ~kScriptTalking;
		if (_vm->_talkActorIndex == 10) {
			if (_vm->_animIndex != -1)
				_vm->_actors[_vm->_animIndex].visible = true;
			_vm->_actors[10].life = 0;
			_vm->_screen->enableTransitionEffect();
		} else if (_vm->_animIndex != -1) {
			Actor *actor = _vm->getActor(_vm->_talkActorIndex);
			_vm->actorSetAnimNumber(actor, _vm->_animIndex);
			actor->animPlayFrameIndex = _vm->_animPlayFrameIndex;
			actor->animFrameIndex = _vm->_animFrameIndex;
			_vm->_animIndex = -1;
		}
	} else {
		_yield = true;
	}
}

void ScriptInterpreter::runScript(int scriptNumber) {

	_curScriptNumber = scriptNumber;
	Script *script = _scripts[scriptNumber];

	_yield = false;

#if 0
	if (script->status & kScriptWalking)
		debug(2, "kScriptWalking %d", scriptNumber);
	if (script->status & kScriptSleeping)
		debug(2, "kScriptSleeping %d", scriptNumber);
	if (script->status & kScriptAnimPlaying)
		debug(2, "kScriptAnimPlaying %d", scriptNumber);
	if (script->status & kScriptSynchronize)
		debug(2, "kScriptSynchronize %d", scriptNumber);
	if (script->status & kScriptDialogRunning)
		debug(2, "kScriptDialogRunning %d", scriptNumber);
	if (script->status & kScriptPaused)
		debug(2, "kScriptPaused %d", scriptNumber);
	if (script->status & kScriptTalking)
		debug(2, "kScriptTalking %d", scriptNumber);
#endif

	if (script->status & kScriptPaused)
		return;
		
	if (script->status & kScriptSynchronize)
		processScriptSynchronize(script);

	if (script->status & kScriptSleeping)
		processScriptSleep(script);

	if (script->status & kScriptWalking)
		processScriptWalk(script);

	if (script->status & kScriptAnimPlaying)
		processScriptAnim(script);

	if (script->status & kScriptDialogRunning)
		processScriptDialog(script);

	if (script->status & kScriptTalking)
		processScriptTalk(script);

	while (!_yield) {
		byte opcode = script->readByte();
		/* DEBUG: So that o1_nop can print the opcode
			This will be removed again once all opcodes are implemented.
		*/
		script->debugOpcode = opcode;
		debug(2, "[%02d:%08X] %d", _curScriptNumber, script->ip, opcode);
		if (opcode >= _opcodes.size())
			error("CometEngine::runScript() Invalid opcode %d", opcode);
		debug(2, "%s", _opcodeNames[opcode].c_str());
		(*_opcodes[opcode])(script);
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
		return &_vm->_inventoryItemStatus[varIndex - 2000];
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

Actor *ScriptInterpreter::getActor(int index) {
	return _vm->getActor(index);
}

/* Script functions */

void ScriptInterpreter::o1_nop(Script *script) {
	debug("Unimplemented opcode %02X", script->debugOpcode);
}

void ScriptInterpreter::o1_actorSetDirection(Script *script) {
	debug(2, "o1_actorSetDirection");

	int direction = script->readByte();
	script->actor()->status = 0;
	_vm->actorSetDirection(script->actor(), direction);

}

void ScriptInterpreter::o1_break(Script *script) {
	script->status |= kScriptPaused;
	_yield = true;
}

void ScriptInterpreter::o1_jump(Script *script) {
	script->jump();
}

void ScriptInterpreter::o1_actorWalkToX(Script *script) {
	ARG_BYTEX(newX);
	script->actor()->status = 0;
	if (_vm->actorStartWalking(script->actorIndex, newX, script->actor()->y)) {
		script->actor()->walkStatus |= 8;
		script->status |= kScriptWalking;
		_yield = true;
	}
}

void ScriptInterpreter::o1_actorWalkToY(Script *script) {
	ARG_BYTE(newY);
	script->actor()->status = 0;
	if (_vm->actorStartWalking(script->actorIndex, script->actor()->x, newY)) {
		script->actor()->walkStatus |= 0x10;
		script->status |= kScriptWalking;
		_yield = true;
	}
}

void ScriptInterpreter::o1_loop(Script *script) {
	byte loopCount = script->code[script->ip + 2];
	script->loopCounter++;
	if (script->loopCounter < loopCount) {
		script->jump();
	} else {
		script->loopCounter = 0;
		script->ip += 3;
	}
}

void ScriptInterpreter::o1_actorSetPosition(Script *script) {
	ARG_BYTEX(x);
	ARG_BYTE(y);
	_vm->actorSetPosition(script->actorIndex, x, y);
}

void ScriptInterpreter::o1_synchronize(Script *script) {
	ARG_BYTE(scriptNumber);
	script->scriptNumber = scriptNumber;
	script->status |= kScriptSynchronize;
	_yield = true;
}

void ScriptInterpreter::o1_sleep(Script *script) {
	ARG_BYTE(sleepCount);
	script->scriptNumber = sleepCount;
	script->status |= kScriptSleeping;
	_yield = true;
	
}

void ScriptInterpreter::o1_if(Script *script) {
	ARG_VAR(value1);
	ARG_BYTE(boolOp);
	ARG_VALUE(value2);
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

void ScriptInterpreter::o1_actorWalkToMainActorX(Script *script) {
	ARG_BYTE(delta);
	script->actor()->status = 0;
	if (_vm->actorStartWalking(script->actorIndex, _vm->_actors[0].x + delta, script->actor()->y)) {
		script->actor()->walkStatus |= 8;
		script->status |= kScriptWalking;
		_yield = true;
	}
}

void ScriptInterpreter::o1_actorWalkToMainActorY(Script *script) {
	ARG_BYTE(delta);
	script->actor()->status = 0;
	if (_vm->actorStartWalking(script->actorIndex, script->actor()->x, _vm->_actors[0].y + delta)) {
		script->actor()->walkStatus |= 0x10;
		script->status |= kScriptWalking;
		_yield = true;
	}
}

void ScriptInterpreter::o1_actorWalkToMainActorXY(Script *script) {
	ARG_BYTE(deltaX);
	ARG_BYTE(deltaY);
	Actor *mainActor = _vm->getActor(0);
	Actor *actor = script->actor();
	int x = mainActor->x + deltaX;
	int y = mainActor->y + deltaY;
	actor->status = 0;
	_vm->_scene->superFilterWalkDestXY(x, y, actor->deltaX, actor->deltaY);
	actor->walkStatus = 0;
	if (_vm->actorStartWalking(script->actorIndex, x, y)) {
		script->status |= kScriptWalking;
		_yield = true;
	}
}

void ScriptInterpreter::o1_blockInput(Script *script) {
	ARG_BYTE(flagIndex);
	_vm->blockInput(flagIndex);
}

void ScriptInterpreter::o1_unblockInput(Script *script) {
	_vm->unblockInput();
}

void ScriptInterpreter::o1_actorSetDirectionToHero(Script *script) {
	Actor *mainActor = _vm->getActor(0);
	int direction = _vm->calcDirection(script->actor()->x, script->actor()->y, mainActor->x, mainActor->y);
	script->actor()->status = 0;
	_vm->actorSetDirection(script->actor(), direction);
}

void ScriptInterpreter::o1_selectActor(Script *script) {
	ARG_BYTE(actorIndex);
	script->actorIndex = actorIndex;
}

void ScriptInterpreter::o1_initSceneBounds(Script *script) {
	_vm->_scene->initBounds(script->code + script->ip);
	script->ip += script->code[script->ip] * 2 + 1;
}

void ScriptInterpreter::o1_initSceneExits(Script *script) {
	_vm->_scene->initExits(script->code + script->ip);
	script->ip += script->code[script->ip] * 5 + 1;
}

void ScriptInterpreter::o1_addSceneObject(Script *script) {
	ARG_BYTE(itemIndex);
	ARG_BYTEX(x);
	ARG_BYTE(y);
	_vm->_scene->addSceneItem(itemIndex, x, y, 0);
}

void ScriptInterpreter::o1_endIntroLoop(Script *script) {
	_vm->_endIntroLoop = true;
}

void ScriptInterpreter::o1_startScript(Script *script) {
	ARG_BYTE(scriptNumber);
	if (scriptNumber < _scriptCount) {
		initScript(scriptNumber);
		_scripts[scriptNumber]->status &= ~kScriptPaused;
	}
}

void ScriptInterpreter::o1_stopScript(Script *script) {
	ARG_BYTE(scriptNumber);
	_scripts[scriptNumber]->status |= kScriptPaused;
}

void ScriptInterpreter::o1_startMultipleScripts(Script *script) {
	int scriptNumber;
	while ((scriptNumber = script->readByte()) != 255) {
		initScript(scriptNumber);
		_scripts[scriptNumber]->status &= ~kScriptPaused;
	}
}

void ScriptInterpreter::o1_playCutscene(Script *script) {
	ARG_BYTE(fileIndex);
	ARG_BYTE(frameListIndex);
	ARG_INT16(backgroundIndex);
	ARG_BYTE(loopCount);
	ARG_BYTE(soundFramesCount);
	_vm->playCutscene(fileIndex, frameListIndex, backgroundIndex, loopCount, soundFramesCount, script->code + script->ip);
	script->ip += soundFramesCount * 3;
}

void ScriptInterpreter::o1_setVar(Script *script) {
	ARG_INT16(varIndex);
	ARG_VALUE(value);
	int16 *varPtr = getVarPointer(varIndex);
	*varPtr = value;
}

void ScriptInterpreter::o1_incVar(Script *script) {
	ARG_INT16(varIndex);
	ARG_VALUE(value);
	int16 *varPtr = getVarPointer(varIndex);
	*varPtr += value;
}

void ScriptInterpreter::o1_subVar(Script *script) {
	ARG_INT16(varIndex);
	ARG_VALUE(value);
	int16 *varPtr = getVarPointer(varIndex);
	*varPtr -= value;
}

void ScriptInterpreter::o1_actorDisableCollisions(Script *script) {
	script->actor()->collisionType = kCollisionDisabled;
}

void ScriptInterpreter::o1_actorEnableCollisions(Script *script) {
	script->actor()->collisionType = kCollisionNone;
}

void ScriptInterpreter::o1_actorWalkTo(Script *script) {
	ARG_BYTEX(x);
	ARG_BYTE(y);
	script->actor()->status = 0;
	if (_vm->actorStartWalking(script->actorIndex, x, y)) {
		script->status |= kScriptWalking;
		_yield = true;
	}
}

void ScriptInterpreter::o1_setPaletteBrightness(Script *script) {
	ARG_BYTE(paletteBrightness);
	_vm->_paletteBrightness = paletteBrightness;
	_vm->_screen->buildPalette(_vm->_gamePalette, _vm->_screenPalette, _vm->_paletteBrightness);
	_vm->_screen->setFullPalette(_vm->_screenPalette);
}

void ScriptInterpreter::o1_setSceneNumber(Script *script) {
	ARG_BYTE(sceneNumber);
	_vm->_sceneNumber = sceneNumber == 255 ? -1 : sceneNumber; 
}

void ScriptInterpreter::o1_setupActorAnim(Script *script) {
	ARG_BYTE(animIndex);
	ARG_BYTE(animFrameIndex);
	script->actor()->animIndex = animIndex;
	script->actor()->animFrameIndex = animFrameIndex;
	script->actor()->animPlayFrameIndex = animFrameIndex;
	script->actor()->status = 2;
}

void ScriptInterpreter::o1_setAnimationType(Script *script) {
	ARG_BYTE(animationType);
	_vm->_animationType = animationType;
}

void ScriptInterpreter::o1_heroIncPositionY(Script *script) {
	Actor *mainActor = _vm->getActor(0);
	_vm->actorSetPosition(script->actorIndex, mainActor->x, mainActor->y + 1);
}

void ScriptInterpreter::o1_setZoom(Script *script) {
	ARG_BYTEX(zoomX);
	ARG_BYTE(zoomY);
	ARG_BYTE(zoomFactor);
	_vm->_screen->setZoom(zoomFactor, zoomX, zoomY);
}

void ScriptInterpreter::o1_setZoomByActor(Script *script) {
	ARG_BYTE(actorIndex);
	ARG_BYTE(zoomFactor);
	Actor *actor = _vm->getActor(actorIndex);
	_vm->_screen->setZoom(zoomFactor, actor->x, actor->y);
}

void ScriptInterpreter::o1_startDialog(Script *script) {
	_vm->_dialog->start(script);
	_vm->waitForKeys();
	script->status |= kScriptDialogRunning;
	_yield = true;
}

void ScriptInterpreter::o1_waitUntilHeroExitZone(Script *script) {
	if (_vm->isPlayerInZone(script->zoneX1, script->zoneY1, script->zoneX2, script->zoneY2)) {
		script->ip--;
		_yield = true;
	}
}

void ScriptInterpreter::o1_waitUntilHeroEnterZone(Script *script) {
	ARG_BYTEX(zoneX1);
	ARG_BYTE(zoneY1);
	ARG_BYTEX(zoneX2);
	ARG_BYTE(zoneY2);
	script->zoneX1 = zoneX1;
	script->zoneY1 = zoneY1;
	script->zoneX2 = zoneX2;
	script->zoneY2 = zoneY2;
	if (!_vm->isPlayerInZone(script->zoneX1, script->zoneY1, script->zoneX2, script->zoneY2)) {
		script->ip -= 5;
		_yield = true;
	}
}

void ScriptInterpreter::o1_actorDelete(Script *script) {
	if (script->actorIndex != 0) {
		script->actor()->life = 0;
		if (script->actor()->animationSlot != -1)
			_vm->unloadActorSprite(script->actor());
	}
}

void ScriptInterpreter::o1_actorSetClipX(Script *script) {
	ARG_BYTEX(clipX1);
	ARG_BYTEX(clipX2);
	script->actor()->clipX1 = clipX1;
	script->actor()->clipX2 = clipX2;
}

void ScriptInterpreter::o1_actorSetClipY(Script *script) {
	ARG_BYTE(clipY1);
	ARG_BYTE(clipY2);
	script->actor()->clipY1 = clipY1;
	script->actor()->clipY2 = clipY2;
}

void ScriptInterpreter::o1_clearScreen(Script *script) {
	_vm->_clearScreenRequest = true;
}

void ScriptInterpreter::o1_orVar(Script *script) {
	ARG_INT16(varIndex);
	ARG_VALUE(value);
	int16 *varPtr = getVarPointer(varIndex);
	*varPtr |= value;
}

void ScriptInterpreter::o1_andVar(Script *script) {
	ARG_INT16(varIndex);
	ARG_VALUE(value);
	int16 *varPtr = getVarPointer(varIndex);
	*varPtr &= value;
}

void ScriptInterpreter::o1_loadScene(Script *script) {
	ARG_BYTEX(backgroundFileIndex);
	_vm->_backgroundFileIndex = backgroundFileIndex;
	_vm->initSceneBackground();
}

void ScriptInterpreter::o1_addBlockingRect(Script *script) {
	ARG_BYTEX(x1);
	ARG_BYTE(y1);
	ARG_BYTEX(x2);
	ARG_BYTE(y2);
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
	ARG_BYTE(actorIndex);
	if (_vm->_cmdTalk && _vm->isActorNearActor(0, actorIndex, 40, 40)) {
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
	ARG_BYTEX(zoneX1);
	ARG_BYTE(zoneY1);
	ARG_BYTEX(zoneX2);
	ARG_BYTE(zoneY2);
	script->zoneX1 = zoneX1;
	script->zoneY1 = zoneY1;
	script->zoneX2 = zoneX2;
	script->zoneY2 = zoneY2;
	Actor *mainActor = _vm->getActor(0);
	Common::Rect rect1(script->zoneX1, script->zoneY1, script->zoneX2, script->zoneY2);
	Common::Rect rect2(
		mainActor->x - mainActor->deltaX,
		mainActor->y - mainActor->deltaY,
		mainActor->x + mainActor->deltaX,
		mainActor->y);
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
	ARG_BYTE(actorIndex);
	if (_vm->_cmdLook && _vm->isActorNearActor(0, actorIndex, 40, 40)) {
		script->ip += 2;
		_vm->_cmdLook = false;
	} else {
		script->jump();
	}
}

void ScriptInterpreter::o1_ifLookZone(Script *script) {
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
	ARG_BYTEX(x1);
	ARG_BYTE(y1);
	ARG_BYTEX(x2);
	ARG_BYTE(y2);
	_vm->addBeam(x1, y1, x2, y2);
}

void ScriptInterpreter::o1_removeBlockingRect(Script *script) {
	ARG_BYTEX(x);
	ARG_BYTE(y);
	_vm->_scene->removeBlockingRect(x, y);
}

void ScriptInterpreter::o1_actorSetTextColor(Script *script) {
	script->actor()->textColor = script->readByte();
}

void ScriptInterpreter::o1_actorSetTextPosition(Script *script) {
	ARG_BYTEX(textX);
	ARG_BYTE(textY);
	script->actor()->textX = textX;
	script->actor()->textY = textY;
}

void ScriptInterpreter::o1_breakLoop(Script *script) {
	script->loopCounter = 0;
	script->jump();
}

void ScriptInterpreter::o1_playMusic(Script *script) {
	ARG_BYTE(musicIndex);
	_vm->playMusic(musicIndex);
}

void ScriptInterpreter::o1_setRandomValue(Script *script) {
	ARG_BYTE(maxValue);
	_vm->_scriptRandomValue = _vm->random(maxValue);
}

void ScriptInterpreter::o1_gotoModule(Script *script) {
	ARG_BYTE(moduleNumber);
	_vm->_moduleNumber = moduleNumber;
	_yield = true;
}

void ScriptInterpreter::o1_actorTalk(Script *script) {
	ARG_BYTE(actorIndex);
	ARG_INT16(talkTextIndex);
	ARG_BYTE(animNumber);
	_vm->actorTalkWithAnim(actorIndex, talkTextIndex, animNumber);
	script->status |= kScriptTalking;
	_yield = true;
}

void ScriptInterpreter::o1_loadSavegame(Script *script) {
	_vm->_loadgameRequested = true;
	_yield = true;
}

void ScriptInterpreter::o1_addSceneItem2(Script *script) {
	ARG_BYTE(itemIndex);
	ARG_INT16(x);
	ARG_BYTE(y);
	_vm->_scene->addSceneItem(itemIndex, x, y, 1);
}

void ScriptInterpreter::o1_playActorAnim(Script *script) {
	o1_actorSetAnimNumber(script);
	script->status |= kScriptAnimPlaying;
	_yield = true;
}

void ScriptInterpreter::o1_actorSetAnimNumber(Script *script) {
	ARG_BYTE(animIndex);
	_vm->actorSetAnimNumber(script->actor(), animIndex);
	script->actor()->status = 2;
}

void ScriptInterpreter::o1_actorTalkPortrait(Script *script) {
	ARG_BYTE(actorIndex);
	ARG_INT16(talkTextIndex);
	ARG_BYTE(animNumber);
	ARG_BYTE(fileIndex);
	_vm->actorTalkPortrait(actorIndex, talkTextIndex, animNumber, fileIndex);
	script->status |= kScriptTalking;
	_yield = true;
}

void ScriptInterpreter::o1_initActor(Script *script) {
	o1_selectActor(script);
	if (script->actorIndex != 0) {
		_vm->actorInit(script->actorIndex, -1);
	}
}

void ScriptInterpreter::o1_loadActorSprite(Script *script) {
	ARG_BYTE(fileIndex);
	_vm->unloadActorSprite(script->actor());
	script->actor()->animationSlot = _vm->_animationMan->getAnimationResource(_vm->_animationType, fileIndex);
	_vm->_animationType = 0;
}

void ScriptInterpreter::o1_setActorVisible(Script *script) {
	ARG_BYTE(visible);
	script->actor()->visible = visible;
}

void ScriptInterpreter::o1_paletteFadeIn(Script *script) {
	ARG_BYTE(fadeStep);
	_vm->_screen->setFadeType(kFadeIn);
	_vm->_screen->setFadeStep(fadeStep);
}

void ScriptInterpreter::o1_paletteFadeOut(Script *script) {
	ARG_BYTE(fadeStep);
	_vm->_screen->setFadeType(kFadeOut);
	_vm->_screen->setFadeStep(fadeStep);
}

void ScriptInterpreter::o1_setNarFileIndex(Script *script) {
	ARG_BYTE(narFileIndex);
	_vm->_narFileIndex = narFileIndex;
	_vm->setVoiceFileIndex(narFileIndex);
}

void ScriptInterpreter::o1_ifNearActor(Script *script) {
	ARG_BYTE(actorIndex);
	ARG_BYTE(deltaX);
	ARG_BYTE(deltaY);
	if (_vm->isActorNearActor(0, actorIndex, deltaX, deltaY)) {
		script->ip += 2;
	} else {
		script->jump();
	}
}

void ScriptInterpreter::o1_removeSceneItem(Script *script) {
	ARG_BYTE(itemIndex);
	_vm->_scene->removeSceneItem(itemIndex);
}

void ScriptInterpreter::o1_playSample(Script *script) {
	ARG_BYTE(sampleIndex);
	_vm->playSample(sampleIndex, 1);
}

void ScriptInterpreter::o1_playSampleLooping(Script *script) {
	// CHECKME: It's just a guess that this plays looping samples
	ARG_BYTE(sampleIndex);
	ARG_BYTE(loopCount);
	_vm->playSample(sampleIndex, loopCount);
}

void ScriptInterpreter::o1_setRedPalette(Script *script) {
	ARG_BYTE(paletteRedness);
	// TODO: Remember "redness" value
	_vm->_screen->buildRedPalette(_vm->_gamePalette, _vm->_screenPalette, paletteRedness);
	_vm->_screen->setFullPalette(_vm->_screenPalette);
}

void ScriptInterpreter::o1_setWhitePalette(Script *script) {
	ARG_BYTE(value);
	_vm->_screen->setWhitePalette(value);
}

} // End of namespace Comet
