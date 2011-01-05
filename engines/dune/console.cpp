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

// Console module

#include "dune/animation.h"
#include "dune/console.h"
#include "dune/resource.h"
#include "dune/sentences.h"

namespace Dune {

DuneConsole::DuneConsole(DuneEngine *engine) : GUI::Debugger(),
	_engine(engine) {

	DCmd_Register("dump",				WRAP_METHOD(DuneConsole, cmdDump));
	DCmd_Register("sentences",			WRAP_METHOD(DuneConsole, cmdSentences));
	DCmd_Register("animinfo",			WRAP_METHOD(DuneConsole, cmdAnimInfo));
	DCmd_Register("animshow",			WRAP_METHOD(DuneConsole, cmdAnimShow));
}

DuneConsole::~DuneConsole() {
}

bool DuneConsole::cmdDump(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Decompresses the given HSQ file into a raw uncompressed file\n");
		DebugPrintf("  Usage: %s <file name>\n\n", argv[0]);
		DebugPrintf("  Example: %s phrase11.hsq\n", argv[0]);
		DebugPrintf("  The above will uncompress phrase11.hsq into phrase11.hsq.raw\n");
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Resource *hsqResource = new Resource(fileName);
	hsqResource->dump(fileName + ".raw");
	delete hsqResource;

	DebugPrintf("%s has been dumped to %s\n", fileName.c_str(), (fileName + ".raw").c_str());
	return true;
}

bool DuneConsole::cmdSentences(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Shows information about a sentence file, or prints a specific sentence from a file\n");
		DebugPrintf("  Usage: %s <file name> <sentence number>\n\n", argv[0]);
		DebugPrintf("  Example: \"%s phrase12.hsq\" - show information on file phrase12.hsq\n", argv[0]);
		DebugPrintf("  Example: \"%s phrase12.hsq 0\" - print sentence with index 0 from file phrase12.hsq\n\n", argv[0]);
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Resource *hsqResource = new Resource(fileName);
	Sentences *s = new Sentences(hsqResource->_stream);
	if (argc == 2) {
		DebugPrintf("File contains %d sentences\n", s->count());
	} else {
		if (atoi(argv[2]) >= s->count())
			DebugPrintf("Invalid sentence\n");
		else
			DebugPrintf("%s\n", s->getSentence(atoi(argv[2]), true).c_str());
	}
	delete s;
	delete hsqResource;

	return true;
}

bool DuneConsole::cmdAnimInfo(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Shows information about an animation or image file\n");
		DebugPrintf("  Usage: %s <file name>\n\n", argv[0]);
		DebugPrintf("  Example: \"%s balcon.hsq\" - show information on file balcon.hsq\n", argv[0]);
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Resource *hsqResource = new Resource(fileName);
	Animation *a = new Animation(hsqResource->_stream, _engine->_system);

	uint16 frameCount = a->getFrameCount();
	DebugPrintf("Frame count: %d\n", frameCount);
	for (int i = 0; i < frameCount; i++) {
		FrameInfo info = a->getFrameInfo(i);
		DebugPrintf("%d: offset %d, comp: %d, size: %dx%d, pal offset: %d\n",
				i, info.offset, info.isCompressed, info.width, info.height, info.palOffset);
	}

	delete a;
	delete hsqResource;

	return true;
}

bool DuneConsole::cmdAnimShow(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Shows an animation or image file on screen\n");
		DebugPrintf("  Usage: %s <file name> <frame number>\n\n", argv[0]);
		DebugPrintf("  Example: \"%s stars.hsq\" - plays animation stars.hsq\n", argv[0]);
		DebugPrintf("  Example: \"%s stars.hsq 0\" - shows frame 0 of the stars.hsq animation\n", argv[0]);
		DebugPrintf("  Example: \"%s balcon.hsq\" - shows image balcon.hsq\n", argv[0]);
		return true;
	}

	Common::String fileName(argv[1]);
	if (!fileName.contains('.'))
		fileName += ".hsq";

	Resource *hsqResource = new Resource(fileName);
	Animation *a = new Animation(hsqResource->_stream, _engine->_system);

	uint16 frameCount = a->getFrameCount();

	if (argc == 2) {
		// TODO
		DebugPrintf("TODO: Animation playing. Showing frame 0 instead\n");
		a->setPalette();
		a->drawFrame(0);
	} else {
		if (atoi(argv[2]) >= frameCount) {
			DebugPrintf("Invalid frame\n");
			delete a;
			delete hsqResource;
			return true;
		} else {
			a->setPalette();
			a->drawFrame(atoi(argv[2]));
		}
	}

	delete a;
	delete hsqResource;

	return false;
}

} // End of namespace Dune
