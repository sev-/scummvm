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

#ifndef FREESCAPE_WB_H
#define FREESCAPE_WB_H

#include "audio/audiostream.h"
#include "common/types.h"

namespace Freescape {

/**
 * Create a music stream for the Wally Beben custom music engine
 * used in the Amiga version of Dark Side.
 *
 * @param data     Raw TEXT segment data from HDSMUSIC.AM (after 0x1C GEMDOS header)
 * @param dataSize Size of the TEXT segment (0xF4BC for Dark Side)
 * @param songNum  Song number to play (1 or 2)
 * @param rate     Output sample rate
 * @param stereo   Whether to produce stereo output
 * @return A new AudioStream, or nullptr on error
 */
Audio::AudioStream *makeWallyBebenStream(const byte *data, uint32 dataSize,
                                         int songNum = 1, int rate = 44100,
                                         bool stereo = true);

} // End of namespace Freescape

#endif
