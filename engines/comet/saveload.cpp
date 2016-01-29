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
#include "comet/input.h"
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

void CometEngine::syncAsPoint(Common::Serializer &s, Common::Point &point) {
	s.syncAsUint16LE(point.x);
	s.syncAsUint16LE(point.y);
}

void CometEngine::syncAsRect(Common::Serializer &s, Common::Rect &rect) {
	s.syncAsUint16LE(rect.left);
	s.syncAsUint16LE(rect.top);
	s.syncAsUint16LE(rect.right);
	s.syncAsUint16LE(rect.bottom);
}

void CometEngine::syncScriptVars(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(_scriptVars); ++i)
		s.syncAsUint16LE(_scriptVars[i]);
}

void CometEngine::savegame(const char *filename, const char *description) {

	// TODO Later use Serializer and add serializer code to the various classes

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

	Common::Serializer s(0, out);

	out->writeUint16LE(_moduleNumber);
	out->writeUint16LE(_currentModuleNumber);
	out->writeUint16LE(_prevModuleNumber);
	out->writeUint16LE(_sceneNumber);
	out->writeUint16LE(_currentSceneNumber);
	out->writeUint16LE(_prevSceneNumber);
	
	out->writeUint16LE(_backgroundFileIndex);
	out->writeUint16LE(_talkText->_textTableIndex);
	out->writeUint32LE(_gameLoopCounter);
	out->writeByte(_input->getBlockedInput());

	out->writeByte(_scene->_exits.size());
	for (Common::Array<SceneExitItem>::iterator iter = _scene->_exits.begin(); iter != _scene->_exits.end(); ++iter) {
		SceneExitItem &sceneExit = *iter;
		sceneExit.sync(s);
	}

	out->writeByte(_script->_scriptCount);
	for (int i = 0; i < _script->_scriptCount; i++) {
		Script &script = *_script->_scripts[i];
		script.sync(s);
	}
	
	out->writeByte(_scene->_bounds.size());
	for (PointArray::iterator iter = _scene->_bounds.begin(); iter != _scene->_bounds.end(); ++iter) {
		Common::Point &bound = *iter;
		syncAsPoint(s, bound);
	}
	
	// TODO Later move the loop into the Actors class and actual serilization code into Actor
	for (uint actorIndex = 0; actorIndex < _actors->getCount(); ++actorIndex) {
		Actor *actor = _actors->getActor(actorIndex);
		actor->sync(s);
	}
	
	for (uint i = 0; i < kAnimationSlotCount; i++) {
		const AnimationSlot *marcheItem = _animationMan->getAnimationSlot(i);
		out->writeUint16LE(marcheItem->animationType);
		out->writeUint16LE(marcheItem->fileIndex);
	}
	
	out->writeByte(_scene->_sceneItems.size());
	for (Common::Array<SceneItem>::iterator iter = _scene->_sceneItems.begin(); iter != _scene->_sceneItems.end(); ++iter) {
		SceneItem &sceneItem = *iter;
		sceneItem.sync(s);
	}

	out->writeByte(_scene->_blockingRects.size());
	for (Common::Array<Common::Rect>::iterator iter = _scene->_blockingRects.begin(); iter != _scene->_blockingRects.end(); ++iter) {
		Common::Rect &blockingRect = *iter;
		syncAsRect(s, blockingRect);
	}

	out->writeByte(_paletteBrightness);
	out->writeByte(_paletteRedness);
	out->writeUint16LE(_screen->_zoomX);
	out->writeUint16LE(_screen->_zoomY);

	out->write(_scene->_boundsMap, 320);
	
	_inventory.sync(s);
	syncScriptVars(s);

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
	
	Common::Serializer s(in, 0);

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
		_input->setBlockedInput(in->readByte());
	else
		_input->setBlockedInput(0);

	_scene->_exits.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		SceneExitItem sceneExit;
		sceneExit.sync(s);
		_scene->_exits.push_back(sceneExit);
	}

	_script->_scriptCount = in->readByte();
	for (int i = 0; i < _script->_scriptCount; i++) {
		Script &script = *_script->_scripts[i];
		script.sync(s);
	}

	_scene->_bounds.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		Common::Point bound;
		syncAsPoint(s, bound);
		_scene->_bounds.push_back(bound);
	}

	for (uint actorIndex = 0; actorIndex < _actors->getCount(); ++actorIndex) {
		Actor *actor = _actors->getActor(actorIndex);
		actor->sync(s);
	}

	for (uint i = 0; i < kAnimationSlotCount; i++) {
		AnimationSlot *marcheItem = _animationMan->getAnimationSlot(i);
		marcheItem->animationType = in->readUint16LE();
		marcheItem->fileIndex = (int16)in->readUint16LE();
		marcheItem->anim = NULL;
	}

	_scene->_sceneItems.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		SceneItem sceneItem;
		sceneItem.sync(s);
		_scene->_sceneItems.push_back(sceneItem);
	}

	_scene->_blockingRects.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		Common::Rect blockingRect;
		syncAsRect(s, blockingRect);
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

	_inventory.sync(s);
	syncScriptVars(s);

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
