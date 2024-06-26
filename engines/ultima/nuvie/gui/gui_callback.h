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

#ifndef NUVIE_GUI_GUI_CALLBACK_H
#define NUVIE_GUI_GUI_CALLBACK_H

#include "ultima/nuvie/gui/gui_status.h"
#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

class GUI_CallBack {

public:

	GUI_CallBack() {
		return;
	}
	virtual ~GUI_CallBack() {
		return;
	}
	virtual GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) {
		DEBUG(false, LEVEL_WARNING, "Unhandled callback. msg (%x)\n", msg);
		return GUI_PASS;
	}

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
