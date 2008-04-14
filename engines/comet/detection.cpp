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

#include "common/scummsys.h"

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/file.h"

#include "comet/comet.h"


namespace Comet {

struct CometGameDescription {
	Common::ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 CometEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 CometEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform CometEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 CometEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor cometGames[] = {
	{"comet", "Shadow of the Comet game"},

	{0, 0}
};


namespace Comet {

static const CometGameDescription gameDescriptions[] = {

	{
		// Comet English version
		{
			"comet",
			"English",
			AD_ENTRY1("r00.cc4", "f664b9cf60c9895f6f460d9432d45d85"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		0,
		0,
		0,
		0,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the Comet engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static CometGameDescription g_fallbackDesc = {
	{
		"", // Not used by the fallback descriptor, it uses the EncapsulatedADGameDesc's gameid
		"", // Not used by the fallback descriptor, it uses the EncapsulatedADGameDesc's extra
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformPC,
		Common::ADGF_NO_FLAGS
	},
	0,
	0,
	0,
	0,
};

Common::EncapsulatedADGameDesc fallbackDetector(const FSList *fslist) {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	g_fallbackDesc.desc.language = Common::UNK_LANG;
	g_fallbackDesc.desc.platform = Common::kPlatformPC;
	g_fallbackDesc.desc.flags = Common::ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's CometGameDescription part.
	g_fallbackDesc.gameID = 0;
	g_fallbackDesc.features = 0;
	g_fallbackDesc.version = 0;

	Common::EncapsulatedADGameDesc result;

	return result;
}

} // End of namespace Comet

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Comet::gameDescriptions,
	// Size of that superset structure
	sizeof(Comet::CometGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	cometGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"comet",
	// List of files for file-based fallback detection (optional)
	0,
	// Fallback callback
	Comet::fallbackDetector,
	// Flags
	Common::kADFlagAugmentPreferredTarget
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(COMET, Comet::CometEngine, detectionParams);

REGISTER_PLUGIN(COMET, "Comet Engine", "Shadow of the Comet");

namespace Comet {

bool CometEngine::initGame() {
	Common::EncapsulatedADGameDesc encapsulatedDesc = Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	_gameDescription = (const CometGameDescription *)(encapsulatedDesc.realDesc);

	return (_gameDescription != 0);
}

} // End of namespace Comet

