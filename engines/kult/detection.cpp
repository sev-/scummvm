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
 * $URL: https://svn.scummvm.org:4444/svn/kult/detection.cpp $
 * $Id: detection.cpp 111 2010-12-05 13:30:14Z md5 $
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"

#include "kult/kult.h"


namespace Kult {

struct KultGameDescription {
	ADGameDescription desc;
};

uint32 KultEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language KultEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor kultGames[] = {
	{"kult", "Kult"},
	{0, 0}
};


namespace Kult {

static const KultGameDescription gameDescriptions[] = {

	{
		// Kult
		{
			"kult",
			0,
			AD_ENTRY1s("kult2.pxi", "36a9a0a14c1badfff7643ac3fed24b43", 142658),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Kult

class KultMetaEngine : public AdvancedMetaEngine {
public:
	KultMetaEngine() : AdvancedMetaEngine(Kult::gameDescriptions, sizeof(Kult::KultGameDescription), kultGames) {
		_singleid = "kult";
	}

	virtual const char *getName() const {
		return "Kult Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Kult Engine";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool KultMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
	/*
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
//		(f == kSupportsDeleteSave) ||
	   	(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
	*/		
}

bool Kult::KultEngine::hasFeature(EngineFeature f) const {
	return false;
	/*
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
	*/		
}

bool KultMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Kult::KultGameDescription *gd = (const Kult::KultGameDescription *)desc;
	if (gd) {
		*engine = new Kult::KultEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(KULT)
	REGISTER_PLUGIN_DYNAMIC(KULT, PLUGIN_TYPE_ENGINE, KultMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(KULT, PLUGIN_TYPE_ENGINE, KultMetaEngine);
#endif
