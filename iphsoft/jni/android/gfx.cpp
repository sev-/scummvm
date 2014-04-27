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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

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

#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"

#include "graphics/conversion.h"
#include "graphics/decoders/png.h"

#include "backends/platform/android/android.h"
#include "backends/platform/android/jni.h"

#include "backends/platform/android/AndroidPortAdditions.h"

static inline GLfixed xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}

const OSystem::GraphicsMode *OSystem_Android::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_supportedGraphicsModes[] = { {
			"default", "Default", 0 }, { "filter", "Linear filtering", 1 }, { 0,
			0, 0 }, };

	return s_supportedGraphicsModes;
}

int OSystem_Android::getDefaultGraphicsMode() const {
	return 0;
}

bool OSystem_Android::setGraphicsMode(int mode) {
	ENTER("%d", mode);

	if (_game_texture)
		_game_texture->setLinearFilter(mode == 1);

	_graphicsMode = mode;

	return true;
}

int OSystem_Android::getGraphicsMode() const {
	return _graphicsMode;
}

#ifdef USE_RGB_COLOR
Graphics::PixelFormat OSystem_Android::getScreenFormat() const {
	return _game_texture->getPixelFormat();
}

Common::List<Graphics::PixelFormat> OSystem_Android::getSupportedFormats() const {
	Common::List<Graphics::PixelFormat> res;
	res.push_back(GLES565Texture::pixelFormat());

	res.push_back(Graphics::PixelFormat::createFormatCLUT8());

	return res;
}

Common::String OSystem_Android::getPixelFormatName(
		const Graphics::PixelFormat &format) const {
	if (format.bytesPerPixel == 1)
		return "CLUT8";

	if (format.aLoss == 8)
		return Common::String::format("RGB%u%u%u", 8 - format.rLoss,
				8 - format.gLoss, 8 - format.bLoss);

	return Common::String::format("RGBA%u%u%u%u", 8 - format.rLoss,
			8 - format.gLoss, 8 - format.bLoss, 8 - format.aLoss);
}

void OSystem_Android::initTexture(GLESBaseTexture **texture, uint width,
		uint height, const Graphics::PixelFormat *format) {
	assert(texture);
	Graphics::PixelFormat format_clut8 =
			Graphics::PixelFormat::createFormatCLUT8();
	Graphics::PixelFormat format_current;
	Graphics::PixelFormat format_new;

	if (*texture)
		format_current = (*texture)->getPixelFormat();
	else
		format_current = Graphics::PixelFormat();

	if (format)
		format_new = *format;
	else
		format_new = format_clut8;

	if (format_current != format_new) {
		if (*texture)
			LOGD("switching pixel format from: %s",
					getPixelFormatName((*texture)->getPixelFormat()).c_str());

		*texture = new GLESFakePalette565Texture;

		LOGD("new pixel format: %s",
				getPixelFormatName((*texture)->getPixelFormat()).c_str());
	}

	(*texture)->allocBuffer(width, height);
}
#endif

void OSystem_Android::initSurface() {
	LOGD("initializing surface");

	assert(!JNI::haveSurface());

	_screen_changeid = JNI::surface_changeid;
	_egl_surface_width = JNI::egl_surface_width;
	_egl_surface_height = JNI::egl_surface_height;

	assert(_egl_surface_width > 0 && _egl_surface_height > 0);

	JNI::initSurface();

	// Initialize OpenGLES context.
	GLESBaseTexture::initGLExtensions();

	if (_game_texture) {
		_game_texture->reinit();
		_game_texture->initFramebuffer(_egl_surface_width, _egl_surface_height);
	}
}

void OSystem_Android::deinitSurface() {
	if (!JNI::haveSurface())
		return;

	LOGD("deinitializing surface");

	_screen_changeid = JNI::surface_changeid;
	_egl_surface_width = 0;
	_egl_surface_height = 0;

	// release texture resources
	if (_game_texture)
		_game_texture->release();

	JNI::deinitSurface();
}

void OSystem_Android::initViewport() {
	LOGD("initializing viewport");

	assert(JNI::haveSurface());

	// Turn off anything that looks like 3D ;)
	GLCALL(glDisable(GL_CULL_FACE));
	GLCALL(glDisable(GL_DEPTH_TEST));
	GLCALL(glDisable(GL_DITHER));

	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GLCALL(glEnable(GL_BLEND));

	GLCALL(glViewport(0, 0, _egl_surface_width, _egl_surface_height));

	clearFocusRectangle();
}

void OSystem_Android::initOverlay() {
	// minimum of 320x200
	// (surface can get smaller when opening the virtual keyboard on *QVGA*)
	int overlay_width = MAX(_egl_surface_width, 320);
	int overlay_height = MAX(_egl_surface_height, 200);

	// the 'normal' theme layout uses a max height of 400 pixels. if the
	// surface is too big we use only a quarter of the size so that the widgets
	// don't get too small. if the surface height has less than 800 pixels, this
	// enforces the 'lowres' layout, which will be scaled back up by factor 2x,
	// but this looks way better than the 'normal' layout scaled by some
	// calculated factors
	while (overlay_height > 480) {
		overlay_width /= 2;
		overlay_height /= 2;
	}

	LOGI("overlay size is %ux%u", overlay_width, overlay_height);

}

void OSystem_Android::reinitGameTextureSize(uint16 w, uint16 h) {
	// Force the game texture to reallocate itself
	_game_texture->initTextureSize();
	_game_texture->setDirty();
}

void OSystem_Android::initSize(uint width, uint height,
		const Graphics::PixelFormat *format) {
	ENTER("%d, %d, %p", width, height, format);

	GLTHREADCHECK;

#ifdef USE_RGB_COLOR
	initTexture(&_game_texture, width, height, format);
#else
	_game_texture->allocBuffer(width, height);
#endif

	updateScreenRect();
	updateEventScale();

	clearScreen(kClear);

	_game_texture->initFramebuffer(_egl_surface_width, _egl_surface_height);
}

void OSystem_Android::clearScreen(FixupType type, byte count) {

	LOGD("OSystem_Android::clearScreen: ");

	assert(count > 0);

	bool sm = _show_mouse;
	_show_mouse = false;

	GLCALL(glDisable(GL_SCISSOR_TEST));

	for (byte i = 0; i < count; ++i) {
		// clear screen
		GLCALL(glClearColor(0, 0, 0, 1));
		GLCALL(glClear(GL_COLOR_BUFFER_BIT));

		switch (type) {
		case kClear:
			break;

		case kClearSwap:
			JNI::swapBuffers();
			break;

		case kClearUpdate:
			_force_redraw = true;
			updateScreen();
			break;
		}
	}

	//if (!_show_overlay)
	//	GLCALL(glEnable(GL_SCISSOR_TEST));

	_show_mouse = sm;
	_force_redraw = true;
}

void OSystem_Android::updateScreenRect() {

	LOGD("OSystem_Android::updateScreenRect: ");

	Common::Rect rect(0, 0, _egl_surface_width, _egl_surface_height);

	uint16 w = _game_texture->width();
	uint16 h = _game_texture->height();

	if (w && h && !_fullscreen) {
		if (_ar_correction && w == 320 && h == 200)
			h = 240;

		float dpi[2];
		JNI::getDPI(dpi);

		float screen_ar;
		if (dpi[0] != 0.0 && dpi[1] != 0.0) {
			// horizontal orientation
			screen_ar = (dpi[1] * _egl_surface_width)
					/ (dpi[0] * _egl_surface_height);
		} else {
			screen_ar = float(_egl_surface_width) / float(_egl_surface_height);
		}

		float game_ar = float(w) / float(h);

		if (screen_ar > game_ar) {
			rect.setWidth(round(_egl_surface_height * game_ar));
			rect.moveTo((_egl_surface_width - rect.width()) / 2, 0);
		} else {
			rect.setHeight(round(_egl_surface_width / game_ar));
			rect.moveTo((_egl_surface_height - rect.height()) / 2, 0);
		}
	}

//	GLCALL(glScissor(rect.left, rect.top, rect.width(), rect.height()));

	_game_texture->setDrawRect(rect);
}

int OSystem_Android::getScreenChangeID() const {
	return _screen_changeid;
}

int16 OSystem_Android::getHeight() {
	return _game_texture->height();
}

int16 OSystem_Android::getWidth() {
	return _game_texture->width();
}

void OSystem_Android::setPalette(const byte *colors, uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

#ifdef USE_RGB_COLOR
	assert(_game_texture->hasPalette());
#endif

	GLTHREADCHECK;

	if (!_use_mouse_palette)
		setCursorPaletteInternal(colors, start, num);

	const Graphics::PixelFormat &pf = _game_texture->getPalettePixelFormat();
	byte *p = _game_texture->palette() + start * 2;

	for (uint i = 0; i < num; ++i, colors += 3, p += 2)
		WRITE_UINT16(p, pf.RGBToColor(colors[0], colors[1], colors[2]));
}

void OSystem_Android::grabPalette(byte *colors, uint start, uint num) {
	ENTER("%p, %u, %u", colors, start, num);

#ifdef USE_RGB_COLOR
	assert(_game_texture->hasPalette());
#endif

	GLTHREADCHECK;

	const Graphics::PixelFormat &pf = _game_texture->getPalettePixelFormat();
	const byte *p = _game_texture->palette_const() + start * 2;

	for (uint i = 0; i < num; ++i, colors += 3, p += 2)
		pf.colorToRGB(READ_UINT16(p), colors[0], colors[1], colors[2]);
}

void OSystem_Android::copyRectToScreen(const void *buf, int pitch, int x, int y,
		int w, int h) {

	LOGD("OSystem_Android::copyRectToScreen %d %d %d %d", x, y, w, h);

	ENTER("%p, %d, %d, %d, %d, %d", buf, pitch, x, y, w, h);

	GLTHREADCHECK;

	_game_texture->updateBuffer(x, y, w, h, buf, pitch);
}

void OSystem_Android::updateScreen() {

//	LOGD("OSystem_Android::updateScreen: ");

	//ENTER();

	GLTHREADCHECK;

	if (!JNI::haveSurface())
		return;

//	if (!_force_redraw &&
//			!_game_texture->dirty())
//	return;

	_force_redraw = false;

	// clear pointer leftovers in dead areas
	// also, HTC's GLES drivers are made of fail and don't preserve the buffer
	// ( http://www.khronos.org/registry/egl/specs/EGLTechNote0001.html )
	if ((_show_overlay || _htc_fail) && !_fullscreen)
		clearScreen(kClear);

// TODO this doesnt work on those sucky drivers, do it differently
//	if (_show_overlay)
//		GLCALL(glColor4ub(0x9f, 0x9f, 0x9f, 0x9f));

#ifdef ANDROID_DEBUG_GL_MEASURE_RENDER_TIME
	bool shouldMeasure = _game_texture->dirty();
#else
	bool shouldMeasure = _game_texture->dirty()
	&& AndroidPortAdditions::instance()->shouldMeasureRenderTime() && !AndroidPortAdditions::instance()->isInAutoloadState();
#endif

	// Measure render time if needed
	uint64 startTimestamp;
	if (shouldMeasure) {
		startTimestamp = AndroidPortUtils::getTimeOfDayMillis();

	}

	_game_texture->drawTextureRect();


	// If we are in auto-load state, skip swapBuffers (causes a crash if no drawing occured)
	if (AndroidPortAdditions::instance()->isInAutoloadState()) {

		return;
	}

	if (!JNI::swapBuffers()) {
		LOGW("swapBuffers failed: 0x%x", glGetError());
	}

	if (shouldMeasure) {
		AndroidPortAdditions::instance()->onRenderTimeMeasure(
				AndroidPortUtils::getTimeOfDayMillis() - startTimestamp);
	}

}

Graphics::Surface *OSystem_Android::lockScreen() {

	ENTER();

	GLTHREADCHECK;

	Graphics::Surface *surface = _game_texture->surface();
	assert(surface->pixels);

	_game_texture->setDirty();

	return surface;
}

void OSystem_Android::unlockScreen() {

	ENTER();

	GLTHREADCHECK;

	assert(_game_texture->dirty());

}

void OSystem_Android::setShakePos(int shake_offset) {
	ENTER("%d", shake_offset);

	if (_shake_offset != shake_offset) {
		_shake_offset = shake_offset;
		_force_redraw = true;
	}
}

void OSystem_Android::fillScreen(uint32 col) {
	ENTER("%u", col);

	GLTHREADCHECK;

	_game_texture->fillBuffer(col);
}

void OSystem_Android::setFocusRectangle(const Common::Rect& rect) {
	ENTER("%d, %d, %d, %d", rect.left, rect.top, rect.right, rect.bottom);

	if (_enable_zoning) {
		_focus_rect = rect;
		_force_redraw = true;
	}
}

void OSystem_Android::clearFocusRectangle() {
	ENTER();

	if (_enable_zoning) {
		_focus_rect = Common::Rect();
		_force_redraw = true;
	}
}

void OSystem_Android::showOverlay() {
	ENTER();

}

void OSystem_Android::hideOverlay() {
	ENTER();
}

void OSystem_Android::clearOverlay() {
	ENTER();
}

void OSystem_Android::grabOverlay(void *buf, int pitch) {
	ENTER("%p, %d", buf, pitch);

}

void OSystem_Android::copyRectToOverlay(const void *buf, int pitch, int x,
		int y, int w, int h) {
	ENTER("%p, %d, %d, %d, %d, %d", buf, pitch, x, y, w, h);

}

int16 OSystem_Android::getOverlayHeight() {
	return _game_texture->height();
}

int16 OSystem_Android::getOverlayWidth() {
	return _game_texture->width();
}

Graphics::PixelFormat OSystem_Android::getOverlayFormat() const {
	return _game_texture->getPixelFormat();
}

bool OSystem_Android::showMouse(bool visible) {
	ENTER("%d", visible);

	_show_mouse = visible;

	AndroidPortAdditions::instance()->onShowMouse(visible);

	return true;
}

void OSystem_Android::setMouseCursor(const void *buf, uint w, uint h,
		int hotspotX, int hotspotY, uint32 keycolor, bool dontScale,
		const Graphics::PixelFormat *format) {
	ENTER("%p, %u, %u, %d, %d, %u, %d, %p",
			buf, w, h, hotspotX, hotspotY, keycolor, dontScale, format);

}

void OSystem_Android::setCursorPaletteInternal(const byte *colors, uint start,
		uint num) {

}

void OSystem_Android::setCursorPalette(const byte *colors, uint start,
		uint num) {
	ENTER("%p, %u, %u", colors, start, num);

}

void OSystem_Android::disableCursorPalette() {

}

#endif
