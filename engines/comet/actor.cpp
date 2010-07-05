#include "comet/comet.h"

#include "comet/animation.h"
#include "comet/animationmgr.h"
#include "comet/scene.h"
#include "comet/screen.h"

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
	actor->animPlayFrameIndex = -1;

	if (itemIndex == 0)
		actor->textColor = 21;
	else
		actor->life = 10;

}

void CometEngine::actorSetDirection(Actor *actor, int direction) {
	if (actor->direction != direction && direction != 0 && actor->status != 2) {
		actor->direction = direction;
		actor->status = 1;
	}
}

void CometEngine::actorSetDirectionAdd(Actor *actor, int directionAdd) {
	if (actor->directionAdd != directionAdd && actor->status != 2) {
		actor->directionAdd = directionAdd;
		actor->status = 1;
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
	actor->animPlayFrameIndex = -1;
	
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
	if (actor->direction == 1 && actor->walkDestY > y)
		y = actor->walkDestY;
	else if (actor->direction == 2 && actor->walkDestX < x)	
		x = actor->walkDestX;
	else if (actor->direction == 3 && actor->walkDestY < y)	
		y = actor->walkDestY;
	else if (actor->direction == 4 && actor->walkDestX > x)	
		x = actor->walkDestX;
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

void CometEngine::actorMoveAroundObstacle(int actorIndex, Actor *actor, Common::Rect &obstacleRect) {

	int x = actor->x;
	int y = actor->y;

	debug(4, "CometEngine::actorMoveAroundObstacle() 1) actorIndex = %d; x = %d; y = %d", actorIndex, x, y);

	switch (actor->direction) {
	case 1:
	case 3:
		if (random(2) == 0) {
			x = obstacleRect.left - (actor->deltaX + 2);
		} else {
			x = obstacleRect.right + (actor->deltaX + 2);
		}
		break;
	case 2:
	case 4:
		if (random(2) == 0) {
			y = obstacleRect.top - 2;
		} else {
			y = obstacleRect.bottom + (actor->deltaY + 2);
		}
		break;
	}

	debug(4, "CometEngine::actorMoveAroundObstacle() 2) actorIndex = %d; x = %d; y = %d", actorIndex, x, y);

	actorStartWalking(actorIndex, x, y);

}

void CometEngine::handleActorCollision(int actorIndex, Actor *actor, Common::Rect &obstacleRect) {

	debug(4, "CometEngine::handleActorCollision() actorIndex = %d", actorIndex);
	debug(4, "CometEngine::handleActorCollision() actor->collisionType = %d", actor->collisionType);

	if (actor->collisionType == kCollisionBounds || actor->collisionType == kCollisionBoundsOff) {
		// TODO
		moveActorAroundBounds(actorIndex, actor);
	} else if (actor->collisionType == kCollisionActor && actor->value6 == 6 && actor->collisionIndex == 0) {
		// TODO
		//debug(4, "CometEngine::handleActorCollision()");
		actor->value6 = 0;
		actorStopWalking(actor);
		if (actor->flag2 == 1) {
			actorUpdateLife(actor, actor->life);
		}
	} else {
		actorMoveAroundObstacle(actorIndex, actor, obstacleRect);
	}

}

void CometEngine::actorUpdateWalking(Actor *actor, int actorIndex, bool flag, Common::Rect &obstacleRect) {

	if (!flag)
		handleActorCollision(actorIndex, actor, obstacleRect);

	int comp = comparePointXY(actor->x, actor->y, actor->walkDestX, actor->walkDestY);
	
	if (_debugRectangles) {
		_screen->fillRect(actor->walkDestX - 6, actor->walkDestY - 6, actor->walkDestX + 6, actor->walkDestY + 6, 220);
		_screen->drawDottedLine(actor->x, actor->y, actor->walkDestX, actor->walkDestY, 100);
	}

	if (comp == 3 || ((actor->walkStatus & 8) && (comp == 1)) || ((actor->walkStatus & 0x10) && (comp == 2))) {
		if (actor->walkStatus & 4) {
			actorStartWalking(actorIndex, actor->savedWalkDestX, actor->savedWalkDestY);
			actor->walkStatus &= ~4;
		} else {
			actorStopWalking(actor);
		}
	} else if ((actor->walkStatus & 3) == comp) {
		actor->walkStatus ^= 3;
		actorCalcDirection(actor);
	}

}

bool CometEngine::updateActorPosition(int actorIndex, Common::Rect &obstacleRect) {

	Actor *actor = getActor(actorIndex);

	if (actor->directionAdd != 4)
		return false;

	int newX = actor->x;
	int newY = actor->y;

	Animation *anim = _animationMan->getAnimation(actor->animationSlot);
	AnimationFrame *frame = anim->_anims[actor->animIndex]->frames[actor->animFrameIndex];

 	int16 xAdd = frame->xOffs;
 	int16 yAdd = frame->yOffs;

 	// TODO: SceneObject_sub_8243(actor->direction, &xAdd, &yAdd); (but has no effect in Comet CD)

 	newX += xAdd;
 	newY += yAdd;
 	
 	if (actor->walkStatus & 3) {
		actorGetNextWalkDestXY(actor, newX, newY);
	}

	if (actor->collisionType != kCollisionDisabled) {
		uint16 collisionType = checkCollision(actorIndex, newX, newY, actor->deltaX, actor->deltaY, actor->direction, obstacleRect);
		if (collisionType != 0) {
			collisionType = updateCollision(actor, actorIndex, collisionType);
			if (collisionType == 0)
				return false;
		} else {
			actor->collisionType = kCollisionNone;
		}
	}

	actor->x = newX;
	actor->y = newY;

	return true;

}

void CometEngine::actorTalk(int actorIndex, int talkTextIndex, int color) {

	_talkActorIndex = actorIndex;
	_talkTextIndex = talkTextIndex;
	
	if (_talkieMode == 0 || _talkieMode == 1) {
		setText(_textReader->getString(_narFileIndex + 3, _talkTextIndex));
	}

	if (_talkieMode == 2 || _talkieMode == 1) {
		playVoice(_talkTextIndex);
	}

	_textActive = true;
	_talkTextColor = color;

}

void CometEngine::actorTalkWithAnim(int actorIndex, int talkTextIndex, int animNumber) {

	Actor *actor = getActor(actorIndex);
	
	actorTalk(actorIndex, talkTextIndex, actor->textColor);

	if (animNumber != 255) {
		_animIndex = actor->animIndex;
		_animPlayFrameIndex = actor->animPlayFrameIndex;
		_animFrameIndex = actor->animFrameIndex;
		actorSetAnimNumber(actor, animNumber);
		actor->status = 2;
	} else {
		_animIndex = -1;
	}

}

void CometEngine::actorTalkPortrait(int actorIndex, int talkTextIndex, int animNumber, int fileIndex) {
	int16 animationSlot = _animationMan->getAnimationResource(_animationType, fileIndex);
	actorInit(10, animationSlot);
	if (actorIndex != -1) {
		_actors[10].textX = 0;
		_actors[10].textY = 160;
		_actors[10].textColor = getActor(actorIndex)->textColor;
	}
	_animationType = 0;
	actorSetPosition(10, 0, 199);
	actorTalkWithAnim(10, talkTextIndex, animNumber);
	_animIndex = actorIndex;
	_screen->enableTransitionEffect();
}


bool CometEngine::isActorNearActor(int actorIndex1, int actorIndex2, int x, int y) {

	Actor *actor1 = getActor(actorIndex1);
	Actor *actor2 = getActor(actorIndex2);

	Common::Rect actorRect1(
		actor1->x - actor1->deltaX, actor1->y - actor1->deltaY,
		actor1->x + actor1->deltaX, actor1->y);

	Common::Rect actorRect2(
		actor2->x - x / 2, actor2->y - y / 2,
		actor2->x + x / 2, actor2->y + y / 2);

	return rectCompare(actorRect1, actorRect2);

}

bool CometEngine::isPlayerInZone(int x1, int y1, int x2, int y2) {

	Actor *mainActor = getActor(0);

	Common::Rect zoneRect(x1, y1, x2, y2);
	Common::Rect playerRect(
		mainActor->x - mainActor->deltaX, mainActor->y - mainActor->deltaY,
		mainActor->x + mainActor->deltaX, mainActor->y);
	
	return rectCompare(zoneRect, playerRect);

}

void CometEngine::moveActorAroundBounds(int index, Actor *actor) {

	int x = actor->x;
	int y = actor->walkDestY;

	debug(1, "moveActorAroundBounds(%d); 1) x = %d; y = %d", index, x, y);
	
	switch (actor->direction) {
	case 1:
	case 3:
		x = actor->walkDestX;
		break;
	case 2:
		if (actor->walkDestY <= actor->y) {
			y = _scene->findBoundsRight(x + actor->deltaX, y - actor->deltaY) +
				actor->deltaY + 2;
		}
		break;
	case 4:
		if (actor->walkDestY <= actor->y) {
			y = _scene->findBoundsLeft(x - actor->deltaX, y - actor->deltaY) +
				actor->deltaY + 1;
		}
		break;
	}
	
	debug(1, "moveActorAroundBounds(%d); 2) x = %d; y = %d", index, x, y);

	actorStartWalking(index, x, y);

}

void CometEngine::updatePortraitAnimation(Actor *actor) {

	if (actor->animPlayFrameIndex == -1) {

		// FIXME: This check is not in the original, find out why it's needed here...
		if (actor->animationSlot == -1)
			return;

		AnimationFrame *frame = _animationMan->getAnimation(actor->animationSlot)->_anims[actor->animIndex]->frames[actor->animFrameIndex];

		uint16 value = frame->flags & 0x3FFF;
		uint16 gfxMode = frame->flags >> 14;

		if (gfxMode == 1) {
			if (value < 1)
				value = 1;
			if (actor->interpolationStep >= value - 1) {
				actor->interpolationStep = 0;
				actor->animFrameIndex++;
			}
		} else {
			actor->animFrameIndex++;
		}

		if (actor->animFrameIndex >= actor->animFrameCount) {
			actor->animFrameIndex = 0;
			if (actor->animIndex < 4) {
				if (_portraitTalkCounter == 0) {
					if (_talkieMode == 0) {
						_portraitTalkAnimNumber = random(4);
						if (_portraitTalkAnimNumber == 0)
							_portraitTalkCounter = 1;
					} else {
						_portraitTalkAnimNumber = random(3);
						if (!_narOkFlag)
					  		_portraitTalkAnimNumber = 0;
					}
				} else {
					_portraitTalkCounter++;
					if (((_talkieMode == 1 || _talkieMode == 2) && _portraitTalkCounter == 1) || _portraitTalkCounter == 10)
						_portraitTalkCounter = 0;
				}
				actorSetAnimNumber(actor, _portraitTalkAnimNumber);
			}
		}

	} else {
		actor->interpolationStep = 0;
	}

}

void CometEngine::updateActorAnimation(Actor *actor) {

	if (actor->status == 1) {
		actor->status = 0;
		actorSetAnimNumber(actor, actor->direction + actor->directionAdd - 1);
	} else {

		if (actor->animPlayFrameIndex == -1) {

			/* NOTE: See note below, but here we bail out. */
			if (actor->animIndex >= (int)_animationMan->getAnimation(actor->animationSlot)->_anims.size())
				return;

			/* NOTE: After watching the ritual the players' frame number is out-of-bounds.
				I don't know yet why this happens, but setting it to 0 at least avoids a crash. */
			if (actor->animFrameIndex >= (int)_animationMan->getAnimation(actor->animationSlot)->_anims[actor->animIndex]->frames.size())
				actor->animFrameIndex = 0;

			AnimationFrame *frame = _animationMan->getAnimation(actor->animationSlot)->_anims[actor->animIndex]->frames[actor->animFrameIndex];

			uint16 maxInterpolationStep = frame->flags & 0x3FFF;
			uint16 gfxMode = frame->flags >> 14;

			if (gfxMode == 1) {
				if (maxInterpolationStep < 1)
					maxInterpolationStep = 1;
				if (actor->interpolationStep >= maxInterpolationStep - 1) {
					actor->interpolationStep = 0;
					actor->animFrameIndex++;
				}
			} else {
				actor->animFrameIndex++;
			}
			
			if (actor->animFrameIndex >= actor->animFrameCount)
				actor->animFrameIndex = 0;

		} else {
			actor->interpolationStep = 0;
		}
		
	}
}

void CometEngine::unloadActorSprite(Actor *actor) {
	if (actor->animationSlot != -1) {
		AnimationSlot *animationSlot = _animationMan->getAnimationSlot(actor->animationSlot);
		if (animationSlot->anim && animationSlot->animationType == 0 && !isAnimationSlotUsed(actor->animationSlot)) {
			clearAnimationSlotByIndex(actor->animationSlot);
			delete animationSlot->anim;
			animationSlot->anim = NULL;
		}
	}
}

bool CometEngine::isAnimationSlotUsed(int16 animationSlot) {
	for (int i = 0; i < 11; i++) {
		if (_actors[i].animationSlot == animationSlot && _actors[i].life != 0)
			return true;
	}
	return false;
}

void CometEngine::clearAnimationSlotByIndex(int16 animationSlot) {
	for (int i = 1; i < 11; i++) {
		if (_actors[i].animationSlot == animationSlot) {
			_actors[i].animationSlot = -1;
			_actors[i].life = 0;
		}
	}
}

Animation *CometEngine::getGlobalAnimationResource(int16 animationType) {
	switch (animationType) {
	case 1:
		return _heroSprite;
	case 2:
		return _sceneDecorationSprite;
	//case 3: //TODO??? returns NULL var (maybe used in Eternam?)
	default:
		warning("CometEngine::getGlobalAnimationResource() Invalid animationType (%d)", animationType);
		return NULL;
	}
}

/* SceneObjects */

void CometEngine::resetActorsLife() {
	for (int i = 1; i < 11; i++) {
		_actors[i].life = 0;
	}
}

} // End of namespace Comet
