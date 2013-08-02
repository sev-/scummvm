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

#include "enchantia/metaengine.h"
#include "enchantia/detection.h"
#include "enchantia/enchantia.h"

const char *EnchantiaMetaEngine::getName() const {
	return "enchantia";
}

Common::Error EnchantiaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Enchantia::EnchantiaEngine(syst, desc);
	return Common::kNoError;
}

bool EnchantiaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool Enchantia::EnchantiaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList EnchantiaMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Enchantia::EnchantiaEngine::SaveHeader header;
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
				if (Enchantia::EnchantiaEngine::readSaveHeader(in, false, header) == Enchantia::EnchantiaEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(this, slotNum, header.description));
				}
				delete in;
			}
		}
	}
	return saveList;
}

int EnchantiaMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void EnchantiaMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Enchantia::EnchantiaEngine::getSavegameFilename(target, slot);
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
		if (slotNum > slot) {
			saveFileMan->renameSavefile(file->c_str(), filename.c_str());
			filename = Enchantia::EnchantiaEngine::getSavegameFilename(target, ++slot);
		}
	}
}

SaveStateDescriptor EnchantiaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Enchantia::EnchantiaEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());
	if (in) {
		Enchantia::EnchantiaEngine::SaveHeader header;
		Enchantia::EnchantiaEngine::kReadSaveHeaderError error;
		error = Enchantia::EnchantiaEngine::readSaveHeader(in, true, header);
		delete in;
		if (error == Enchantia::EnchantiaEngine::kRSHENoError) {
			SaveStateDescriptor desc(this, slot, header.description);
			desc.setDeletableFlag(true);
			desc.setWriteProtectedFlag(false);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveDate & 0xFFFF, (header.saveDate >> 16) & 0xFF, (header.saveDate >> 24) & 0xFF);
			desc.setSaveTime((header.saveTime >> 16) & 0xFF, (header.saveTime >> 8) & 0xFF);
			desc.setPlayTime(header.playTime * 1000);
			return desc;
		}
	}
	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(ENCHANTIA)
REGISTER_PLUGIN_DYNAMIC(ENCHANTIA, PLUGIN_TYPE_ENGINE, EnchantiaMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ENCHANTIA, PLUGIN_TYPE_ENGINE, EnchantiaMetaEngine);
#endif
