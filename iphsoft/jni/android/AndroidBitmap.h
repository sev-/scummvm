/*
 * AndroidBitmap.h
 *
 *  Created on: Feb 3, 2013
 *      Author: omergilad
 */

#ifndef ANDROIDBITMAP_H_
#define ANDROIDBITMAP_H_

#include "common/scummsys.h"
#include "common/str.h"

using Common::String;

struct AndroidBitmap {
	uint16 width;
	uint16 height;
	float ratio; // height / width
	byte* pixels;
	uint16 glTextureX;
	uint16 glTextureY;
	uint16 displayWidth;
	uint16 displayHeight;
	String bitmapName;
	bool sourceContainsAlpha;
};

#endif /* ANDROIDBITMAP_H_ */
