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

#ifndef NANCY_STATE_STATE_H
#define NANCY_STATE_STATE_H

#include "common/types.h"

namespace Nancy {
namespace State {

class State {
public:
    State() {}
    virtual ~State() =default;

    virtual void process() =0;
    virtual void onStateEnter() {}

    // Returns whether the object destroyed itself after exit
    virtual bool onStateExit() { return true; }
};

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_STATE_H
