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
 */

#include "graphics/scalerplugin.h"

#include "graphics/scaler/aspect.h"

ScalerPluginObject::ScalerPluginObject()
	: _factor(1), _aspectRatio(false) {
}

void ScalerPluginObject::initialize(const Graphics::PixelFormat &format) {
	_format = format;
}

int ScalerPluginObject::scaleXCoordinate(int coord, bool inverse) const {
	if (inverse) {
		return coord / _factor;
	} else {
		return coord * _factor;
	}
}

int ScalerPluginObject::scaleYCoordinate(int coord, bool inverse) const {
	if (inverse) {
		int result = coord / _factor;
		if (_aspectRatio) {
			result = aspect2Real(result);
		}
		return result;
	} else {
		int result = coord * _factor;
		if (_aspectRatio) {
			result = real2Aspect(result);
		}
		return result;
	}
}

namespace {
/**
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destination.
 */
template<typename Pixel>
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	// Spot the case when it can all be done in 1 hit
	int BytesPerPixel = sizeof(Pixel);
	if ((srcPitch == BytesPerPixel * (uint)width) && (dstPitch == BytesPerPixel * (uint)width)) {
		memcpy(dstPtr, srcPtr, BytesPerPixel * width * height);
		return;
	}
	while (height--) {
		memcpy(dstPtr, srcPtr, BytesPerPixel * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}
} // End of anonymous namespace

void ScalerPluginObject::scale(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr,
	                           uint32 dstPitch, int width, int height, int x, int y) {
	if (_factor == 1) {
		if (_format.bytesPerPixel == 2) {
			Normal1x<uint16>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		} else {
			Normal1x<uint32>(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		}
	} else {
		scaleIntern(srcPtr, srcPitch, dstPtr, dstPitch, width, height, x, y);
	}

	if (_aspectRatio) {
		correctAspect(dstPtr, dstPitch, width, height, x, y);
	}
}

void ScalerPluginObject::correctAspect(uint8 *buffer, uint32 pitch, int width, int height, int x, int y) {
#ifdef USE_ASPECT
	stretch200To240(buffer, pitch, width * _factor, height * _factor, 0, 0, 0);
#endif
}

SourceScaler::SourceScaler() : _oldSrc(NULL), _enable(false) {
}

SourceScaler::~SourceScaler() {
	if (_oldSrc != NULL)
		delete[] _oldSrc;
}

void SourceScaler::deinitialize() {
	_bufferedOutput.free();
	ScalerPluginObject::deinitialize();
}

void SourceScaler::setSource(const byte *src, uint pitch, int width, int height, int padding) {
	if (_oldSrc != NULL)
		delete[] _oldSrc;

	_padding = padding;
	// Give _oldSrc same pitch
	int size = (height + padding * 2) * pitch;
	_oldSrc = new byte[size];
	memset(_oldSrc, 0, size);

	_bufferedOutput.create(width * _factor, height * _factor, _format);
}

void SourceScaler::scaleIntern(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr,
                         uint32 dstPitch, int width, int height, int x, int y) {
	if (!_enable) {
		// Do not pass _oldSrc, do not update _oldSrc
		internScale(srcPtr, srcPitch,
		            dstPtr, dstPitch,
		            NULL, 0,
		            width, height,
		            NULL, 0);
		return;
	}
	int offset = (_padding + x) * _format.bytesPerPixel + (_padding + y) * srcPitch;
	// Call user defined scale function
	internScale(srcPtr, srcPitch,
	            dstPtr, dstPitch,
	            _oldSrc + offset, srcPitch,
	            width, height,
	            (uint8 *)_bufferedOutput.getBasePtr(x * _factor, y * _factor), _bufferedOutput.pitch);

	// Update the destination buffer
	byte *buffer = (byte *)_bufferedOutput.getBasePtr(x * _factor, y * _factor);
	for (uint i = 0; i < height * _factor; ++i) {
		memcpy(buffer, dstPtr, width * _factor * _format.bytesPerPixel);
		buffer += _bufferedOutput.pitch;
		dstPtr += dstPitch;
	}

	// Update old src
	byte *oldSrc = _oldSrc + offset;
	while (height--) {
		memcpy(oldSrc, srcPtr, width * _format.bytesPerPixel);
		oldSrc += srcPitch;
		srcPtr += srcPitch;
	}
}

