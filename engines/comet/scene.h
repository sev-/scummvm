/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * $URL: https://www.switchlink.se/svn/comet/scene.h $
 * $Id: comet.h 46 2009-06-07 14:17:12Z johndoe $
 *
 */

#ifndef COMET_SCENE_H
#define COMET_SCENE_H

#include "common/func.h"

#include "comet/comet.h"
#include "comet/animation.h"

namespace Comet {

#if 0
struct SceneExitItem {
	int directionIndex;
	int chapterNumber;
	int sceneNumber;
	//int unused;
	int x1, x2;
};
#endif

class Scene {
public:
	Scene(CometEngine *vm);
	~Scene();

/*
	void initSceneBackground();
	void initStaticObjectRects();
	void loadSceneBackground();
	void loadStaticObjects();
	void drawSceneForeground();
*/

	void initPoints(byte *data);

	void initSceneExits(byte *data);
	void getSceneExitLink(int index, int &chapterNumber, int &sceneNumber);

	void addBlockingRect(int x1, int y1, int x2, int y2);
	void removeBlockingRect(int x, int y);

	int checkCollisionWithBounds(const Common::Rect &collisionRect, int direction);
	int checkCollisionWithExits(const Common::Rect &collisionRect, int direction);
	int checkCollisionWithBlockingRects(Common::Rect &collisionRect, Common::Rect &obstacleRect);

	void getSceneExitRect(int index, int &x1, int &y1, int &x2, int &y2);

	int Points_getY_sub_8419(int x, int y);
	int Points_getY_sub_8477(int x, int y);
	void rect_sub_CC94(int &x, int &y, int deltaX, int deltaY);

//protected:
public: // while still in progress
	CometEngine *_vm;

	Common::Array<Common::Rect> _blockingRects;
	Common::Array<SceneExitItem> _sceneExits;
	Animation *_sceneObjectsSprite;
	PointArray _bounds;
	byte _boundsMap[320];
	
	void initSceneBoundsMap();
	
};

} // End of namespace Comet

#endif
