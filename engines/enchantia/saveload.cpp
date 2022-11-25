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

#include "common/savefile.h"

#include "graphics/thumbnail.h"

#include "enchantia/enchantia.h"
#include "enchantia/datfile.h"

namespace Enchantia {

#define ENCHANTIA_SAVEGAME_VERSION 0

EnchantiaEngine::kReadSaveHeaderError EnchantiaEngine::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {

	header.version = in->readUint32LE();
	if (header.version > ENCHANTIA_SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--)
		header.description += (char)in->readByte();

	if (!Graphics::loadThumbnail(*in, header.thumbnail, !loadThumbnail))
		return kRSHEIoError;

	// Not used yet, reserved for future usage
	header.gameID = in->readByte();
	header.flags = in->readUint32LE();

	header.saveDate = in->readUint32LE();
	header.saveTime = in->readUint32LE();
	header.playTime = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

void EnchantiaEngine::savegame(const char *filename, const char *description) {

	debug("savegame() filename: %s; description: %s", filename, description);

	Common::OutSaveFile *out;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return;
	}

	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	// Header start
	out->writeUint32LE(ENCHANTIA_SAVEGAME_VERSION);

	byte descriptionLen = strlen(description);
	out->writeByte(descriptionLen);
	out->write(description, descriptionLen);

	Graphics::saveThumbnail(*out);

	// Not used yet, reserved for future usage
	out->writeByte(0);
	out->writeUint32LE(0);
	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint32 saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;
	out->writeUint32LE(saveDate);
	out->writeUint32LE(saveTime);
	out->writeUint32LE(playTime);
	// Header end

	// Sprites
	out->writeUint16LE(_sceneSpritesCount);
	for (uint i = 0; i < _sceneSpritesCount + 2; ++i) {
		Sprite *sprite = &_sprites[i];
		out->writeByte(sprite->status);
		out->writeByte(sprite->frameIndex);
		out->writeUint16LE(sprite->x);
		out->writeUint16LE(sprite->y);
		out->writeUint16LE(sprite->width);
		out->writeUint16LE(sprite->height);
		out->writeUint16LE(sprite->heightAdd);
		out->writeUint16LE(sprite->yAdd);
		out->writeUint16LE(sprite->id);
		out->writeUint16LE(sprite->anim.codeId);
		out->writeUint16LE(sprite->anim.index);
		out->writeUint16LE(sprite->anim.ticks);
		out->writeUint16LE(sprite->anim.initialTicks);
		out->writeUint16LE(sprite->moveX.codeId);
		out->writeUint16LE(sprite->moveX.index);
		out->writeUint16LE(sprite->moveX.ticks);
		out->writeUint16LE(sprite->moveX.initialTicks);
		out->writeUint16LE(sprite->moveY.codeId);
		out->writeUint16LE(sprite->moveY.index);
		out->writeUint16LE(sprite->moveY.ticks);
		out->writeUint16LE(sprite->moveY.initialTicks);
	}

	// Scene decorations
	out->writeUint16LE(_sceneDecorationCount);
	for (uint i = 0; i < _sceneDecorationCount; ++i) {
		SceneDecoration *sceneDecoration = &_sceneDecorations[i];
		out->writeUint16LE(sceneDecoration->x);
		out->writeUint16LE(sceneDecoration->y);
		out->writeByte(sceneDecoration->frameIndex);
	}

	out->writeByte(_spriteResourceType);
	out->writeUint16LE(_scrollBorderLeft);
	out->writeUint16LE(_scrollBorderRight);
	out->writeUint16LE(_cameraStripX);
	out->writeByte(_collectItemIgnoreActorPosition ? 1 : 0);
	out->writeByte(_flgCanRunBoxClick ? 1 : 0);
	out->writeByte(_flgCanRunMenu ? 1 : 0);
	out->writeByte(_flgWalkBoxEnabled ? 1 : 0);
	out->writeByte(_cameraStripNum);
	out->writeUint16LE(_currSceneLink);
	out->writeUint16LE(_walkDistance);
	out->writeUint16LE(_walkIncrY2);
	out->writeUint16LE(_walkErrorX);
	out->writeUint16LE(_someX);
	out->writeUint16LE(_walkErrorY);
	out->writeUint16LE(_someY);
	out->writeUint16LE(_walkSlopeErrX);
	out->writeUint16LE(_walkSlopeX);
	out->writeUint16LE(_walkSlopeErrY);
	out->writeUint16LE(_walkSlopeY);
	for (uint i = 0; i < 10; ++i)
		out->writeUint16LE(_actorXTable[i]);
	out->writeUint16LE(_theScore);
	out->writeByte(_flags1);
	out->writeByte(_flags2);
	out->writeByte(_rockBasherCounters[0]);
	out->writeByte(_rockBasherCounters[1]);
	out->writeByte(_rockBasherCounters[2]);
	out->writeByte(_scene63BabySleepCounter);
	_dat->saveSpriteDefs(out);
	for (uint i = 0; i < kInventoryTableCount; ++i)
		out->writeUint16LE(_inventoryTable[i]);
	for (uint i = 0; i < kTalkTableCount; ++i)
		out->writeUint16LE(_talkTable[i]);

	out->finalize();
	delete out;
}

void EnchantiaEngine::loadgame(const char *filename) {
	Common::InSaveFile *in;
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		warning("Can't open file '%s', game not loaded", filename);
		return;
	}

	SaveHeader header;

	kReadSaveHeaderError errorCode = readSaveHeader(in, false, header);

	if (errorCode != kRSHENoError) {
		warning("Error loading savegame '%s'", filename);
		delete in;
		return;
	}

	g_engine->setTotalPlayTime(header.playTime * 1000);

	// Sprites
	_sceneSpritesCount = in->readUint16LE();
	for (uint i = 0; i < kSpriteCount - 1; i++)
		_sprites[i].status = 0;
	for (uint i = 0; i < _sceneSpritesCount + 2; ++i) {
		Sprite *sprite = &_sprites[i];
		sprite->status = in->readByte();
		sprite->frameIndex = in->readByte();
		sprite->x = in->readUint16LE();
		sprite->y = in->readUint16LE();
		sprite->width = in->readUint16LE();
		sprite->height = in->readUint16LE();
		sprite->heightAdd = in->readUint16LE();
		sprite->yAdd = in->readUint16LE();
		sprite->id = in->readUint16LE();
		sprite->anim.codeId = in->readUint16LE();
		sprite->anim.index = in->readUint16LE();
		sprite->anim.ticks = in->readUint16LE();
		sprite->anim.initialTicks = in->readUint16LE();
		sprite->moveX.codeId = in->readUint16LE();
		sprite->moveX.index = in->readUint16LE();
		sprite->moveX.ticks = in->readUint16LE();
		sprite->moveX.initialTicks = in->readUint16LE();
		sprite->moveY.codeId = in->readUint16LE();
		sprite->moveY.index = in->readUint16LE();
		sprite->moveY.ticks = in->readUint16LE();
		sprite->moveY.initialTicks = in->readUint16LE();
		sprite->spriteResource = _sceneSpr;
	}

	// Scene decorations
	_sceneDecorationCount = in->readUint16LE();
	for (uint i = 0; i < _sceneDecorationCount; ++i) {
		SceneDecoration *sceneDecoration = &_sceneDecorations[i];
		sceneDecoration->x = in->readUint16LE();
		sceneDecoration->y = in->readUint16LE();
		sceneDecoration->frameIndex = in->readByte();
		sceneDecoration->spriteResource = _sceneSpr;
	}

	_spriteResourceType = in->readByte();
	_scrollBorderLeft = in->readUint16LE();
	_scrollBorderRight = in->readUint16LE();
	_cameraStripX = in->readUint16LE();
	_collectItemIgnoreActorPosition = in->readByte() != 0;
	_flgCanRunBoxClick = in->readByte() != 0;
	_flgCanRunMenu = in->readByte() != 0;
	_flgWalkBoxEnabled = in->readByte() != 0;
	_cameraStripNum = in->readByte();
	_currSceneLink = in->readUint16LE();
	_walkDistance = in->readUint16LE();
	_walkIncrY2 = in->readUint16LE();
	_walkErrorX = in->readUint16LE();
	_someX = in->readUint16LE();
	_walkErrorY = in->readUint16LE();
	_someY = in->readUint16LE();
	_walkSlopeErrX = in->readUint16LE();
	_walkSlopeX = in->readUint16LE();
	_walkSlopeErrY = in->readUint16LE();
	_walkSlopeY = in->readUint16LE();
	for (uint i = 0; i < 10; ++i)
		_actorXTable[i] = in->readUint16LE();
	_theScore = in->readUint16LE();
	_flags1 = in->readByte();
	_flags2 = in->readByte();
	_rockBasherCounters[0] = in->readByte();
	_rockBasherCounters[1] = in->readByte();
	_rockBasherCounters[2] = in->readByte();
	_scene63BabySleepCounter = in->readByte();
	_dat->restoreSpriteDefs(in);
	for (uint i = 0; i < kInventoryTableCount; ++i)
		_inventoryTable[i] = in->readUint16LE();
	for (uint i = 0; i < kTalkTableCount; ++i)
		_talkTable[i] = in->readUint16LE();

	loadScene(_currSceneLink, true);

	delete in;

}

Common::Error EnchantiaEngine::loadGameState(int slot) {
	const char *fileName = getSavegameFilename(slot);
	loadgame(fileName);
	return Common::kNoError;
}

Common::Error EnchantiaEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	const char *fileName = getSavegameFilename(slot);
	savegame(fileName, description.c_str());
	return Common::kNoError;
}

const char *EnchantiaEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String EnchantiaEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);
	return Common::String::format("%s.%03d", target.c_str(), num);
}

} // End of namespace Enchantia
