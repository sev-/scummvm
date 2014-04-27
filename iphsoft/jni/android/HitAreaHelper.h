/*
 * HitAreaHelper.h
 *
 *  Created on: Jun 12, 2013
 *      Author: omergilad
 */

#ifndef HITAREAHELPER_H_
#define HITAREAHELPER_H_



#include "common/rect.h"
#include "Constants.h"

using Common::Rect;
using Common::Point;

struct Hotspot {

	Point mDisplayPoint;
	Point mCursorPoint;

	Hotspot() {
	}

	Hotspot(Point display, Point cursor)
			: mDisplayPoint(display), mCursorPoint(cursor) {
	}

	void clear() {
		mDisplayPoint = Point();
		mCursorPoint = Point();
	}

};

class HitAreaHelper {
public:
	HitAreaHelper();
	virtual ~HitAreaHelper();

	/**
	 * Returns the closest hit area to a game coordinate, according to a defined distance threshold
	 */
	Hotspot getClosestHotspot(int x, int y);

	uint16 getAllInteractionHotspots(Point* hotspots, uint16 max);

	uint16 getAllChatHotspots(Point* hotspots, uint16 max);

private:

	bool isPointIsolated(Point p, Rect* original);

	void updateInteractionHitAreas();
	void updateChatHitAreas();


	Rect* mInteractionHitAreas;
	uint16 mInteractionHitAreaCount;

	Rect* mChatHitAreas;
	uint16 mChatHitAreaCount;
};

#endif /* HITAREAHELPER_H_ */
