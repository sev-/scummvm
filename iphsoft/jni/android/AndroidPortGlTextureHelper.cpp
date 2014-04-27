/*
 * AndroidPortGlTextureHelper.cpp
 *
 *  Created on: Feb 3, 2013
 *      Author: omergilad
 */

/*
#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "backends/platform/android/android.h"*/

#include "AndroidPortGlTextureHelper.h"

AndroidPortGlTextureHelper::AndroidPortGlTextureHelper() {

	// Initialize the allocation array
	// 'true' means allocated, 'false' means free
	mGridW = GL_TEXTURE_WIDTH / GL_TEXTURE_GRID_SIZE;
	mGridH = GL_TEXTURE_HEIGHT / GL_TEXTURE_GRID_SIZE;

	for (uint16 y = 0; y < mGridH; ++y) {
		for (uint16 x = 0; x < mGridW; ++x) {
			mAllocationGrid[x][y] = false;
		}
	}
}

AndroidPortGlTextureHelper::~AndroidPortGlTextureHelper() {

}

void AndroidPortGlTextureHelper::allocateBitmapInGlTexture(
		AndroidBitmap* bitmap) {
	// Initialize the grid size of the bitmap.
	// The grid size must contain the actual bitmap dimensions.
	uint16 bitmapGridW = bitmap->width / GL_TEXTURE_GRID_SIZE + 1;
	uint16 bitmapGridH = bitmap->height / GL_TEXTURE_GRID_SIZE + 1;

	// Iterate over the grid and find space for the bitmap
	for (uint16 y = 0; y < mGridH; ++y) {
		for (uint16 x = 0; x < mGridW; ++x) {
			if (attemptToAllocateSpaceInGrid(x, y, bitmapGridW, bitmapGridH)) {

				// We allocated at those coordinates - set them in the bitmap and return.
				bitmap->glTextureX = x * GL_TEXTURE_GRID_SIZE;
				bitmap->glTextureY = y * GL_TEXTURE_GRID_SIZE;

				LOGD("AndroidPortGlTextureHelper::allocateBitmapInGlTexture: allocated size: %d %d at %d %d", bitmap->width, bitmap->height, bitmap->glTextureX, bitmap->glTextureY);

				return;
			}
		}
	}

	// If we got here, there is not enough space - error
	LOGE(
			"AndroidPortGlTextureHelper::allocateBitmapInGlTexture: not enough space for bitmap");
}

bool AndroidPortGlTextureHelper::attemptToAllocateSpaceInGrid(uint16 x,
		uint16 y, uint16 w, uint16 h) {
	// Check for index out of bounds first
	if (y + h > mGridH || x + w > mGridW) {
		return false;
	}

	// Check for available space in that rect
	for (uint16 y2 = y; y2 < y + h; ++y2) {
		for (uint16 x2 = x; x2 < x + w; ++x2) {
			if (mAllocationGrid[x2][y2]) {
				// We found a slot inside out rect that is not free
				return false;
			}
		}
	}

	// The rect is free - allocate it
	for (uint16 y2 = y; y2 < y + h; ++y2) {
		for (uint16 x2 = x; x2 < x + w; ++x2) {
			mAllocationGrid[x2][y2] = true;
		}
	}

	return true;
}
