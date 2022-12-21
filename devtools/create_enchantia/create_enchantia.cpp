/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#define COMPRESS

#include <vector>
#include "create_enchantia.h"
#include "autotables.h"
#include "usertables.h"
#include "md5.h"

void writeString(FILE *f, const char *str) {
	int len = strlen(str);
	writeByte(f, len);
	fwrite(str, len, 1, f);
}

FILE *datFile;
uint32 dataStart; // Start of the data segment
byte *data;
uint32 dataSize;
uint32 gameID;

const char *getString(uint32 offset) {
	return (const char*)(data + dataStart + offset);
}

uint32 getOffset(uint32 offset) {
	return READ_LE_UINT16(data + offset);
}

uint32 getOffsetAt(uint32 offset, int index) {
	return getOffset(offset + index * 2);
}

void loadExe(const char *filename) {
	FILE *exe = fopen(filename, "rb");
	dataSize = fileSize(exe);
	data = new byte[dataSize];
	fread(data, dataSize, 1, exe);
	fclose(exe);
}

struct SpriteDef {
	uint16 selfId;
	byte type;
	byte status;
	byte spriteIndex;
	int16 x, y;
	uint16 templateId;
};

struct SpriteTemplate {
	uint16 selfId;
	int16 heightAdd;
	int16 yAdd;
	uint16 id;
	uint16 animListId;
	byte animListTicks;
	byte animListInitialTicks;
	uint16 moveXListId;
	byte moveXListTicks;
	byte moveXListInitialTicks;
	uint16 moveYListId;
	byte moveYListTicks;
	byte moveYListInitialTicks;
};

struct SceneStripInfo {
	uint16 tableOffset;
	uint32 stripOffset;
	byte count;
	byte *table;
};

struct AnimCodeItem {
	uint16 selfId;
	byte *code;
	int len;
};

const int kAnim		= 0;
const int kMoveX	= 1;
const int kMoveY	= 2;

std::vector<SpriteDef> spriteDefs;
std::vector<SpriteTemplate> spriteTemplates;
std::vector<AnimCodeItem> animCodeTables[3]; // anim/moveX/moveY

int calcAnimCommandsSize(byte *cmds, bool isAnim, uint16 &nextOffset) {
	// Parse the anim code until an 'exit'-opcode is encountered
	// nextOffset returns the offset of the switched code, if any, else 0
	byte *code = cmds;
	int len = 0;
	nextOffset = 0;
	// Anim list
	// 0x7F		jump to beginning of the code
	// 0x7E		change command list				2 bytes
	// 0x7D		terminate the sprite
	// 0x7C		terminate this list
	// 0x7B										1 byte
	// 0x7A										1 byte
	// 0x79		repeat previous
	// 0x78										2 bytes
	// 0xFF
	// 0xFE
	// 0xFD
	// 0xFC
	// 0xFB										1 byte
	// 0xFA
	// 0xF9
	// 0xF8										1 byte
	// 0xF7										2 bytes
	// 0xF6										2 bytes
	// MoveX/MoveY list
	// 0x80
	// 0x7F		jump to beginning of the code
	// 0x7E		change command list				2 bytes
	// 0x7D		terminate the sprite
	// 0x7C		terminate this list
	// 0x7B										1 byte
	// 0x7A										1 byte
	// 0x79		repeat previous
	// 0x78										2 bytes
	while (1) {
		byte cmd = *code++;
		if (cmd == 0x7E)
			nextOffset = READ_LE_UINT16(code);
		if (cmd == 0x7E || cmd == 78 || (isAnim && cmd == 0xF7) || (isAnim && cmd == 0xF6))
			code += 2;
		else if (cmd == 0x7B || cmd == 0x7A || (isAnim && cmd == 0xFB) || (isAnim && cmd == 0xF8))
			code++;
		if (cmd == 0x7F || cmd == 0x7E || cmd == 0x7D || cmd == 0x7C || cmd == 0x79)
			break;
	}
	len = code - cmds;
	/*
	printf("len = %d\n", len);
	for (int i = 0; i < len; i++)
		printf("%02X ", cmds[i]);
	printf("\n");
	*/
	return len;
}

uint16 extractAnimTable(uint16 offset, int type) {
	if (offset) {
		std::vector<AnimCodeItem> &table = animCodeTables[type];
		byte *item = data + dataStart + offset;
		bool append = true;
		uint16 nextOffset;
		AnimCodeItem animCodeItem;
		animCodeItem.selfId = offset;
		animCodeItem.code = item;
		animCodeItem.len = calcAnimCommandsSize(item, true, nextOffset);
		for (uint32 i = 0; i < table.size(); i++) {
			if (table[i].selfId == animCodeItem.selfId) {
				append = false;
				break;
			}
		}
		if (append) {
			//printf("nextOffset: %04X\n", nextOffset);
			table.push_back(animCodeItem);
			if (nextOffset)
				extractAnimTable(nextOffset, type);
		}
	}
	return offset;
}

/*
uint16 extractAnimTable(uint16 offset) {
	if (offset) {
		byte *item = data + dataStart + offset;
		int len = calcAnimCommandsSize(item, true);
	}
	return offset;
}
*/

void extractSceneItemInfo(uint32 offset, uint32 count) {
	byte *item = data + dataStart + offset;
	writeUint32LE(datFile, count);
	for (uint32 i = 0; i < count; i++) {
		byte flags = item[0];
		byte spriteIndex = item[1];
		writeByte(datFile, flags);
		writeByte(datFile, spriteIndex);
		item += 2;
	}
}

void addSpriteDef(SpriteDef &spriteDef) {
	for (uint32 i = 0; i < spriteDefs.size(); i++) {
		if (spriteDefs[i].selfId == spriteDef.selfId)
			return;
	}
	spriteDefs.push_back(spriteDef);
}

void addSpriteTemplate(SpriteTemplate &spriteTemplate) {
	for (uint32 i = 0; i < spriteTemplates.size(); i++) {
		if (spriteTemplates[i].selfId == spriteTemplate.selfId)
			return;
	}
	spriteTemplates.push_back(spriteTemplate);
}

uint16 extractSpriteTemplate(uint16 offset) {
	if (offset) {
		byte *item = data + dataStart + offset;
		SpriteTemplate spriteTemplate;
		spriteTemplate.selfId = offset;
		spriteTemplate.heightAdd = READ_LE_UINT16(item + 0);
		spriteTemplate.yAdd = READ_LE_UINT16(item + 2);
		spriteTemplate.id = READ_LE_UINT16(item + 4);
		spriteTemplate.animListId = extractAnimTable(READ_LE_UINT16(item + 6), kAnim);
		spriteTemplate.animListTicks = item[8];
		spriteTemplate.animListInitialTicks = item[9];
		spriteTemplate.moveXListId = extractAnimTable(READ_LE_UINT16(item + 10), kMoveX);
		spriteTemplate.moveXListTicks = item[12];
		spriteTemplate.moveXListInitialTicks = item[13];
		spriteTemplate.moveYListId = extractAnimTable(READ_LE_UINT16(item + 14), kMoveY);
		spriteTemplate.moveYListTicks = item[16];
		spriteTemplate.moveYListInitialTicks = item[17];
		addSpriteTemplate(spriteTemplate);
		/*
		printf("SPRTMP(%04X) %03d; %03d; id: %04X; (%04X %02d %02d);  (%04X %02d %02d);  (%04X %02d %02d)\n",
			spriteTemplate.selfId, spriteTemplate.heightAdd, spriteTemplate.yAdd, spriteTemplate.id,
			spriteTemplate.animListId, spriteTemplate.animListTicks, spriteTemplate.animListInitialTicks,
			spriteTemplate.moveXListId, spriteTemplate.moveXListTicks, spriteTemplate.moveXListInitialTicks,
			spriteTemplate.moveYListId, spriteTemplate.moveYListTicks, spriteTemplate.moveYListInitialTicks);
		*/
	}
	return offset;
}

void extractSpriteDef(uint32 offset) {
	byte *item = data + dataStart + offset;
	SpriteDef spriteDef;
	spriteDef.selfId = offset;
	spriteDef.type = item[0];
	spriteDef.status = item[1];
	spriteDef.spriteIndex = item[2];
	spriteDef.x = READ_LE_UINT16(item + 3);
	spriteDef.y = READ_LE_UINT16(item + 5);
	spriteDef.templateId = extractSpriteTemplate(READ_LE_UINT16(item + 7));
	addSpriteDef(spriteDef);
	/*
	printf("SPRDEF(%04X): type: %02X; status: %02X; spriteIndex: %03d; x: %3d; y: %3d; templateId: %04X\n",
		spriteDef.selfId, spriteDef.type, spriteDef.status, spriteDef.spriteIndex, spriteDef.x, spriteDef.y, spriteDef.templateId);
	*/
}

void extractSceneSpriteDefList(uint32 offset) {
	std::vector<uint16> spriteDefList;
	while (READ_LE_UINT16(data + dataStart + offset) != 0xFFFF) {
		spriteDefList.push_back(offset);
		extractSpriteDef(offset);
		offset += 9;
	}
	// Write the SpriteDef list
	writeUint32LE(datFile, spriteDefList.size());
	for (uint32 i = 0; i < spriteDefList.size(); i++)
		writeUint16LE(datFile, spriteDefList[i]);
}

void extractSceneSpriteDefTables(uint32 offset, uint32 count) {
	byte *item = data + dataStart + offset;
	writeUint32LE(datFile, count);
	for (uint32 i = 0; i < count; i++) {
		uint16 listOffset = READ_LE_UINT16(item);
		//printf("listOffset: %04X\n", listOffset);
		item += 2;
		extractSceneSpriteDefList(listOffset);
		//printf("---------------\n");
	}
}

void extractSceneStripInfos(uint32 offset, uint32 count) {
	std::vector<SceneStripInfo> sceneStripInfos;
	std::vector<uint16> stripTable;
	byte *item = data + dataStart + offset;
	uint32 stripOffset = 0;
	for (uint32 i = 0; i < count; i++) {
		uint16 tableOffset = READ_LE_UINT16(item);
		bool doAppend = true;
		uint32 thisStripOffset = stripOffset;
		SceneStripInfo sceneStripInfo;
		sceneStripInfo.tableOffset = tableOffset;
		sceneStripInfo.stripOffset = stripOffset;
		sceneStripInfo.table = data + dataStart + tableOffset;
		sceneStripInfo.count = *sceneStripInfo.table++;
		for (uint32 j = 0; j < sceneStripInfos.size(); j++) {
			if (sceneStripInfos[j].tableOffset == tableOffset) {
				doAppend = false;
				thisStripOffset = sceneStripInfos[j].stripOffset;
			}
		}
		if (doAppend) {
			sceneStripInfos.push_back(sceneStripInfo);
			stripOffset += sceneStripInfo.count + 1;
		}
		stripTable.push_back(thisStripOffset);
		item += 2;
	}
	writeUint32LE(datFile, stripOffset);
	for (uint32 i = 0; i < sceneStripInfos.size(); i++) {
		SceneStripInfo &sceneStripInfo = sceneStripInfos[i];
		writeByte(datFile, sceneStripInfo.count);
		fwrite(sceneStripInfo.table, sceneStripInfo.count, 1, datFile);
	}
	writeUint32LE(datFile, count);
	for (uint32 i = 0; i < stripTable.size(); i++) {
		writeUint16LE(datFile, stripTable[i]);
	}
}

void extractSceneFilenames(uint32 offset, uint32 count) {
	byte *item = data + dataStart + offset;
	writeUint32LE(datFile, count);
	for (uint32 i = 0; i < count; i++) {
		for (int n = 0; n < 3; n++) {
			uint16 nameOffset = READ_LE_UINT16(item);
			if (nameOffset) {
				const char *name = getString(nameOffset);
				writeString(datFile, name);
			} else {
				writeByte(datFile, 0);
			}
			item += 2;
		}
		item += 2;
	}
}

void extractBrdFilenames(uint32 offset, uint32 count) {
	byte *item = data + dataStart + offset;
	writeUint32LE(datFile, count);
	for (uint32 i = 0; i < count; i++) {
		uint16 nameOffset = READ_LE_UINT16(item);
		if (nameOffset) {
			const char *name = getString(nameOffset);
			writeString(datFile, name);
		} else {
			writeByte(datFile, 0);
		}
		item += 2;
	}
}

void extractSceneInitItems(uint32 offset, uint32 count) {
	byte *item = data + dataStart + offset;
	writeUint32LE(datFile, count);
	for (uint32 i = 0; i < count; i++) {
		byte status = item[0];
		byte spriteIndex = item[1];
		int16 x = READ_LE_UINT16(item + 2);
		int16 y = READ_LE_UINT16(item + 4);
		uint16 animList = extractAnimTable(READ_LE_UINT16(item + 6), kAnim);
		uint16 moveXList = extractAnimTable(READ_LE_UINT16(item + 8), kMoveX);
		uint16 moveYList = extractAnimTable(READ_LE_UINT16(item + 10), kMoveY);
		byte cameraStripNum = item[12];
		byte spriteResourceType = item[13];
		writeByte(datFile, status);
		writeByte(datFile, spriteIndex);
		writeUint16LE(datFile, x);
		writeUint16LE(datFile, y);
		writeUint16LE(datFile, animList);
		writeUint16LE(datFile, moveXList);
		writeUint16LE(datFile, moveYList);
		writeByte(datFile, cameraStripNum);
		writeByte(datFile, spriteResourceType);
		item += 14;
	}
}

void extractAnimTablesByTable(const uint32 *table, uint32 count, int kind) {
	for (uint32 i = 0; i < count; i++)
		if (table[i])
			extractAnimTable(table[i], kind);
}

void extractSpriteDefsByTable(const uint32 *table, uint32 count) {
	for (uint32 i = 0; i < count; i++)
		if (table[i])
			extractSpriteDef(table[i]);
}

void extractSpriteTemplatesByTable(const uint32 *table, uint32 count) {
	for (uint32 i = 0; i < count; i++)
		if (table[i])
			extractSpriteTemplate(table[i]);
}

// Not part of any table
void extractGridSprite() {
	byte* sprStart = data + dataStart + 0xBC10;
	byte* sprEnd   = data + dataStart + 0xBDD2;
	writeUint32LE(datFile, 0xBDD2 - 0xBC10);

	byte* i = sprStart;
	while (i < sprEnd) {
		writeByte(datFile, *i);
		i++;
	}
}

void extractCreditsText() {
	byte* txtStart = data + dataStart + 0xB74E;
	byte* txtEnd   = data + dataStart + 0xB949;
	writeUint16LE(datFile, 0xB949 - 0xB74E);

	byte* i = txtStart;
	// First bytes point to positions of paragraphs of text - rewrite them relative to starting offset
	while (true) {
		uint16 addr = *(i + 1) << 8 | *(i);
		i = i + 2;

		if (addr == 0xFFFF) {
			writeUint16LE(datFile, 0xFFFF);
			break;
		}
		writeUint16LE(datFile, addr - 0xB74E);
	}
	while (i < txtEnd) {
		writeByte(datFile, *i);
		i++;
	}
}

void extractMusic(uint32 offset, uint32 endOffset, uint32 notesOffset) {
	byte *musicStart = data + 0x200 + offset;
	byte *tracksEnd = data + 0x200 + notesOffset;
	byte *music = musicStart;
	uint32 size = endOffset - offset;
	// Convert the track table
	while (READ_LE_UINT16(music) != 0xFFFE) {
		//printf("1: %08X\n", READ_LE_UINT16(music));
		if (READ_LE_UINT16(music) != 0xFFFF)
			WRITE_LE_UINT16(music, READ_LE_UINT16(music) - offset);
		music += 2;
	}
	// Convert the tracks
	while (music < tracksEnd) {
		//printf("2: %08X ", READ_LE_UINT16(music));
		if (READ_LE_UINT16(music) == 0xFFFE) {
			//printf("-> set loop counter\n");
			music += 4;
		} else if (READ_LE_UINT16(music) == 0xFFFD) {
			//printf("-> jump\n");
			WRITE_LE_UINT16(music + 2, READ_LE_UINT16(music + 2) - offset);
			music += 4;
		} else if (READ_LE_UINT16(music) == 0xFFFF) {
			//printf("-> end\n");
		} else {
			//printf("-> play\n");
			WRITE_LE_UINT16(music, READ_LE_UINT16(music) - offset);
			music += 2;
		}
	}
	writeUint32LE(datFile, size);
	fwrite(musicStart, size, 1, datFile);
}

void extractSoundTablesSB() {
	const uint32 standardItemCount = 15;
	const uint32 sceneItemCount = 119;
	const uint32 sceneSoundTableCount = 78;

	std::vector<uint16> tableOffsets, itemOffsets;
	byte *tables = data + dataStart + 0x901E;
	byte *sceneSoundTable = data + dataStart + 0x9048;
	uint32 patchIndex = 0;

	for (uint32 i = 0; i < 21; i++) {
		tableOffsets.push_back(READ_LE_UINT16(tables));
		tables += 2;
	}

	// Add standard items offsets
	for (uint32 i = 0; i < standardItemCount; i++)
		itemOffsets.push_back(0x8FD3 + i * 5);

	// Add scene items offsets
	for (uint32 i = 0; i < sceneItemCount; i++)
		itemOffsets.push_back(0x9096 + i * 5);

	writeUint32LE(datFile, itemOffsets.size());
	for (uint32 i = 0; i < itemOffsets.size(); i++) {
		uint16 itemOffset = itemOffsets[i];
		byte *item = data + dataStart + itemOffset;
		if (itemOffset == tableOffsets[patchIndex]) {
			tableOffsets[patchIndex] = i;
			patchIndex++;
		}
		writeByte(datFile, item[0]); // priority
		writeByte(datFile, item[1]); // freq
		writeUint16LE(datFile, READ_LE_UINT16(item + 2)); // sizeDecr
		writeByte(datFile, item[4]); // index
	}

	writeUint32LE(datFile, sceneSoundTableCount);
	for (uint32 i = 0; i < sceneSoundTableCount; i++)
		writeByte(datFile, tableOffsets[sceneSoundTable[i]]);

}

int main(int argc, char *argv[]) {

	//dataStart = 0xFE00;
	dataStart = 0xF510;
	// TODO: get filename from arguments
	loadExe("CURSE.EXE");

	datFile = fopen("enchantia.dat", "wb");

	extractSceneItemInfo(0x524D, 178);
	extractSceneSpriteDefTables(0x53B5, 78);
	extractSceneStripInfos(0x757E, 79);
	extractSceneFilenames(0x784E, 79);
	extractBrdFilenames(0x0918, 29);
	extractSceneInitItems(0x859E, 127);

	// Extract the tables whose offsets have been extracted from the original code
	extractAnimTablesByTable(autoAnimCodeTable, ARRAYSIZE(autoAnimCodeTable), kAnim);
	extractAnimTablesByTable(autoMoveXCodeTable, ARRAYSIZE(autoMoveXCodeTable), kMoveX);
	extractAnimTablesByTable(autoMoveYCodeTable, ARRAYSIZE(autoMoveYCodeTable), kMoveY);
	// None yet extractSpriteDefsByTable(autoSpriteDefTable, ARRAYSIZE(autoSpriteDefTable));
	extractSpriteTemplatesByTable(autoTemplateTable, ARRAYSIZE(autoTemplateTable));

	// Extract the tables whose offsets have been entered manually
	extractAnimTablesByTable(userAnimCodeTable, ARRAYSIZE(userAnimCodeTable), kAnim);
	extractAnimTablesByTable(userMoveXCodeTable, ARRAYSIZE(userMoveXCodeTable), kMoveX);
	extractAnimTablesByTable(userMoveYCodeTable, ARRAYSIZE(userMoveYCodeTable), kMoveY);
	extractSpriteDefsByTable(userSpriteDefTable, ARRAYSIZE(userSpriteDefTable));
	extractSpriteTemplatesByTable(userTemplateTable, ARRAYSIZE(userTemplateTable));

	// Write SpriteDefs
	writeUint32LE(datFile, spriteDefs.size());
	for (uint32 i = 0; i < spriteDefs.size(); i++) {
		SpriteDef &spriteDef = spriteDefs[i];
		writeUint16LE(datFile, spriteDef.selfId);
		writeByte(datFile, spriteDef.type);
		writeByte(datFile, spriteDef.status);
		writeByte(datFile, spriteDef.spriteIndex);
		writeUint16LE(datFile, spriteDef.x);
		writeUint16LE(datFile, spriteDef.y);
		writeUint16LE(datFile, spriteDef.templateId);
	}

	// Write SpriteTemplates
	writeUint32LE(datFile, spriteTemplates.size());
	for (uint32 i = 0; i < spriteTemplates.size(); i++) {
		SpriteTemplate &spriteTemplate = spriteTemplates[i];
		writeUint16LE(datFile, spriteTemplate.selfId);
		writeUint16LE(datFile, spriteTemplate.heightAdd);
		writeUint16LE(datFile, spriteTemplate.yAdd);
		writeUint16LE(datFile, spriteTemplate.id);
		writeUint16LE(datFile, spriteTemplate.animListId);
		writeByte(datFile, spriteTemplate.animListTicks);
		writeByte(datFile, spriteTemplate.animListInitialTicks);
		writeUint16LE(datFile, spriteTemplate.moveXListId);
		writeByte(datFile, spriteTemplate.moveXListTicks);
		writeByte(datFile, spriteTemplate.moveXListInitialTicks);
		writeUint16LE(datFile, spriteTemplate.moveYListId);
		writeByte(datFile, spriteTemplate.moveYListTicks);
		writeByte(datFile, spriteTemplate.moveYListInitialTicks);
	}

	// Write anim code tables
	for (int type = 0; type < 3; type++) {
		std::vector<AnimCodeItem> &table = animCodeTables[type];
		uint32 totalSize = 0, itemOffset = 0;
		for (uint32 i = 0; i < table.size(); i++)
			totalSize += table[i].len;
		writeUint32LE(datFile, table.size());
		writeUint32LE(datFile, totalSize);
		for (uint32 i = 0; i < table.size(); i++) {
			AnimCodeItem &animCodeItem = table[i];
			writeUint16LE(datFile, animCodeItem.selfId);
			writeUint16LE(datFile, itemOffset);
			itemOffset += animCodeItem.len;
		}
		for (uint32 i = 0; i < table.size(); i++) {
			AnimCodeItem &animCodeItem = table[i];
			fwrite(animCodeItem.code, animCodeItem.len, 1, datFile);
		}
	}

	// The music data is written as-is, with just the offsets relocated
	// so the base is 0 for each music buffer
	writeUint32LE(datFile, 2);
	extractMusic(0x90CF, 0x945B, 0x917F);
	extractMusic(0x945B, 0x958F, 0x94D7);

	extractSoundTablesSB();

	extractGridSprite();

	extractCreditsText();

	fclose(datFile);

	printf("Done.\n");

	return 0;
}
