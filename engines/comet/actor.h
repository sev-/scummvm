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

#ifndef COMET_ACTOR_H
#define COMET_ACTOR_H

#include "common/endian.h"
#include "common/rect.h"

namespace Comet {

class CometEngine;

class Actor {
public:
	Actor(CometEngine *vm, int itemIndex);
	~Actor();
	void init(int16 animationSlot);
	void setDirection(int direction);
	void forceDirection(int direction);
	void setDirectionAdd(int directionAdd);
	void setAnimationIndex(int index);
	void stopWalking();
	void setPosition(int x, int y);
	void updateLife(int lifeDelta);
	bool startWalking(int x, int y);
	bool startWalkToX(int x);
	bool startWalkToY(int y);
	bool startWalkToXY(int x, int y);
	void updateMovement();
	void updateAnimation();
	void updateActorAnimation();
	void loadSprite(int animationSlot);
	void unloadSprite();
	Common::Rect getRect();
	Common::Rect getProximityRect(int areaWidth, int areaHeight);
	void enableCollisions();
	void disableCollisions();
	void setupActorAnim(int animIndex, int animFrameIndex);
	void setClipX(int x1, int x2);
	void setClipY(int y1, int y2);
	void setTextXY(int x, int y);
	void setVisible(bool visible);
	void calcSightRect(Common::Rect &rect, int delta1, int delta2);
	void draw();
	void updateHealth();
protected:
	void saveWalkDestXY();
	void updateDirection();
	void getNextWalkDestXY(int &x, int &y);
	bool updatePosition(Common::Rect &obstacleRect);
	void updateWalking(bool skipCollision, Common::Rect &obstacleRect);
	void handleCollision(Common::Rect &obstacleRect);
	void moveAroundObstacle(Common::Rect &obstacleRect);
	void moveAroundSceneBounds();
	void updatePortraitAnimation();
	uint16 checkCollision(Common::Rect &testRect, Common::Rect &obstacleRect);
	uint16 updateCollision(uint16 collisionType);
public://protected:
	CometEngine *_vm;
	int _itemIndex;
	int16 _x, _y;
	int16 _directionAdd, _direction;
	byte _flag2;
	int16 _interpolationStep;
	int16 _animFrameCount;
	int16 _deltaX, _deltaY;
	uint16 _collisionType;
	int16 _collisionIndex;
	byte _value6;
	byte _value7;
	uint16 _walkStatus;
	int16 _walkDestX, _walkDestY;
	int16 _savedWalkDestX, _savedWalkDestY;
	int16 _clipX1, _clipY1, _clipX2, _clipY2;
	bool _visible;
public: // TODO Make these protected
	int16 _animationSlot;
	byte _textColor;
	int16 _life;
	int16 _status;
	int16 _animIndex;
	int16 _animPlayFrameIndex;
	int16 _animFrameIndex;
	int16 _textX, _textY;
};

const uint kActorsCount = 11;

class Actors {
public:
	Actors(CometEngine *vm);
	~Actors();
	void updateAnimations();
	void updateMovement();
	void enqueueActorsForDrawing();
	bool isAnimationSlotUsed(int16 animationSlot);
	void clearAnimationSlotByIndex(int16 animationSlot);
	uint16 checkCollisionWithActors(uint selfActorIndex, Common::Rect &testRect, Common::Rect &obstacleRect);
	void resetHealth();
	uint getCount() const { return kActorsCount; } // TODO Remove later, only used in saveload
	Actor *getActor(uint index);
protected:
	CometEngine *_vm;
	Actor *_actors[kActorsCount];
};

}

#endif
