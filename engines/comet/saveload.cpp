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

void CometEngine::syncModuleSceneInfo(Common::Serializer &s) {
	s.syncAsUint16LE(_moduleNumber);
	s.syncAsUint16LE(_currentModuleNumber);
	s.syncAsUint16LE(_prevModuleNumber);
	s.syncAsUint16LE(_sceneNumber);
	s.syncAsUint16LE(_currentSceneNumber);
	s.syncAsUint16LE(_prevSceneNumber);
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

	syncModuleSceneInfo(s);

	out->writeUint16LE(_backgroundFileIndex);
	out->writeUint16LE(_talkText->_textTableIndex);
	out->writeUint32LE(_gameLoopCounter);
	out->writeByte(_input->getBlockedInput());

	_scene->syncExits(s);
	_script->syncScripts(s);
	_scene->syncBounds(s);
	
	_actors->sync(s);
	_animationMan->sync(s);
	_scene->syncSceneItems(s);
	_scene->syncBlockingRects(s);

	out->writeByte(_paletteBrightness);
	out->writeByte(_paletteRedness);

	_screen->syncZoom(s);
	_scene->syncBoundsMap(s);
	
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

	_animationMan->purgeAnimationSlots();
	
	Common::Serializer s(in, 0);
	
	syncModuleSceneInfo(s);

	_backgroundFileIndex = in->readUint16LE();
	uint voiceFileIndex = in->readUint16LE();
	_gameLoopCounter = in->readUint32LE();
	
	if (header.version > 0)//REMOVEME	
		_input->setBlockedInput(in->readByte());
	else
		_input->setBlockedInput(0);

	_scene->syncExits(s);
	_script->syncScripts(s);
	_scene->syncBounds(s);

	_actors->sync(s);
	_animationMan->sync(s);
	_scene->syncSceneItems(s);
	_scene->syncBlockingRects(s);

	_paletteBrightness = in->readByte();
	
	if (header.version > 0)//REMOVEME	
		_paletteRedness = in->readByte();
	else
		_paletteRedness = 0;
	
	_screen->syncZoom(s);
	_scene->syncBoundsMap(s);

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
