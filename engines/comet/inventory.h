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

#ifndef COMET_INVENTORY_H
#define COMET_INVENTORY_H

#include "common/array.h"
#include "common/savefile.h"
#include "common/serializer.h"

namespace Comet {

class Inventory {
public:
	Inventory();
	void clear();
	int16 getStatus(int itemIndex);
	void setStatus(int itemIndex, int16 status);
	int16 *getStatusPtr(int itemIndex);
	int getSelectedItem();
	void selectItem(int itemIndex);
	void requestGetItem(int itemIndex);
	void requestUseSelectedItem();
	void testSelectedItemRemoved();
	void testSelectFirstItem();
	void resetStatus();
	void buildItems(Common::Array<uint16> &items, int &firstItem, int &currentItem);
	void sync(Common::Serializer &s);
protected:
	int16 _itemStatus[256];
	int _currentItem;
	int _itemIndex;
};

}

#endif
