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
 * $URL: https://svn.scummvm.org:4444/svn/kult/kult.cpp $
 * $Id: kult.cpp 127 2011-05-04 04:52:04Z digitall $
 *
 */

#include "common/error.h"
#include "graphics/surface.h"
#include "engines/util.h"

#include "kult/kult.h"
#include "kult/resource.h"
#include "kult/script.h"

namespace Kult {

// Script

Script::Script(KultEngine *vm) : _vm(vm) {
	_scriptData = new ResourceData(1);
	setupOpcodes();
}

Script::~Script() {
	delete _scriptData;
}

void Script::loadFromFile(const char *filename) {
	_scriptData->loadFromFile(filename);
}

void Script::loadFromStream(Common::SeekableReadStream &stream) {
	_scriptData->loadFromStream(stream);
}

void Script::gotoScript(uint index) {
	uint size;
	_ip = _scriptData->get(index, size);
	_end = _ip + size;
}

ScriptResult Script::runScript() {
	ScriptResult result = kScriptContinue;
	while (result == kScriptContinue) {
		if (_ip >= _end)
			result = kScriptDone;
		else {
			byte opcode = readByte();
			if (opcode == 0 || opcode == 0xFF)
				result = kScriptDone;
			else if (opcode <= _opcodes.size())
				result = execOpcode(opcode - 1);
			else {
				warning("Invalid opcode %02X", opcode - 1);
			}
		}			
	}
	return result;
}

ScriptResult Script::execOpcode(byte opcode) {
	debug("opcode: %02X (%s)", opcode, _opcodeNames[opcode].c_str());
	return kScriptContinue;
}

byte Script::readByte() {
	return *_ip++; 
}

uint16 Script::readUint16() { 
	uint16 value = READ_LE_UINT16(_ip); 
	_ip += 2; 
	return value; 
}

typedef Common::Functor0Mem<ScriptResult, Script> ScriptOpcode;
#define RegisterOpcode(x) \
	_opcodes.push_back(new ScriptOpcode(this, &Script::x));  \
	_opcodeNames.push_back(#x);
	
void Script::setupOpcodes() {
//	RegisterOpcode();

	RegisterOpcode(op_exchangeObject);
	RegisterOpcode(op_exchangeObject3);
	RegisterOpcode(op_drawObjectBox);
	RegisterOpcode(op_takeZapStik);
	RegisterOpcode(op_drawIconeLeftRight);
	RegisterOpcode(op_drawIconeRightLeft);
	RegisterOpcode(op_drawIconeTopDown);
	RegisterOpcode(op_drawIconeDownTop);
	RegisterOpcode(op_drawIcone6);
	RegisterOpcode(op_drawIcone6x_UNU);
	RegisterOpcode(op_drawIconeOther2);
	RegisterOpcode(op_drawIconeOther);
	RegisterOpcode(op_drawIcone5);
	RegisterOpcode(op_drawIconeZoomIn);
	RegisterOpcode(op_sub_273A1_unu);
	RegisterOpcode(op_drawIcone7);
	RegisterOpcode(op_drawPUZZL2);
	RegisterOpcode(op_gotoScript);
	RegisterOpcode(op_drawARPLA2);
	RegisterOpcode(op_showTextBox);
	RegisterOpcode(op_setCurrZoneRect);
	RegisterOpcode(op_sub_292C0);
	RegisterOpcode(op_showDIALETextBubble3);
	RegisterOpcode(op_playAnico);
	RegisterOpcode(op_removeIconeLeftRight);
	RegisterOpcode(op_removeIconeRightLeft);
	RegisterOpcode(op_removeIconeTopDown);
	RegisterOpcode(op_removeIconeDownTop);
	RegisterOpcode(op_restoreDirtyRectx_UNU);
	RegisterOpcode(op_removeIconeOther2);
	RegisterOpcode(op_removeIconeOther);
	RegisterOpcode(op_removeIconeUnk);
	RegisterOpcode(op_sub_27F56_UNU);
	RegisterOpcode(j_op_sub_26DBB);
	RegisterOpcode(op_restoreDirtyRect);
	RegisterOpcode(op_removeIcones);
	RegisterOpcode(op_gotoZone3);
	RegisterOpcode(op_gameOver);
	RegisterOpcode(op_drawGaussTextBubble);
	RegisterOpcode(op_selectMouseZone);
	RegisterOpcode(op_drawTextBubble);
	RegisterOpcode(op_sub_29890_unu);
	RegisterOpcode(op_removeTextBubbles);
	RegisterOpcode(op_waitForMouseClickTimeout2);
	RegisterOpcode(op_waitForMouseClickTimeout);
	RegisterOpcode(op_waitForMouseClick);
	RegisterOpcode(op_sub_29310);
	RegisterOpcode(op_sub_27E05);
	RegisterOpcode(op_sub_2939C);
	RegisterOpcode(op_sub_29484);
	RegisterOpcode(op_jump);    
	RegisterOpcode(op_call);
	RegisterOpcode(op_return);
	RegisterOpcode(op_gotoZone1);
	RegisterOpcode(op_drawTextBox);
	RegisterOpcode(op_playAnimation);
	RegisterOpcode(op_sub_2852E);
	RegisterOpcode(op_loc_28564);
	RegisterOpcode(op_setValue);
	RegisterOpcode(op_if);
	RegisterOpcode(op_runBrain);  
	RegisterOpcode(op_sub_27505);
	RegisterOpcode(op_restoreDirtyRect);
	RegisterOpcode(op_removeTextBoxes);
	RegisterOpcode(op_sub_28313);
	RegisterOpcode(op_gotoZone2);  
	RegisterOpcode(op_sub_288BD);
	RegisterOpcode(op_sub_2639C);
	RegisterOpcode(op_sub_2774D);
	RegisterOpcode(op_sub_27952);
	RegisterOpcode(op_sub_27902);  
	RegisterOpcode(op_sub_278D8);
	RegisterOpcode(op_sub_27927);
	RegisterOpcode(op_sub_2797F_UNU);
	RegisterOpcode(op_sub_294F0);
	RegisterOpcode(op_sub_28AF1);
	RegisterOpcode(op_sub_1DED5);
	RegisterOpcode(op_loc_2951F);
	RegisterOpcode(op_sub_29519);
	RegisterOpcode(op_sub_2954E);
	RegisterOpcode(op_exchangeObject2); 
	RegisterOpcode(op_sub_28AAD);
	RegisterOpcode(op_setCurrObjectByFlags);
	RegisterOpcode(op_sub_27981);
	RegisterOpcode(op_sub_295C1);
	RegisterOpcode(op_sub_279AA);  
	RegisterOpcode(op_drawLutin2);
	RegisterOpcode(op_drawLutin1);
	RegisterOpcode(op_sub_28AD6);
	RegisterOpcode(op_nop0);
	RegisterOpcode(op_sub_27A0A);  
	RegisterOpcode(op_sub_2964D);
	RegisterOpcode(op_sub_29668);
	RegisterOpcode(op_nop1);
	RegisterOpcode(op_drawPUZZL);
	RegisterOpcode(op_sub_297DA);  
	RegisterOpcode(op_showDIALETextBubble4);
	RegisterOpcode(op_doSciPower);
	RegisterOpcode(op_sub_29827);
	RegisterOpcode(op_sub_27FBB);
	RegisterOpcode(op_sub_27889);  
	RegisterOpcode(op_sub_2785C);
	RegisterOpcode(op_eat2);
	RegisterOpcode(op_sub_1EF81);
	RegisterOpcode(op_playSound);
	RegisterOpcode(op_eat1);     
	RegisterOpcode(op_drawARPLA);

}

// Opcodes

ScriptResult Script::op_exchangeObject() {
	return kScriptContinue;
}

ScriptResult Script::op_exchangeObject3() {
	return kScriptContinue;
}

ScriptResult Script::op_drawObjectBox() {
	return kScriptContinue;
}

ScriptResult Script::op_takeZapStik() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIconeLeftRight() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIconeRightLeft() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIconeTopDown() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIconeDownTop() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIcone6() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIcone6x_UNU() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIconeOther2() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIconeOther() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIcone5() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIconeZoomIn() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_273A1_unu() {
	return kScriptContinue;
}

ScriptResult Script::op_drawIcone7() {
	return kScriptContinue;
}

ScriptResult Script::op_drawPUZZL2() {
	return kScriptContinue;
}

ScriptResult Script::op_gotoScript() {
	return kScriptContinue;
}

ScriptResult Script::op_drawARPLA2() {
	return kScriptContinue;
}

ScriptResult Script::op_showTextBox() {
	return kScriptContinue;
}

ScriptResult Script::op_setCurrZoneRect() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_292C0() {
	return kScriptContinue;
}

ScriptResult Script::op_showDIALETextBubble3() {
	return kScriptContinue;
}

ScriptResult Script::op_playAnico() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIconeLeftRight() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIconeRightLeft() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIconeTopDown() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIconeDownTop() {
	return kScriptContinue;
}

ScriptResult Script::op_restoreDirtyRectx_UNU() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIconeOther2() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIconeOther() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIconeUnk() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27F56_UNU() {
	return kScriptContinue;
}

ScriptResult Script::j_op_sub_26DBB() {
	return kScriptContinue;
}

ScriptResult Script::op_restoreDirtyRect() {
	return kScriptContinue;
}

ScriptResult Script::op_removeIcones() {
	return kScriptContinue;
}

ScriptResult Script::op_gotoZone3() {
	return kScriptContinue;
}

ScriptResult Script::op_gameOver() {
	return kScriptContinue;
}

ScriptResult Script::op_drawGaussTextBubble() {
	return kScriptContinue;
}

ScriptResult Script::op_selectMouseZone() {
	return kScriptContinue;
}

ScriptResult Script::op_drawTextBubble() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_29890_unu() {
	return kScriptContinue;
}

ScriptResult Script::op_removeTextBubbles() {
	return kScriptContinue;
}

ScriptResult Script::op_waitForMouseClickTimeout2() {
	return kScriptContinue;
}

ScriptResult Script::op_waitForMouseClickTimeout() {
	return kScriptContinue;
}

ScriptResult Script::op_waitForMouseClick() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_29310() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27E05() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_2939C() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_29484() {
	return kScriptContinue;
}

ScriptResult Script::op_jump() {
	return kScriptContinue;
}
    
ScriptResult Script::op_call() {
	return kScriptContinue;
}

ScriptResult Script::op_return() {
	return kScriptContinue;
}

ScriptResult Script::op_gotoZone1() {
	return kScriptContinue;
}

ScriptResult Script::op_drawTextBox() {
	return kScriptContinue;
}

ScriptResult Script::op_playAnimation() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_2852E() {
	return kScriptContinue;
}

ScriptResult Script::op_loc_28564() {
	return kScriptContinue;
}

ScriptResult Script::op_setValue() {
	return kScriptContinue;
}

ScriptResult Script::op_if() {
	return kScriptContinue;
}

ScriptResult Script::op_runBrain() {
	return kScriptContinue;
}
  
ScriptResult Script::op_sub_27505() {
	return kScriptContinue;
}

//ScriptResult Script::op_restoreDirtyRect() {}
ScriptResult Script::op_removeTextBoxes() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_28313() {
	return kScriptContinue;
}

ScriptResult Script::op_gotoZone2() {
	return kScriptContinue;
}
  
ScriptResult Script::op_sub_288BD() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_2639C() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_2774D() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27952() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27902() {
	return kScriptContinue;
}
  
ScriptResult Script::op_sub_278D8() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27927() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_2797F_UNU() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_294F0() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_28AF1() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_1DED5() {
	return kScriptContinue;
}

ScriptResult Script::op_loc_2951F() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_29519() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_2954E() {
	return kScriptContinue;
}

ScriptResult Script::op_exchangeObject2() {
	return kScriptContinue;
}
 
ScriptResult Script::op_sub_28AAD() {
	return kScriptContinue;
}

ScriptResult Script::op_setCurrObjectByFlags() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27981() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_295C1() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_279AA() {
	return kScriptContinue;
}
  
ScriptResult Script::op_drawLutin2() {
	return kScriptContinue;
}

ScriptResult Script::op_drawLutin1() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_28AD6() {
	return kScriptContinue;
}

ScriptResult Script::op_nop0() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27A0A() {
	return kScriptContinue;
}
  
ScriptResult Script::op_sub_2964D() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_29668() {
	return kScriptContinue;
}

ScriptResult Script::op_nop1() {
	return kScriptContinue;
}

ScriptResult Script::op_drawPUZZL() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_297DA() {
	return kScriptContinue;
}
  
ScriptResult Script::op_showDIALETextBubble4() {
	return kScriptContinue;
}

ScriptResult Script::op_doSciPower() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_29827() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27FBB() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_27889() {
	return kScriptContinue;
}
  
ScriptResult Script::op_sub_2785C() {
	return kScriptContinue;
}

ScriptResult Script::op_eat2() {
	return kScriptContinue;
}

ScriptResult Script::op_sub_1EF81() {
	return kScriptContinue;
}

ScriptResult Script::op_playSound() {
	return kScriptContinue;
}

ScriptResult Script::op_eat1() {
	return kScriptContinue;
}

ScriptResult Script::op_drawARPLA() {
	return kScriptContinue;
}


} // End of namespace Kult
