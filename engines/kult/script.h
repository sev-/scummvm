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
 * $URL: https://svn.scummvm.org:4444/svn/kult/kult.h $
 * $Id: kult.h 126 2011-04-30 00:09:02Z digitall $
 *
 */

#ifndef KULT_SCRIPT_H
#define KULT_SCRIPT_H

#include "common/array.h"
#include "common/func.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/str.h"

#include "kult/kult.h"
#include "kult/resource.h"

namespace Kult {

enum ScriptResult {
	kScriptContinue,
	kScriptBreak,
	kScriptDone
};

typedef Common::Functor0<ScriptResult> ScriptOpcodeImpl;

class Script {
public:
	Script(KultEngine *vm);
	~Script();
	void loadFromFile(const char *filename);
	void loadFromStream(Common::SeekableReadStream &stream);
	void gotoScript(uint index);
	ScriptResult runScript();
protected:
	KultEngine *_vm;
	ResourceData *_scriptData;
	byte *_ip, *_end;
	Common::Array<ScriptOpcodeImpl*> _opcodes;
	Common::Array<Common::String> _opcodeNames;
	ScriptResult execOpcode(byte opcode);
	byte readByte();
	uint16 readUint16();
	void setupOpcodes();
	// Opcodes
	ScriptResult op_exchangeObject();
	ScriptResult op_exchangeObject3();
	ScriptResult op_drawObjectBox();
	ScriptResult op_takeZapStik();
	ScriptResult op_drawIconeLeftRight();
	ScriptResult op_drawIconeRightLeft();
	ScriptResult op_drawIconeTopDown();
	ScriptResult op_drawIconeDownTop();
	ScriptResult op_drawIcone6();
	ScriptResult op_drawIcone6x_UNU();
	ScriptResult op_drawIconeOther2();
	ScriptResult op_drawIconeOther();
	ScriptResult op_drawIcone5();
	ScriptResult op_drawIconeZoomIn();
	ScriptResult op_sub_273A1_unu();
	ScriptResult op_drawIcone7();
	ScriptResult op_drawPUZZL2();
	ScriptResult op_gotoScript();
	ScriptResult op_drawARPLA2();
	ScriptResult op_showTextBox();
	ScriptResult op_setCurrZoneRect();
	ScriptResult op_sub_292C0();
	ScriptResult op_showDIALETextBubble3();
	ScriptResult op_playAnico();
	ScriptResult op_removeIconeLeftRight();
	ScriptResult op_removeIconeRightLeft();
	ScriptResult op_removeIconeTopDown();
	ScriptResult op_removeIconeDownTop();
	ScriptResult op_restoreDirtyRectx_UNU();
	ScriptResult op_removeIconeOther2();
	ScriptResult op_removeIconeOther();
	ScriptResult op_removeIconeUnk();
	ScriptResult op_sub_27F56_UNU();
	ScriptResult j_op_sub_26DBB();
	ScriptResult op_restoreDirtyRect();
	ScriptResult op_removeIcones();
	ScriptResult op_gotoZone3();
	ScriptResult op_gameOver();
	ScriptResult op_drawGaussTextBubble();
	ScriptResult op_selectMouseZone();
	ScriptResult op_drawTextBubble();
	ScriptResult op_sub_29890_unu();
	ScriptResult op_removeTextBubbles();
	ScriptResult op_waitForMouseClickTimeout2();
	ScriptResult op_waitForMouseClickTimeout();
	ScriptResult op_waitForMouseClick();
	ScriptResult op_sub_29310();
	ScriptResult op_sub_27E05();
	ScriptResult op_sub_2939C();
	ScriptResult op_sub_29484();
	ScriptResult op_jump();    
	ScriptResult op_call();
	ScriptResult op_return();
	ScriptResult op_gotoZone1();
	ScriptResult op_drawTextBox();
	ScriptResult op_playAnimation();
	ScriptResult op_sub_2852E();
	ScriptResult op_loc_28564();
	ScriptResult op_setValue();
	ScriptResult op_if();
	ScriptResult op_runBrain();  
	ScriptResult op_sub_27505();
	//ScriptResult op_restoreDirtyRect();
	ScriptResult op_removeTextBoxes();
	ScriptResult op_sub_28313();
	ScriptResult op_gotoZone2();  
	ScriptResult op_sub_288BD();
	ScriptResult op_sub_2639C();
	ScriptResult op_sub_2774D();
	ScriptResult op_sub_27952();
	ScriptResult op_sub_27902();  
	ScriptResult op_sub_278D8();
	ScriptResult op_sub_27927();
	ScriptResult op_sub_2797F_UNU();
	ScriptResult op_sub_294F0();
	ScriptResult op_sub_28AF1();
	ScriptResult op_sub_1DED5();
	ScriptResult op_loc_2951F();
	ScriptResult op_sub_29519();
	ScriptResult op_sub_2954E();
	ScriptResult op_exchangeObject2(); 
	ScriptResult op_sub_28AAD();
	ScriptResult op_setCurrObjectByFlags();
	ScriptResult op_sub_27981();
	ScriptResult op_sub_295C1();
	ScriptResult op_sub_279AA();  
	ScriptResult op_drawLutin2();
	ScriptResult op_drawLutin1();
	ScriptResult op_sub_28AD6();
	ScriptResult op_nop0();
	ScriptResult op_sub_27A0A();  
	ScriptResult op_sub_2964D();
	ScriptResult op_sub_29668();
	ScriptResult op_nop1();
	ScriptResult op_drawPUZZL();
	ScriptResult op_sub_297DA();  
	ScriptResult op_showDIALETextBubble4();
	ScriptResult op_doSciPower();
	ScriptResult op_sub_29827();
	ScriptResult op_sub_27FBB();
	ScriptResult op_sub_27889();  
	ScriptResult op_sub_2785C();
	ScriptResult op_eat2();
	ScriptResult op_sub_1EF81();
	ScriptResult op_playSound();
	ScriptResult op_eat1();     
	ScriptResult op_drawARPLA();
};

} // End of namespace Kult

#endif /* KULT_SCRIPT_H */
