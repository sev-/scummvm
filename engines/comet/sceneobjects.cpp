#include "comet/comet.h"

#include "comet/animation.h"
#include "comet/animationmgr.h"
#include "comet/scene.h"

namespace Comet {

/* Actor */

void CometEngine::actorInit(int itemIndex, int16 animationSlot) {

	Actor *actor = &_actors[itemIndex];
	
	memset(actor, 0, sizeof(Actor));

	actor->directionAdd = 4;
	actor->direction = 1;
	actorSetDirectionAdd(actor, 0);
	actorSetAnimNumber(actor, 0);
	actor->animationSlot = animationSlot;
	
	actor->deltaX = 4;
	actor->deltaY = 2;
	actor->flag2 = 0;
	actor->clipX1 = 0;
	actor->clipY1 = 0;
	actor->clipX2 = 319;
	actor->clipY2 = 199;
	actor->visible = true;
	actor->textX = -1;
	actor->textY = -1;
	actor->animSubIndex2 = -1;

	if (itemIndex == 0)
		actor->textColor = 21;
	else
		actor->life = 10;

}

void CometEngine::actorSetDirection(Actor *actor, int direction) {
	if (actor->direction != direction && direction != 0 && actor->directionChanged != 2) {
		actor->direction = direction;
		actor->directionChanged = 1;
	}
}

void CometEngine::actorSetDirectionAdd(Actor *actor, int directionAdd) {
	if (actor->directionAdd != directionAdd && actor->directionChanged != 2) {
		actor->directionAdd = directionAdd;
		actor->directionChanged = 1;
	}
}

void CometEngine::actorSetAnimNumber(Actor *actor, int index) {
	if (actor->animationSlot != -1) {
		actor->animFrameCount = _animationMan->getAnimation(actor->animationSlot)->_anims[index]->frames.size();
	} else {
		actor->animFrameCount = 0;
	}
	actor->animFrameIndex = 0;
	actor->animIndex = index;
	actor->animSubIndex2 = -1;
	
	debug(5, "actorSetAnimNumber() animIndex = %d; animFrameIndex = %d; animFrameCount = %d",
		actor->animIndex, actor->animFrameIndex, actor->animFrameCount);
	
}

void CometEngine::actorStopWalking(Actor *actor) {
	actor->walkStatus = 0;
	actorSetDirectionAdd(actor, 0);
}

void CometEngine::actorCalcDirection(Actor *actor) {

	int deltaX, deltaY, direction, walkFlag;
	
	walkFlag = actor->walkStatus & 3;
	deltaX = actor->walkDestX - actor->x;
	deltaY = actor->walkDestY - actor->y;
	direction = actor->direction;

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

	actorSetDirection(actor, direction);

}

void CometEngine::actorGetNextWalkDestXY(Actor *actor, int &x, int &y) {
	switch (actor->direction) {
	case 1:
		if (actor->walkDestY > y)
			y = actor->walkDestY;
		break;
	case 2:
		if (actor->walkDestX < x)
			x = actor->walkDestX;
		break;
	case 3:
		if (actor->walkDestY < y)
			y = actor->walkDestY;
		break;
	case 4:
		if (actor->walkDestX > x)
			x = actor->walkDestX;
		break;
	}
}

void CometEngine::actorSetPosition(int index, int x, int y) {
	Actor *actor = getActor(index);
	actor->x = x;
	actor->y = y;
	actor->value6 = 0;
	actor->walkStatus = 0;
}

void CometEngine::actorUpdateLife(Actor *actor, int flag) {
	actor->life = MAX(0, actor->life - flag);
}

void CometEngine::actorSaveWalkDestXY(Actor *actor) {
	if (((actor->walkStatus & 3) != 0) && ((actor->walkStatus & 4) == 0)) {
		actor->savedWalkDestX = actor->walkDestX;
		actor->savedWalkDestY = actor->walkDestY;
		actor->walkStatus |= 4;
	}
}

bool CometEngine::actorStartWalking(int objectIndex, int x, int y) {

	debug(4, "CometEngine::actorStartWalking() objectIndex = %d; (%d, %d)", objectIndex, x, y);

	Actor *actor = getActor(objectIndex);
	
	if (actor->collisionType != kCollisionDisabled) {
		_scene->filterWalkDestXY(x, y, actor->deltaX, actor->deltaY);
	}
		
	int compareFlags = comparePointXY(actor->x, actor->y, x, y);

	debug(4, "CometEngine::actorStartWalking() compareFlags = %d", compareFlags);

	// No need to walk since we're already there
	if (compareFlags == 3)
		return false;

	actorSaveWalkDestXY(actor);

	actor->walkDestX = x;
	actor->walkDestY = y;

	if (compareFlags == 0) {
		if (actor->walkStatus & 3) {
			actor->walkStatus ^= 3;
		} else {
			actor->walkStatus |= (random(2) + 1);
		}
	} else {
		actor->walkStatus &= ~3;
		actor->walkStatus |= (compareFlags ^ 3);
	}

 	actorSetDirectionAdd(actor, 4);
 	actorCalcDirection(actor);

	return true;

}

Actor *CometEngine::getActor(int index) {
	return &_actors[index];
}

/* SceneObjects */

void CometEngine::resetActorsLife() {
	for (int i = 1; i < 11; i++) {
		_actors[i].life = 0;
	}
}

} // End of namespace Comet
