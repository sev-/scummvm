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

static constexpr const ScriptOp kScriptOpMap[] = {
	ScriptOp::Nop,
	ScriptOp::Dup,
	ScriptOp::PushAddr,
	ScriptOp::PushValue,
	ScriptOp::Deref,
	ScriptOp::Nop,
	ScriptOp::Pop1,
	ScriptOp::Store,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::PushDynAddr,
	ScriptOp::Nop,
	ScriptOp::ScriptCall,
	ScriptOp::KernelCall,
	ScriptOp::JumpIfFalse,
	ScriptOp::JumpIfTrue,
	ScriptOp::Jump,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Add,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Equals,
	ScriptOp::NotEquals,
	ScriptOp::BitAnd,
	ScriptOp::BitOr,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::Nop,
	ScriptOp::ReturnVoid
};

static constexpr const ScriptKernelTask kScriptKernelTaskMap[] = {
	ScriptKernelTask::Nop,
	ScriptKernelTask::SayText,
	ScriptKernelTask::Go,
	ScriptKernelTask::Delay,
	ScriptKernelTask::PlaySound,
	ScriptKernelTask::FadeIn,
	ScriptKernelTask::FadeOut,
	ScriptKernelTask::Put,
	ScriptKernelTask::ChangeRoom,
	ScriptKernelTask::PlayVideo,
	ScriptKernelTask::StopAndTurn,
	ScriptKernelTask::StopAndTurnMe,
	ScriptKernelTask::On,
	ScriptKernelTask::Off,
	ScriptKernelTask::Pickup,
	ScriptKernelTask::Animate,
	ScriptKernelTask::SheriffTakesCharacter,
	ScriptKernelTask::ChangeCharacter,
	ScriptKernelTask::LerpCamToObjectKeepingZ,
	ScriptKernelTask::Drop,
	ScriptKernelTask::ChangeDoor,
	ScriptKernelTask::Disguise,
	ScriptKernelTask::ToggleRoomFloor,
	ScriptKernelTask::SetDialogLineReturn,
	ScriptKernelTask::DialogMenu,
	ScriptKernelTask::ChangeCharacterRoom,
	ScriptKernelTask::PlayMusic,
	ScriptKernelTask::StopMusic,
	ScriptKernelTask::WaitForMusicToEnd
};

static constexpr const int kKernelTaskArgCounts[] = {
	0,
	2,
	3,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	2,
	1,
	1,
	1,
	2,
	2,
	0,
	1,
	2,
	1,
	1,
	1,
	0,
	1,
	0,
	2,
	1,
	0,
	0
};

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

	Point getResolution() override {
		return Point(800, 600);
	}

	static constexpr const char *kMapFiles[] = {
		"oeste.emc",
		"terror.emc",
		"global.emc",
		nullptr
	};
	const char *const *getMapFiles() override {
		return kMapFiles;
	}

	GameFileReference getScriptFileRef() override {
		// V1 embeds the script into global.emc, it is overridden during world load
		return {};
	}

	const char *getDialogFileName() override {
		return "TEXTOS.TXT";
	}

	const char *getObjectFileName() override {
		return "OBJETOS.TXT";
	}

	char getTextFileKey() override {
		return kNoXORKey;
	}

	const char *getInitScriptName() override {
		return "INICIALIZAR_MUNDO";
	}

	Span<const ScriptOp> getScriptOpMap() override {
		return { kScriptOpMap, ARRAYSIZE(kScriptOpMap) };
	}

	Span<const ScriptKernelTask> getScriptKernelTaskMap() override {
		return { kScriptKernelTaskMap, ARRAYSIZE(kScriptKernelTaskMap) };
	}

	int32 getKernelTaskArgCount(int32 taskI) override {
		assert(taskI >= 0 && taskI < ARRAYSIZE(kKernelTaskArgCounts));
		return kKernelTaskArgCounts[taskI];
	}

	void missingAnimation(const String &fileName) override {
		static const char *exemptions[] = {
			nullptr
		};

		const auto isInExemptions = [&] (const char *const *const list) {
			for (const char *const *exemption = list; *exemption != nullptr; exemption++) {
				if (fileName.equalsIgnoreCase(*exemption))
					return true;
			}
			return false;
		};

		if (isInExemptions(exemptions))
			debugC(1, kDebugGraphics, "Animation exemption triggered: %s", fileName.c_str());
		else
			Game::missingAnimation(fileName);
	}
};

Game *Game::createForMovieAdventureOriginal() {
	return new GameMovieAdventureOriginal();
}

}
