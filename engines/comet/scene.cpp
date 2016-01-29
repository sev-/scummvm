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

#include "comet/comet.h"
#include "comet/music.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/dialog.h"

namespace Comet {

// SceneExitItem

void SceneExitItem::sync(Common::Serializer &s) {
	s.syncAsUint16LE(directionIndex);
	s.syncAsUint16LE(moduleNumber);
	s.syncAsUint16LE(sceneNumber);
	s.syncAsUint16LE(x1);
	s.syncAsUint16LE(x2);
}

// SceneItem

void SceneItem::sync(Common::Serializer &s) {
	s.syncAsUint16LE(itemIndex);
	s.syncAsByte(active);
	s.syncAsByte(paramType);
	s.syncAsUint16LE(x);
	s.syncAsUint16LE(y);
}

// Scene

Scene::Scene(CometEngine *vm) : _vm(vm) {
}

Scene::~Scene() {
}

void Scene::initBounds(byte *data) {
	byte count = *data++;
	_bounds.clear();
	while (count--) {
		int x = (*data++) * 2;
		int y = *data++;
		_bounds.push_back(Common::Point(x, y));
	}
	initBoundsMap();
}

void Scene::initExits(byte *data) {
	byte count = *data++;
	_exits.clear();
	while (count--) {
		SceneExitItem sceneExitItem;
		sceneExitItem.directionIndex = data[0];
		sceneExitItem.moduleNumber = data[1];
		sceneExitItem.sceneNumber = data[2];
		sceneExitItem.x1 = data[3] * 2;
		sceneExitItem.x2 = data[4] * 2;
		data += 5;
		_exits.push_back(sceneExitItem);
	}
}

void Scene::getExitLink(int index, int &moduleNumber, int &sceneNumber) {
	moduleNumber = _exits[index].moduleNumber;
	sceneNumber = _exits[index].sceneNumber;
}

void Scene::addBlockingRect(int x1, int y1, int x2, int y2) {
	_blockingRects.push_back(Common::Rect(x1, y1, x2, y2));
}

void Scene::removeBlockingRect(int x, int y) {
	for (int i = _blockingRects.size() - 1; i >= 0; i--) {
		if (_blockingRects[i].left == x && _blockingRects[i].top == y) {
			_blockingRects.remove_at(i);
			break;
		}
	}
}

uint16 Scene::checkCollisionWithBounds(const Common::Rect &rect, int direction) {
	uint16 result = 0;
	int x1 = CLIP<int>(rect.left, 0, 319);
	int x2 = CLIP<int>(rect.right, 0, 319);
	if (rect.bottom > 199)
		result = COLLISION(kCollisionBoundsOff, 0);
	else if ((direction == 1 && (rect.top <= _boundsMap[x1] || rect.top <= _boundsMap[x2])) ||
		(direction == 2 && (_boundsMap[x2] >= rect.top || x2 == 319)) ||
		(direction == 4 && (_boundsMap[x1] >= rect.top || x1 == 0)))
		result = COLLISION(kCollisionBounds, 0);
	return result;
}

uint16 Scene::checkCollisionWithExits(const Common::Rect &rect, int direction) {
	for (uint index = 0; index < _exits.size(); index++) {
		if (_exits[index].directionIndex == direction) {
			int exitX1, exitY1, exitX2, exitY2;
			getExitRect(index, exitX1, exitY1, exitX2, exitY2);
			if (((direction == 1 || direction == 3) && rect.left >= exitX1 && rect.right <= exitX2) ||
				(direction == 2 && rect.top >= exitY1 && rect.top <= exitY2 && rect.right >= exitX1) ||
				(direction == 4 && rect.top >= exitY1 && rect.top <= exitY2 && rect.left <= exitX2))
				return COLLISION(kCollisionSceneExit, index);
		}
	}
	return 0;
}

uint16 Scene::checkCollisionWithBlockingRects(Common::Rect &rect, Common::Rect &obstacleRect) {
	for (uint index = 0; index < _blockingRects.size(); index++) {
		obstacleRect = _blockingRects[index];
		if (_blockingRects[index].left != _blockingRects[index].right && _vm->rectCompare(obstacleRect, rect))
			return COLLISION(kCollisionBlocking, index);
	}
	return 0;
}

void Scene::getExitRect(int index, int &x1, int &y1, int &x2, int &y2) {

	SceneExitItem *exitItem = &_exits[index];

	x1 = exitItem->x1;
	x2 = exitItem->x2;

	y1 = _boundsMap[x1];

	if (x1 == x2) {
		y2 = 199;
	} else if (exitItem->directionIndex == 3) {
		y1 = 199;
		y2 = 199;
	} else {
		y2 = _boundsMap[x2];
	}

	if (y1 > y2)
		SWAP(y1, y2);

}

void Scene::findExitRect(int sceneNumber, int moduleNumber, int direction, int &x1, int &y1, int &x2, int &y2, int &outDirection) {

	static const int kInvertedDirections[] = {0, 3, 4, 1, 2};

	outDirection = 1;
	x1 = 160;
	x2 = 160;
	y1 = 190;
	y2 = 190;
	
	for (uint exitIndex = 0; exitIndex < _exits.size(); exitIndex++) {
		SceneExitItem *exitItem = &_exits[exitIndex];
		if (exitItem->sceneNumber == sceneNumber && exitItem->moduleNumber == moduleNumber) {
			outDirection = kInvertedDirections[exitItem->directionIndex];
			if (direction == outDirection) {
				getExitRect(exitIndex, x1, y1, x2, y2);
				break;
			}
		}
	}

}

void Scene::addSceneItem(int itemIndex, int x, int y, int paramType) {
	SceneItem sceneItem;
	sceneItem.itemIndex = itemIndex;
	sceneItem.active = true;
	sceneItem.paramType = paramType;
	sceneItem.x = x;
	sceneItem.y = y;
	_sceneItems.push_back(sceneItem);
}

void Scene::removeSceneItem(int itemIndex) {
	uint index = 0;
	while (index < _sceneItems.size()) {
		if (_sceneItems[index].itemIndex == itemIndex) {
			_sceneItems.remove_at(index);
		} else {
			index++;
		}
	}
}

uint16 Scene::findSceneItemAt(const Common::Rect &rect) {
	for (uint i = 0; i < _sceneItems.size(); i++) {
		if (_sceneItems[i].active) {
			Common::Rect itemRect(_sceneItems[i].x - 8, _sceneItems[i].y - 8, _sceneItems[i].x + 8, _sceneItems[i].y + 8);
			if (_vm->rectCompare(rect, itemRect)) {
				return COLLISION(kCollisionSceneItem, i);
			}
		}
	}
	return 0;
}

SceneItem& Scene::getSceneItem(int itemIndex) {
	return _sceneItems[itemIndex];
}

bool Scene::getSceneItemAt(const Common::Rect &rect, SceneItem &sceneItem) {
	int sceneItemIndex = findSceneItemAt(rect);
	if (sceneItemIndex != 0)
		sceneItem = getSceneItem(sceneItemIndex & 0xFF);
	return sceneItemIndex != 0;
}

int Scene::findBoundsRight(int x, int y) {
	int yp = 0;
	for (uint i = 0; i < _bounds.size(); i++) {
		yp = _bounds[i].y;
		if (_bounds[i].x > x && yp >= y)
			break;
	}
	return (yp >= 199) ? 190 : yp;
}

int Scene::findBoundsLeft(int x, int y) {
	int yp = 0;
	for (int i = _bounds.size() - 1; i >= 0; i--) {
		yp = _bounds[i].y;
		if (_bounds[i].x < x && yp >= y)
			break;
	}
	return (yp >= 199) ? 190 : yp;
}

void Scene::filterWalkDestXY(int &x, int &y, int deltaX, int deltaY) {

	if (x - deltaX < 0)
		x = deltaX + 1;
	if (x + deltaX > 319)
		x = 319 - deltaX - 1;

	Common::Rect tempRect(x - deltaX, y - deltaY, x + deltaX, y);

	if (checkCollisionWithExits(tempRect, 1) ||
		checkCollisionWithExits(tempRect, 2) ||
		checkCollisionWithExits(tempRect, 3) ||
		checkCollisionWithExits(tempRect, 4))
		return;

	if (y - deltaY <= _boundsMap[x - deltaX])
		y = _boundsMap[x - deltaX] + deltaY + 2;

	if (y - deltaY <= _boundsMap[x + deltaX])
		y = _boundsMap[x + deltaX] + deltaY + 2;

	if (y > 199)
		y = 199;

}

void Scene::superFilterWalkDestXY(int &x, int &y, int deltaX, int deltaY) {

	if (x - deltaX < 0)
		x = deltaX + 1;
	if (x + deltaX > 319)
		x = 319 - deltaX - 1;

	if (y - deltaY <= _boundsMap[x - deltaX])
		y = _boundsMap[x - deltaX] + deltaY + 2;

	if (y - deltaY <= _boundsMap[x + deltaX])
		y = _boundsMap[x + deltaX] + deltaY + 2;

	if (y > 199)
		y = 199;

}

void Scene::clear() {
 	_exits.clear();
	_sceneItems.clear();
}

void Scene::initBlockingRectsFromAnimation(AnimationResource *animation) {
	Common::Rect blockingRect;
	_blockingRects.clear();
	AnimationElement *element = animation->getElement(0);
	for (uint i = 0; i < element->getCommandCount(); ++i) {
		AnimationCommand *cmd = element->getCommand(i);
		if (cmd->getBlockingRect(animation, blockingRect)) {
			addBlockingRect(blockingRect.left * 2, blockingRect.top, blockingRect.right * 2, blockingRect.bottom);
		}
	}
}

void Scene::drawExits() {
	for (uint32 i = 0; i < _exits.size(); i++) {
		if (_exits[i].directionIndex == 3) {
			_vm->_screen->fillRect(_exits[i].x1, 198, _exits[i].x2, 199, 120);
			_vm->_screen->hLine(_exits[i].x1 + 1, 199, _exits[i].x2 - 2, 127);
		}
	}
}

void Scene::sync(Common::Serializer &s) {
	syncExits(s);
	syncBounds(s);
	syncSceneItems(s);
	syncBlockingRects(s);
	syncBoundsMap(s);
}

void Scene::syncExits(Common::Serializer &s) {
	uint count = s.isSaving() ? _exits.size() : 0;
	s.syncAsByte(count);
	if (s.isLoading()) {
		_exits.clear();
		_exits.resize(count);
	}
	for (Common::Array<SceneExitItem>::iterator iter = _exits.begin(); iter != _exits.end(); ++iter) {
		SceneExitItem &sceneExit = *iter;
		sceneExit.sync(s);
	}
}

void Scene::syncSceneItems(Common::Serializer &s) {
	uint count = s.isSaving() ? _sceneItems.size() : 0;
	s.syncAsByte(count);
	if (s.isLoading()) {
		_sceneItems.clear();
		_sceneItems.resize(count);
	}
	for (Common::Array<SceneItem>::iterator iter = _sceneItems.begin(); iter != _sceneItems.end(); ++iter) {
		SceneItem &sceneItem = *iter;
		sceneItem.sync(s);
	}
}

void Scene::syncBounds(Common::Serializer &s) {
	uint count = s.isSaving() ? _bounds.size() : 0;
	s.syncAsByte(count);
	if (s.isLoading()) {
		_bounds.clear();
		_bounds.resize(count);
	}
	for (PointArray::iterator iter = _bounds.begin(); iter != _bounds.end(); ++iter) {
		Common::Point &bound = *iter;
		_vm->syncAsPoint(s, bound);
	}
}

void Scene::syncBlockingRects(Common::Serializer &s) {
	uint count = s.isSaving() ? _blockingRects.size() : 0;
	s.syncAsByte(count);
	if (s.isLoading()) {
		_blockingRects.clear();
		_blockingRects.resize(count);
	}
	for (Common::Array<Common::Rect>::iterator iter = _blockingRects.begin(); iter != _blockingRects.end(); ++iter) {
		Common::Rect &blockingRect = *iter;
		_vm->syncAsRect(s, blockingRect);
	}
}

void Scene::syncBoundsMap(Common::Serializer &s) {
	s.syncBytes(_boundsMap, 320);
}

void Scene::initBoundsMap() {

	int x1, y1, x2, y2, errorX, errorY = 0;
	byte *boundsMapPtr = _boundsMap;

	for (uint i = 0; i < _bounds.size() - 1; i++) {
		x1 = _bounds[i].x;
		y1 = _bounds[i].y;
		x2 = _bounds[i + 1].x;
		y2 = _bounds[i + 1].y;
		x2 -= x1;
		if (x2 != 0) {
			y2 -= y1;
			errorY = y1;
			y1 = 1;
			if (y2 < 0) {
				y2 = -y2;
				y1 = -y1;
			}
			if (x2 > y2) {
				errorX = x2 >> 1;
				for (int j = 0; j < x2; j++) {
					*boundsMapPtr++ = errorY;
					errorX += y2;
					if (errorX >= x2) {
						errorX -= x2;
						errorY += y1;
					}
				}
			} else {
				errorX = y2 >> 1;
				for (int j = 0; j < y2; j++) {
					errorY += y1;
					errorX += x2;
					if (errorX >= y2) {
						errorX -= y2;
						*boundsMapPtr++ = errorY;
					}
				}
			}
		}
	}

	*boundsMapPtr++ = errorY;

}

} // End of namespace Comet
