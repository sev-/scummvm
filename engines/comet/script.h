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

#ifndef COMET_SCRIPT_H
#define COMET_SCRIPT_H

#include "comet/comet.h"
#include "common/func.h"
#include "common/serializer.h"

namespace Comet {

enum {
	kScriptWalking			= 0x01,
	kScriptSleeping			= 0x02,
	kScriptAnimPlaying		= 0x04,
	kScriptSynchronize		= 0x08,
	kScriptDialogRunning	= 0x10,
	kScriptPaused			= 0x20,
	kScriptTalking			= 0x40
};

class ScriptInterpreter;
class ScriptResource;

class Script {
public:
	byte *code;
	uint16 ip;
	byte debugOpcode;
	int16 actorIndex;
	uint16 status;
	int scriptNumber;
	int loopCounter;
	int zoneX1, zoneY1, zoneX2, zoneY2;
	Script(ScriptInterpreter *inter) : _inter(inter) {}
	byte readByte();
	int16 readInt16();
	void jump();
	uint16 loadVarValue();
	uint16 loadValue();
	Actor *actor() const;
	void sync(Common::Serializer &s);
private:
	ScriptInterpreter *_inter;
};

typedef Common::Functor1<Script*, void> ScriptOpcode;

const int kMaxScriptCount = 17;

class ScriptInterpreter {
public:
	ScriptInterpreter(CometEngine *vm);
	~ScriptInterpreter();

protected:
	CometEngine *_vm;

//protected:
// Everything is public during the transition phase to more object-oriented design
public:
	ScriptResource *_scriptResource;
	int _scriptCount;
	Script *_scripts[kMaxScriptCount];
	int _curScriptNumber;
	bool _yield;

	Common::Array<ScriptOpcode*> _opcodes;
	Common::Array<Common::String> _opcodeNames;

	void setupOpcodes();

	void loadScript(const char *filename, int index);
	void initializeScript();
	void initializeScriptAfterLoadGame();
	void initScript(int scriptNumber);
	void runScript(int scriptNumber);
	void runAllScripts();
	//TODO: Use something like getGlobalVar(index) and setGlobalVar(index, value) instead?
	int16 *getVarPointer(int varIndex);

	bool evalBoolOp(int value1, int value2, int boolOp);

	Actor *getActor(int index);

	void processScriptSynchronize(Script *script);
	void processScriptSleep(Script *script);
	void processScriptWalk(Script *script);
	void processScriptAnim(Script *script);
	void processScriptDialog(Script *script);
	void processScriptTalk(Script *script);
	
	bool isHeroInZone(Script *script);

	// Script functions

	void o1_nop(Script *script);
	void o1_actorSetDirection(Script *script);
	void o1_break(Script *script);
	void o1_jump(Script *script);
	void o1_actorWalkToX(Script *script);
	void o1_actorWalkToY(Script *script);
	void o1_loop(Script *script);
	void o1_actorSetPosition(Script *script);
	void o1_synchronize(Script *script);
	void o1_sleep(Script *script);
	void o1_if(Script *script);
	void o1_ifHeroInZone(Script *script);
	void o1_actorWalkToMainActorX(Script *script);
	void o1_actorWalkToMainActorY(Script *script);
	void o1_actorWalkToMainActorXY(Script *script);
	void o1_blockInput(Script *script);
	void o1_unblockInput(Script *script);
	void o1_actorSetDirectionToHero(Script *script);
	void o1_selectActor(Script *script);
	void o1_initSceneBounds(Script *script);
	void o1_initSceneExits(Script *script);
	void o1_addSceneObject(Script *script);
	void o1_endIntroLoop(Script *script);
	void o1_startScript(Script *script);
	void o1_stopScript(Script *script);
	void o1_startMultipleScripts(Script *script);
	void o1_playCutscene(Script *script);
	void o1_setVar(Script *script);
	void o1_incVar(Script *script);
	void o1_subVar(Script *script);
	void o1_actorDisableCollisions(Script *script);
	void o1_actorEnableCollisions(Script *script);
	void o1_actorWalkTo(Script *script);
	void o1_setPaletteBrightness(Script *script);
	void o1_setSceneNumber(Script *script);
	void o1_setupActorAnim(Script *script);
	void o1_setAnimationType(Script *script);
	void o1_heroIncPositionY(Script *script);
	void o1_setZoom(Script *script);
	void o1_setZoomByActor(Script *script);
	void o1_startDialog(Script *script);
	void o1_waitUntilHeroExitZone(Script *script);
	void o1_waitUntilHeroEnterZone(Script *script);
	void o1_actorDelete(Script *script);
	void o1_actorSetClipX(Script *script);
	void o1_actorSetClipY(Script *script);
	void o1_clearScreen(Script *script);
	void o1_orVar(Script *script);
	void o1_andVar(Script *script);
	void o1_loadScene(Script *script);
	void o1_actorSetAnimNumber(Script *script);
	void o1_addBlockingRect(Script *script);
	void o1_ifSpeak(Script *script);
	void o1_ifSpeakTo(Script *script);
	void o1_ifSpeakZone(Script *script);
	void o1_ifLook(Script *script);
	void o1_ifLookAt(Script *script);
	void o1_ifLookZone(Script *script);
	void o1_addBeam(Script *script);
	void o1_removeBlockingRect(Script *script);
	void o1_actorSetTextColor(Script *script);
	void o1_actorSetTextPosition(Script *script);
	void o1_breakLoop(Script *script);
	void o1_playMusic(Script *script);
	void o1_setRandomValue(Script *script);
	void o1_gotoModule(Script *script);
	void o1_actorTalk(Script *script);
	void o1_loadSavegame(Script *script);
	void o1_addSceneItem2(Script *script);
	void o1_waitForKeyPress(Script *script);
	void o1_playActorAnim(Script *script);
	void o1_actorTalkPortrait(Script *script);
	void o1_initActor(Script *script);
	void o1_loadActorSprite(Script *script);
	void o1_setActorVisible(Script *script);
	void o1_paletteFadeIn(Script *script);
	void o1_paletteFadeOut(Script *script);
	void o1_setNarFileIndex(Script *script);
	void o1_ifNearActor(Script *script);
	void o1_removeSceneItem(Script *script);
	void o1_playSample(Script *script);
	void o1_playSampleLooping(Script *script);
	void o1_setRedPalette(Script *script);
	void o1_setWhitePalette(Script *script);
};

// Macros for convenience and clarity
#define ARG_BYTE(name) \
	int16 name = script->readByte(); \
	debugC(1, kDebugScript, "byte() " #name " = %d", name);
#define ARG_BYTEX(name) \
	int16 name = script->readByte() * 2; \
	debugC(1, kDebugScript, "bytex() " #name " = %d", name);
#define ARG_INT16(name) \
	int16 name = script->readInt16(); \
	debugC(1, kDebugScript, "int16() " #name " = %d", name);
#define ARG_VAR(name) \
	int16 name = script->loadVarValue(); \
	debugC(1, kDebugScript, "var() " #name " = %d", name);
#define ARG_VALUE(name) \
	int16 name = script->loadValue(); \
	debugC(1, kDebugScript, "value() " #name " = %d", name);

} // End of namespace Comet

#endif
