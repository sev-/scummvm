#include "comet/comet.h"

#include "comet/animation.h"
#include "comet/scene.h"

namespace Comet {

/* SceneObject */

void CometEngine::sceneObjectInit(int itemIndex, int marcheIndex) {

	SceneObject *sceneObject = &_sceneObjects[itemIndex];
	
	memset(sceneObject, 0, sizeof(SceneObject));

	sceneObject->directionAdd = 4;
	sceneObject->direction = 1;
	sceneObjectSetDirectionAdd(sceneObject, 0);
	sceneObjectSetAnimNumber(sceneObject, 0);
	sceneObject->marcheIndex = marcheIndex;
	
	sceneObject->deltaX = 4;
	sceneObject->deltaY = 2;
	sceneObject->flag2 = 0;
	sceneObject->clipX1 = 0;
	sceneObject->clipY1 = 0;
	sceneObject->clipX2 = 319;
	sceneObject->clipY2 = 199;
	sceneObject->visible = true;
	sceneObject->textX = -1;
	sceneObject->textY = -1;
	sceneObject->animSubIndex2 = -1;

	if (itemIndex == 0)
		sceneObject->textColor = 21;
	else
		sceneObject->life = 10;

}

void CometEngine::sceneObjectSetDirection(SceneObject *sceneObject, int direction) {
	if (sceneObject->direction != direction && direction != 0 && sceneObject->directionChanged != 2) {
		sceneObject->direction = direction;
		sceneObject->directionChanged = 1;
	}
}

void CometEngine::sceneObjectSetDirectionAdd(SceneObject *sceneObject, int directionAdd) {
	if (sceneObject->directionAdd != directionAdd && sceneObject->directionChanged != 2) {
		sceneObject->directionAdd = directionAdd;
		sceneObject->directionChanged = 1;
	}
}

void CometEngine::sceneObjectSetAnimNumber(SceneObject *sceneObject, int index) {
	if (sceneObject->marcheIndex != -1) {
		sceneObject->animFrameCount = _marcheItems[sceneObject->marcheIndex].anim->_anims[index]->frames.size();
	} else {
		sceneObject->animFrameCount = 0;
	}
	sceneObject->animFrameIndex = 0;
	sceneObject->animIndex = index;
	sceneObject->animSubIndex2 = -1;
}

void CometEngine::sceneObjectStopWalking(SceneObject *sceneObject) {
	sceneObject->walkStatus = 0;
	sceneObjectSetDirectionAdd(sceneObject, 0);
}

void CometEngine::sceneObjectCalcDirection(SceneObject *sceneObject) {

	int deltaX, deltaY, direction, walkFlag;
	
	walkFlag = sceneObject->walkStatus & 3;
	deltaX = sceneObject->walkDestX - sceneObject->x;
	deltaY = sceneObject->walkDestY - sceneObject->y;
	direction = sceneObject->direction;

	if (walkFlag == 1 || (walkFlag == 0 && (ABS(deltaX) > ABS(deltaY)))) {
		if (deltaX > 0)
			direction = 2;
		else if (deltaX < 0)
			direction = 4;
	} else if (walkFlag == 2 || (walkFlag == 0 && (ABS(deltaY) > ABS(deltaX)))) {
		if (deltaY > 0)
			direction = 3;
		else if (deltaY < 0)
			direction = 1;
	}

	sceneObjectSetDirection(sceneObject, direction);

}

void CometEngine::sceneObjectGetNextWalkDestXY(SceneObject *sceneObject, int &x, int &y) {
	switch (sceneObject->direction) {
	case 1:
		if (sceneObject->walkDestY > y)
			y = sceneObject->walkDestY;
		break;
	case 2:
		if (sceneObject->walkDestX < x)
			x = sceneObject->walkDestX;
		break;
	case 3:
		if (sceneObject->walkDestY < y)
			y = sceneObject->walkDestY;
		break;
	case 4:
		if (sceneObject->walkDestX > x)
			x = sceneObject->walkDestX;
		break;
	}
}

void CometEngine::sceneObjectSetPosition(int index, int x, int y) {
	SceneObject *sceneObject = getSceneObject(index);
	sceneObject->x = x;
	sceneObject->y = y;
	sceneObject->value6 = 0;
	sceneObject->walkStatus = 0;
}

void CometEngine::sceneObjectUpdateLife(SceneObject *sceneObject, int flag) {
	sceneObject->life = MAX(0, sceneObject->life - flag);
}

void CometEngine::sceneObjectSaveWalkDestXY(SceneObject *sceneObject) {
	if (((sceneObject->walkStatus & 3) != 0) && ((sceneObject->walkStatus & 4) == 0)) {
		sceneObject->savedWalkDestX = sceneObject->walkDestX;
		sceneObject->savedWalkDestY = sceneObject->walkDestY;
		sceneObject->walkStatus |= 4;
	}
}

bool CometEngine::sceneObjectStartWalking(int objectIndex, int x, int y) {

	debug(4, "CometEngine::sceneObjectStartWalking() objectIndex = %d; (%d, %d)", objectIndex, x, y);

	SceneObject *sceneObject = getSceneObject(objectIndex);
	
	if (sceneObject->collisionType != 8) {
		_scene->filterWalkDestXY(x, y, sceneObject->deltaX, sceneObject->deltaY);
	}
		
	int compareFlags = comparePointXY(sceneObject->x, sceneObject->y, x, y);

	debug(4, "CometEngine::sceneObjectStartWalking() compareFlags = %d", compareFlags);

	// No need to walk since we're already there
	if (compareFlags == 3)
		return false;

	sceneObjectSaveWalkDestXY(sceneObject);

	sceneObject->walkDestX = x;
	sceneObject->walkDestY = y;

	if (compareFlags == 0) {
		if (sceneObject->walkStatus & 3) {
			sceneObject->walkStatus ^= 3;
		} else {
			sceneObject->walkStatus |= 1;//DEBUG (random(2) + 1);
		}
	} else {
		sceneObject->walkStatus &= ~3;
		sceneObject->walkStatus |= (compareFlags ^ 3);
	}

 	sceneObjectSetDirectionAdd(sceneObject, 4);
 	sceneObjectCalcDirection(sceneObject);

	return true;

}

SceneObject *CometEngine::getSceneObject(int index) {
	return &_sceneObjects[index];
}

/* SceneObjects */

void CometEngine::sceneObjectsResetFlags() {
	for (int i = 1; i < 11; i++) {
		_sceneObjects[i].life = 0;
	}
}

} // End of namespace Comet
