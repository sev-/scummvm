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

#include "base/plugins.h"
#include "enchantia/detection.h"

namespace Enchantia {

const PlainGameDescriptor enchantiaGames[] = {
	{ "enchantia", "Curse of Enchantia" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"enchantia",
		"",
		AD_ENTRY1s("title.dat","839b7c735f720bea86623d0599d5025c", 47064),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // namespace Enchantia

const DebugChannelDef EnchantiaMetaEngineDetection::debugFlagList[] = {
	{ Enchantia::kDebugTools, "Tools", "Tools debug level" },
	DEBUG_CHANNEL_END
};

EnchantiaMetaEngineDetection::EnchantiaMetaEngineDetection() : AdvancedMetaEngineDetection(Enchantia::gameDescriptions,
	sizeof(ADGameDescription), Enchantia::enchantiaGames) {
}

REGISTER_PLUGIN_STATIC(ENCHANTIA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, EnchantiaMetaEngineDetection);
