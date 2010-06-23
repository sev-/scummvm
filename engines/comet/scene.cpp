#include "comet/comet.h"
#include "comet/music.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/dialog.h"

namespace Comet {

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

void Scene::clearExits() {
	_exits.clear();
}

void Scene::getExitLink(int index, int &moduleNumber, int &sceneNumber) {
	moduleNumber = _exits[index].moduleNumber;
	sceneNumber = _exits[index].sceneNumber;
}

void Scene::addBlockingRect(int x1, int y1, int x2, int y2) {
	_blockingRects.push_back(Common::Rect(x1 * 2, y1, x2 * 2, y2));
}

void Scene::removeBlockingRect(int x, int y) {
	for (int i = _blockingRects.size() - 1; i >= 0; i--) {
		if (_blockingRects[i].left == x && _blockingRects[i].top == y) {
			_blockingRects.remove_at(i);
			break;
		}
	}
}

int Scene::checkCollisionWithBounds(const Common::Rect &rect, int direction) {

	int x, y, x2, y2, y3, y4, result;

	result = 0;

	x = CLIP<int>(rect.left, 0, 319);
	x2 = CLIP<int>(rect.right, 0, 319);

	y = rect.top;
	y2 = rect.bottom;

	y3 = _boundsMap[x];
	y4 = _boundsMap[x2];

	switch (direction) {
	case 1:
		if (y <= y3 || y <= y4)
			result = COLLISION(kCollisionBounds, 0);
		break;
	case 2:
		if (y4 >= y || x2 == 319)
			result = COLLISION(kCollisionBounds, 0);
		break;
	case 4:
		if (y3 >= y || x == 0)
			result = COLLISION(kCollisionBounds, 0);
		break;
	default:
		// Nothing
		break;
	}

	if (y2 > 199)
		result = COLLISION(kCollisionBoundsOff, 0);

	return result;

}

int Scene::checkCollisionWithExits(const Common::Rect &rect, int direction) {

	int x, y, x2, x3, y3, x4, y4;

	x = rect.left;
	y = rect.top;
	x2 = rect.right;

	for (uint32 index = 0; index < _exits.size(); index++) {
		bool flag = false;
		if (_exits[index].directionIndex == direction) {
			getExitRect(index, x3, y3, x4, y4);
			if (direction == 1 || direction == 3) {
				flag = (x >= x3) && (x2 <= x4);
			} else if (direction == 2) {
				flag = (y >= y3) && (y <= y4) && (x2 >= x3);
			} else if (direction == 4) {
				flag = (y >= y3) && (y <= y4) && (x <= x4);
			}
			if (flag)
				return COLLISION(kCollisionSceneExit, index);
		}
	}

	return 0;
}

int Scene::checkCollisionWithBlockingRects(Common::Rect &rect, Common::Rect &obstacleRect) {

	for (uint32 index = 0; index < _blockingRects.size(); index++) {
		obstacleRect = _blockingRects[index];
		if (_blockingRects[index].left != _blockingRects[index].right) {
			if (_vm->rectCompare(obstacleRect, rect)) {
				return COLLISION(kCollisionBlocking, index);
			}
		}
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

	static const int directionArray[] = {0, 3, 4, 1, 2};

	outDirection = 1;
	x1 = 160;
	x2 = 160;
	y1 = 190;
	y2 = 190;
	
	for (uint exitIndex = 0; exitIndex < _exits.size(); exitIndex++) {
		SceneExitItem *exitItem = &_exits[exitIndex];
		if (exitItem->sceneNumber == sceneNumber && exitItem->moduleNumber == moduleNumber) {
			outDirection = directionArray[exitItem->directionIndex];
			if (direction == outDirection) {
				getExitRect(exitIndex, x1, y1, x2, y2);
				break;
			}
		}
	}

}

int Scene::findBoundsRight(int x, int y) {
	int yp = 0;
	for (uint32 i = 0; i < _bounds.size(); i++) {
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

void Scene::initBoundsMap() {

	int x1, y1, x2, y2, errorX, errorY = 0;
	byte *boundsMapPtr = _boundsMap;

	for (uint32 i = 0; i < _bounds.size() - 1; i++) {
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
