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

void Scene::initPoints(byte *data) {
	byte count = *data++;
	_bounds.clear();
	while (count--) {
		int x = (*data++) * 2;
		int y = *data++;
		_bounds.push_back(Common::Point(x, y));
	}
	initSceneBoundsMap();
}

void Scene::initSceneExits(byte *data) {
	byte count = *data++;
	_sceneExits.clear();
	while (count--) {
		SceneExitItem sceneExitItem;
		sceneExitItem.directionIndex = data[0];
		sceneExitItem.chapterNumber = data[1];
		sceneExitItem.sceneNumber = data[2];
		sceneExitItem.x1 = data[3] * 2;
		sceneExitItem.x2 = data[4] * 2;
		data += 5;
		_sceneExits.push_back(sceneExitItem);
	}
}

void Scene::getSceneExitLink(int index, int &chapterNumber, int &sceneNumber) {
	chapterNumber = _sceneExits[index].chapterNumber;
	sceneNumber = _sceneExits[index].sceneNumber;
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
			result = 0x100;
		break;
	case 2:
		if (y4 >= y || x2 == 319)
			result = 0x100;
		break;
	case 4:
		if (y3 >= y || x == 0)
			result = 0x100;
		break;
	default:
		// Nothing
		break;
	}

	if (y2 > 199)
		result = 0x200;

	return result;

}

int Scene::checkCollisionWithExits(const Common::Rect &rect, int direction) {

	int x, y, x2, x3, y3, x4, y4;

	x = rect.left;
	y = rect.top;
	x2 = rect.right;

	for (uint32 index = 0; index < _sceneExits.size(); index++) {
		bool flag = false;
		if (_sceneExits[index].directionIndex == direction) {
			getSceneExitRect(index, x3, y3, x4, y4);
			if (direction == 1 || direction == 3) {
				flag = (x >= x3) && (x2 <= x4);
			} else if (direction == 2) {
				flag = (y >= y3) && (y <= y4) && (x2 >= x3);
			} else if (direction == 4) {
				flag = (y >= y3) && (y <= y4) && (x <= x4);
			}
			if (flag)
				return 0x400 | index;
		}
	}

	return 0;
}

int Scene::checkCollisionWithBlockingRects(Common::Rect &rect, Common::Rect &obstacleRect) {

	for (uint32 index = 0; index < _blockingRects.size(); index++) {
		obstacleRect = _blockingRects[index];
		if (_blockingRects[index].left != _blockingRects[index].right) {
			if (_vm->rectCompare(obstacleRect, rect)) {
				return 0x300 | index;
			}
		}
	}

	return 0;

}

void Scene::getSceneExitRect(int index, int &x1, int &y1, int &x2, int &y2) {

	SceneExitItem *sceneExitItem = &_sceneExits[index];

	x1 = sceneExitItem->x1;
	x2 = sceneExitItem->x2;

	y1 = _boundsMap[x1];

	if (x1 == x2) {
		y2 = 199;
	} else if (sceneExitItem->directionIndex == 3) {
		y1 = 199;
		y2 = 199;
	} else {
		y2 = _boundsMap[x2];
	}

	if (y1 > y2)
		SWAP(y1, y2);

}

int Scene::Points_getY_sub_8419(int x, int y) {
	int yp = 0;
	for (uint32 i = 0; i < _bounds.size(); i++) {
		yp = _bounds[i].y;
		if (_bounds[i].x > x && yp >= y)
			break;
	}
	if (yp >= 199)
		return 190;
	else
		return yp;
}

int Scene::Points_getY_sub_8477(int x, int y) {
	int yp = 0;
	for (int i = _bounds.size() - 1; i >= 0; i--) {
		yp = _bounds[i].y;
		if (_bounds[i].x < x && yp >= y)
			break;
	}
	if (yp >= 199)
		return 190;
	else
		return yp;
}

void Scene::rect_sub_CC94(int &x, int &y, int deltaX, int deltaY) {

	debug(1, "rect_sub_CC94() 1) x = %d; y = %d; deltaX = %d; deltaY = %d", x, y, deltaX, deltaY);

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

	debug(1, "rect_sub_CC94() 1b) x1 = %d; x2 = %d", _boundsMap[x - deltaX], _boundsMap[x + deltaX]);

	if (y - deltaY <= _boundsMap[x - deltaX])
		y = _boundsMap[x - deltaX] + deltaY + 2;

	if (y - deltaY <= _boundsMap[x + deltaX])
		y = _boundsMap[x + deltaX] + deltaY + 2;

	if (y > 199)
		y = 199;

	debug(1, "rect_sub_CC94() 2) x = %d; y = %d; deltaX = %d; deltaY = %d", x, y, deltaX, deltaY);

}

void Scene::initSceneBoundsMap() {

	int x1, y1, x2, y2, errorX, errorY = 0;
	byte *xb = _boundsMap;

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
					*xb++ = errorY;
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
						*xb++ = errorY;
					}
				}
			}
		}
	}

	*xb++ = errorY;

}


} // End of namespace Comet
