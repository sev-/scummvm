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

#include "comet/inventory.h"

namespace Comet {

// Inventory

Inventory::Inventory() {
	clear();
}

void Inventory::clear() {
	_itemIndex = -1;
	_currentItem = -1;
	for (uint i = 0; i < ARRAYSIZE(_itemStatus); ++i)
		_itemStatus[i] = 0;
}

int16 Inventory::getStatus(int itemIndex) {
	return _itemStatus[itemIndex];
}

void Inventory::setStatus(int itemIndex, int16 status) {
	_itemStatus[itemIndex] = status;
}

int16 *Inventory::getStatusPtr(int itemIndex) {
	return &_itemStatus[itemIndex];
}

int Inventory::getSelectedItem() {
	return _currentItem;
}

void Inventory::selectItem(int itemIndex) {
	_currentItem = itemIndex;
}

void Inventory::requestGetItem(int itemIndex) {
	_itemStatus[itemIndex] = 1;
	_itemIndex = itemIndex;
}

void Inventory::requestUseSelectedItem() {
	if (_currentItem != -1 && _itemStatus[_currentItem] == 1)
		_itemStatus[_currentItem] = 2;
}

void Inventory::testSelectedItemRemoved() {
	// If the currently selected item is disabled, scan for the preceeding item
	// and set it as selected item.
	if (_currentItem >= 0 && _itemStatus[_currentItem] == 0) {
		if (_currentItem >= 1) {
			for (_currentItem = _currentItem - 1;
				_currentItem >= 0 && _itemStatus[_currentItem] == 0;
				--_currentItem);
		} else {
			_currentItem = -1;
		}
	}
}

void Inventory::testSelectFirstItem() {
	// If the currently selected item is disabled, try to select the first
	// enabled item.
	if (_currentItem >= 0 && _itemStatus[_currentItem] == 0) {
		_currentItem = -1;
		for (int i = 0; i < ARRAYSIZE(_itemStatus); ++i)
			if (_itemStatus[i] > 0) {
				_currentItem = i;
				break;
			}
	}
}

void Inventory::resetStatus() {
	for (uint i = 0; i < ARRAYSIZE(_itemStatus); ++i)
		if (_itemStatus[i] == 2)
			_itemStatus[i] = 1;
}

void Inventory::buildItems(Common::Array<uint16> &items, int &firstItem, int &currentItem) {
	// Build items array and set up variables
	for (int i = 0; i < ARRAYSIZE(_itemStatus); i++) {
		if (_itemIndex != 0 && _itemIndex == i) {
			_currentItem = i;
			_itemIndex = 0;
		}
		if (_itemStatus[i] >= 1) {
			items.push_back(i);
			if (i == _currentItem) {
				firstItem = items.size() < 5 ? 0 : items.size() - 5;
				currentItem = items.size() - 1;
			}
		}
	}
}

void Inventory::sync(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(_itemStatus); ++i)
		s.syncAsUint16LE(_itemStatus[i]);
}

} // End of namespace Comet
