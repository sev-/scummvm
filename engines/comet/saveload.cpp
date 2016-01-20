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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/thumbnail.h"

#include "audio/mixer.h"

#include "comet/comet.h"
#include "comet/actor.h"
#include "comet/animationmgr.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/script.h"
#include "comet/talktext.h"

namespace Comet {

// TODO:
//	- Save with F7; Load with F9
//	- Remove REMOVEME code once saveload code is finalized (this is just so my old savegames still work)
//  - Save playtime info

#define SAVEGAME_VERSION 2 // < 1000 is dev version until in official SVN

CometEngine::kReadSaveHeaderError CometEngine::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {
	header.version = in->readUint32LE();
	if (header.version > SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--)
		header.description += (char)in->readByte();

	if (loadThumbnail && header.version > 1 /*REMOVEME*/ ) {
		header.thumbnail = Graphics::loadThumbnail(*in);
	} else {
		Graphics::skipThumbnail(*in);
		header.thumbnail = 0;
	}

	// Not used yet, reserved for future usage
	header.gameID = in->readByte();
	header.flags = in->readUint32LE();

	return ((in->eos() || in->err()) ? kRSHEIoError : kRSHENoError);
}

void CometEngine::savegame(const char *filename, const char *description) {

	debug("Saving %s [%s]...", filename, description);

	Common::OutSaveFile *out;
	if (!(out = g_system->getSavefileManager()->openForSaving(filename))) {
		warning("Can't create file '%s', game not saved", filename);
		return;
	}

	out->writeUint32LE(SAVEGAME_VERSION);

	byte descriptionLen = strlen(description);
	out->writeByte(descriptionLen);
	out->write(description, descriptionLen);
	
	Graphics::saveThumbnail(*out);

	// Not used yet, reserved for future usage
	out->writeByte(0); // gameID
	out->writeUint32LE(0); // flags

	out->writeUint16LE(_moduleNumber);
	out->writeUint16LE(_currentModuleNumber);
	out->writeUint16LE(_prevModuleNumber);
	out->writeUint16LE(_sceneNumber);
	out->writeUint16LE(_currentSceneNumber);
	out->writeUint16LE(_prevSceneNumber);
	
	out->writeUint16LE(_backgroundFileIndex);
	out->writeUint16LE(_talkText->_textTableIndex);
	out->writeUint32LE(_gameLoopCounter);
	out->writeByte(_blockedInput);

	out->writeByte(_scene->_exits.size());
	for (Common::Array<SceneExitItem>::iterator iter = _scene->_exits.begin(); iter != _scene->_exits.end(); ++iter) {
		const SceneExitItem &sceneExit = *iter;
		out->writeUint16LE(sceneExit.directionIndex);
		out->writeUint16LE(sceneExit.moduleNumber);
		out->writeUint16LE(sceneExit.sceneNumber);
		out->writeUint16LE(sceneExit.x1);
		out->writeUint16LE(sceneExit.x2);
	}

	out->writeByte(_script->_scriptCount);
	for (int i = 0; i < _script->_scriptCount; i++) {
		const Script &script = *_script->_scripts[i];
		out->writeUint16LE(script.ip);
		out->writeByte(script.actorIndex);
		out->writeUint16LE(script.status);
		out->writeUint16LE(script.scriptNumber);
		out->writeUint16LE(script.loopCounter);
		out->writeUint16LE(script.zoneX1);
		out->writeUint16LE(script.zoneY1);
		out->writeUint16LE(script.zoneX2);
		out->writeUint16LE(script.zoneY2);
	}
	
	out->writeByte(_scene->_bounds.size());
	for (PointArray::iterator iter = _scene->_bounds.begin(); iter != _scene->_bounds.end(); ++iter) {
		const Common::Point &bound = *iter;
		out->writeUint16LE(bound.x);
		out->writeUint16LE(bound.y);
	}
	
	// TODO Later move the loop into the Actors class and actual serilization code into Actor
	for (uint actorIndex = 0; actorIndex < _actors->getCount(); ++actorIndex) {
		Actor *actor = _actors->getActor(actorIndex);
		out->writeUint16LE(actor->_x);
		out->writeUint16LE(actor->_y);
		out->writeUint16LE(actor->_directionAdd);
		out->writeUint16LE(actor->_status);
		out->writeUint16LE(actor->_direction);
		out->writeByte(actor->_flag2);
		out->writeUint16LE(actor->_animationSlot);
		out->writeUint16LE(actor->_animIndex);
		out->writeUint16LE(actor->_animFrameIndex);
		out->writeUint16LE(actor->_interpolationStep);
		out->writeUint16LE(actor->_animFrameCount);
		out->writeUint16LE(actor->_animPlayFrameIndex);
		out->writeUint16LE(actor->_deltaX);
		out->writeUint16LE(actor->_deltaY);
		out->writeUint16LE(actor->_collisionType);
		out->writeUint16LE(actor->_collisionIndex);
		out->writeByte(actor->_value6);
		out->writeUint16LE(actor->_life);
		out->writeByte(actor->_textColor);
		out->writeByte(actor->_value7);
		out->writeUint16LE(actor->_textX);
		out->writeUint16LE(actor->_textY);
		out->writeUint16LE(actor->_walkStatus);
		out->writeUint16LE(actor->_walkDestX);
		out->writeUint16LE(actor->_walkDestY);
		out->writeUint16LE(actor->_savedWalkDestX);
		out->writeUint16LE(actor->_savedWalkDestY);
		out->writeUint16LE(actor->_clipX1);
		out->writeUint16LE(actor->_clipY1);
		out->writeUint16LE(actor->_clipX2);
		out->writeUint16LE(actor->_clipY2);
		out->writeByte(actor->_visible ? 1 : 0);
	}
	
	for (uint i = 0; i < kAnimationSlotCount; i++) {
		const AnimationSlot *marcheItem = _animationMan->getAnimationSlot(i);
		out->writeUint16LE(marcheItem->animationType);
		out->writeUint16LE(marcheItem->fileIndex);
	}
	
	out->writeByte(_scene->_sceneItems.size());
	for (Common::Array<SceneItem>::iterator iter = _scene->_sceneItems.begin(); iter != _scene->_sceneItems.end(); ++iter) {
		const SceneItem &sceneItem = *iter;
		out->writeUint16LE(sceneItem.itemIndex);
		out->writeByte(sceneItem.active ? 1 : 0);
		out->writeByte(sceneItem.paramType);
		out->writeUint16LE(sceneItem.x);
		out->writeUint16LE(sceneItem.y);
	}

	out->writeByte(_scene->_blockingRects.size());
	for (Common::Array<Common::Rect>::iterator iter = _scene->_blockingRects.begin(); iter != _scene->_blockingRects.end(); ++iter) {
		const Common::Rect &blockingRect = *iter;
		out->writeUint16LE(blockingRect.left);
		out->writeUint16LE(blockingRect.top);
		out->writeUint16LE(blockingRect.right);
		out->writeUint16LE(blockingRect.bottom);
	}

	out->writeByte(_paletteBrightness);
	out->writeByte(_paletteRedness);
	out->writeUint16LE(_screen->_zoomX);
	out->writeUint16LE(_screen->_zoomY);

	out->write(_scene->_boundsMap, 320);
	
	for (int i = 0; i < 256; i++)
		out->writeUint16LE(_inventoryItemStatus[i]);

	for (int i = 0; i < 256; i++)
		out->writeUint16LE(_scriptVars[i]);

	delete out;

}

void CometEngine::loadgame(const char *filename) {

	debug("Loading %s...", filename);

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

	int count;
	
	_animationMan->purgeAnimationSlots();
	
	_moduleNumber = in->readUint16LE();
	_currentModuleNumber = in->readUint16LE();
	_prevModuleNumber = in->readUint16LE();
	_sceneNumber = in->readUint16LE();
	_currentSceneNumber = in->readUint16LE();
	_prevSceneNumber = in->readUint16LE();

	_backgroundFileIndex = in->readUint16LE();
	uint voiceFileIndex = in->readUint16LE();
	_gameLoopCounter = in->readUint32LE();
	
	if (header.version > 0)//REMOVEME	
		_blockedInput = in->readByte();
	else
		_blockedInput = 0; 

	_scene->_exits.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		SceneExitItem sceneExit;
		sceneExit.directionIndex = in->readUint16LE();
		sceneExit.moduleNumber = in->readUint16LE();
		sceneExit.sceneNumber = in->readUint16LE();
		sceneExit.x1 = in->readUint16LE();
		sceneExit.x2 = in->readUint16LE();
		_scene->_exits.push_back(sceneExit);
	}

	_script->_scriptCount = in->readByte();
	for (int i = 0; i < _script->_scriptCount; i++) {
		Script &script = *_script->_scripts[i];
		script.ip = in->readUint16LE();
		script.actorIndex = in->readByte();
		script.status = in->readUint16LE();
		script.scriptNumber = in->readUint16LE();
		script.loopCounter = in->readUint16LE();
		script.zoneX1 = in->readUint16LE();
		script.zoneY1 = in->readUint16LE();
		script.zoneX2 = in->readUint16LE();
		script.zoneY2 = in->readUint16LE();
	}

	_scene->_bounds.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		Common::Point bound;
		bound.x = in->readUint16LE();
		bound.y = in->readUint16LE();
		_scene->_bounds.push_back(bound);
	}

	// TODO Later move the loop into the Actors class and actual serilization code into Actor
	for (uint actorIndex = 0; actorIndex < _actors->getCount(); ++actorIndex) {
		Actor *actor = _actors->getActor(actorIndex);
		actor->_x = in->readUint16LE();
		actor->_y = in->readUint16LE();
		actor->_directionAdd = in->readUint16LE();
		actor->_status = in->readUint16LE();
		actor->_direction = in->readUint16LE();
		actor->_flag2 = in->readByte();
		actor->_animationSlot = in->readUint16LE();
		actor->_animIndex = in->readUint16LE();
		actor->_animFrameIndex = in->readUint16LE();
		actor->_interpolationStep = in->readUint16LE();
		actor->_animFrameCount = in->readUint16LE();
		actor->_animPlayFrameIndex = in->readUint16LE();
		actor->_deltaX = in->readUint16LE();
		actor->_deltaY = in->readUint16LE();
		actor->_collisionType = in->readUint16LE();
		actor->_collisionIndex = in->readUint16LE();
		actor->_value6 = in->readByte();
		actor->_life = in->readUint16LE();
		actor->_textColor = in->readByte();
		actor->_value7 = in->readByte();
		actor->_textX = in->readUint16LE();
		actor->_textY = in->readUint16LE();
		actor->_walkStatus = in->readUint16LE();
		actor->_walkDestX = in->readUint16LE();
		actor->_walkDestY = in->readUint16LE();
		actor->_savedWalkDestX = in->readUint16LE();
		actor->_savedWalkDestY = in->readUint16LE();
		actor->_clipX1 = in->readUint16LE();
		actor->_clipY1 = in->readUint16LE();
		actor->_clipX2 = in->readUint16LE();
		actor->_clipY2 = in->readUint16LE();
		actor->_visible = in->readByte() != 0;
	}

	for (uint i = 0; i < kAnimationSlotCount; i++) {
		AnimationSlot *marcheItem = _animationMan->getAnimationSlot(i);
		marcheItem->animationType = in->readUint16LE();
		marcheItem->fileIndex = (int16)in->readUint16LE();
		marcheItem->anim = NULL;
		debug("marcheItem.animationType = %d; marcheItem.fileIndex = %d", marcheItem->animationType, marcheItem->fileIndex);
	}

	_scene->_sceneItems.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		SceneItem sceneItem;
		sceneItem.itemIndex = in->readUint16LE();
		sceneItem.active = in->readByte() != 0;
		sceneItem.paramType = in->readByte();
		sceneItem.x = in->readUint16LE();
		sceneItem.y = in->readUint16LE();
		_scene->_sceneItems.push_back(sceneItem);
	}

	_scene->_blockingRects.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		Common::Rect blockingRect;
		blockingRect.left = in->readUint16LE();
		blockingRect.top = in->readUint16LE();
		blockingRect.right = in->readUint16LE();
		blockingRect.bottom = in->readUint16LE();
		_scene->_blockingRects.push_back(blockingRect);
	}

	_paletteBrightness = in->readByte();
	
	if (header.version > 0)//REMOVEME	
		_paletteRedness = in->readByte();
	else
		_paletteRedness = 0;
	
	_screen->_zoomX = in->readUint16LE();
	_screen->_zoomY = in->readUint16LE();

	in->read(_scene->_boundsMap, 320);

	for (int i = 0; i < 256; i++)
		_inventoryItemStatus[i] = in->readUint16LE();

	for (int i = 0; i < 256; i++)
		_scriptVars[i] = in->readUint16LE();

	delete in;
	
	setModuleAndScene(_currentModuleNumber, _currentSceneNumber);
	loadAndRunScript(true);
	initSceneBackground(true);
	_animationMan->restoreAnimationSlots();
	
	if (_paletteRedness != 0)
		_screen->buildRedPalette(_gamePalette, _screenPalette, _paletteRedness);
	else		
		_screen->buildPalette(_gamePalette, _screenPalette, _paletteBrightness);
	_screen->setFullPalette(_screenPalette);

	_talkText->setVoiceFileIndex(voiceFileIndex);

}

Common::Error CometEngine::loadGameState(int slot) {
	const char *fileName = getSavegameFilename(slot);
	loadgame(fileName);
	return Common::kNoError;
}

Common::Error CometEngine::saveGameState(int slot, const Common::String &description) {
	const char *fileName = getSavegameFilename(slot);
	savegame(fileName, description.c_str());
	return Common::kNoError;
}

const char *CometEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String CometEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);

	Common::String extension;
	extension = Common::String::format("%03d", num);

	return target + "." + extension;
}

} // End of namespace Comet
