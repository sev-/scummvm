/*
 * HitAreaHelper.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: omergilad
 */

#include "HitAreaHelper.h"
#include "engines/engine.h"

extern Engine* g_engine;

HitAreaHelper::HitAreaHelper()
		: mInteractionHitAreaCount(0) {

	mInteractionHitAreas = new Rect[HIT_AREA_MAX];
	mChatHitAreas = new Rect[HIT_AREA_MAX];


}

HitAreaHelper::~HitAreaHelper() {

	delete[] mInteractionHitAreas;
	delete[] mChatHitAreas;

}

uint16 HitAreaHelper::getAllChatHotspots(Point* hotspots, uint16 max) {

	LOGD("HitAreaHelper::getAllChatHotspots: ");

	updateChatHitAreas();

	LOGD("HitAreaHelper::getAllChatHotspots: count %d", mChatHitAreaCount);

	int maxCount = MIN(mChatHitAreaCount, max);
	for (int i = 0; i < maxCount; ++i) {

		LOGD("HitAreaHelper::getAllChatHotspots: %s", mChatHitAreas[i].debugStr());

		hotspots[i].x = (mChatHitAreas[i].right + mChatHitAreas[i].left) / 2;
		hotspots[i].y = (mChatHitAreas[i].bottom + mChatHitAreas[i].top) / 2;
	}

	return maxCount;
}

uint16 HitAreaHelper::getAllInteractionHotspots(Point* hotspots, uint16 max) {
	updateInteractionHitAreas();

	int maxCount = MIN(mInteractionHitAreaCount, max);
	for (int i = 0; i < maxCount; ++i) {
		hotspots[i].x = (mInteractionHitAreas[i].right + mInteractionHitAreas[i].left) / 2;
		hotspots[i].y = (mInteractionHitAreas[i].bottom + mInteractionHitAreas[i].top) / 2;
	}

	return maxCount;
}

Hotspot HitAreaHelper::getClosestHotspot(int x, int y) {
	updateInteractionHitAreas();

	//LOGD("HitAreaHelper::getClosestHitArea: %d %d", x, y);

	Rect* resultRect = NULL;

	// First, check if we're inside one of the hit areas
	for (int i = 0; i < mInteractionHitAreaCount; ++i) {

	//	LOGD("HitAreaHelper::getClosestHotspot: hit area %s",
		//		mHitAreas[i].debugStr());

		if (x >= mInteractionHitAreas[i].left && y >= mInteractionHitAreas[i].top
				&& x <= mInteractionHitAreas[i].right && y <= mInteractionHitAreas[i].bottom) {

			//LOGD("HitAreaHelper::getClosestHitArea: inside %d", i);
			// We're inside this hit area, return result
			resultRect = mInteractionHitAreas + i;

		}
	}
	if (resultRect == NULL) {
		// If not, check for the smallest distance that is below the threshold
		int closestDistanceSquare = DISTANCE_THRESHOLD * DISTANCE_THRESHOLD;
		for (int i = 0; i < mInteractionHitAreaCount; ++i) {
			int centerX = (mInteractionHitAreas[i].right + mInteractionHitAreas[i].left) / 2;
			int centerY = (mInteractionHitAreas[i].bottom + mInteractionHitAreas[i].top) / 2;
			int dx = abs(x - centerX);
			int dy = abs(y - centerY);
			int distanceSquare = dx * dx + dy * dy;

			if (distanceSquare <= closestDistanceSquare) {
				//	LOGD("HitAreaHelper::getClosestHitArea: distanceSquare %d", distanceSquare);
				resultRect = mInteractionHitAreas + i;
				closestDistanceSquare = distanceSquare;
			}
		}
	}

	Hotspot resultHotspot;

	// Return the middle point, a corner (if there's a clash in the middle) or 0 point
	if (resultRect != NULL) {

		// Middle
		resultHotspot.mDisplayPoint = Point(
				(resultRect->left + resultRect->right) / 2,
				(resultRect->top + resultRect->bottom) / 2);
		resultHotspot.mCursorPoint = resultHotspot.mDisplayPoint;
		if (isPointIsolated(resultHotspot.mCursorPoint, resultRect)) {
			return resultHotspot;
		}

		// top left
		resultHotspot.mCursorPoint = Point(resultRect->left + 1,
				resultRect->top + 1);
		if (isPointIsolated(resultHotspot.mCursorPoint, resultRect)) {
			return resultHotspot;
		}

		// top right
		resultHotspot.mCursorPoint = Point(resultRect->right - 1,
				resultRect->top + 1);
		if (isPointIsolated(resultHotspot.mCursorPoint, resultRect)) {
			return resultHotspot;
		}

		// bottom left
		resultHotspot.mCursorPoint = Point(resultRect->left + 1,
				resultRect->bottom - 1);
		if (isPointIsolated(resultHotspot.mCursorPoint, resultRect)) {
			return resultHotspot;
		}

		// bottom right
		resultHotspot.mCursorPoint = Point(resultRect->right - 1,
				resultRect->bottom - 1);
		if (isPointIsolated(resultHotspot.mCursorPoint, resultRect)) {
			return resultHotspot;
		}

		LOGE(
				"HitAreaHelper::getClosestHotspot: clash detected - hit area %s is not isolated",
				resultRect->debugStr());

		// Use middle anyway
		resultHotspot.mCursorPoint = resultHotspot.mDisplayPoint;

		return resultHotspot;
	} else {
		resultHotspot.mCursorPoint = Point();
		resultHotspot.mDisplayPoint = Point();
		return resultHotspot;
	}
}

void HitAreaHelper::updateInteractionHitAreas() {

	g_engine->getInteractionHitAreas(mInteractionHitAreas, mInteractionHitAreaCount);

}

void HitAreaHelper::updateChatHitAreas() {

	g_engine->getChatHitAreas(mChatHitAreas, mChatHitAreaCount);

}

bool HitAreaHelper::isPointIsolated(Point p, Rect* original) {
	// Check if the point is inside another hit area beside the original
	for (int i = 0; i < mInteractionHitAreaCount; ++i) {

		if (mInteractionHitAreas + i != original) {
			if (p.x >= mInteractionHitAreas[i].left && p.y >= mInteractionHitAreas[i].top
					&& p.x <= mInteractionHitAreas[i].right
					&& p.y <= mInteractionHitAreas[i].bottom) {

				// Clash detected
				return false;
			}
		}
	}

	return true;
}

