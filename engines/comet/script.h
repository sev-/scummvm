/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#include "common/func.h"
#include "comet/comet.h"

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

class Script {
public:
	byte *code;
	byte *ip;
	byte debugOpcode;
	int16 objectIndex;
	uint16 status;
	uint16 resumeIp; // FIXME: Remove this hacky thing
	int scriptNumber;
	int counter;
	int x, y, x2, y2;
	Script(ScriptInterpreter *inter) : _inter(inter) {}
	byte readByte();
	int16 readInt16();
	void jump();
	uint16 loadVarValue();
	uint16 loadValue();
	SceneObject *object() const;
private:
	ScriptInterpreter *_inter;
};

typedef Common::Functor1<Script*, void> ScriptOpcode;

class ScriptInterpreter {
public:
	ScriptInterpreter(CometEngine *vm);
	~ScriptInterpreter();

protected:
	CometEngine *_vm;

//protected:
// Everything is public during the transition phase to more object-oriented design
public:
	
	byte *_scriptData;
	int _scriptCount;
	Script *_scripts[17];
	int _curScriptNumber;
	Script *_curScript;
	bool _yield;

	Common::Array<ScriptOpcode*> _opcodes;
	Common::Array<Common::String> _opcodeNames;

	void setupOpcodes();

	void initializeScript();
	void initializeScriptAfterLoadGame();
	void prepareScript(int scriptNumber);
	void runScript(int scriptNumber);
	void runAllScripts();
	//TODO: Use something like getGlobalVar(index) and setGlobalVar(index, value) instead?
	int16 *getVarPointer(int varIndex);

	bool evalBoolOp(int value1, int value2, int boolOp);

	SceneObject *getScriptSceneObject();
	SceneObject *getSceneObject(int index);

	void processScriptSynchronize();
	void processScriptSleep();
	void processScriptWalk();
	void processScriptAnim();
	void processScriptDialog();
	void processScriptTalk();
	
	void objectWalkToXY(Script *script, bool xyFlag);
	void objectWalkToXYRel(Script *script, bool xyFlag);

	bool isHeroInZone(Script *script);
	void o1_addSceneItem(Script *script, int paramType);

	/* Script functions */

	void o1_nop(Script *script);
	void o1_sceneObjectSetDirection(Script *script);
	void o1_break(Script *script);
	void o1_jump(Script *script);
	void o1_objectWalkToX(Script *script);
	void o1_objectWalkToY(Script *script);
	void o1_loop(Script *script);
	void o1_objectSetPosition(Script *script);
	void o1_synchronize(Script *script);
	void o1_sleep(Script *script);
	void o1_if(Script *script);
	void o1_ifHeroInZone(Script *script);
	void o1_objectWalkToXRel(Script *script);
	void o1_objectWalkToYRel(Script *script);
	void o1_objectWalkToXYRel(Script *script);
	void o1_blockInput(Script *script);
	void o1_unblockInput(Script *script);
	void o1_sceneObjectSetDirectionToHero(Script *script);
	void o1_selectObject(Script *script);
	void o1_initSceneBounds(Script *script);
	void o1_initSceneExits(Script *script);
	void o1_addSceneObject(Script *script);
	void o1_startScript(Script *script);
	void o1_stopScript(Script *script);
	void o1_startMultipleScripts(Script *script);
	void o1_playCutscene(Script *script);
	void o1_setVar(Script *script);
	void o1_incVar(Script *script);
	void o1_subVar(Script *script);
	void o1_sceneObjectDisableCollisions(Script *script);
	void o1_sceneObjectEnableCollisions(Script *script);
	void o1_objectWalkTo(Script *script);
	void o1_setPaletteBrightness(Script *script);
	void o1_setSceneNumber(Script *script);
	void o1_setAnimValues(Script *script);
	void o1_setMarcheNumber(Script *script);
	void o1_heroIncPositionY(Script *script);
	void o1_setZoom(Script *script);
	void o1_setZoomByItem(Script *script);
	void o1_startDialog(Script *script);
	void o1_waitUntilHeroExitZone(Script *script);
	void o1_waitUntilHeroEnterZone(Script *script);
	void o1_sceneObjectDelete(Script *script);
	void o1_setObjectClipX(Script *script);
	void o1_setObjectClipY(Script *script);
	void o1_clearScreen(Script *script);
	void o1_orVar(Script *script);
	void o1_andVar(Script *script);
	void o1_loadScene(Script *script);
	void o1_sceneObjectSetAnimNumber(Script *script);
	void o1_addBlockingRect(Script *script);
	void o1_ifSpeak(Script *script);
	void o1_ifSpeakTo(Script *script);
	void o1_ifSpeakZone(Script *script);
	void o1_ifLook(Script *script);
	void o1_ifLookAt(Script *script);
	void o1_ifLookZone(Script *script);
	void o1_addBeam(Script *script);
	void o1_removeBlockingRect(Script *script);
	void o1_objectSetTextColor(Script *script);
	void o1_setTextXY(Script *script);
	void o1_breakLoop(Script *script);
	void o1_playMusic(Script *script);
	void o1_setRandomValue(Script *script);
	void o1_setChapterNumber(Script *script);
	void o1_actorTalk(Script *script);
	void o1_loadSavegame(Script *script);
	void o1_addSceneItem2(Script *script);
	void o1_playAnim(Script *script);
	void o1_actorTalkPortrait(Script *script);
	void o1_initSceneObject(Script *script);
	void o1_loadSceneObjectSprite(Script *script);
	void o1_setObjectVisible(Script *script);
	void o1_paletteFadeIn(Script *script);
	void o1_paletteFadeOut(Script *script);
	void o1_setNarFileIndex(Script *script);
	void o1_deactivateSceneItem(Script *script);
	void o1_sample_2(Script *script);
	void o1_sample_1(Script *script);

};

} // End of namespace Comet

#endif
