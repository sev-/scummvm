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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/debugger/debug_scene.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/scene/grid.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

DebugScene::DebugScene(TwinEEngine *engine) : _engine(engine) {}

void DebugScene::drawClip(const Common::Rect &rect) {
	if (!showingClips) {
		return;
	}
	_engine->_menu->drawBox(rect);
}

void DebugScene::drawBoundingBoxProjectPoints(Vec3 *pPoint3d, Vec3 *pPoint3dProjected) {
	_engine->_renderer->projectPositionOnScreen(pPoint3d->x, pPoint3d->y, pPoint3d->z);

	pPoint3dProjected->x = _engine->_renderer->projPos.x;
	pPoint3dProjected->y = _engine->_renderer->projPos.y;
	pPoint3dProjected->z = _engine->_renderer->projPos.z;

	if (_engine->_redraw->renderRect.left > _engine->_renderer->projPos.x) {
		_engine->_redraw->renderRect.left = _engine->_renderer->projPos.x;
	}

	if (_engine->_redraw->renderRect.right < _engine->_renderer->projPos.x) {
		_engine->_redraw->renderRect.right = _engine->_renderer->projPos.x;
	}

	if (_engine->_redraw->renderRect.top > _engine->_renderer->projPos.y) {
		_engine->_redraw->renderRect.top = _engine->_renderer->projPos.y;
	}

	if (_engine->_redraw->renderRect.bottom < _engine->_renderer->projPos.y) {
		_engine->_redraw->renderRect.bottom = _engine->_renderer->projPos.y;
	}
}

int32 DebugScene::checkZoneType(int32 type) const {
	switch (type) {
	case ZoneType::kCube:
		if (typeZones & 0x01)
			return 1;
		break;
	case ZoneType::kCamera:
		if (typeZones & 0x02)
			return 1;
		break;
	case ZoneType::kSceneric:
		if (typeZones & 0x04)
			return 1;
		break;
	case ZoneType::kGrid:
		if (typeZones & 0x08)
			return 1;
		break;
	case ZoneType::kObject:
		if (typeZones & 0x10)
			return 1;
		break;
	case ZoneType::kText:
		if (typeZones & 0x20)
			return 1;
		break;
	case ZoneType::kLadder:
		if (typeZones & 0x40)
			return 1;
		break;
	default:
		return 1;
	}

	return 0;
}

DebugScene::ScenePositionsProjected DebugScene::calculateBoxPositions(const Vec3 &bottomLeft, const Vec3 &topRight) {
	ScenePositionsProjected positions;
	// compute the points in 3D
	positions.frontBottomLeftPoint.x = bottomLeft.x - _engine->_grid->camera.x;
	positions.frontBottomLeftPoint.y = bottomLeft.y - _engine->_grid->camera.y;
	positions.frontBottomLeftPoint.z = topRight.z - _engine->_grid->camera.z;

	positions.frontBottomRightPoint.x = topRight.x - _engine->_grid->camera.x;
	positions.frontBottomRightPoint.y = bottomLeft.y - _engine->_grid->camera.y;
	positions.frontBottomRightPoint.z = topRight.z - _engine->_grid->camera.z;

	positions.frontTopLeftPoint.x = bottomLeft.x - _engine->_grid->camera.x;
	positions.frontTopLeftPoint.y = topRight.y - _engine->_grid->camera.y;
	positions.frontTopLeftPoint.z = topRight.z - _engine->_grid->camera.z;

	positions.frontTopRightPoint.x = topRight.x - _engine->_grid->camera.x;
	positions.frontTopRightPoint.y = topRight.y - _engine->_grid->camera.y;
	positions.frontTopRightPoint.z = topRight.z - _engine->_grid->camera.z;

	positions.backBottomLeftPoint.x = bottomLeft.x - _engine->_grid->camera.x;
	positions.backBottomLeftPoint.y = bottomLeft.y - _engine->_grid->camera.y;
	positions.backBottomLeftPoint.z = bottomLeft.z - _engine->_grid->camera.z;

	positions.backBottomRightPoint.x = topRight.x - _engine->_grid->camera.x;
	positions.backBottomRightPoint.y = bottomLeft.y - _engine->_grid->camera.y;
	positions.backBottomRightPoint.z = bottomLeft.z - _engine->_grid->camera.z;

	positions.backTopLeftPoint.x = bottomLeft.x - _engine->_grid->camera.x;
	positions.backTopLeftPoint.y = topRight.y - _engine->_grid->camera.y;
	positions.backTopLeftPoint.z = bottomLeft.z - _engine->_grid->camera.z;

	positions.backTopRightPoint.x = topRight.x - _engine->_grid->camera.x;
	positions.backTopRightPoint.y = topRight.y - _engine->_grid->camera.y;
	positions.backTopRightPoint.z = bottomLeft.z - _engine->_grid->camera.z;

	// project all points

	drawBoundingBoxProjectPoints(&positions.frontBottomLeftPoint, &positions.frontBottomLeftPoint2D);
	drawBoundingBoxProjectPoints(&positions.frontBottomRightPoint, &positions.frontBottomRightPoint2D);
	drawBoundingBoxProjectPoints(&positions.frontTopLeftPoint, &positions.frontTopLeftPoint2D);
	drawBoundingBoxProjectPoints(&positions.frontTopRightPoint, &positions.frontTopRightPoint2D);
	drawBoundingBoxProjectPoints(&positions.backBottomLeftPoint, &positions.backBottomLeftPoint2D);
	drawBoundingBoxProjectPoints(&positions.backBottomRightPoint, &positions.backBottomRightPoint2D);
	drawBoundingBoxProjectPoints(&positions.backTopLeftPoint, &positions.backTopLeftPoint2D);
	drawBoundingBoxProjectPoints(&positions.backTopRightPoint, &positions.backTopRightPoint2D);

	return positions;
}

bool DebugScene::drawBox(const ScenePositionsProjected &positions, uint8 color) {
	bool state = false;
	// draw front part
	state |= _engine->_interface->drawLine(positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, color);

	// draw top part
	state |= _engine->_interface->drawLine(positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontTopLeftPoint2D.x, positions.frontTopLeftPoint2D.y, color);

	// draw back part
	state |= _engine->_interface->drawLine(positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopLeftPoint2D.x, positions.backTopLeftPoint2D.y, positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backTopRightPoint2D.x, positions.backTopRightPoint2D.y, positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, color);

	// draw bottom part
	state |= _engine->_interface->drawLine(positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backBottomLeftPoint2D.x, positions.backBottomLeftPoint2D.y, positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.backBottomRightPoint2D.x, positions.backBottomRightPoint2D.y, positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, color);
	state |= _engine->_interface->drawLine(positions.frontBottomRightPoint2D.x, positions.frontBottomRightPoint2D.y, positions.frontBottomLeftPoint2D.x, positions.frontBottomLeftPoint2D.y, color);

	return state;
}

// TODO: redrawing doesn't work properly yet for moving actors
bool DebugScene::displayActors() {
	bool state = false;
	for (int i = 0; i < _engine->_scene->sceneNumActors; i++) {
		const ActorStruct *actorPtr = _engine->_scene->getActor(i);
		const Vec3 &pos = actorPtr->pos;
		const ZVBox &bbox = actorPtr->boudingBox;
		const Vec3 mins(bbox.x.bottomLeft, bbox.y.bottomLeft, bbox.z.bottomLeft);
		const Vec3 maxs(bbox.x.topRight, bbox.y.topRight, bbox.z.topRight);
		const ScenePositionsProjected &positions = calculateBoxPositions(pos + mins, pos + maxs);
		if (!drawBox(positions, COLOR_WHITE)) {
			continue;
		}
		const int boxwidth = 150;
		const int lineHeight = 14;
		const int boxheight = 2 * lineHeight;
		const Common::Rect filledRect(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontTopRightPoint2D.x + boxwidth, positions.frontTopRightPoint2D.y + boxheight);
		_engine->_interface->drawFilledRect(filledRect, COLOR_WHITE);
		_engine->_menu->drawBox(filledRect);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, Common::String::format("Actor: %i", i), true, false, boxwidth);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y + lineHeight, Common::String::format("pos: %i:%i:%i", positions.frontTopRightPoint.x, positions.frontTopRightPoint.y, positions.frontTopRightPoint.z), true, false, boxwidth);
		state = true;
	}
	return state;
}

// TODO: implement the rendering points of all tracks as a dot with the id
bool DebugScene::displayTracks() {
#if 0
	for (int i = 0; i < _engine->_scene->sceneNumTracks; i++) {
		const Vec3 *trackPoint = &_engine->_scene->sceneTracks[i];

	}
#endif
	return false;
}

bool DebugScene::displayZones() {
	bool state = false;
	for (int i = 0; i < _engine->_scene->sceneNumZones; i++) {
		const ZoneStruct *zonePtr = &_engine->_scene->sceneZones[i];

		if (!checkZoneType(zonePtr->type)) {
			continue;
		}

		const ScenePositionsProjected &positions = calculateBoxPositions(zonePtr->bottomLeft, zonePtr->topRight);
		const uint8 color = 15 * 3 + zonePtr->type * 16;
		if (!drawBox(positions, color)) {
			continue;
		}

		const int boxwidth = 150;
		const int lineHeight = 14;
		const int boxheight = 2 * lineHeight;
		const Common::Rect filledRect(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, positions.frontTopRightPoint2D.x + boxwidth, positions.frontTopRightPoint2D.y + boxheight);
		_engine->_interface->drawFilledRect(filledRect, COLOR_WHITE);
		_engine->_menu->drawBox(filledRect);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y, Common::String::format("Type: %i (%i)", zonePtr->type, i), true, false, boxwidth);
		_engine->drawText(positions.frontTopRightPoint2D.x, positions.frontTopRightPoint2D.y + lineHeight, Common::String::format("pos: %i:%i:%i", positions.frontTopRightPoint.x, positions.frontTopRightPoint.y, positions.frontTopRightPoint.z), true, false, boxwidth);
		state = true;
	}
	return state;
}

void DebugScene::renderDebugView() {
	bool dirty = false;
	if (showingZones) {
		dirty |= displayZones();
	}
	if (showingActors) {
		dirty |= displayActors();
	}
	if (showingTracks) {
		dirty |= displayTracks();
	}
	if (dirty) {
		_engine->flip();
	}
}

} // namespace TwinE
