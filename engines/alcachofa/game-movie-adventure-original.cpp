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

#include "alcachofa/alcachofa.h"
#include "alcachofa/game.h"
#include "alcachofa/script.h"

#include "common/config-manager.h"

using namespace Common;

namespace Alcachofa {

class GameMovieAdventureOriginal : public Game {
public:
	GameMovieAdventureOriginal() {
		const auto &desc = g_engine->gameDescription();
		if (desc.desc.flags & ADGF_CD) {
			const Path gameDir = ConfMan.getPath("path");
			SearchMan.addDirectory(gameDir.append("disk1/Install"));
			SearchMan.addDirectory(gameDir.append("disk2/Install"));
		}
	}

	Point getResolution() {
		return Point(800, 600);
	}

	static constexpr const char *kMapFiles[] = {
		"oeste.emc",
		"terror.emc",
		"global.emc",
		nullptr
	};
	const char *const *getMapFiles() {
		return kMapFiles;
	}
};

Game *Game::createForMovieAdventureOriginal() {
	return new GameMovieAdventureOriginal();
}

}
