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
#include "enchantia/enchantia.h"

#include "common/system.h"

const char *EnchantiaMetaEngine::getName() const {
	return "enchantia";
}

Common::Error EnchantiaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Enchantia::EnchantiaEngine(syst, desc);
	return Common::kNoError;
}

bool EnchantiaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSavesUseExtendedFormat) ||
		(f == kSimpleSavesNames) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup);
}

bool Enchantia::EnchantiaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList EnchantiaMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ExtendedSavegameHeader header;
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
				if (MetaEngine::readSavegameHeader(in, &header, false)) {
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
		ExtendedSavegameHeader header;
		if (MetaEngine::readSavegameHeader(in, &header)) {
			delete in;
			SaveStateDescriptor desc(this, slot, header.description);
			desc.setDeletableFlag(true);
			desc.setWriteProtectedFlag(false);
			desc.setThumbnail(header.thumbnail);

			int day = (header.date >> 24) & 0xFF;
			int month = (header.date >> 16) & 0xFF;
			int year = header.date & 0xFFFF;
			desc.setSaveDate(year, month, day);

			int hour = (header.time >> 8) & 0xFF;
			int min = header.time & 0xFF;
			desc.setSaveTime(hour, min);

			desc.setPlayTime(header.playtime * 1000);
			return desc;
		}
		delete in;
	}
	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(ENCHANTIA)
REGISTER_PLUGIN_DYNAMIC(ENCHANTIA, PLUGIN_TYPE_ENGINE, EnchantiaMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ENCHANTIA, PLUGIN_TYPE_ENGINE, EnchantiaMetaEngine);
#endif
