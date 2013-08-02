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

#include "enchantia/datfile.h"

namespace Enchantia {

// TODO Currently all array here are searched with linear search.
// If needed for speed reasons, change to hash map or binary search etc.

void readString(Common::File &fd, Common::String &str) {
	byte len;
	char temp[256];
	len = fd.readByte();
	fd.read(temp, len);
	temp[len] = 0;
	str = temp;
}

DatFile::DatFile() :
	_sceneItemInfos(NULL), _sceneItemInfosCount(0),
	_sceneSpriteDefTables(NULL), _sceneSpriteDefTablesCount(0),
	_sceneStripData(NULL), _sceneStrips(NULL),
	_sceneFilenames(NULL), _brdFilenames(NULL),
	_sceneInitItems(NULL), _spriteDefs(NULL),
	_spriteTemplates(NULL), _musicItems(NULL),
	_soundItems(NULL), _sceneSoundRefs(NULL) {

}

DatFile::~DatFile() {
	delete[] _sceneItemInfos;
	delete[] _sceneSpriteDefTables;
	delete[] _sceneStripData;
	delete[] _sceneStrips;
	delete[] _sceneFilenames;
	delete[] _brdFilenames;
	delete[] _sceneInitItems;
	delete[] _spriteDefs;
	delete[] _spriteTemplates;
	delete[] _musicItems;
	delete[] _soundItems;
	delete[] _sceneSoundRefs;
}

void DatFile::load(const char *filename) {
	Common::File fd;
	if (!fd.open(filename))
		error("DatFile::load() Could not open %s", filename);

	// Load SceneItemInfos
	_sceneItemInfosCount = fd.readUint32LE();
	debug("_sceneItemInfosCount = %d", _sceneItemInfosCount);
	_sceneItemInfos = new SceneItemInfo[_sceneItemInfosCount];
	for (uint i = 0; i < _sceneItemInfosCount; i++) {
		_sceneItemInfos[i].flags = fd.readByte();
		_sceneItemInfos[i].frameIndex = fd.readByte();
	}

	// Load SceneSpriteDefTables
	_sceneSpriteDefTablesCount = fd.readUint32LE();
	debug("_sceneSpriteDefTablesCount = %d", _sceneSpriteDefTablesCount);
	_sceneSpriteDefTables = new SpriteDefList[_sceneSpriteDefTablesCount];
	for (uint i = 0; i < _sceneSpriteDefTablesCount; i++) {
		uint count = fd.readUint32LE();
		for (uint j = 0; j < count; j++) {
			uint16 id = fd.readUint16LE();
			_sceneSpriteDefTables[i].push_back(id);
		}
	}

	// Load scene strip tables
	uint32 sceneStripDataSize = fd.readUint32LE();
	debug("sceneStripDataSize = %d", sceneStripDataSize);
	_sceneStripData = new byte[sceneStripDataSize];
	fd.read(_sceneStripData, sceneStripDataSize);
	_sceneStripsCount = fd.readUint32LE();
	debug("_sceneStripsCount = %d", _sceneStripsCount);
	_sceneStrips = new byte*[_sceneStripsCount];
	for (uint i = 0; i < _sceneStripsCount; i++) {
		_sceneStrips[i] = _sceneStripData + fd.readUint16LE();
	}

	// Load scene filenames
	_sceneFilenamesCount = fd.readUint32LE();
	debug("_sceneFilenamesCount = %d", _sceneFilenamesCount);
	_sceneFilenames = new SceneFilenames[_sceneFilenamesCount];
	for (uint i = 0; i < _sceneFilenamesCount; i++) {
		readString(fd, _sceneFilenames[i].mapFilename);
		readString(fd, _sceneFilenames[i].sprFilename);
		readString(fd, _sceneFilenames[i].sfxFilename);
	}

	// Load brd filenames
	_brdFilenamesCount = fd.readUint32LE();
	debug("_brdFilenamesCount = %d", _brdFilenamesCount);
	_brdFilenames = new Common::String[_brdFilenamesCount];
	for (uint i = 0; i < _brdFilenamesCount; i++) {
		readString(fd, _brdFilenames[i]);
	}

	// Load SceneInitItems
	_sceneInitItemsCount = fd.readUint32LE();
	debug("_sceneInitItemsCount = %d", _sceneInitItemsCount);
	_sceneInitItems = new SceneInitItem[_sceneInitItemsCount];
	for (uint i = 0; i < _sceneInitItemsCount; i++) {
		_sceneInitItems[i].status = fd.readByte();
		_sceneInitItems[i].frameIndex = fd.readByte();
		_sceneInitItems[i].x = fd.readUint16LE();
		_sceneInitItems[i].y = fd.readUint16LE();
		_sceneInitItems[i].animListId = fd.readUint16LE();
		_sceneInitItems[i].moveXListId = fd.readUint16LE();
		_sceneInitItems[i].moveYListId = fd.readUint16LE();
		_sceneInitItems[i].cameraStripNum = fd.readByte();
		_sceneInitItems[i].spriteResourceType = fd.readByte();
	}

	// Load SpriteDefs
	_spriteDefsCount = fd.readUint32LE();
	debug("_spriteDefsCount = %d", _spriteDefsCount);
	_spriteDefs = new SpriteDef[_spriteDefsCount];
	for (uint i = 0; i < _spriteDefsCount; i++) {
		_spriteDefs[i].selfId = fd.readUint16LE();
		_spriteDefs[i].type = fd.readByte();
		_spriteDefs[i].status = fd.readByte();
		_spriteDefs[i].frameIndex = fd.readByte();
		_spriteDefs[i].x = fd.readUint16LE();
		_spriteDefs[i].y = fd.readUint16LE();
		_spriteDefs[i].templateId = fd.readUint16LE();
	}

	// Load SpriteTemplates
	_spriteTemplatesCount = fd.readUint32LE();
	debug("_spriteTemplatesCount = %d", _spriteTemplatesCount);
	_spriteTemplates = new SpriteTemplate[_spriteTemplatesCount];
	for (uint i = 0; i < _spriteTemplatesCount; i++) {
		_spriteTemplates[i].selfId = fd.readUint16LE();
		_spriteTemplates[i].heightAdd = fd.readUint16LE();
		_spriteTemplates[i].yAdd = fd.readUint16LE();
		_spriteTemplates[i].id = fd.readUint16LE();
		_spriteTemplates[i].animListId = fd.readUint16LE();
		_spriteTemplates[i].animListTicks = fd.readByte();
		_spriteTemplates[i].animListInitialTicks = fd.readByte();
		_spriteTemplates[i].moveXListId = fd.readUint16LE();
		_spriteTemplates[i].moveXListTicks = fd.readByte();
		_spriteTemplates[i].moveXListInitialTicks = fd.readByte();
		_spriteTemplates[i].moveYListId = fd.readUint16LE();
		_spriteTemplates[i].moveYListTicks = fd.readByte();
		_spriteTemplates[i].moveYListInitialTicks = fd.readByte();
	}

	// Load anim code tables (anim, moveX, moveY)
	for (uint type = 0; type < 3; type++) {
		_animCode[type].itemCount = fd.readUint32LE();
		uint32 codeSize = fd.readUint32LE();
		_animCode[type].code = new byte[codeSize];
		_animCode[type].items = new AnimCodeTableItem[_animCode[type].itemCount];
		for (uint i = 0; i < _animCode[type].itemCount; i++) {
			_animCode[type].items[i].selfId = fd.readUint16LE();
			_animCode[type].items[i].offset = fd.readUint16LE();
		}
		fd.read(_animCode[type].code, codeSize);
	}
	debug("%08X", fd.pos());

	// Load AdLib music
	_musicItemsCount = fd.readUint32LE();
	_musicItems = new MusicItem[_musicItemsCount];
	for (uint i = 0; i < _musicItemsCount; i++) {
		_musicItems[i].size = fd.readUint32LE();
		_musicItems[i].data = new byte[_musicItems[i].size];
		fd.read(_musicItems[i].data, _musicItems[i].size);
	}

	_soundItemsCount = fd.readUint32LE();
	_soundItems = new SoundItem[_soundItemsCount];
	for (uint i = 0; i < _soundItemsCount; i++) {
		_soundItems[i].priority = fd.readByte();
		_soundItems[i].freq = fd.readUint16LE();
		_soundItems[i].sizeDecr = fd.readByte();
		_soundItems[i].index = fd.readByte();
	}

	_sceneSoundRefsCount = fd.readUint32LE();
	_sceneSoundRefs = new byte[_sceneSoundRefsCount];
	fd.read(_sceneSoundRefs, _sceneSoundRefsCount);

	fd.close();
}

SpriteDef *DatFile::getSpriteDef(uint16 id) {
	for (uint i = 0; i < _spriteDefsCount; i++)
		if (_spriteDefs[i].selfId == id)
			return &_spriteDefs[i];
	error("DatFile::getSpriteDef() SpriteDef with ID %04X not found", id);
	return NULL;
}

SpriteTemplate *DatFile::getSpriteTemplate(uint16 id) {
	for (uint i = 0; i < _spriteTemplatesCount; i++)
		if (_spriteTemplates[i].selfId == id)
			return &_spriteTemplates[i];
	error("DatFile::getSpriteTemplate() SpriteTemplate with ID %04X not found", id);
	return NULL;
}

byte *DatFile::getAnyAnimCode(uint type, uint16 id) {
	AnimCodeTable &table = _animCode[type];
	for (uint i = 0; i < table.itemCount; i++) {
		if (table.items[i].selfId == id)
			return table.code + table.items[i].offset;
	}
	error("DatFile::getAnyAnimCode() Code in table %d with ID %04X not found", type, id);
	return NULL;
}

SpriteDef *DatFile::getSceneSpriteDef(uint sceneIndex, uint index) {
	SpriteDefList &spriteDefList = getSceneSpriteDefTable(sceneIndex);
	return getSpriteDef(spriteDefList[index]);
}

const SoundItem &DatFile::getSound(byte soundNum, int16 sceneIndex) {
	if (soundNum < 15)
		return _soundItems[soundNum];
	else
		return _soundItems[_sceneSoundRefs[sceneIndex] + soundNum - 15];
}

void DatFile::saveSpriteDefs(Common::OutSaveFile *out) {
	for (uint i = 0; i < _spriteDefsCount; ++i) {
		SpriteDef *spriteDef = &_spriteDefs[i];
		out->writeUint16LE(spriteDef->selfId);
		out->writeByte(spriteDef->type);
		out->writeByte(spriteDef->status);
		out->writeByte(spriteDef->frameIndex);
		out->writeUint16LE(spriteDef->x);
		out->writeUint16LE(spriteDef->y);
		out->writeUint16LE(spriteDef->templateId);
	}
}

void DatFile::restoreSpriteDefs(Common::InSaveFile *in) {
	for (uint i = 0; i < _spriteDefsCount; ++i) {
		SpriteDef *spriteDef = &_spriteDefs[i];
		spriteDef->selfId = in->readUint16LE();
		spriteDef->type = in->readByte();
		spriteDef->status = in->readByte();
		spriteDef->frameIndex = in->readByte();
		spriteDef->x = in->readUint16LE();
		spriteDef->y = in->readUint16LE();
		spriteDef->templateId = in->readUint16LE();
	}
}

} // End of namespace Enchantia
