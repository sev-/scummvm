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

#include "sound/mixer.h"

#include "comet/comet.h"
#include "comet/animationmgr.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/script.h"

namespace Comet {

/* TODO:
	- Saveload is working so far but only one slot is supported until the game menu is implemented
	- Save with F7; Load with F9
	- Saving during an animation (AnimationPlayer) is not working correctly yet
	- Maybe switch to SCUMM/Tinsel serialization approach?
*/

#define SAVEGAME_VERSION 0 // 0 is dev version until in official SVN

CometEngine::kReadSaveHeaderError CometEngine::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {

	header.version = in->readUint32LE();
	if (header.version != SAVEGAME_VERSION)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--)
		header.description += (char)in->readByte();

	if (loadThumbnail) {
		header.thumbnail = new Graphics::Surface();
		assert(header.thumbnail);
		if (!Graphics::loadThumbnail(*in, *header.thumbnail)) {
			delete header.thumbnail;
			header.thumbnail = 0;
		}
	} else {
		Graphics::skipThumbnailHeader(*in);
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
	
//	Graphics::saveThumbnail(*out);

	// Not used yet, reserved for future usage
	out->writeByte(0); // gameID
	out->writeUint32LE(0); // flags

	// Save...
	/* TODO: This will need some refactoring so that each class saves its own stuff. */
	
	out->writeUint16LE(_chapterNumber);
	out->writeUint16LE(_currentChapterNumber);
	out->writeUint16LE(_prevChapterNumber);
	out->writeUint16LE(_sceneNumber);
	out->writeUint16LE(_currentSceneNumber);
	out->writeUint16LE(_prevSceneNumber);
	
	out->writeUint16LE(_backgroundFileIndex);
	out->writeUint16LE(_narFileIndex);
	out->writeUint32LE(_gameLoopCounter);

	out->writeByte(_scene->_sceneExits.size());
	for (Common::Array<SceneExitItem>::iterator iter = _scene->_sceneExits.begin(); iter != _scene->_sceneExits.end(); ++iter) {
		const SceneExitItem &sceneExit = *iter;
		out->writeUint16LE(sceneExit.directionIndex);
		out->writeUint16LE(sceneExit.chapterNumber);
		out->writeUint16LE(sceneExit.sceneNumber);
		out->writeUint16LE(sceneExit.x1);
		out->writeUint16LE(sceneExit.x2);
	}

	out->writeByte(_script->_scriptCount);
	for (int i = 0; i < _script->_scriptCount; i++) {
		const Script &script = *_script->_scripts[i];
		out->writeUint16LE(script.ip - script.code);
		out->writeByte(script.objectIndex);
		out->writeUint16LE(script.status);
		out->writeUint16LE(script.scriptNumber);
		out->writeUint16LE(script.counter);
		out->writeUint16LE(script.x);
		out->writeUint16LE(script.y);
		out->writeUint16LE(script.x2);
		out->writeUint16LE(script.y2);
	}
	
	out->writeByte(_scene->_bounds.size());
	for (PointArray::iterator iter = _scene->_bounds.begin(); iter != _scene->_bounds.end(); ++iter) {
		const Common::Point &bound = *iter;
		out->writeUint16LE(bound.x);
		out->writeUint16LE(bound.y);
	}
	
	for (int i = 0; i < ARRAYSIZE(_sceneObjects); i++) {
		const SceneObject &sceneObject = _sceneObjects[i];
		out->writeUint16LE(sceneObject.x);
		out->writeUint16LE(sceneObject.y);
		out->writeUint16LE(sceneObject.directionAdd);
		out->writeUint16LE(sceneObject.directionChanged);
		out->writeUint16LE(sceneObject.direction);
		out->writeByte(sceneObject.flag2);
		out->writeUint16LE(sceneObject.animationSlot);
		out->writeUint16LE(sceneObject.animIndex);
		out->writeUint16LE(sceneObject.animFrameIndex);
		out->writeUint16LE(sceneObject.value4);
		out->writeUint16LE(sceneObject.animFrameCount);
		out->writeUint16LE(sceneObject.animSubIndex2);
		out->writeUint16LE(sceneObject.deltaX);
		out->writeUint16LE(sceneObject.deltaY);
		out->writeUint16LE(sceneObject.collisionType);
		out->writeUint16LE(sceneObject.collisionIndex);
		out->writeByte(sceneObject.value6);
		out->writeUint16LE(sceneObject.life);
		out->writeByte(sceneObject.textColor);
		out->writeByte(sceneObject.value7);
		out->writeUint16LE(sceneObject.textX);
		out->writeUint16LE(sceneObject.textY);
		out->writeUint16LE(sceneObject.walkStatus);
		out->writeUint16LE(sceneObject.walkDestX);
		out->writeUint16LE(sceneObject.walkDestY);
		out->writeUint16LE(sceneObject.savedWalkDestX);
		out->writeUint16LE(sceneObject.savedWalkDestY);
		out->writeUint16LE(sceneObject.clipX1);
		out->writeUint16LE(sceneObject.clipY1);
		out->writeUint16LE(sceneObject.clipX2);
		out->writeUint16LE(sceneObject.clipY2);
		out->writeByte(sceneObject.visible ? 1 : 0);
	}
	
	for (int i = 0; i < ARRAYSIZE(_animationMan->_animationSlots); i++) {
		const AnimationSlot &marcheItem = _animationMan->_animationSlots[i];
		out->writeUint16LE(marcheItem.animationType);
		out->writeUint16LE(marcheItem.fileIndex);
	}
	
	out->writeByte(_sceneItems.size());
	for (Common::Array<SceneItem>::iterator iter = _sceneItems.begin(); iter != _sceneItems.end(); ++iter) {
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
	out->writeUint16LE(_screen->_zoomX);
	out->writeUint16LE(_screen->_zoomY);

	out->write(_scene->_boundsMap, 320);
	
	for (int i = 0; i < 256; i++)
		out->writeUint16LE(_itemStatus[i]);

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

	// Load...
	/* TODO: This will need some refactoring so that each class loads its own stuff. */

	int count;
	
	_animationMan->purgeAnimationSlots();
	
	_chapterNumber = in->readUint16LE();
	_currentChapterNumber = in->readUint16LE();
	_prevChapterNumber = in->readUint16LE();
	_sceneNumber = in->readUint16LE();
	_currentSceneNumber = in->readUint16LE();
	_prevSceneNumber = in->readUint16LE();

	_backgroundFileIndex = in->readUint16LE();
	_narFileIndex = in->readUint16LE();
	_gameLoopCounter = in->readUint32LE();

	_scene->_sceneExits.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		SceneExitItem sceneExit;
		sceneExit.directionIndex = in->readUint16LE();
		sceneExit.chapterNumber = in->readUint16LE();
		sceneExit.sceneNumber = in->readUint16LE();
		sceneExit.x1 = in->readUint16LE();
		sceneExit.x2 = in->readUint16LE();
		_scene->_sceneExits.push_back(sceneExit);
	}

	_script->_scriptCount = in->readByte();
	for (int i = 0; i < _script->_scriptCount; i++) {
		Script &script = *_script->_scripts[i];
		script.resumeIp = in->readUint16LE();
		script.objectIndex = in->readByte();
		script.status = in->readUint16LE();
		script.scriptNumber = in->readUint16LE();
		script.counter = in->readUint16LE();
		script.x = in->readUint16LE();
		script.y = in->readUint16LE();
		script.x2 = in->readUint16LE();
		script.y2 = in->readUint16LE();
	}

	_scene->_bounds.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		Common::Point bound;
		bound.x = in->readUint16LE();
		bound.y = in->readUint16LE();
		_scene->_bounds.push_back(bound);
	}

	for (int i = 0; i < ARRAYSIZE(_sceneObjects); i++) {
		SceneObject &sceneObject = _sceneObjects[i];
		sceneObject.x = in->readUint16LE();
		sceneObject.y = in->readUint16LE();
		sceneObject.directionAdd = in->readUint16LE();
		sceneObject.directionChanged = in->readUint16LE();
		sceneObject.direction = in->readUint16LE();
		sceneObject.flag2 = in->readByte();
		sceneObject.animationSlot = in->readUint16LE();
		sceneObject.animIndex = in->readUint16LE();
		sceneObject.animFrameIndex = in->readUint16LE();
		sceneObject.value4 = in->readUint16LE();
		sceneObject.animFrameCount = in->readUint16LE();
		sceneObject.animSubIndex2 = in->readUint16LE();
		sceneObject.deltaX = in->readUint16LE();
		sceneObject.deltaY = in->readUint16LE();
		sceneObject.collisionType = in->readUint16LE();
		sceneObject.collisionIndex = in->readUint16LE();
		sceneObject.value6 = in->readByte();
		sceneObject.life = in->readUint16LE();
		sceneObject.textColor = in->readByte();
		sceneObject.value7 = in->readByte();
		sceneObject.textX = in->readUint16LE();
		sceneObject.textY = in->readUint16LE();
		sceneObject.walkStatus = in->readUint16LE();
		sceneObject.walkDestX = in->readUint16LE();
		sceneObject.walkDestY = in->readUint16LE();
		sceneObject.savedWalkDestX = in->readUint16LE();
		sceneObject.savedWalkDestY = in->readUint16LE();
		sceneObject.clipX1 = in->readUint16LE();
		sceneObject.clipY1 = in->readUint16LE();
		sceneObject.clipX2 = in->readUint16LE();
		sceneObject.clipY2 = in->readUint16LE();
		sceneObject.visible = in->readByte() != 0;
	}

	for (int i = 0; i < ARRAYSIZE(_animationMan->_animationSlots); i++) {
		AnimationSlot &marcheItem = _animationMan->_animationSlots[i];
		marcheItem.animationType = in->readUint16LE();
		marcheItem.fileIndex = (int16)in->readUint16LE();
		debug("marcheItem.animationType = %d; marcheItem.fileIndex = %d", marcheItem.animationType, marcheItem.fileIndex);
		marcheItem.anim = NULL;
	}

	_sceneItems.clear();
	count = in->readByte();
	for (int i = 0; i < count; i++) {
		SceneItem sceneItem;
		sceneItem.itemIndex = in->readUint16LE();
		sceneItem.active = in->readByte() != 0;
		sceneItem.paramType = in->readByte();
		sceneItem.x = in->readUint16LE();
		sceneItem.y = in->readUint16LE();
		_sceneItems.push_back(sceneItem);
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
	_screen->_zoomX = in->readUint16LE();
	_screen->_zoomY = in->readUint16LE();

	in->read(_scene->_boundsMap, 320);

	for (int i = 0; i < 256; i++)
		_itemStatus[i] = in->readUint16LE();

	for (int i = 0; i < 256; i++)
		_scriptVars[i] = in->readUint16LE();

	delete in;
	
	setChapterAndScene(_currentChapterNumber, _currentSceneNumber);
	loadAndRunScript(true);
	initSceneBackground(true);
	_animationMan->restoreAnimationSlots();
	_screen->buildPalette(_ctuPal, _palette, _paletteBrightness);
	_screen->setFullPalette(_palette);

	openVoiceFile(_narFileIndex); // NEW in reimplementation

	// TODO: palStuff2

}

Common::Error CometEngine::loadGameState(int slot) {
	const char *fileName = getSavegameFilename(slot);
	loadgame(fileName);
	return Common::kNoError;
}

Common::Error CometEngine::saveGameState(int slot, const char *description) {
	const char *fileName = getSavegameFilename(slot);
	savegame(fileName, description);
	return Common::kNoError;
}

const char *CometEngine::getSavegameFilename(int num) {
	static Common::String filename;
	filename = getSavegameFilename(_targetName, num);
	return filename.c_str();
}

Common::String CometEngine::getSavegameFilename(const Common::String &target, int num) {
	assert(num >= 0 && num <= 999);

	char extension[5];
	sprintf(extension, "%03d", num);

	return target + "." + extension;
}

} // End of namespace Comet
