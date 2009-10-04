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

#include "engines/advancedDetector.h"
#include "common/file.h"

#include "comet/comet.h"


namespace Comet {

struct CometGameDescription {
	ADGameDescription desc;

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
	{"comet", "Shadow of the Comet"},

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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		0,
		0,
		0,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

static const ADParams detectionParams = {
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
	// Flags
	0,
	// GUI options
	Common::GUIO_NONE
};

} // End of namespace Comet

using namespace Comet;

class CometMetaEngine : public AdvancedMetaEngine {
public:
	CometMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Comet Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Shadow of the Comet";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool CometMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Comet::CometGameDescription *gd = (const Comet::CometGameDescription *)desc;
	if (gd) {
		*engine = new Comet::CometEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(COMET)
REGISTER_PLUGIN_DYNAMIC(COMET, PLUGIN_TYPE_ENGINE, CometMetaEngine);
#else
REGISTER_PLUGIN_STATIC(COMET, PLUGIN_TYPE_ENGINE, CometMetaEngine);
#endif
