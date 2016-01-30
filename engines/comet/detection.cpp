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

Common::Language CometEngine::getLanguage() const {
	return _gameDescription->desc.language;
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
	{"musee", "Lovecraft Museum"},

	{0, 0}
};

namespace Comet {

static const CometGameDescription gameDescriptions[] = {

	// Shadow Of The Comet - English Floppy Version
	{
		{
			"comet",
			"English",
			{{"r00.cc4", 0, "b3a8616c12b87f8cddccd389e02d8a55", -1},
			 {  "e.cc4", 0, "5a5eeea3a5112a32009a3ec9c540554a", -1},
			 AD_LISTEND},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_COMET,
		0,
		GF_FLOPPY,
		0
	},

	// Shadow Of The Comet - English Version
	{
		{
			"comet",
			"English",
			{{"r00.cc4", 0, "f664b9cf60c9895f6f460d9432d45d85", -1},
			 {  "e.cc4", 0, "494620a4e5c27ca0b350f9a8b07911e8", -1},
			 AD_LISTEND},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_COMET,
		0,
		0,
		0
	},

	// Shadow Of The Comet - German Version
	{
		{
			"comet",
			"German",
			{{"r00.cc4", 0, "f664b9cf60c9895f6f460d9432d45d85", -1},
			 {  "d.cc4", 0, "84546199e56cb461cf6e130a052a72d8", -1},
			 AD_LISTEND},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_COMET,
		0,
		0,
		0
	},

	// Shadow Of The Comet - Italian Version
	{
		{
			"comet",
			"Italian",
			{{"r00.cc4", 0, "f664b9cf60c9895f6f460d9432d45d85", -1},
			 {  "i.cc4", 0, "542351b71e06e8610b6cc145b3123db2", -1},
			 AD_LISTEND},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_COMET,
		0,
		0,
		0
	},

	// Shadow Of The Comet - Spanish Version
	{
		{
			"comet",
			"Spanish",
			{{"r00.cc4", 0, "f664b9cf60c9895f6f460d9432d45d85", -1},
			 {  "s.cc4", 0, "95d64f493878dd222864371f16d29ca7", -1},
			 AD_LISTEND},
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_COMET,
		0,
		0,
		0
	},

	// Shadow Of The Comet - French Version
	{
		{
			"comet",
			"French",
			{{"r00.cc4", 0, "f664b9cf60c9895f6f460d9432d45d85", -1},
			 {  "t.cc4", 0, "3f9be4f98216f425c24fad1c3d306dcb", -1},
			 AD_LISTEND},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_COMET,
		0,
		0,
		0
	},

	// Lovecraft Museum - English Version
	{
		{
			"musee",
			"English",
			{{"r00.cc4", 0, "7219f01576e81fc9d5a9330de898b21a", -1},
			 {  "e.cc4", 0, "420714a8cd2528095f11607ef470f9d4", -1},
			 AD_LISTEND},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GID_MUSEUM,
		0,
		0,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

} // End of namespace Comet

using namespace Comet;

class CometMetaEngine : public AdvancedMetaEngine {
public:
	CometMetaEngine() : AdvancedMetaEngine(Comet::gameDescriptions, sizeof(Comet::CometGameDescription), cometGames) {
		_singleid = "comet";
	}

	virtual const char *getName() const {
		return "Comet Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Shadow of the Comet";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool CometMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Comet::CometGameDescription *gd = (const Comet::CometGameDescription *)desc;
	if (gd) {
		*engine = new Comet::CometEngine(syst, gd);
	}
	return gd != 0;
}

bool CometMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail);
}

bool Comet::CometEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList CometMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Comet::CometEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Comet::CometEngine::readSaveHeader(in, false, header) == Comet::CometEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}

	return saveList;
}

int CometMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void CometMetaEngine::removeSaveState(const char *target, int slot) const {
	// Slot 0 can't be deleted, it's for restarting the game(s)
	if (slot == 0)
		return;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Comet::CometEngine::getSavegameFilename(target, slot);

	saveFileMan->removeSavefile(filename.c_str());

	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		// Rename every slot greater than the deleted slot,
		// Also do not rename quicksaves.
		if (slotNum > slot && slotNum < 990) {
			// FIXME: Our savefile renaming done here is inconsitent with what we do in
			// GUI_v2::deleteMenu. While here we rename every slot with a greater equal
			// number of the deleted slot to deleted slot, deleted slot + 1 etc.,
			// we only rename the following slots in GUI_v2::deleteMenu until a slot
			// is missing.
			saveFileMan->renameSavefile(file->c_str(), filename.c_str());
			filename = Comet::CometEngine::getSavegameFilename(target, ++slot);
		}
	}

}

SaveStateDescriptor CometMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Comet::CometEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Comet::CometEngine::SaveHeader header;
		Comet::CometEngine::kReadSaveHeaderError error;

		error = Comet::CometEngine::readSaveHeader(in, true, header);
		delete in;

		if (error == Comet::CometEngine::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);

			desc.setDeletableFlag(false);
			desc.setWriteProtectedFlag(false);
			desc.setThumbnail(header.thumbnail);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(COMET)
REGISTER_PLUGIN_DYNAMIC(COMET, PLUGIN_TYPE_ENGINE, CometMetaEngine);
#else
REGISTER_PLUGIN_STATIC(COMET, PLUGIN_TYPE_ENGINE, CometMetaEngine);
#endif
