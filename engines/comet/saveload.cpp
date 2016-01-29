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

const uint SAVEGAME_VERSION = 3;     // < 1000 is dev version until in official SVN
const uint SAVEGAME_VERSION_MIN = 3; // Minimum supported savegame version

CometEngine::kReadSaveHeaderError CometEngine::readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header) {
	header.version = in->readUint32LE();
	if (header.version > SAVEGAME_VERSION || header.version < SAVEGAME_VERSION_MIN)
		return kRSHEInvalidVersion;

	byte descriptionLen = in->readByte();
	header.description = "";
	while (descriptionLen--)
		header.description += (char)in->readByte();

	if (loadThumbnail) {
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

void CometEngine::syncPaletteInfo(Common::Serializer &s) {
	s.syncAsByte(_paletteBrightness);
	s.syncAsByte(_paletteRedness);
}

void CometEngine::sync(Common::Serializer &s) {
	s.syncAsUint32LE(_gameLoopCounter);
	s.syncAsUint16LE(_backgroundFileIndex);
	syncModuleSceneInfo(s);
	_actors->sync(s);
	_animationMan->sync(s);
	_input->sync(s);
	_scene->sync(s);
	_talkText->sync(s);
	_script->syncScripts(s);
	syncPaletteInfo(s);
	_screen->syncZoom(s);
	_inventory.sync(s);
	syncScriptVars(s);
}

void CometEngine::savegame(const char *filename, const char *description) {

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
	sync(s);

	delete out;

}

void CometEngine::loadgame(const char *filename) {

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
	sync(s);

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
