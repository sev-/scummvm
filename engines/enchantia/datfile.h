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

#ifndef ENCHANTIA_DATFILE_H
#define ENCHANTIA_DATFILE_H

#include "enchantia/enchantia.h"

namespace Enchantia {

typedef Common::Array<uint16> SpriteDefList;

struct AnimCodeTableItem {
	uint16 selfId;
	uint16 offset;
};

struct AnimCodeTable {
	byte *code;
	AnimCodeTableItem *items;
	uint itemCount;
	AnimCodeTable() : code(NULL), items(NULL) {
	}
	~AnimCodeTable() {
		delete[] code;
		delete[] items;
	}
};

struct MusicItem {
	uint32 size;
	byte *data;
	MusicItem() : size(0), data(NULL) {
	}
	~MusicItem() {
		delete[] data;
	}
};

class DatFile {
public:
	DatFile();
	~DatFile();
	void load(const char *filename);
	SceneItemInfo &getSceneItemInfo(uint index) const { return _sceneItemInfos[index]; }
	uint getSceneItemInfoCount() const { return _sceneItemInfosCount; }
	SpriteDefList &getSceneSpriteDefTable(uint index) { return _sceneSpriteDefTables[index]; }
	uint getSceneSpriteDefTablesCount() const { return _sceneSpriteDefTablesCount; }
	const byte *getSceneStrips(uint index) const { return _sceneStrips[index]; }
	uint getSceneStripsCount() const { return _sceneStripsCount; }
	const SceneFilenames &getSceneFilenames(uint index) const { return _sceneFilenames[index]; }
	uint getSceneFilenamesCount() const { return _sceneFilenamesCount; }
	const Common::String &getBrdFilename(uint index) const { return _brdFilenames[index]; }
	uint getBrdFilenamesCount() const { return _brdFilenamesCount; }
	SceneInitItem &getSceneInitItem(uint index) { return _sceneInitItems[index]; }
	uint getSceneInitItemsCount() const { return _sceneInitItemsCount; }
	SpriteDef *getSpriteDef(uint16 id);
	SpriteTemplate *getSpriteTemplate(uint16 id);
	byte *getAnimCode(uint16 id) { return getAnyAnimCode(0, id); }
	byte *getMoveXCode(uint16 id) { return getAnyAnimCode(1, id); }
	byte *getMoveYCode(uint16 id) { return getAnyAnimCode(2, id); }
	SpriteDef *getSceneSpriteDef(uint sceneIndex, uint index);
	MusicItem *getMusic(uint index) { return &_musicItems[index]; }
	const SoundItem &getSound(byte soundNum, int16 sceneIndex);
	void saveSpriteDefs(Common::OutSaveFile *out);
	void restoreSpriteDefs(Common::InSaveFile *in);
	byte *getGridSprite();
	byte *getScoreCreditsTxt();
protected:
	SceneItemInfo *_sceneItemInfos;
	uint _sceneItemInfosCount;
	SpriteDefList *_sceneSpriteDefTables;
	uint _sceneSpriteDefTablesCount;
	byte *_sceneStripData;
	byte **_sceneStrips;
	uint _sceneStripsCount;
	SceneFilenames *_sceneFilenames;
	uint _sceneFilenamesCount;
	Common::String *_brdFilenames;
	uint _brdFilenamesCount;
	SceneInitItem *_sceneInitItems;
	uint _sceneInitItemsCount;
	SpriteDef *_spriteDefs;
	uint _spriteDefsCount;
	SpriteTemplate *_spriteTemplates;
	uint _spriteTemplatesCount;
	AnimCodeTable _animCode[3];
	uint _musicItemsCount;
	MusicItem *_musicItems;
	uint _soundItemsCount;
	SoundItem *_soundItems;
	uint _sceneSoundRefsCount;
	byte *_sceneSoundRefs;
	uint _gridSpriteCount;
	byte *_gridSprite;
	uint _scoreCreditsCount;
	byte *_scoreCreditsTxt;
	byte *getAnyAnimCode(uint type, uint16 id);
};

} // End of namespace Enchantia

#endif
