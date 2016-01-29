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

#include "comet/actor.h"
#include "comet/animationmgr.h"
#include "comet/comet.h"
#include "comet/console.h"
#include "comet/resource.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/talktext.h"
#include "common/util.h"

namespace Comet {

// Actor

Actor::Actor(CometEngine *vm, int itemIndex)
	: _vm(vm), _itemIndex(itemIndex),
	_x(0), _y(0), _directionAdd(0), _direction(0), _flag2(0), _interpolationStep(0),
	_animFrameCount(0), _deltaX(0), _deltaY(0), _collisionType(0), _collisionIndex(0),
	_value6(0), _value7(0), _walkStatus(0), _walkDestX(0), _walkDestY(0),
	_savedWalkDestX(0), _savedWalkDestY(0), _clipX1(0), _clipY1(0), _clipX2(0), _clipY2(0),
	_visible(false), _animationSlot(0), _textColor(0), _life(0), _status(0), _animIndex(0),
	_animPlayFrameIndex(0), _animFrameIndex(0), _textX(0), _textY(0) {
}

Actor::~Actor() {
}

void Actor::init(int16 animationSlot) {
	_x = 0;
	_y = 0;
	_status = 0;
	_flag2 = 0;
	_animIndex = 0;
	_animFrameIndex = 0;
	_interpolationStep = 0;
	_animFrameCount = 0;
	_animPlayFrameIndex = 0;
	_collisionType = 0;
	_collisionIndex = 0;
	_value6 = 0;
	_life = 0;
	_value7 = 0;
	_walkStatus = 0;
	_walkDestX = 0;
	_walkDestY = 0;
	_savedWalkDestX = 0;
	_savedWalkDestY = 0;
	_textColor = 0;
	_directionAdd = 4;
	_direction = 1;
	setDirectionAdd(0);
	setAnimationIndex(0);
	_animationSlot = animationSlot;
	_deltaX = 4;
	_deltaY = 2;
	_flag2 = 0;
	_clipX1 = 0;
	_clipY1 = 0;
	_clipX2 = 319;
	_clipY2 = 199;
	_visible = true;
	_textX = -1;
	_textY = -1;
	_animPlayFrameIndex = -1;
	if (_itemIndex == 0)
		_textColor = 21;
	else
		_life = 10;
}

void Actor::setDirection(int direction) {
	if (_direction != direction && direction != 0 && _status != 2) {
		_direction = direction;
		_status = 1;
	}
}

void Actor::forceDirection(int direction) {
	_status = 0;
	setDirection(direction);
}

void Actor::setDirectionAdd(int directionAdd) {
	if (_directionAdd != directionAdd && _status != 2) {
		_directionAdd = directionAdd;
		_status = 1;
	}
}

void Actor::setAnimationIndex(int index) {
	if (_animationSlot != -1) {
		_animFrameCount = _vm->_animationMan->getAnimation(_animationSlot)->getFrameList(index)->getFrameCount();
	} else {
		_animFrameCount = 0;
	}
	_animFrameIndex = 0;
	_animIndex = index;
	_animPlayFrameIndex = -1;
}

void Actor::stopWalking() {
	_walkStatus = 0;
	setDirectionAdd(0);
}

void Actor::updateDirection() {
	int walkFlag = _walkStatus & 3;
	int deltaX = _walkDestX - _x;
	int deltaY = _walkDestY - _y;
	int newDirection = _direction;
	if (walkFlag == 1 || (walkFlag == 0 && (ABS(deltaX) > ABS(deltaY)))) {
		if (deltaX > 0)
			newDirection = 2;
		else if (deltaX < 0)
			newDirection = 4;
	} else if (walkFlag == 2 || (walkFlag == 0 && (ABS(deltaY) > ABS(deltaX)))) {
		if (deltaY > 0)
			newDirection = 3;
		else if (deltaY < 0)
			newDirection = 1;
	}
	setDirection(newDirection);
}

void Actor::getNextWalkDestXY(int &x, int &y) {
	if (_direction == 1 && _walkDestY > y)
		y = _walkDestY;
	else if (_direction == 2 && _walkDestX < x)
		x = _walkDestX;
	else if (_direction == 3 && _walkDestY < y)
		y = _walkDestY;
	else if (_direction == 4 && _walkDestX > x)
		x = _walkDestX;
}

void Actor::setPosition(int x, int y) {
	_x = x;
	_y = y;
	_value6 = 0;
	_walkStatus = 0;
}

void Actor::updateLife(int lifeDelta) {
	_life = MAX(0, _life - lifeDelta);
}

void Actor::saveWalkDestXY() {
	if (((_walkStatus & 3) != 0) && ((_walkStatus & 4) == 0)) {
		_savedWalkDestX = _walkDestX;
		_savedWalkDestY = _walkDestY;
		_walkStatus |= 4;
	}
}

bool Actor::startWalking(int x, int y) {
	debug(4, "Actor::actorStartWalking() objectIndex = %d; (%d, %d)", _itemIndex, x, y);
	if (_collisionType != kCollisionDisabled)
		_vm->_scene->filterWalkDestXY(x, y, _deltaX, _deltaY);
	int compareFlags = _vm->comparePointXY(_x, _y, x, y);
	// No need to walk since we're already there
	if (compareFlags == 3)
		return false;
	saveWalkDestXY();
	_walkDestX = x;
	_walkDestY = y;
	if (compareFlags == 0) {
		if (_walkStatus & 3) {
			_walkStatus ^= 3;
		} else {
			_walkStatus |= (_vm->randomValue(2) + 1);
		}
	} else {
		_walkStatus &= ~3;
		_walkStatus |= (compareFlags ^ 3);
	}
	setDirectionAdd(4);
	updateDirection();
	return true;
}

bool Actor::startWalkToX(int x) {
	_status = 0;
	if (startWalking(x, _y)) {
		_walkStatus |= 0x08;
		return true;
	}
	return false;
}

bool Actor::startWalkToY(int y) {
	_status = 0;
	if (startWalking(_x, y)) {
		_walkStatus |= 0x10;
		return true;
	}
	return false;
}

bool Actor::startWalkToXY(int x, int y) {
	_status = 0;
	_walkStatus = 0;
	return startWalking(x, y);
}

void Actor::moveAroundObstacle(Common::Rect &obstacleRect) {
	int x = _x;
	int y = _y;
	switch (_direction) {
	case 1:
	case 3:
		if (_vm->randomValue(2) == 0) {
			x = obstacleRect.left - (_deltaX + 2);
		} else {
			x = obstacleRect.right + (_deltaX + 2);
		}
		break;
	case 2:
	case 4:
		if (_vm->randomValue(2) == 0) {
			y = obstacleRect.top - 2;
		} else {
			y = obstacleRect.bottom + (_deltaY + 2);
		}
		break;
	}
	startWalking(x, y);
}

void Actor::handleCollision(Common::Rect &obstacleRect) {
	debug(4, "Actor::handleActorCollision() actorIndex = %d", _itemIndex);
	debug(4, "Actor::handleActorCollision() _collisionType = %d", _collisionType);
	if (_collisionType == kCollisionBounds || _collisionType == kCollisionBoundsOff) {
		moveAroundSceneBounds();
	} else if (_collisionType == kCollisionActor && _value6 == 6 && _collisionIndex == 0) {
		// NOTE Never called in Comet CD since value6 is never 6 there, maybe used in Eternam so it's kept for now
		_value6 = 0;
		stopWalking();
		if (_flag2 == 1)
			updateLife(_life);
	} else {
		moveAroundObstacle(obstacleRect);
	}
}

void Actor::moveAroundSceneBounds() {
	int x = _x;
	int y = _walkDestY;
	switch (_direction) {
	case 1:
	case 3:
		x = _walkDestX;
		break;
	case 2:
		if (_walkDestY <= _y) {
			y = _vm->_scene->findBoundsRight(x + _deltaX, y - _deltaY) + _deltaY + 2;
		}
		break;
	case 4:
		if (_walkDestY <= _y) {
			y = _vm->_scene->findBoundsLeft(x - _deltaX, y - _deltaY) + _deltaY + 1;
		}
		break;
	}
	startWalking(x, y);
}

void Actor::updateWalking(bool skipCollision, Common::Rect &obstacleRect) {
	if (!skipCollision)
		handleCollision(obstacleRect);
	int comp = _vm->comparePointXY(_x, _y, _walkDestX, _walkDestY);
	if (debugRectangles) {
		_vm->_screen->fillRect(_walkDestX - 6, _walkDestY - 6, _walkDestX + 6, _walkDestY + 6, 220);
		_vm->_screen->drawDottedLine(_x, _y, _walkDestX, _walkDestY, 100);
	}
	if (comp == 3 || ((_walkStatus & 8) && (comp == 1)) || ((_walkStatus & 0x10) && (comp == 2))) {
		if (_walkStatus & 4) {
			startWalking(_savedWalkDestX, _savedWalkDestY);
			_walkStatus &= ~4;
		} else
			stopWalking();
	} else if ((_walkStatus & 3) == comp) {
		_walkStatus ^= 3;
		updateDirection();
	}
}

bool Actor::updatePosition(Common::Rect &obstacleRect) {
	if (_directionAdd != 4)
		return false;

	int newX = _x;
	int newY = _y;
	AnimationResource *anim = _vm->_animationMan->getAnimation(_animationSlot);
	AnimationFrame *frame = anim->getFrameListFrame(_animIndex, _animFrameIndex);

	frame->accumulateDrawOffset(newX, newY);
 	
	if (_walkStatus & 3)
		getNextWalkDestXY(newX, newY);

	if (_collisionType != kCollisionDisabled) {
		Common::Rect testRect(newX - _deltaX, newY - _deltaY, newX + _deltaX, newY);
		uint16 collisionType = checkCollision(testRect, obstacleRect);
		if (collisionType != 0) {
			collisionType = updateCollision(collisionType);
			if (collisionType == 0)
				return false;
		} else
			_collisionType = kCollisionNone;
	}

	_x = newX;
	_y = newY;
	return true;
}

void Actor::updateMovement() {
	if (_life != 0) {
		Common::Rect obstacleRect;
		bool skipCollision = updatePosition(obstacleRect);
		if (_walkStatus & 3)
			updateWalking(skipCollision, obstacleRect);
	}
}

void Actor::updatePortraitAnimation() {
	if (_animPlayFrameIndex == -1) {
		// FIXME: This check is not in the original, find out why it's needed here...
		if (_animationSlot < 0)
			return;
		AnimationFrame *frame = _vm->getAnimationFrame(_animationSlot, _animIndex, _animFrameIndex);
		uint16 maxInterpolationStep = frame->getMaxInterpolationStep();
		uint16 gfxMode = frame->getDrawMode();
		if (gfxMode == 1) {
			if (_interpolationStep >= MAX<int>(1, maxInterpolationStep) - 1) {
				_interpolationStep = 0;
				++_animFrameIndex;
			}
		} else {
			++_animFrameIndex;
		}
		if (_animFrameIndex >= _animFrameCount) {
			_animFrameIndex = 0;
			if (_animIndex < 4) {
				int portraitTalkAnimNumber = _vm->_talkText->getPortraitTalkAnimNumber();
				setAnimationIndex(portraitTalkAnimNumber);
			}
		}
	} else {
		_interpolationStep = 0;
	}
}

void Actor::updateAnimation() {
	if (_status == 1) {
		_status = 0;
		setAnimationIndex(_direction + _directionAdd - 1);
	} else {
		if (_animPlayFrameIndex == -1) {
			// NOTE: See note below, but here we bail out.
			if (_animIndex >= (int)_vm->_animationMan->getAnimation(_animationSlot)->getFrameListCount())
				return;
			// NOTE: After watching the ritual the players' frame number is out-of-bounds.
			//	I don't know yet why this happens, but setting it to 0 at least avoids a crash.
			if (_animFrameIndex >= (int)_vm->_animationMan->getAnimation(_animationSlot)->getFrameList(_animIndex)->getFrameCount())
				_animFrameIndex = 0;
			AnimationFrame *frame = _vm->getAnimationFrame(_animationSlot, _animIndex, _animFrameIndex);
			uint16 maxInterpolationStep = frame->getMaxInterpolationStep();
			uint16 gfxMode = frame->getDrawMode();
			if (gfxMode == 1) {
				if (_interpolationStep >= MAX<int>(1, maxInterpolationStep) - 1) {
					_interpolationStep = 0;
					_animFrameIndex++;
				}
			} else {
				_animFrameIndex++;
			}
			if (_animFrameIndex >= _animFrameCount)
				_animFrameIndex = 0;
		} else {
			_interpolationStep = 0;
		}
	}
}

void Actor::updateActorAnimation() {
	if (_itemIndex == kActorPortrait) {
		updatePortraitAnimation();
	} else if (_life != 0) {
		updateAnimation();
	}
}

void Actor::loadSprite(int animationSlot) {
	unloadSprite();
	_animationSlot = animationSlot;
}

void Actor::unloadSprite() {
	if (_animationSlot != -1) {
		// TODO Refactor; Try not to access _vm->_actors
		AnimationSlot *animationSlot = _vm->_animationMan->getAnimationSlot(_animationSlot);
		if (animationSlot->anim && animationSlot->animationType == 0 && !_vm->_actors->isAnimationSlotUsed(_animationSlot)) {
			_vm->_actors->clearAnimationSlotByIndex(_animationSlot);
			delete animationSlot->anim;
			animationSlot->anim = NULL;
		}
	}
}

Common::Rect Actor::getRect() {
	return Common::Rect(_x - _deltaX, _y - _deltaY, _x + _deltaX, _y);
}

Common::Rect Actor::getProximityRect(int areaWidth, int areaHeight) {
	return Common::Rect(_x - areaWidth / 2, _y - areaHeight / 2, _x + areaWidth / 2, _y + areaHeight / 2);
}

void Actor::enableCollisions() {
	_collisionType = kCollisionNone;
}

void Actor::disableCollisions() {
	_collisionType = kCollisionDisabled;
}

void Actor::setupActorAnim(int animIndex, int animFrameIndex) {
	_animIndex = animIndex;
	_animFrameIndex = animFrameIndex;
	_animPlayFrameIndex = animFrameIndex;
	_status = 2;
}

void Actor::setClipX(int x1, int x2) {
	_clipX1 = x1;
	_clipX2 = x2;
}

void Actor::setClipY(int y1, int y2) {
	_clipY1 = y1;
	_clipY2 = y2;
}

void Actor::setTextXY(int x, int y) {
	_textX = x;
	_textY = y;
}

void Actor::setVisible(bool visible) {
	_visible = visible;
}

Common::Rect Actor::calcSightRect(int delta1, int delta2) {
	int x1 = _x - _deltaX - 8;
	int y1 = _y - _deltaY - 8;
	int x2 = _x + _deltaX + 8;
	int y2 = _y + 8;
	switch (_direction) {
	case 1:
		y1 -= delta2;
		y2 -= 20;
		x1 -= delta1;
		x2 += delta1;
		break;
	case 2:
		x2 += delta2;
		x1 += 25;
		y1 -= 32;
		break;
	case 3:
		y2 += delta2;
		y1 += 20;
		break;
	case 4:
		x1 -= delta2;
		x2 -= 25;
		y1 -= 32;
		break;
	}
	Common::Rect rect;
	rect.left = MAX(x1, 0);
	rect.top = MAX(y1, 0);
	rect.right = MIN(x2, 319);
	rect.bottom = MIN(y2, 199);
	return rect;
}

void Actor::draw() {
	int x = _x;
	int y = _y;

	AnimationResource *animation = _vm->_animationMan->getAnimation(_animationSlot);

	// NOTE: Yet another workaround for a crash (see updateActorAnimation).
	if (_animIndex >= (int)animation->getFrameListCount()) {
		_animIndex = 0;
		_animFrameIndex = 0;
		_animFrameCount = animation->getFrameList(0)->getFrameCount();
	}

	AnimationFrameList *frameList = animation->getFrameList(_animIndex);

	_vm->_screen->setClipRect(_clipX1, _clipY1, _clipX2 + 1, _clipY2 + 1);

	if (_status == 2) {
		_interpolationStep = _vm->_screen->drawAnimation(animation, frameList, _animFrameIndex, _interpolationStep,
			x, y, _animFrameCount);
	} else {
		if (_vm->_itemInSight && _itemIndex == 0 && _direction == 1)
			_vm->drawLineOfSight();
		_vm->_screen->drawAnimationElement(animation, frameList->getFrame(_animFrameIndex)->getElementIndex(), x, y);
	}

	_vm->_screen->setClipRect(0, 0, 320, 200);

	if (debugShowActorNum) {
		// DEBUG: Show actor number
		Common::String temp;
		temp = Common::String::format("%d", _itemIndex);
		_vm->_screen->drawText(CLIP(x, 16, 320 - 16), CLIP(y, 16, 200 - 16), (const byte*)temp.c_str());
	}
}

void Actor::updateHealth() {
	if (_life > 0 && _life < 99)
		++_life;
}

void Actor::sync(Common::Serializer &s) {
	s.syncAsUint16LE(_x);
	s.syncAsUint16LE(_y);
	s.syncAsUint16LE(_directionAdd);
	s.syncAsUint16LE(_status);
	s.syncAsUint16LE(_direction);
	s.syncAsByte(_flag2);
	s.syncAsUint16LE(_animationSlot);
	s.syncAsUint16LE(_animIndex);
	s.syncAsUint16LE(_animFrameIndex);
	s.syncAsUint16LE(_interpolationStep);
	s.syncAsUint16LE(_animFrameCount);
	s.syncAsUint16LE(_animPlayFrameIndex);
	s.syncAsUint16LE(_deltaX);
	s.syncAsUint16LE(_deltaY);
	s.syncAsUint16LE(_collisionType);
	s.syncAsUint16LE(_collisionIndex);
	s.syncAsByte(_value6);
	s.syncAsUint16LE(_life);
	s.syncAsByte(_textColor);
	s.syncAsByte(_value7);
	s.syncAsUint16LE(_textX);
	s.syncAsUint16LE(_textY);
	s.syncAsUint16LE(_walkStatus);
	s.syncAsUint16LE(_walkDestX);
	s.syncAsUint16LE(_walkDestY);
	s.syncAsUint16LE(_savedWalkDestX);
	s.syncAsUint16LE(_savedWalkDestY);
	s.syncAsUint16LE(_clipX1);
	s.syncAsUint16LE(_clipY1);
	s.syncAsUint16LE(_clipX2);
	s.syncAsUint16LE(_clipY2);
	s.syncAsByte(_visible);
}

uint16 Actor::checkCollision(Common::Rect &testRect, Common::Rect &obstacleRect) {
	uint16 collisionType = 0;
	collisionType = _vm->_scene->checkCollisionWithBounds(testRect, _direction);
	if (collisionType != 0) {
		uint16 sceneExitCollision = _vm->_scene->checkCollisionWithExits(testRect, _direction);
		if (sceneExitCollision != 0)
			collisionType = sceneExitCollision;
	} else {
		collisionType = _vm->_scene->checkCollisionWithBlockingRects(testRect, obstacleRect);
		if (collisionType == 0)
			collisionType = _vm->_actors->checkCollisionWithActors(_itemIndex, testRect, obstacleRect);
	}
	return collisionType;
}

uint16 Actor::updateCollision(uint16 collisionType) {
	int result = 0;
	_collisionType = COLLISION_TYPE(collisionType);
	_collisionIndex = COLLISION_INDEX(collisionType);
	if (_itemIndex == 0 && _collisionType == kCollisionSceneExit)
		result = _vm->handleSceneExitCollision(_collisionIndex);
	if (result == 0) {
		setDirectionAdd(0);
		updateAnimation();
	}
	return result;
}

// Actors

Actors::Actors(CometEngine *vm)
	: _vm(vm) {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex)
		_actors[actorIndex] = new Actor(_vm, (int)actorIndex);
}

Actors::~Actors() {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex)
		delete _actors[actorIndex];
}

void Actors::updateAnimations() {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex)
		_actors[actorIndex]->updateActorAnimation();
}

void Actors::updateMovement() {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex)
		_actors[actorIndex]->updateMovement();
}

void Actors::enqueueActorsForDrawing() {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex)
		if (_actors[actorIndex]->_visible && _actors[actorIndex]->_life > 0)
			_vm->enqueueActorForDrawing(_actors[actorIndex]->_y, (int)actorIndex);
}

bool Actors::isAnimationSlotUsed(int16 animationSlot) {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex)
		if (_actors[actorIndex]->_animationSlot == animationSlot && _actors[actorIndex]->_life != 0)
			return true;
	return false;
}

void Actors::clearAnimationSlotByIndex(int16 animationSlot) {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex) {
		if (_actors[actorIndex]->_animationSlot == animationSlot) {
			_actors[actorIndex]->_animationSlot = -1;
			_actors[actorIndex]->_life = 0;
		}
	}
}

uint16 Actors::checkCollisionWithActors(uint selfActorIndex, Common::Rect &testRect, Common::Rect &obstacleRect) {
	for (uint actorIndex = 0; actorIndex < ARRAYSIZE(_actors); ++actorIndex) {
		Actor *actor = _actors[actorIndex];
		if (actorIndex != selfActorIndex && actor->_life != 0 && actor->_collisionType != kCollisionDisabled) {
			obstacleRect = actor->getRect();
			if (_vm->rectCompare(testRect, obstacleRect))
				return COLLISION(kCollisionActor, actorIndex);
		}
	}
	return 0;
}

void Actors::resetHealth() {
	// NOTE Don't reset the main actor's life (index 0)
	for (uint actorIndex = 1; actorIndex < ARRAYSIZE(_actors); ++actorIndex)
		_actors[actorIndex]->_life = 0;
}

Actor *Actors::getActor(uint index) {
	assert(_actors[index]);
	return _actors[index];
}

AnimationResource *CometEngine::getGlobalAnimationResource(int16 animationType) {
	switch (animationType) {
	case 1:
		return _heroSprite;
	case 2:
		return _sceneDecorationSprite;
	//case 3: // NOTE unused/returns NULL var (maybe used in Eternam?)
	default:
		warning("CometEngine::getGlobalAnimationResource() Invalid animationType (%d)", animationType);
		return NULL;
	}
}

bool CometEngine::isActorNearActor(int actorIndex1, int actorIndex2, int x, int y) {
	Common::Rect actorRect1 = _actors->getActor(actorIndex1)->getRect();
	Common::Rect actorRect2 = _actors->getActor(actorIndex2)->getProximityRect(x, y);
	return rectCompare(actorRect1, actorRect2);
}

bool CometEngine::isPlayerInZone(int x1, int y1, int x2, int y2) {
	Common::Rect zoneRect(x1, y1, x2, y2);
	Common::Rect playerRect = _actors->getActor(0)->getRect();
	return rectCompare(zoneRect, playerRect);
}

} // End of namespace Comet
