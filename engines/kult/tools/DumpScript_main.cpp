#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef unsigned char byte;
typedef unsigned short uint16;

// NOTE: Contains slow and unoptimized code...

int dumpLoadText(const char *filename, int index) {
    FILE *bin = fopen(filename, "rb");
    while (1) {
        byte len;
        byte buffer[256];
        fread(&len, 1, 1, bin);
        if (feof(bin))
            break;
        len--;
        fread(buffer, len, 1, bin);
        if (index == 0) {
            byte *p = buffer;
            while (len--) {
                byte ch = *p++;
                printf("%c", (ch & 0x3F) + 0x20);
                if ((ch & 0xC0) == 0xC0) printf("A");
                else if (ch & 0x80) printf("E");
                else if (ch & 0x40) printf(" ");
            }
            break;
        }
        index--;
    }
    fclose(bin);
}

byte *scriptData;
int scriptSize;
int baseOffs;

byte readByte() {
    return *scriptData++;
};

uint16 readUint16() {
    return readByte() | (readByte() << 8);
}

uint16 readUint16BE() {
    return (readByte() << 8) | readByte();
}

uint16 readDESCIndex() {
    uint16 descIndex = readByte();
    if (descIndex < 4)
        descIndex = (descIndex << 8) | readByte();
    return descIndex - 4;
};

struct GameVar {
    int tableNum, offset;
    std::string name;
};

std::vector<GameVar> gameVars;

void addGameVar(int tableNum, int offset, const char *name) {
    GameVar gameVar;
    gameVar.tableNum = tableNum;
    gameVar.offset = offset;
    gameVar.name = name;
    gameVars.push_back(gameVar);
}


bool dumpGameVar(int tableNum, int offset) {
    for (std::vector<GameVar>::iterator it = gameVars.begin(); it != gameVars.end(); it++) {
        const GameVar &gameVar = *it;
        if (gameVar.tableNum == tableNum && gameVar.offset == offset) {
            printf("__%s", gameVar.name.c_str());
            return true;
        }
    }
    return false;
}

void dumpGameVars() {
    for (std::vector<GameVar>::iterator it = gameVars.begin(); it != gameVars.end(); it++) {
        const GameVar &gameVar = *it;
        printf("%02d %03d %s\n", gameVar.tableNum, gameVar.offset, gameVar.name.c_str());
    }
}

int tableMax = -1;
int tableMaxOffset[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

void dumpLoadValue() {
    byte flags = readByte();
    if (flags & 0x80) {
        // variable
        int tableNum = flags & 0x1F;
        int offset = readByte();
        int entryNum = -1;
        if (flags & 0x40)
            entryNum = readByte();

        if (tableNum > tableMax)
            tableMax = tableNum;
        if (offset > tableMaxOffset[tableNum])
            tableMaxOffset[tableNum] = offset;

        if (entryNum >= 0 || !dumpGameVar(tableNum, offset)) {
            printf("var(");
            printf("table(%d)", tableNum);
            if (entryNum >= 0)
                printf("{###%d}", entryNum); // Never used in KULT EGA
            printf("[%d]", offset);
            if (flags & 0x20)
                printf(".w");
            else
                printf(".b");
            printf(")");
        }
            
    } else {
        // constant
        int value = readByte();
        if (flags & 0x20)
            value = (value << 8) | readByte();
        printf("%d(%04X)", value, value);
        if (flags & 0x20)
            printf(".w");
        else
            printf(".b");
    }
}

void dumpEvalBinaryOp(byte flags) {
    if (flags & 0x40) {
        if (flags & 0x20) printf(" = ");
        if (flags & 0x10) printf(" < ");
        if (flags & 0x08) printf(" > ");
        if (flags & 0x04) printf(" != ");
        if (flags & 0x02) printf(" <= ");
        if (flags & 0x01) printf(" >= ");
    } else {
        if (flags & 0x20) printf(" + ");
        if (flags & 0x10) printf(" - ");
        if (flags & 0x08) printf(" & ");
        if (flags & 0x04) printf(" | ");
        if (flags & 0x02) printf(" ^ ");
    }
}

void dumpEvalExpression() {
    dumpLoadValue();
    while (1) {
        byte flags = readByte();
        if (flags & 0x80)
            break;
        dumpEvalBinaryOp(flags);
        dumpLoadValue();
    }
}

struct Opcode {
    const char *name, *args;
};

Opcode opcodes[108];

/* Args:
    b byte
    B byte (hex)
    w word LE
    W word BE
    c word LE (hex)
    C word BE (hex)
    V value
    E expression
    O jump offset
    D text index; 1: DESC; 2: DIAL
    M brain parameters
    I drawIcone parameters
*/

void setOpcode(int index, const char *args) {
    opcodes[index].args = args;
}

void setupOpcodes() {
    setOpcode(0, "exchangeObject");
    setOpcode(1, "exchangeObject3");
    setOpcode(2, "drawObjectBox(flag: ?b)");
    setOpcode(3, "takeZapStik");
    setOpcode(4, "drawIconeLeftRight(?I)");
    setOpcode(5, "drawIconeRightLeft(?I)");
    setOpcode(6, "drawIconeTopDown(?I)");
    setOpcode(7, "drawIconeDownTop(?I)");
    setOpcode(8, "drawIcone6(?I)");
    // op 9 unused
    setOpcode(10, "drawIconeOther2(?I)");
    setOpcode(11, "drawIconeOther(?I)");
    setOpcode(12, "drawIcone5(?I)");
    setOpcode(13, "drawIconeZoomIn(?I)");
    // op 14 unused
    setOpcode(15, "drawIcone7(?I, value: ?b)");
    setOpcode(16, "drawPUZZL2(index: ?b; x: ?b; y: ?b)");
    setOpcode(17, "gotoScript(?c)");
    setOpcode(18, "drawARPLA2(index: ?b)");
    setOpcode(19, "showTextBox(?D1)");
    setOpcode(20, "setCurrZoneRect(mask: ?B; value: ?B)");
    setOpcode(21, "op_sub_292C0");
    setOpcode(22, "showDIALETextBubble3(index: ?D2)");
    setOpcode(23, "playAnico(?b, ?b, ?b)");
    setOpcode(24, "removeIconeLeftRight(index: ?b)");
    setOpcode(25, "removeIconeRightLeft(index: ?b)");
    setOpcode(26, "removeIconeTopDown(index: ?b)");
    setOpcode(27, "removeIconeDownTop(index: ?b)");
    // op 28 unused
    setOpcode(29, "removeIconeOther2(index: ?b)");
    setOpcode(30, "removeIconeOther(index: ?b)");
    setOpcode(31, "removeIconeUnk(index: ?b)");
    // op 32 unused
    setOpcode(33, "j_op_sub_26DBB(index: ?b)");
    setOpcode(34, "op_removeUnkStruct1(index: ?b)");
    setOpcode(35, "removeIcones");
    setOpcode(36, "gotoZone3(zoneIndex: ?b)");
    setOpcode(37, "gameOver");
    setOpcode(38, "drawGaussTextBubble(index: ?D2)");
    setOpcode(39, "selectMouseZone(cursor: ?b; zoneMask: ?B; zoneValue: ?B)");
    setOpcode(40, "drawTextBubble(index: ?D2; maxTextWidth: ?b; x: ?b; y: ?b)");
    // op 41 unused
    setOpcode(42, "removeTextBubbles");
    setOpcode(44, "waitForMouseClickTimeout(?b)");
    setOpcode(45, "waitForMouseClick");
    setOpcode(46, "op_sub_29310");
    setOpcode(47, "op_sub_27E05");
    setOpcode(48, "op_sub_2939C");
    setOpcode(49, "op_sub_29484");
    setOpcode(50, "goto ?O");
    setOpcode(51, "call ?O");
    setOpcode(52, "return");
    setOpcode(53, "gotoZone1(?b)");
    setOpcode(54, "drawTextBox(index: ?D1; maxTextWidth: ?b; x: ?b; y: ?b)");
    setOpcode(55, "playAnimation(index: ?b; x: ?b; y: ?b)");
    setOpcode(56, "op_sub_2852E(?b)");
    setOpcode(57, "op_loc_28564(?b)");
    setOpcode(58, "setVar ?V to ?E");
    setOpcode(59, "if ?E else goto ?O");
    setOpcode(60, "runBrain(?M)");
    setOpcode(61, "op_sub_27505");
    setOpcode(63, "removeTextBoxes");
    setOpcode(64, "op_sub_28313");
    setOpcode(65, "gotoZone2(?b)");
    setOpcode(66, "op_sub_288BD");
    setOpcode(67, "op_sub_2639C");
    setOpcode(68, "op_sub_2774D");
    setOpcode(69, "op_sub_27952");
    setOpcode(70, "op_sub_27902");
    setOpcode(71, "op_sub_278D8");
    setOpcode(72, "op_sub_27927");
    // op 73 unused
    setOpcode(74, "op_sub_294F0");
    setOpcode(75, "op_sub_28AF1");
    setOpcode(76, "op_sub_1DED5(animCommand: ?c)");
    setOpcode(77, "op_loc_2951F");
    setOpcode(78, "op_sub_29519");
    setOpcode(79, "op_sub_2954E(roomObject: ?b)");
    setOpcode(80, "exchangeObject2");
    setOpcode(81, "op_sub_28AAD");
    setOpcode(82, "setCurrObjectByFlags(index: ?b; count: ?b; flags: ?B)");
    setOpcode(83, "op_sub_27981");
    setOpcode(84, "op_sub_295C1");
    setOpcode(85, "op_sub_279AA");
    setOpcode(87, "drawLutin1(index: ?b; x: ?b; y: ?b)");
    setOpcode(88, "op_sub_28AD6");
    setOpcode(89, "nop0");
    setOpcode(90, "op_sub_27A0A");
    setOpcode(91, "op_sub_2964D");
    setOpcode(92, "op_sub_29668");
    setOpcode(93, "nop1(?b)");
    setOpcode(94, "drawPUZZL(index: ?b; x: ?b; y: ?b)");
    setOpcode(95, "op_sub_297DA");
    setOpcode(96, "showDIALETextBubble4(?D2)");
    setOpcode(97, "doSciPower(index: ?b)");
    setOpcode(98, "op_sub_29827");
    setOpcode(99, "op_sub_27FBB");
    setOpcode(100, "op_sub_27889(flag: ?b)");
    setOpcode(101, "op_sub_2785C");
    setOpcode(102, "eat1(?b)");
    setOpcode(103, "op_sub_1EF81(sfx: ?b)");
    setOpcode(104, "playSound(sfx: ?b; ?b)");
    setOpcode(105, "nop");
    setOpcode(106, "drawARPLA(index: ?b)");
}

void setupGameVars() {
    // Table 1
    addGameVar(1, 0, "sciPowerStickyFingersDefaultAnimCommand1");
    addGameVar(1, 2, "sciPowerKnowMindDefaultAnimCommand");
    addGameVar(1, 4, "sciPowerZoneScanDefaultAnimCommand");
    addGameVar(1, 6, "sciPowerTuneInDefaultAnimCommand");
    addGameVar(1, 8, "sciPowerExtremeViolenceDefaultAnimCommand1");
    addGameVar(1, 10, "sciPowerSciShiftDefaultAnimCommand1");
    addGameVar(1, 12, "sciPowerBrainWarpDefaultAnimCommand");
    addGameVar(1, 14, "waitReactionAnimCommmand");
    addGameVar(1, 16, "gameTime");
    for (int i = 0; i < 15; i++) {
        char str[128];
        sprintf(str, "sciPowerRect[%d].sciPowerStickyFingersAnimCommand", i);
        addGameVar(1, 18 + 10 * i, str);
        sprintf(str, "sciPowerRect[%d].sciPowerBrainWarpAnimCommand", i);
        addGameVar(1, 20 + 10 * i, str);
        sprintf(str, "sciPowerRect[%d].sciPowerBrainWarpAnimCommand", i);
        addGameVar(1, 22 + 10 * i, str);
        sprintf(str, "sciPowerRect[%d].sciPowerZoneShiftAnimCommand", i);
        addGameVar(1, 24 + 10 * i, str);
        sprintf(str, "sciPowerRect[%d].sciPowerExtremeViolenceAnimCommand", i);
        addGameVar(1, 26 + 10 * i, str);
    }
    addGameVar(1, 168, "animCommand5");
    addGameVar(1, 170, "sciPowerSolarEyesDefaultAnimCommand");
    addGameVar(1, 172, "sciPowerStickyFingersDefaultAnimCommand2");
    addGameVar(1, 174, "animCommmand13");
    addGameVar(1, 176, "sciPowerSciShiftDefaultAnimCommand2");
    addGameVar(1, 178, "sciPowerExtremeViolenceDefaultAnimCommand2");
    addGameVar(1, 180, "sciPowerTuneInDefaultAnimCommand2");
    addGameVar(1, 182, "animCommand3");
    addGameVar(1, 184, "sciPowerZoneScanDefaultAnimCommand2");
    // Table 2
    int offset = 0;
    addGameVar(2, offset++, "zoneIndex");
    addGameVar(2, offset++, "ARPLA_index");
    addGameVar(2, offset++, "byte_19989");
    addGameVar(2, offset++, "currZoneRectIndex");
    addGameVar(2, offset++, "gf_byte_1998B");
    addGameVar(2, offset++, "prevZoneIndex");
    addGameVar(2, offset++, "unk998D");
    addGameVar(2, offset++, "unk998E");
    addGameVar(2, offset++, "gf_byte_1998F");
    addGameVar(2, offset++, "unk9990");
    addGameVar(2, offset++, "unk9991");
    addGameVar(2, offset++, "gf_byte_19992");
    addGameVar(2, offset++, "currZoneId");
    addGameVar(2, offset++, "unkStruct4Flag");
    addGameVar(2, offset++, "smallTickCount");
    addGameVar(2, offset++, "byte_19996");
    addGameVar(2, offset++, "unk9997");
    addGameVar(2, offset++, "timer");
    addGameVar(2, offset++, "byte_19999");
    addGameVar(2, offset++, "testMouseZoneMask");
    addGameVar(2, offset++, "testMouseZoneValue");
    addGameVar(2, offset++, "unk999C");
    addGameVar(2, offset++, "unk999D");
    addGameVar(2, offset++, "unk999E");
    addGameVar(2, offset++, "unk999F");
    addGameVar(2, offset++, "unk99A0");
    addGameVar(2, offset++, "unk99A1");
    addGameVar(2, offset++, "unk99A2");
    addGameVar(2, offset++, "unk99A3");
    addGameVar(2, offset++, "unk99A4");
    addGameVar(2, offset++, "unk99A5");
    addGameVar(2, offset++, "unk99A6");
    addGameVar(2, offset++, "currUnkStruct4Index");
    addGameVar(2, offset++, "waitValue2");
    addGameVar(2, offset++, "gf_byte_199A9");
    addGameVar(2, offset++, "mouseOverObjectRectIndex");
    addGameVar(2, offset++, "unk99AB");
    addGameVar(2, offset++, "gf_waitCounter");
    addGameVar(2, offset++, "gf_byte_199AD");
    addGameVar(2, offset++, "gf_byte_199AE");
    addGameVar(2, offset++, "unk99AF");
    addGameVar(2, offset++, "unk99B0");
    addGameVar(2, offset++, "unk99B1");
    addGameVar(2, offset++, "ARPLA_index2");
    addGameVar(2, offset++, "waitValue1");
    addGameVar(2, offset++, "unk99B4");
    addGameVar(2, offset++, "zone_byte_199B5");
    addGameVar(2, offset++, "unk99B6");
    addGameVar(2, offset++, "byte_199B7");
    addGameVar(2, offset++, "gf_counter");
    addGameVar(2, offset++, "unk99B9");
    addGameVar(2, offset++, "gf_byte_199BA");
    addGameVar(2, offset++, "gf_byte_199BB");
    addGameVar(2, offset++, "unk99BC");
    addGameVar(2, offset++, "gf_byte_199BD");
    addGameVar(2, offset++, "byte_199BE");
    addGameVar(2, offset++, "gf_zone_byte_199BF");
    addGameVar(2, offset++, "gf_byte_199C0");
    addGameVar(2, offset++, "byte_199C1");
    addGameVar(2, offset++, "gf_counter2");
    addGameVar(2, offset++, "byte_199C3");
    addGameVar(2, offset++, "flagByte199C4");
    addGameVar(2, offset++, "gf_byte_199C5");
    addGameVar(2, offset++, "byte_199C6");
    addGameVar(2, offset++, "byte_199C7");
    addGameVar(2, offset++, "byte_199C8");
    addGameVar(2, offset++, "unk99C9");
    addGameVar(2, offset++, "gf_byte_199CA");
    addGameVar(2, offset++, "dirtyRectsUsedValue");
    addGameVar(2, offset++, "gf_byte_199CC");
    addGameVar(2, offset++, "unk99CD");
    addGameVar(2, offset++, "statusFlags");
    addGameVar(2, offset++, "byte_199CF");
    addGameVar(2, offset++, "zoneRectFlags");
    addGameVar(2, offset++, "unk99D1");
    addGameVar(2, offset++, "gf_byte_199D2");
    addGameVar(2, offset++, "unk99D3");
    addGameVar(2, offset++, "unk99D4");
    addGameVar(2, offset++, "byte_199D5");
    addGameVar(2, offset++, "unk99D6");
    addGameVar(2, offset++, "unk99D7");
    addGameVar(2, offset++, "unk99D8");
    addGameVar(2, offset++, "unk99D9");
    addGameVar(2, offset++, "unk99DA");
    addGameVar(2, offset++, "unk99DB");
    addGameVar(2, offset++, "unk99DC");
    addGameVar(2, offset++, "unk99DD");
    addGameVar(2, offset++, "gf_byte_199DE");
    addGameVar(2, offset++, "unk99DF");
    addGameVar(2, offset++, "unk99E0");
    addGameVar(2, offset++, "sciEnergy");
    addGameVar(2, offset++, "unk99E2");
    addGameVar(2, offset++, "unk99E3");
    addGameVar(2, offset++, "unk99E4");
    addGameVar(2, offset++, "unk99E5");
    addGameVar(2, offset++, "gf_ARPLA_index");
    addGameVar(2, offset++, "byte_199E7");
    addGameVar(2, offset++, "gf_byte_199E8");
    addGameVar(2, offset++, "unk99E9");
    addGameVar(2, offset++, "objectRectsFlag");
    addGameVar(2, offset++, "unk99EB");
    addGameVar(2, offset++, "unk99EC");
    addGameVar(2, offset++, "byte_199ED");
    addGameVar(2, offset++, "byte_199EE");
    addGameVar(2, offset++, "gf_byte_199EF");
    addGameVar(2, offset++, "unk99F0");
    addGameVar(2, offset++, "byte_199F1");
    addGameVar(2, offset++, "unk99F2");
    addGameVar(2, offset++, "gf_byte_199F3");
    addGameVar(2, offset++, "byte_199F4");
    addGameVar(2, offset++, "unk99F5");
    addGameVar(2, offset++, "unk99F6");
    addGameVar(2, offset++, "unk99F7");
    // Table 3
    addGameVar(3, 0, "currObject.flags");
    addGameVar(3, 1, "currObject.zoneId");
    addGameVar(3, 3, "currObject.VEPCE_index");
    // Table 4
    for (int i = 0; i < 32; i++) {
        char str[128];
        sprintf(str, "currZoneRects[%d].rect.x1", i);
        addGameVar(4, 0 + 8 * i, str);
        sprintf(str, "currZoneRects[%d].rect.x2", i);
        addGameVar(4, 1 + 8 * i, str);
        sprintf(str, "currZoneRects[%d].rect.y1", i);
        addGameVar(4, 2 + 8 * i, str);
        sprintf(str, "currZoneRects[%d].rect.y2", i);
        addGameVar(4, 3 + 8 * i, str);
        sprintf(str, "currZoneRects[%d].flags", i);
        addGameVar(4, 4 + 8 * i, str);
        sprintf(str, "currZoneRects[%d].MOTSE_index", i);
        addGameVar(4, 5 + 8 * i, str);
        sprintf(str, "currZoneRects[%d].animCommand", i);
        addGameVar(4, 6 + 8 * i, str);
    }
    // Table 5
    for (int i = 0; i < 41; i++) {
        char str[128];
        sprintf(str, "unkStruct4[%d].zoneId", i);
        addGameVar(5, 0 + 5 * i, str);
        sprintf(str, "unkStruct4[%d].zoneRectIndex", i);
        addGameVar(5, 1 + 5 * i, str);
        sprintf(str, "unkStruct4[%d].MOTSE_index", i);
        addGameVar(5, 2 + 5 * i, str);
        sprintf(str, "unkStruct4[%d].zoneRectValue", i);
        addGameVar(5, 3 + 5 * i, str);
        sprintf(str, "unkStruct4[%d].objectRectIndex", i);
        addGameVar(5, 4 + 5 * i, str);
    }
    // Table 6
    for (int i = 0; i < 18; i++) {
        char str[128];
        sprintf(str, "roomObjects1[%d].flags", i);
        addGameVar(6, 0 + 6 * i, str);
        sprintf(str, "roomObjects1[%d].zoneId", i);
        addGameVar(6, 1 + 6 * i, str);
        sprintf(str, "roomObjects1[%d].SPRIT_index", i);
        addGameVar(6, 2 + 6 * i, str);
        sprintf(str, "roomObjects1[%d].VEPCE_index", i);
        addGameVar(6, 3 + 6 * i, str);
        sprintf(str, "roomObjects1[%d].animCommand", i);
        addGameVar(6, 4 + 6 * i, str);
    }
    for (int i = 0; i < 20; i++) {
        char str[128];
        sprintf(str, "roomObjects2[%d].flags", i);
        addGameVar(6, 18 * 6 + 0 + 6 * i, str);
        sprintf(str, "roomObjects2[%d].zoneId", i);
        addGameVar(6, 18 * 6 + 1 + 6 * i, str);
        sprintf(str, "roomObjects2[%d].SPRIT_index", i);
        addGameVar(6, 18 * 6 + 2 + 6 * i, str);
        sprintf(str, "roomObjects2[%d].VEPCE_index", i);
        addGameVar(6, 18 * 6 + 3 + 6 * i, str);
        sprintf(str, "roomObjects2[%d].animCommand", i);
        addGameVar(6, 18 * 6 + 4 + 6 * i, str);
    }
    // Table 7
    for (int i = 0; i < 17; i++) {
        char str[128];
        sprintf(str, "roomObjects3[%d].flags", i);
        addGameVar(7, 0 + 6 * i, str);
        sprintf(str, "roomObjects3[%d].zoneId", i);
        addGameVar(7, 1 + 6 * i, str);
        sprintf(str, "roomObjects3[%d].SPRIT_index", i);
        addGameVar(7, 2 + 6 * i, str);
        sprintf(str, "roomObjects3[%d].VEPCE_index", i);
        addGameVar(7, 3 + 6 * i, str);
        sprintf(str, "roomObjects3[%d].animCommand", i);
        addGameVar(7, 4 + 6 * i, str);
    }
    for (int i = 0; i < 8; i++) {
        char str[128];
        sprintf(str, "roomObjects4[%d].flags", i);
        addGameVar(7, 17 * 6 + 0 + 6 * i, str);
        sprintf(str, "roomObjects4[%d].zoneId", i);
        addGameVar(7, 17 * 6 + 1 + 6 * i, str);
        sprintf(str, "roomObjects4[%d].SPRIT_index", i);
        addGameVar(7, 17 * 6 + 2 + 6 * i, str);
        sprintf(str, "roomObjects4[%d].VEPCE_index", i);
        addGameVar(7, 17 * 6 + 3 + 6 * i, str);
        sprintf(str, "roomObjects4[%d].animCommand", i);
        addGameVar(7, 17 * 6 + 4 + 6 * i, str);
    }
    // Table 8
    addGameVar(8, 0, "currUnkStruct4.zoneId");
    addGameVar(8, 1, "currUnkStruct4.zoneRectIndex");
    addGameVar(8, 2, "currUnkStruct4.MOTSE_index");
    addGameVar(8, 3, "currUnkStruct4.zoneRectValue");
    addGameVar(8, 4, "currUnkStruct4.objectRectIndex");
}

void dumpScript() {

    // drawIcone4: Right to left

    byte *scriptDataStart = scriptData;
    byte *scriptDataEnd = scriptData + scriptSize;

    while (scriptData < scriptDataEnd) {
        int offs = baseOffs + (scriptData - scriptDataStart);
        uint16 textIndex;
        char textType = 0;

        printf("%04X  ", offs);

        byte op = readByte();
        if (op == 0 || op >= 108) {
            printf("end (%02X)\n\n", op);
            continue;
        }
        
        op--;
        printf("%02X (%03d): ", op, op);
        Opcode &opcode = opcodes[op];

        if (!opcode.args) {
            printf("Opcode %d not implemented!\n", op);
            //system("PAUSE");
            exit(1);
            return;
        }
        
        const char *args = opcode.args;
        
        while (*args != 0) {
            char ch = *args++;
            if (ch == '?') {
                char atype = *args++;
                switch (atype) {
                case 'b':
                    printf("%d", readByte());
                    break;
                case 'B':
                    printf("%02X", readByte());
                    break;
                case 'w':
                    printf("%d", readUint16());
                    break;
                case 'W':
                    printf("%d", readUint16BE());
                    break;
                case 'c':
                    printf("%04X", readUint16());
                    break;
                case 'C':
                    printf("%04X", readUint16BE());
                    break;
                case 'V':
                    dumpLoadValue();
                    break;
                case 'E':
                    dumpEvalExpression();
                    break;
                case 'O':
                    printf("%04X", readUint16());
                    break;
                case 'D':
                    textIndex = readDESCIndex();
                    textType = *args++;
                    printf("%d", textIndex);
                    break;
                case 'M':
                {
                    byte bubbles = readByte();
                    for (int i = 0; i < 8; i++) {
                        if (bubbles & (1 << i)) {
                            byte verb = readByte();
                            uint16 cmd = readUint16BE();
                            printf("{%d: verb(%d:\"", i, verb);
                            dumpLoadText("Q:\\OldGames\\Kult\\chamber\\VEPCE.BIN", verb - 4);
                            printf("\"), cmd(%04X)}", cmd);
                        }
                    }
                    break;
                }
                case 'I':
                {
                    byte index = readByte();
                    if (index != 0xFF) {
                        int x = readByte();
                        int y = readByte();
                        printf("index: %d; x: %d; y: %d", index, x, y);
                    } else {
                        printf("index: %d", index);
                    }
                    break;
                }
                default:
                    printf("Unknown arg type '%c'!\n", atype);
                    break;
                }
            } else
                printf("%c", ch);
        }

        if (textType != 0) {
            printf("\t// ");
            if (textType == '1') {
                dumpLoadText("Q:\\OldGames\\Kult\\chamber\\DESCE.BIN", textIndex);
            } else if (textType == '2') {
                dumpLoadText("Q:\\OldGames\\Kult\\chamber\\DIALE.BIN", textIndex);
            }
        }
        
        printf("\n");
        fflush(stdout);

    }


}

int main(int argc, char *argv[]) {

/*
    dumpLoadText("Q:\\OldGames\\Kult\\chamber\\MOTSE.BIN", 0);
    system("PAUSE");
    return 0;
*/
    FILE *script= fopen("Q:\\OldGames\\Kult\\Res\\KULTEGA.BIN", "rb");
    int count = 0;

    memset(opcodes, 0, sizeof(opcodes));
    
    setupOpcodes();
    setupGameVars();

    freopen("Q:\\OldGames\\Kult\\Tools\\debug.log", "wt", stdout);
    
    //dumpGameVars();

    while (1) {
        byte size;
        byte scriptBuffer[256];
        fread(&size, 1, 1, script);
        baseOffs = ftell(script);
        if (feof(script))
            break;
        size--;
        if (fread(scriptBuffer, size, 1, script) != 1)
            break;

        //if (count == 18)
        {
            printf("size = %d\n", size);
            printf("index = %04X\n", count + 1);
            scriptData = scriptBuffer;
            scriptSize = size;
            dumpScript();
            printf("===================================\n");
            //system("PAUSE");
        }
        
        //break;
        count++;
    }

    fclose(script);

    printf("count = %d\n", count);
    printf("tableMax = %d\n", tableMax);
    for (int i = 0; i <= tableMax; i++)
        printf("tableMaxOffset[%d] = %d\n", i, tableMaxOffset[i]);
    
    //system("PAUSE");
    return EXIT_SUCCESS;
}
