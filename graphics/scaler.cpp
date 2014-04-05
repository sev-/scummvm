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

#include "graphics/scaler/intern.h"
#include "graphics/scaler/scalebit.h"
#include "common/util.h"
#include "common/system.h"
#include "common/textconsole.h"

int gBitFormat = 565;

/** Init the scaler subsystem. */
void InitScalers(uint32 BitFormat) {
	gBitFormat = BitFormat;

	// FIXME: The pixelformat should be param to this function, not the bitformat.
	// Until then, determine the pixelformat in other ways. Unfortunately,
	// calling OSystem::getOverlayFormat() here might not be safe on all ports.
	Graphics::PixelFormat format;
	if (gBitFormat == 555) {
		format = Graphics::createPixelFormat<555>();
	} else if (gBitFormat == 565) {
		format = Graphics::createPixelFormat<565>();
	} else {
		assert(g_system);
		format = g_system->getOverlayFormat();
	}
}

void DestroyScalers() {
// FIXME - Dead Code to remove?
//#ifdef USE_HQ_SCALERS
//	free(RGBtoYUV);
//	RGBtoYUV = 0;
//#endif
}

// FIXME - Dead Code to remove?
/**
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destination.
 */
/*
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	// Spot the case when it can all be done in 1 hit
	if ((srcPitch == sizeof(uint16) * (uint)width) && (dstPitch == sizeof(uint16) * (uint)width)) {
		memcpy(dstPtr, srcPtr, sizeof(uint16) * width * height);
		return;
	}
	while (height--) {
		memcpy(dstPtr, srcPtr, sizeof(uint16) * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}
*/
