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

#ifndef COMET_SCENE_H
#define COMET_SCENE_H

#include "common/func.h"

#include "comet/comet.h"
#include "comet/resource.h"

namespace Comet {

struct SceneExitItem {
	int directionIndex;
	int moduleNumber;
	int sceneNumber;
	int x1, x2;
};

struct SceneItem {
	int itemIndex;
	bool active;
	int paramType;
	int x, y;
};

class Scene {
public:
	Scene(CometEngine *vm);
	~Scene();

	//void initSceneBackground();
	//void initStaticObjectRects();
	//void loadSceneBackground();
	//void loadStaticObjects();
	//void drawSceneForeground();

	void initBounds(byte *data);

	void initExits(byte *data);
	void clearExits();
	void getExitLink(int index, int &chapterNumber, int &sceneNumber);

	void addBlockingRect(int x1, int y1, int x2, int y2);
	void removeBlockingRect(int x, int y);

	uint16 checkCollisionWithBounds(const Common::Rect &collisionRect, int direction);
	uint16 checkCollisionWithExits(const Common::Rect &collisionRect, int direction);
	uint16 checkCollisionWithBlockingRects(Common::Rect &collisionRect, Common::Rect &obstacleRect);

	void getExitRect(int index, int &x1, int &y1, int &x2, int &y2);
	void findExitRect(int sceneNumber, int moduleNumber, int direction, int &x1, int &y1, int &x2, int &y2, int &outDirection);

	void addSceneItem(int itemIndex, int x, int y, int paramType);
	void removeSceneItem(int itemIndex);
	uint16 findSceneItemAt(const Common::Rect &rect);
	SceneItem& getSceneItem(int itemIndex);

	int findBoundsRight(int x, int y);
	int findBoundsLeft(int x, int y);
	void filterWalkDestXY(int &x, int &y, int deltaX, int deltaY);
	void superFilterWalkDestXY(int &x, int &y, int deltaX, int deltaY);

//protected:
public: // while still in progress
	CometEngine *_vm;

	Common::Array<Common::Rect> _blockingRects;
	Common::Array<SceneExitItem> _exits;
	Common::Array<SceneItem> _sceneItems;
	AnimationResource *_sceneObjectsSprite;
	PointArray _bounds;
	byte _boundsMap[320];
	
	void initBoundsMap();
	
};

} // End of namespace Comet

#endif
