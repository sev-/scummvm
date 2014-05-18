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


#include "backends/graphics/opengl/opengl-graphics.h"
#include "backends/graphics/opengl/texture.h"
#include "backends/graphics/opengl/debug.h"
#include "backends/graphics/opengl/extensions.h"

#include "common/textconsole.h"
#include "common/translation.h"
#include "common/advxmlparser.h"
#include "common/algorithm.h"
#include "common/file.h"
#ifdef USE_OSD
#include "common/tokenizer.h"
#include "common/rect.h"
#endif

#include "graphics/conversion.h"
#ifdef USE_OSD
#include "graphics/fontman.h"
#include "graphics/font.h"
#endif

namespace OpenGL {

OpenGLGraphicsManager::OpenGLGraphicsManager()
    : _currentState(), _oldState(), _transactionMode(kTransactionNone), _screenChangeID(1 << (sizeof(int) * 8 - 2)),
      _outputScreenWidth(0), _outputScreenHeight(0), _displayX(0), _displayY(0),
      _displayWidth(0), _displayHeight(0), _defaultFormat(), _defaultFormatAlpha(),
      _gameScreen(nullptr), _gameScreenShakeOffset(0), _overlay(nullptr),
      _overlayVisible(false), _cursor(nullptr),
      _enableShaders(false), _shadersInited(false), _frameCount(0),
      _cursorX(0), _cursorY(0), _cursorHotspotX(0), _cursorHotspotY(0), _cursorHotspotXScaled(0),
      _cursorHotspotYScaled(0), _cursorWidthScaled(0), _cursorHeightScaled(0), _cursorKeyColor(0),
      _cursorVisible(false), _cursorDontScale(false), _cursorPaletteEnabled(false)
#ifdef USE_OSD
      , _osdAlpha(0), _osdFadeStartTime(0), _osd(nullptr)
#endif
    {
	memset(_gamePalette, 0, sizeof(_gamePalette));
}

OpenGLGraphicsManager::~OpenGLGraphicsManager() {
	delete _gameScreen;
	delete _overlay;
	delete _cursor;
#ifdef USE_OSD
	delete _osd;
#endif
}

bool OpenGLGraphicsManager::hasFeature(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
	case OSystem::kFeatureCursorPalette:
		return true;

	case OSystem::kFeatureOverlaySupportsAlpha:
		return _defaultFormatAlpha.aBits() > 3;

	default:
		return false;
	}
}

void OpenGLGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		assert(_transactionMode != kTransactionNone);
		_currentState.aspectRatioCorrection = enable;
		break;

	case OSystem::kFeatureCursorPalette:
		_cursorPaletteEnabled = enable;
		updateCursorPalette();
		break;

	default:
		break;
	}
}

bool OpenGLGraphicsManager::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		return _currentState.aspectRatioCorrection;

	case OSystem::kFeatureCursorPalette:
		return _cursorPaletteEnabled;

	default:
		return false;
	}
}

namespace {

const OSystem::GraphicsMode glGraphicsModes[] = {
	{ "opengl_linear",  _s("OpenGL"),                GFX_LINEAR  },
	{ "opengl_nearest", _s("OpenGL (No filtering)"), GFX_NEAREST },
	{ nullptr, nullptr, 0 }
};

} // End of anonymous namespace

static Common::Array<OSystem::GraphicsMode> *s_supportedGraphicsModes;

static void initGraphicsModes() {
	s_supportedGraphicsModes = new Common::Array<OSystem::GraphicsMode>;
	OSystem::GraphicsMode gm;
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.shader");
	int index = 1;

	// No gl calls can be made since the OpenGL context has not been created yet.
	gm.name = "opengl_nearest";
	gm.id = 0;
	gm.description = "OpenGL";
	s_supportedGraphicsModes->push_back(gm);

	for (Common::ArchiveMemberList::iterator i = files.begin(); i != files.end(); ++i) {
		gm.description = gm.name = strdup((*i)->getName().c_str());
		gm.id = index;
		s_supportedGraphicsModes->push_back(gm);
		index++;
	}

	gm.name = 0;
	gm.description = 0;
	gm.id = 0;
	s_supportedGraphicsModes->push_back(gm);
}


const OSystem::GraphicsMode *OpenGLGraphicsManager::getSupportedGraphicsModes() const {
	if (!s_supportedGraphicsModes)
		initGraphicsModes();

	return &s_supportedGraphicsModes->front();
}

int OpenGLGraphicsManager::getDefaultGraphicsMode() const {
	// First one is default
	return 0;
}

bool OpenGLGraphicsManager::setGraphicsMode(int mode) {
	assert(_transactionMode != kTransactionNone);

	if ((uint)mode >= s_supportedGraphicsModes->size() - 1) {
		warning("OpenGLGraphicsManager::setGraphicsMode(%d): Unknown graphics mode", mode);
		return false;
	}

	_currentState.graphicsMode = mode;

	return true;
}

int OpenGLGraphicsManager::getGraphicsMode() const {
	return _currentState.graphicsMode;
}

#ifdef USE_RGB_COLOR
Graphics::PixelFormat OpenGLGraphicsManager::getScreenFormat() const {
	return _currentState.gameFormat;
}
#endif

void OpenGLGraphicsManager::beginGFXTransaction() {
	assert(_transactionMode == kTransactionNone);

	// Start a transaction.
	_oldState = _currentState;
	_transactionMode = kTransactionActive;
}

OSystem::TransactionError OpenGLGraphicsManager::endGFXTransaction() {
	assert(_transactionMode == kTransactionActive);

	uint transactionError = OSystem::kTransactionSuccess;

	bool setupNewGameScreen = false;
	if (   _oldState.gameWidth  != _currentState.gameWidth
	    || _oldState.gameHeight != _currentState.gameHeight) {
		setupNewGameScreen = true;
	}

#ifdef USE_RGB_COLOR
	if (_oldState.gameFormat != _currentState.gameFormat) {
		setupNewGameScreen = true;
	}

	// Check whether the requested format can actually be used.
	Common::List<Graphics::PixelFormat> supportedFormats = getSupportedFormats();
	// In case the requested format is not usable we will fall back to CLUT8.
	if (Common::find(supportedFormats.begin(), supportedFormats.end(), _currentState.gameFormat) == supportedFormats.end()) {
		_currentState.gameFormat = Graphics::PixelFormat::createFormatCLUT8();
		transactionError |= OSystem::kTransactionFormatNotSupported;
	}
#endif

	do {
		uint requestedWidth  = _currentState.gameWidth;
		uint requestedHeight = _currentState.gameHeight;
		const uint desiredAspect = getDesiredGameScreenAspect();
		requestedHeight = intToFrac(requestedWidth) / desiredAspect;

		if (!loadVideoMode(requestedWidth, requestedHeight,
#ifdef USE_RGB_COLOR
			_currentState.gameFormat
#else
			Graphics::PixelFormat::createFormatCLUT8()
#endif
			)
			// HACK: This is really nasty but we don't have any guarantees of
			// a context existing before, which means we don't know the maximum
			// supported texture size before this. Thus, we check whether the
			// requested game resolution is supported over here.
				|| (   _currentState.gameWidth  > (uint)Texture::getMaximumTextureSize()
				|| _currentState.gameHeight > (uint)Texture::getMaximumTextureSize())) {
			if (_transactionMode == kTransactionActive) {
				// Try to setup the old state in case its valid and is
				// actually different from the new one.
				if (_oldState.valid && _oldState != _currentState) {
					// Give some hints on what failed to set up.
					if (   _oldState.gameWidth  != _currentState.gameWidth
					    || _oldState.gameHeight != _currentState.gameHeight) {
						transactionError |= OSystem::kTransactionSizeChangeFailed;
					}

#ifdef USE_RGB_COLOR
					if (_oldState.gameFormat != _currentState.gameFormat) {
						transactionError |= OSystem::kTransactionFormatNotSupported;
					}
#endif

					if (_oldState.aspectRatioCorrection != _currentState.aspectRatioCorrection) {
						transactionError |= OSystem::kTransactionAspectRatioFailed;
					}

					if (_oldState.graphicsMode != _currentState.graphicsMode) {
						transactionError |= OSystem::kTransactionModeSwitchFailed;
					}

					// Roll back to the old state.
					_currentState = _oldState;
					_transactionMode = kTransactionRollback;

					// Try to set up the old state.
					continue;
				}
			}

			// DON'T use error(), as this tries to bring up the debug
			// console, which WON'T WORK now that we might no have a
			// proper screen.
			warning("OpenGLGraphicsManager::endGFXTransaction: Could not load any graphics mode!");
			g_system->quit();
		}

		// In case we reach this we have a valid state, yay.
		_transactionMode = kTransactionNone;
		_currentState.valid = true;
	} while (_transactionMode == kTransactionRollback);

	if (setupNewGameScreen) {
		delete _gameScreen;
		_gameScreen = nullptr;

#ifdef USE_RGB_COLOR
		_gameScreen = createTexture(_currentState.gameFormat);
#else
		_gameScreen = createTexture(Graphics::PixelFormat::createFormatCLUT8());
#endif
		assert(_gameScreen);
		if (_gameScreen->hasPalette()) {
			_gameScreen->setPalette(0, 256, _gamePalette);
		}

		_gameScreen->allocate(_currentState.gameWidth, _currentState.gameHeight);
		_gameScreen->enableLinearFiltering(_currentState.graphicsMode == GFX_LINEAR);
		// We fill the screen to all black or index 0 for CLUT8.
#ifdef USE_RGB_COLOR
		if (_currentState.gameFormat.bytesPerPixel == 1) {
			_gameScreen->fill(0);
		} else {
			_gameScreen->fill(_gameScreen->getSurface()->format.RGBToColor(0, 0, 0));
		}
#else
		_gameScreen->fill(0);
#endif
	}

	// Update our display area and cursor scaling. This makes sure we pick up
	// aspect ratio correction and game screen changes correctly.
	recalculateDisplayArea();

	// Initialize shaders if present
	initShaders();

	recalculateCursorScaling();

	// Something changed, so update the screen change ID.
	++_screenChangeID;

	// Since transactionError is a ORd list of TransactionErrors this is
	// clearly wrong. But our API is simply broken.
	return (OSystem::TransactionError)transactionError;
}

int OpenGLGraphicsManager::getScreenChangeID() const {
	return _screenChangeID;
}

void OpenGLGraphicsManager::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	Graphics::PixelFormat requestedFormat;
#ifdef USE_RGB_COLOR
	if (!format) {
		requestedFormat = Graphics::PixelFormat::createFormatCLUT8();
	} else {
		requestedFormat = *format;
	}
	_currentState.gameFormat = requestedFormat;
#endif

	_currentState.gameWidth = width;
	_currentState.gameHeight = height;
}

int16 OpenGLGraphicsManager::getWidth() {
	return _currentState.gameWidth;
}

int16 OpenGLGraphicsManager::getHeight() {
	return _currentState.gameHeight;
}

void OpenGLGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_gameScreen->copyRectToTexture(x, y, w, h, buf, pitch);
}

void OpenGLGraphicsManager::fillScreen(uint32 col) {
	// FIXME: This does not conform to the OSystem specs because fillScreen
	// is always taking CLUT8 color values and use color indexed mode. This is,
	// however, plain odd and probably was a forgotten when we introduced
	// RGB support. Thus, we simply do the "sane" thing here and hope OSystem
	// gets fixed one day.
	_gameScreen->fill(col);
}

void OpenGLGraphicsManager::setShakePos(int shakeOffset) {
	_gameScreenShakeOffset = shakeOffset;
}

void OpenGLGraphicsManager::updateScreen() {
	if (!_gameScreen) {
		return;
	}

	// Clear the screen buffer
	GLCALL(glClear(GL_COLOR_BUFFER_BIT));

	const GLfloat shakeOffset = _gameScreenShakeOffset * (GLfloat)_displayHeight / _gameScreen->getHeight();

	// First step: Draw the (virtual) game screen.
	drawTexture(_gameScreen, _displayX, _displayY + shakeOffset, _displayWidth, _displayHeight, _currentShader);

	// Second step: Draw the overlay if visible.
	if (_overlayVisible) {
		drawTexture(_overlay, 0, 0, _outputScreenWidth, _outputScreenHeight);
	}

	// Third step: Draw the cursor if visible.
	if (_cursorVisible && _cursor) {
		// Adjust game screen shake position, but only when the overlay is not
		// visible.
		const GLfloat cursorOffset = _overlayVisible ? 0 : shakeOffset;

		drawTexture(_cursor, _cursorX - _cursorHotspotXScaled, _cursorY - _cursorHotspotYScaled + cursorOffset,
		              _cursorWidthScaled, _cursorHeightScaled);
	}

#ifdef USE_OSD
	// Fourth step: Draw the OSD.
	if (_osdAlpha > 0) {
		Common::StackLock lock(_osdMutex);

		// Update alpha value.
		const int diff = g_system->getMillis(false) - _osdFadeStartTime;
		if (diff > 0) {
			if (diff >= kOSDFadeOutDuration) {
				// Back to full transparency.
				_osdAlpha = 0;
			} else {
				// Do a fade out.
				_osdAlpha = kOSDInitialAlpha - diff * kOSDInitialAlpha / kOSDFadeOutDuration;
			}
		}

		// Set the OSD transparency.
		GLCALL(glColor4f(1.0f, 1.0f, 1.0f, _osdAlpha / 100.0f));

		// Draw the OSD texture.
		drawTexture(_osd, 0, 0, _outputScreenWidth, _outputScreenHeight);

		// Reset color.
		GLCALL(glColor4f(1.0f, 1.0f, 1.0f, 1.0f));
	}
#endif
}

Graphics::Surface *OpenGLGraphicsManager::lockScreen() {
	return _gameScreen->getSurface();
}

void OpenGLGraphicsManager::unlockScreen() {
	_gameScreen->flagDirty();
}

void OpenGLGraphicsManager::setFocusRectangle(const Common::Rect& rect) {
}

void OpenGLGraphicsManager::clearFocusRectangle() {
}

int16 OpenGLGraphicsManager::getOverlayWidth() {
	if (_overlay) {
		return _overlay->getWidth();
	} else {
		return 0;
	}
}

int16 OpenGLGraphicsManager::getOverlayHeight() {
	if (_overlay) {
		return _overlay->getHeight();
	} else {
		return 0;
	}
}

void OpenGLGraphicsManager::showOverlay() {
	_overlayVisible = true;
}

void OpenGLGraphicsManager::hideOverlay() {
	_overlayVisible = false;
}

Graphics::PixelFormat OpenGLGraphicsManager::getOverlayFormat() const {
	return _overlay->getFormat();
}

void OpenGLGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_overlay->copyRectToTexture(x, y, w, h, buf, pitch);
}

void OpenGLGraphicsManager::clearOverlay() {
	_overlay->fill(0);
}

void OpenGLGraphicsManager::grabOverlay(void *buf, int pitch) {
	const Graphics::Surface *overlayData = _overlay->getSurface();

	const byte *src = (const byte *)overlayData->getPixels();
	byte *dst = (byte *)buf;

	for (uint h = overlayData->h; h > 0; --h) {
		memcpy(dst, src, overlayData->w * overlayData->format.bytesPerPixel);
		dst += pitch;
		src += overlayData->pitch;
	}
}

bool OpenGLGraphicsManager::showMouse(bool visible) {
	bool last = _cursorVisible;
	_cursorVisible = visible;
	return last;
}

void OpenGLGraphicsManager::warpMouse(int x, int y) {
	int16 currentX = _cursorX;
	int16 currentY = _cursorY;
	adjustMousePosition(currentX, currentY);

	// Check whether the (virtual) coordinate actually changed. If not, then
	// simply do nothing. This avoids ugly "jittering" due to the actual
	// output screen having a bigger resolution than the virtual coordinates.
	if (currentX == x && currentY == y) {
		return;
	}

	// Scale the virtual coordinates into actual physical coordinates.
	if (_overlayVisible) {
		if (!_overlay) {
			return;
		}
	
		// It might be confusing that we actually have to handle something
		// here when the overlay is visible. This is because for very small
		// resolutions we have a minimal overlay size and have to adjust
		// for that.
		x = (x * _outputScreenWidth)  / _overlay->getWidth();
		y = (y * _outputScreenHeight) / _overlay->getHeight();
	} else {
		if (!_gameScreen) {
			return;
		}

		x = (x * _displayWidth)  / _gameScreen->getWidth();
		y = (y * _displayHeight) / _gameScreen->getHeight();

		x += _displayX;
		y += _displayY;
	}

	setMousePosition(x, y);
	setInternalMousePosition(x, y);
}

namespace {
template<typename DstPixel, typename SrcPixel>
void applyColorKey(DstPixel *dst, const SrcPixel *src, uint w, uint h, uint dstPitch, uint srcPitch, SrcPixel keyColor, DstPixel alphaMask) {
	const uint srcAdd = srcPitch - w * sizeof(SrcPixel);
	const uint dstAdd = dstPitch - w * sizeof(DstPixel);

	while (h-- > 0) {
		for (uint x = w; x > 0; --x, ++dst, ++src) {
			if (*src == keyColor) {
				*dst &= ~alphaMask;
			}
		}

		dst = (DstPixel *)((byte *)dst + dstAdd);
		src = (const SrcPixel *)((const byte *)src + srcAdd);
	}
}
} // End of anonymous namespace

void OpenGLGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	Graphics::PixelFormat inputFormat;
#ifdef USE_RGB_COLOR
	if (format) {
		inputFormat = *format;
	} else {
		inputFormat = Graphics::PixelFormat::createFormatCLUT8();
	}
#else
	inputFormat = Graphics::PixelFormat::createFormatCLUT8();
#endif

	// In case the color format has changed we will need to create the texture.
	if (!_cursor || _cursor->getFormat() != inputFormat) {
		delete _cursor;
		_cursor = nullptr;

		GLenum glIntFormat, glFormat, glType;

		Graphics::PixelFormat textureFormat;
		if (inputFormat.bytesPerPixel == 1 || (inputFormat.aBits() && getGLPixelFormat(inputFormat, glIntFormat, glFormat, glType))) {
			// There is two cases when we can use the cursor format directly.
			// The first is when it's CLUT8, here color key handling can
			// always be applied because we use the alpha channel of
			// _defaultFormatAlpha for that.
			// The other is when the input format has alpha bits and
			// furthermore is directly supported.
			textureFormat = inputFormat;
		} else {
			textureFormat = _defaultFormatAlpha;
		}
		_cursor = createTexture(textureFormat, true);
		assert(_cursor);
		_cursor->enableLinearFiltering(_currentState.graphicsMode == GFX_LINEAR);
	}

	_cursorKeyColor = keycolor;
	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;
	_cursorDontScale = dontScale;

	_cursor->allocate(w, h);
	if (inputFormat.bytesPerPixel == 1) {
		// For CLUT8 cursors we can simply copy the input data into the
		// texture.
		_cursor->copyRectToTexture(0, 0, w, h, buf, w * inputFormat.bytesPerPixel);
	} else {
		// Otherwise it is a bit more ugly because we have to handle a key
		// color properly.

		Graphics::Surface *dst = _cursor->getSurface();
		const uint srcPitch = w * inputFormat.bytesPerPixel;

		// Copy the cursor data to the actual texture surface. This will make
		// sure that the data is also converted to the expected format.
		Graphics::crossBlit((byte *)dst->getPixels(), (const byte *)buf, dst->pitch, srcPitch,
		                    w, h, dst->format, inputFormat);

		// We apply the color key by setting the alpha bits of the pixels to
		// fully transparent.
		const uint32 aMask = (0xFF >> dst->format.aLoss) << dst->format.aShift;
		if (dst->format.bytesPerPixel == 2) {
			if (inputFormat.bytesPerPixel == 2) {
				applyColorKey<uint16, uint16>((uint16 *)dst->getPixels(), (const uint16 *)buf, w, h,
				                              dst->pitch, srcPitch, keycolor, aMask);
			} else if (inputFormat.bytesPerPixel == 4) {
				applyColorKey<uint16, uint32>((uint16 *)dst->getPixels(), (const uint32 *)buf, w, h,
				                              dst->pitch, srcPitch, keycolor, aMask);
			}
		} else {
			if (inputFormat.bytesPerPixel == 2) {
				applyColorKey<uint32, uint16>((uint32 *)dst->getPixels(), (const uint16 *)buf, w, h,
				                              dst->pitch, srcPitch, keycolor, aMask);
			} else if (inputFormat.bytesPerPixel == 4) {
				applyColorKey<uint32, uint32>((uint32 *)dst->getPixels(), (const uint32 *)buf, w, h,
				                              dst->pitch, srcPitch, keycolor, aMask);
			}
		}

		// Flag the texture as dirty.
		_cursor->flagDirty();
	}

	// In case we actually use a palette set that up properly.
	if (inputFormat.bytesPerPixel == 1) {
		updateCursorPalette();
	}

	// Update the scaling.
	recalculateCursorScaling();
}

void OpenGLGraphicsManager::setCursorPalette(const byte *colors, uint start, uint num) {
	// FIXME: For some reason client code assumes that usage of this function
	// automatically enables the cursor palette.
	_cursorPaletteEnabled = true;

	memcpy(_cursorPalette + start * 3, colors, num * 3);
	updateCursorPalette();
}

void OpenGLGraphicsManager::displayMessageOnOSD(const char *msg) {
#ifdef USE_OSD
	// HACK: Actually no client code should use graphics functions from
	// another thread. But the MT-32 emulator still does, thus we need to
	// make sure this doesn't happen while a updateScreen call is done.
	Common::StackLock lock(_osdMutex);

	// Slip up the lines.
	Common::Array<Common::String> osdLines;
	Common::StringTokenizer tokenizer(msg, "\n");
	while (!tokenizer.empty()) {
		osdLines.push_back(tokenizer.nextToken());
	}

	// Do the actual drawing like the SDL backend.
	const Graphics::Font *font = getFontOSD();
	Graphics::Surface *dst = _osd->getSurface();
	_osd->fill(0);
	_osd->flagDirty();

	// Determine a rect which would contain the message string (clipped to the
	// screen dimensions).
	const int vOffset = 6;
	const int lineSpacing = 1;
	const int lineHeight = font->getFontHeight() + 2 * lineSpacing;
	int width = 0;
	int height = lineHeight * osdLines.size() + 2 * vOffset;
	for (uint i = 0; i < osdLines.size(); i++) {
		width = MAX(width, font->getStringWidth(osdLines[i]) + 14);
	}

	// Clip the rect
	width  = MIN<int>(width,  dst->w);
	height = MIN<int>(height, dst->h);

	int dstX = (dst->w - width) / 2;
	int dstY = (dst->h - height) / 2;

	// Draw a dark gray rect.
	const uint32 color = dst->format.RGBToColor(40, 40, 40);
	dst->fillRect(Common::Rect(dstX, dstY, dstX + width, dstY + height), color);

	// Render the message, centered, and in white
	const uint32 white = dst->format.RGBToColor(255, 255, 255);
	for (uint i = 0; i < osdLines.size(); ++i) {
		font->drawString(dst, osdLines[i],
		                 dstX, dstY + i * lineHeight + vOffset + lineSpacing, width,
		                 white, Graphics::kTextAlignCenter);
	}

	// Init the OSD display parameters.
	_osdAlpha = kOSDInitialAlpha;
	_osdFadeStartTime = g_system->getMillis() + kOSDFadeOutDelay;
#endif
}

void OpenGLGraphicsManager::setPalette(const byte *colors, uint start, uint num) {
	assert(_gameScreen->hasPalette());

	memcpy(_gamePalette + start * 3, colors, num * 3);
	_gameScreen->setPalette(start, num, colors);

	// We might need to update the cursor palette here.
	updateCursorPalette();
}

void OpenGLGraphicsManager::grabPalette(byte *colors, uint start, uint num) {
	assert(_gameScreen->hasPalette());

	memcpy(colors, _gamePalette + start * 3, num * 3);
}

void OpenGLGraphicsManager::setActualScreenSize(uint width, uint height) {
	_outputScreenWidth = width;
	_outputScreenHeight = height;

	// Setup coordinates system.
	GLCALL(glViewport(0, 0, _outputScreenWidth, _outputScreenHeight));

	GLCALL(glMatrixMode(GL_PROJECTION));
	GLCALL(glLoadIdentity());
#ifdef USE_GLES
	GLCALL(glOrthof(0, _outputScreenWidth, _outputScreenHeight, 0, -1, 1));
#else
	GLCALL(glOrtho(0, _outputScreenWidth, _outputScreenHeight, 0, -1, 1));
#endif
	GLCALL(glMatrixMode(GL_MODELVIEW));
	GLCALL(glLoadIdentity());

	uint overlayWidth = width;
	uint overlayHeight = height;

	// WORKAROUND: We can only support surfaces up to the maximum supported
	// texture size. Thus, in case we encounter a physical size bigger than
	// this maximum texture size we will simply use an overlay as big as
	// possible and then scale it to the physical display size. This sounds
	// bad but actually all recent chips should support full HD resolution
	// anyway. Thus, it should not be a real issue for modern hardware.
	if (   overlayWidth  > (uint)Texture::getMaximumTextureSize()
	    || overlayHeight > (uint)Texture::getMaximumTextureSize()) {
		const frac_t outputAspect = intToFrac(_outputScreenWidth) / _outputScreenHeight;

		if (outputAspect > (frac_t)FRAC_ONE) {
			overlayWidth  = Texture::getMaximumTextureSize();
			overlayHeight = intToFrac(overlayWidth) / outputAspect;
		} else {
			overlayHeight = Texture::getMaximumTextureSize();
			overlayWidth  = fracToInt(overlayHeight * outputAspect);
		}
	}

	// HACK: We limit the minimal overlay size to 256x200, which is the
	// minimum of the dimensions of the two resolutions 256x240 (NES) and
	// 320x200 (many DOS games use this). This hopefully assure that our
	// GUI has working layouts.
	overlayWidth = MAX<uint>(overlayWidth, 256);
	overlayHeight = MAX<uint>(overlayHeight, 200);

	if (!_overlay || _overlay->getFormat() != _defaultFormatAlpha) {
		delete _overlay;
		_overlay = nullptr;

		_overlay = createTexture(_defaultFormatAlpha);
		assert(_overlay);
		// We always filter the overlay with GL_LINEAR. This assures it's
		// readable in case it needs to be scaled and does not affect it
		// otherwise.
		_overlay->enableLinearFiltering(true);
	}
	_overlay->allocate(overlayWidth, overlayHeight);
	_overlay->fill(0);

#ifdef USE_OSD
	if (!_osd || _osd->getFormat() != _defaultFormatAlpha) {
		delete _osd;
		_osd = nullptr;

		_osd = createTexture(_defaultFormatAlpha);
		assert(_osd);
		// We always filter the osd with GL_LINEAR. This assures it's
		// readable in case it needs to be scaled and does not affect it
		// otherwise.
		_osd->enableLinearFiltering(true);
	}
	_osd->allocate(_overlay->getWidth(), _overlay->getHeight());
	_osd->fill(0);
#endif

	// Re-setup the scaling for the screen and cursor
	recalculateDisplayArea();
	recalculateCursorScaling();

	// Something changed, so update the screen change ID.
	++_screenChangeID;
}

void OpenGLGraphicsManager::notifyContextCreate(const Graphics::PixelFormat &defaultFormat, const Graphics::PixelFormat &defaultFormatAlpha) {
	// Initialize all extensions.
	initializeGLExtensions();

	// Disable 3D properties.
	GLCALL(glDisable(GL_CULL_FACE));
	GLCALL(glDisable(GL_DEPTH_TEST));
	GLCALL(glDisable(GL_LIGHTING));
	GLCALL(glDisable(GL_FOG));
	GLCALL(glDisable(GL_DITHER));
	GLCALL(glShadeModel(GL_FLAT));
	GLCALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST));

	// Default to black as clear color.
	GLCALL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	GLCALL(glColor4f(1.0f, 1.0f, 1.0f, 1.0f));

	// Setup alpha blend (for overlay and cursor).
	GLCALL(glEnable(GL_BLEND));
	GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// Enable rendering with vertex and coord arrays.
	GLCALL(glEnableClientState(GL_VERTEX_ARRAY));
	GLCALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

	GLCALL(glEnable(GL_TEXTURE_2D));

	// We use a "pack" alignment (when reading from textures) to 4 here,
	// since the only place where we really use it is the BMP screenshot
	// code and that requires the same alignment too.
	GLCALL(glPixelStorei(GL_PACK_ALIGNMENT, 4));

	// Query information needed by textures.
	Texture::queryTextureInformation();

	// Refresh the output screen dimensions if some are set up.
	if (_outputScreenWidth != 0 && _outputScreenHeight != 0) {
		setActualScreenSize(_outputScreenWidth, _outputScreenHeight);
	}

	// TODO: Should we try to convert textures into one of those formats if
	// possible? For example, when _gameScreen is CLUT8 we might want to use
	// defaultFormat now.
	_defaultFormat = defaultFormat;
	_defaultFormatAlpha = defaultFormatAlpha;

	if (_gameScreen) {
		_gameScreen->recreateInternalTexture();
	}

	if (_overlay) {
		_overlay->recreateInternalTexture();
	}

	if (_cursor) {
		_cursor->recreateInternalTexture();
	}

#ifdef USE_OSD
	if (_osd) {
		_osd->recreateInternalTexture();
	}
#endif
}

void OpenGLGraphicsManager::notifyContextDestroy() {
	if (_gameScreen) {
		_gameScreen->releaseInternalTexture();
	}

	if (_overlay) {
		_overlay->releaseInternalTexture();
	}

	if (_cursor) {
		_cursor->releaseInternalTexture();
	}

#ifdef USE_OSD
	if (_osd) {
		_osd->releaseInternalTexture();
	}
#endif
}

void OpenGLGraphicsManager::adjustMousePosition(int16 &x, int16 &y) {
	if (_overlayVisible) {
		// It might be confusing that we actually have to handle something
		// here when the overlay is visible. This is because for very small
		// resolutions we have a minimal overlay size and have to adjust
		// for that.
		// This can also happen when the overlay is smaller than the actual
		// display size because of texture size limitations.
		if (_overlay) {
			x = (x * _overlay->getWidth())  / _outputScreenWidth;
			y = (y * _overlay->getHeight()) / _outputScreenHeight;
		}
	} else if (_gameScreen) {
		x -= _displayX;
		y -= _displayY;

		const int16 width  = _gameScreen->getWidth();
		const int16 height = _gameScreen->getHeight();

		x = (x * width)  / _displayWidth;
		y = (y * height) / _displayHeight;

		// Make sure we only supply valid coordinates.
		x = CLIP<int16>(x, 0, width - 1);
		y = CLIP<int16>(y, 0, height - 1);
	}
}

Texture *OpenGLGraphicsManager::createTexture(const Graphics::PixelFormat &format, bool wantAlpha) {
	GLenum glIntFormat, glFormat, glType;
	if (format.bytesPerPixel == 1) {
		const Graphics::PixelFormat &virtFormat = wantAlpha ? _defaultFormatAlpha : _defaultFormat;
		const bool supported = getGLPixelFormat(virtFormat, glIntFormat, glFormat, glType);
		if (!supported) {
			return nullptr;
		} else {
			return new TextureCLUT8(glIntFormat, glFormat, glType, virtFormat);
		}
	} else {
		const bool supported = getGLPixelFormat(format, glIntFormat, glFormat, glType);
		if (!supported) {
			return nullptr;
		} else {
			return new Texture(glIntFormat, glFormat, glType, format);
		}
	}
}

bool OpenGLGraphicsManager::getGLPixelFormat(const Graphics::PixelFormat &pixelFormat, GLenum &glIntFormat, GLenum &glFormat, GLenum &glType) const {
#ifdef SCUMM_LITTLE_ENDIAN
	if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) { // ABGR8888
#else
	if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) { // RGBA8888
#endif
		glIntFormat = GL_RGBA;
		glFormat = GL_RGBA;
		glType = GL_UNSIGNED_BYTE;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)) { // RGB565
		glIntFormat = GL_RGB;
		glFormat = GL_RGB;
		glType = GL_UNSIGNED_SHORT_5_6_5;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0)) { // RGBA5551
		glIntFormat = GL_RGBA;
		glFormat = GL_RGBA;
		glType = GL_UNSIGNED_SHORT_5_5_5_1;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0)) { // RGBA4444
		glIntFormat = GL_RGBA;
		glFormat = GL_RGBA;
		glType = GL_UNSIGNED_SHORT_4_4_4_4;
		return true;
#ifndef USE_GLES
#ifdef SCUMM_LITTLE_ENDIAN
	} else if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0)) { // RGBA8888
		glIntFormat = GL_RGBA;
		glFormat = GL_RGBA;
		glType = GL_UNSIGNED_INT_8_8_8_8;
		return true;
#endif
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0)) { // RGB555
		// GL_BGRA does not exist in every GLES implementation so should not be configured if
		// USE_GLES is set.
		glIntFormat = GL_RGB;
		glFormat = GL_BGRA;
		glType = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24)) { // ARGB8888
		glIntFormat = GL_RGBA;
		glFormat = GL_BGRA;
		glType = GL_UNSIGNED_INT_8_8_8_8_REV;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 4, 4, 4, 4, 8, 4, 0, 12)) { // ARGB4444
		glIntFormat = GL_RGBA;
		glFormat = GL_BGRA;
		glType = GL_UNSIGNED_SHORT_4_4_4_4_REV;
		return true;
#ifdef SCUMM_BIG_ENDIAN
	} else if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24)) { // ABGR8888
		glIntFormat = GL_RGBA;
		glFormat = GL_RGBA;
		glType = GL_UNSIGNED_INT_8_8_8_8_REV;
		return true;
#endif
	} else if (pixelFormat == Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0)) { // BGRA8888
		glIntFormat = GL_RGBA;
		glFormat = GL_BGRA;
		glType = GL_UNSIGNED_INT_8_8_8_8;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0)) { // BGR565
		glIntFormat = GL_RGB;
		glFormat = GL_BGR;
		glType = GL_UNSIGNED_SHORT_5_6_5;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 5, 5, 5, 1, 1, 6, 11, 0)) { // BGRA5551
		glIntFormat = GL_RGBA;
		glFormat = GL_BGRA;
		glType = GL_UNSIGNED_SHORT_5_5_5_1;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 4, 4, 4, 4, 0, 4, 8, 12)) { // ABGR4444
		glIntFormat = GL_RGBA;
		glFormat = GL_RGBA;
		glType = GL_UNSIGNED_SHORT_4_4_4_4_REV;
		return true;
	} else if (pixelFormat == Graphics::PixelFormat(2, 4, 4, 4, 4, 4, 8, 12, 0)) { // BGRA4444
		glIntFormat = GL_RGBA;
		glFormat = GL_BGRA;
		glType = GL_UNSIGNED_SHORT_4_4_4_4;
		return true;
#endif
	} else {
		return false;
	}
}

frac_t OpenGLGraphicsManager::getDesiredGameScreenAspect() const {
	const uint width  = _currentState.gameWidth;
	const uint height = _currentState.gameHeight;

	if (_currentState.aspectRatioCorrection) {
		// In case we enable aspect ratio correction we force a 4/3 ratio.
		// But just for 320x200 and 640x400 games, since other games do not need
		// this.
		if ((width == 320 && height == 200) || (width == 640 && height == 400)) {
			return intToFrac(4) / 3;
		}
	}

	return intToFrac(width) / height;
}

void OpenGLGraphicsManager::recalculateDisplayArea() {
	if (!_gameScreen || _outputScreenHeight == 0) {
		return;
	}

	const frac_t outputAspect = intToFrac(_outputScreenWidth) / _outputScreenHeight;
	const frac_t desiredAspect = getDesiredGameScreenAspect();

	_displayWidth = _outputScreenWidth;
	_displayHeight = _outputScreenHeight;

	// Adjust one dimension for mantaining the aspect ratio.
	if (outputAspect < desiredAspect) {
		_displayHeight = intToFrac(_displayWidth) / desiredAspect;
	} else if (outputAspect > desiredAspect) {
		_displayWidth = fracToInt(_displayHeight * desiredAspect);
	}

	// We center the screen in the middle for now.
	_displayX = (_outputScreenWidth  - _displayWidth ) / 2; 
	_displayY = (_outputScreenHeight - _displayHeight) / 2; 
}

void OpenGLGraphicsManager::updateCursorPalette() {
	if (!_cursor || !_cursor->hasPalette()) {
		return;
	}

	if (_cursorPaletteEnabled) {
		_cursor->setPalette(0, 256, _cursorPalette);
	} else {
		_cursor->setPalette(0, 256, _gamePalette);
	}

	// We remove all alpha bits from the palette entry of the color key.
	// This makes sure its properly handled as color key.
	const Graphics::PixelFormat &hardwareFormat = _cursor->getHardwareFormat();
	const uint32 aMask = (0xFF >> hardwareFormat.aLoss) << hardwareFormat.aShift;

	if (hardwareFormat.bytesPerPixel == 2) {
		uint16 *palette = (uint16 *)_cursor->getPalette() + _cursorKeyColor;
		*palette &= ~aMask;
	} else if (hardwareFormat.bytesPerPixel == 4) {
		uint32 *palette = (uint32 *)_cursor->getPalette() + _cursorKeyColor;
		*palette &= ~aMask;
	} else {
		warning("OpenGLGraphicsManager::updateCursorPalette: Unsupported pixel depth %d", hardwareFormat.bytesPerPixel);
	}
}

void OpenGLGraphicsManager::recalculateCursorScaling() {
	if (!_cursor || !_gameScreen) {
		return;
	}

	// By default we use the unscaled versions.
	_cursorHotspotXScaled = _cursorHotspotX;
	_cursorHotspotYScaled = _cursorHotspotY;
	_cursorWidthScaled = _cursor->getWidth();
	_cursorHeightScaled = _cursor->getHeight();

	// In case scaling is actually enabled we will scale the cursor according
	// to the game screen.
	if (!_cursorDontScale) {
		const frac_t screenScaleFactorX = intToFrac(_displayWidth)  / _gameScreen->getWidth();
		const frac_t screenScaleFactorY = intToFrac(_displayHeight) / _gameScreen->getHeight();

		_cursorHotspotXScaled = fracToInt(_cursorHotspotXScaled * screenScaleFactorX);
		_cursorWidthScaled    = fracToInt(_cursorWidthScaled    * screenScaleFactorX);

		_cursorHotspotYScaled = fracToInt(_cursorHotspotYScaled * screenScaleFactorY);
		_cursorHeightScaled   = fracToInt(_cursorHeightScaled   * screenScaleFactorY);
	}
}

#ifdef USE_OSD
const Graphics::Font *OpenGLGraphicsManager::getFontOSD() {
	return FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);
}
#endif

bool OpenGLGraphicsManager::parseShader(const Common::String &filename, ShaderInfo &info) {
	// FIXME
	Common::AdvXMLParser shaderParser;
	if (!shaderParser.loadFile(filename)) {
		warning("Could not open file:%s", filename.c_str());
		return false;
	}

	warning("Parsing shader: %s", filename.c_str());
	Common::XMLTree *root, *shader = NULL, *t;

	root = shaderParser.parse();

	for (uint i = 0; i < root->children.size(); ++i) {
		if (root->children[i]->type == Common::XMLTree::kKey &&
			root->children[i]->text == "shader") {
			shader = root->children[i];
		}
	}

	if (!shader) {
		warning("No shader element");
		delete root;
		return false;
	}

	info.vertex = 0;

	for (uint i = 0; i < shader->children.size(); ++i) {
		if (shader->children[i]->type != Common::XMLTree::kKey)
			continue;
		t = shader->children[i];

		if (t->text == "vertex") {
			if (t->children[0]->type != Common::XMLTree::kText) {
				warning("Unexpected key");
				delete root;
				return false;
			}
			const Common::String &src = t->children[0]->text;
			info.vertex = compileShader(src, GL_VERTEX_SHADER);
		} else if (t->text == "fragment") {
			if (t->children[0]->type != Common::XMLTree::kText) {
				warning("Unexpected key");
				delete root;
				return false;
			}
			ShaderPass p;
			bool x = false, y = false;
			p.xScaleMethod = ShaderPass::kNotSet;
			p.yScaleMethod = ShaderPass::kNotSet;
			p.filter = GL_NEAREST;
			if (t->attrs.contains("filter")) {
				const Common::String &value = t->attrs["filter"];
				if (value == "nearest") {
					p.filter = GL_NEAREST;
				} else if (value == "linear") {
					p.filter = GL_LINEAR;
				} else {
					warning("filter must be linear or nearest");
					delete root;
					return false;
				}
			}
			if (t->attrs.contains("size")) {
				x = y = true;
				p.xScaleMethod = ShaderPass::kFixed;
				p.yScaleMethod = ShaderPass::kFixed;
				const Common::String &value = t->attrs["size"];
				sscanf(value.c_str(), "%f", &p.xScale);
				p.yScale = p.xScale;
			}
			if (t->attrs.contains("size_x")) {
				if (x) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				x = true;
				p.xScaleMethod = ShaderPass::kFixed;
				const Common::String &value = t->attrs["size_x"];
				sscanf(value.c_str(), "%f", &p.xScale);
			}
			if (t->attrs.contains("size_y")) {
				if (y) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				y = true;
				p.yScaleMethod = ShaderPass::kFixed;
				const Common::String &value = t->attrs["size_y"];
				sscanf(value.c_str(), "%f", &p.yScale);
			}
			if (t->attrs.contains("scale")) {
				if (x || y) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				x = y = true;
				p.xScaleMethod = ShaderPass::kInput;
				p.yScaleMethod = ShaderPass::kInput;
				const Common::String &value = t->attrs["scale"];
				sscanf(value.c_str(), "%f", &p.xScale);
				p.yScale = p.xScale;
			}
			if (t->attrs.contains("scale_x")) {
				if (x) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				x = true;
				p.xScaleMethod = ShaderPass::kInput;
				const Common::String &value = t->attrs["scale_x"];
				sscanf(value.c_str(), "%f", &p.xScale);
			}
			if (t->attrs.contains("scale_y")) {
				if (y) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				y = true;
				p.yScaleMethod = ShaderPass::kInput;
				const Common::String &value = t->attrs["scale_y"];
				sscanf(value.c_str(), "%f", &p.yScale);
			}
			if (t->attrs.contains("outscale")) {
				if (x || y) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				x = y = true;
				p.xScaleMethod = ShaderPass::kOutput;
				p.yScaleMethod = ShaderPass::kOutput;
				const Common::String &value = t->attrs["outscale"];
				sscanf(value.c_str(), "%f", &p.xScale);
				p.yScale = p.xScale;
			}
			if (t->attrs.contains("outscale_x")) {
				if (x) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				x = true;
				p.xScaleMethod = ShaderPass::kOutput;
				const Common::String &value = t->attrs["outscale_x"];
				sscanf(value.c_str(), "%f", &p.xScale);
			}
			if (t->attrs.contains("outscale_y")) {
				if (y) {
					warning("Conflicting attributes");
					delete root;
					return false;
				}
				y = true;
				p.yScaleMethod = ShaderPass::kOutput;
				const Common::String &value = t->attrs["outscale_y"];
				sscanf(value.c_str(), "%f", &p.yScale);
			}
			const Common::String &src = t->children[0]->text;
			p.fragment = compileShader(src, GL_FRAGMENT_SHADER);
			info.passes.push_back(p);
		}

	}

	// The vertex shader may not have been compiled when the pass was parsed.
	// Link the programs now.
	for (uint j = 0; j < info.passes.size(); ++j) {
		ShaderPass &p = info.passes[j];
		p.program = linkShaders(info.vertex, p.fragment);
		p.textureLoc = glGetUniformLocation(p.program, "rubyTexture");
		p.inputSizeLoc = glGetUniformLocation(p.program, "rubyInputSize");
		p.outputSizeLoc = glGetUniformLocation(p.program, "rubyOutputSize");
		p.textureSizeLoc = glGetUniformLocation(p.program, "rubyTextureSize");
		p.frameCountLoc = glGetUniformLocation(p.program, "rubyFrameCount");

		// Non-standard but sometimes used
		p.origTextureLoc = glGetUniformLocation(p.program, "rubyOrigTexture");
		p.origTextureSizeLoc = glGetUniformLocation(p.program, "rubyOrigTextureSize");
		p.origInputSizeLoc = glGetUniformLocation(p.program, "rubyOrigInputSize");
	}

	delete root;

	return true;
}

const char *s_defaultVertex = 
"#version 110\n "
"uniform vec2 rubyTextureSize; "
"void main() { "
"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; "
"  gl_TexCoord[0] = gl_MultiTexCoord0; "
"} ";

const char *s_defaultFragment =
"#version 110\n "
"uniform sampler2D rubyTexture; "
"uniform vec2 rubyTextureSize; "
"void main() { "
"  gl_FragColor = texture2DProj(rubyTexture, gl_TexCoord[0]); "
"} ";

void OpenGLGraphicsManager::initShaders() {
	if (_shadersInited) {
		_currentShader = &_shaders[_currentState.graphicsMode];
		_frameCount = 0;
		return;
	}
	_shadersInited = true;
	const char * versionStr = (const char *)glGetString(GL_VERSION);
	Common::String version;
	int majorVersion, minorVersion;
	Common::StringTokenizer st(versionStr);
	// Version number is 3rd token in OpenGL ES
	// and 1st token in OpenGL.
#ifdef USE_GLES
	st.nextToken();
	st.nextToken();
#endif
	version = st.nextToken();
	sscanf(version.c_str(), "%d.%d", &majorVersion, &minorVersion);
	_enableShaders = majorVersion >= 2;

	if (!_enableShaders)
		return;

	ShaderInfo dInfo;
	ShaderPass p;

	// Initialize built-in shader
	dInfo.vertex = compileShader(s_defaultVertex, GL_VERTEX_SHADER);
	p.fragment = compileShader(s_defaultFragment, GL_FRAGMENT_SHADER);
	p.program = linkShaders(dInfo.vertex, p.fragment);
	dInfo.name = "default";
	p.filter = GL_NEAREST;
	p.textureLoc = glGetUniformLocation(p.program, "rubyTexture");
	p.inputSizeLoc = glGetUniformLocation(p.program, "rubyInputSize");
	p.outputSizeLoc = glGetUniformLocation(p.program, "rubyOutputSize");
	p.textureSizeLoc = glGetUniformLocation(p.program, "rubyTextureSize");
	p.origTextureSizeLoc = glGetUniformLocation(p.program, "rubyOrigTextureSize");
	p.origTextureLoc = glGetUniformLocation(p.program, "rubyOrigTexture");
	p.origInputSizeLoc = glGetUniformLocation(p.program, "rubyOrigInputSize");

	p.xScaleMethod = ShaderPass::kNotSet;
	p.yScaleMethod = ShaderPass::kNotSet;
	dInfo.passes.push_back(p);

	_shaders.push_back(dInfo);

	for (uint i = 1; i < s_supportedGraphicsModes->size() - 1; ++i) {
		ShaderInfo info;
		OSystem::GraphicsMode &gm = (*s_supportedGraphicsModes)[i];
		info.name = Common::String(gm.name);
		if (parseShader(info.name, info)) {
			warning("Successfully compiled %s", info.name.c_str());
			_shaders.push_back(info);
		} else {
			warning("%s is not a shader file", info.name.c_str());
		}
	}

	_defaultShader = &_shaders[0];
	_currentShader = &_shaders[_currentState.graphicsMode];
	_frameCount = 0;
	//_currentShader = &_shaders[0];
}

GLuint OpenGLGraphicsManager::compileShader(const Common::String &src, GLenum type) {
	int size = src.size();
	const char * source = src.c_str();
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, &size);
	glCompileShader(shader);

	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char *buffer;
		int length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		buffer = new char[length];
		glGetShaderInfoLog(shader, length, NULL, buffer);
		if (type == GL_VERTEX_SHADER)
			error("Vertex shader compilation failed:\n%s", buffer);
		else
			error("Fragment shader compilation failed:\n%s", buffer);
	}
	return shader;
}

GLuint OpenGLGraphicsManager::linkShaders(GLuint vertex, GLuint fragment) {
	GLuint program = glCreateProgram();
	if (vertex)
		glAttachShader(program, vertex);
	if (fragment)
		glAttachShader(program, fragment);

	glLinkProgram(program);

	int status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (!status) {
		char *buffer;
		int length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		buffer = new char[length];
		glGetProgramInfoLog(program, length, NULL, buffer);
		error("Shader program link failed:\n%s", buffer);
	}
	return program;
}

void OpenGLGraphicsManager::saveScreenshot(const Common::String &filename) const {
	const uint width  = _outputScreenWidth;
	const uint height = _outputScreenHeight;

	// A line of a BMP image must have a size divisible by 4.
	// We calculate the padding bytes needed here.
	// Since we use a 3 byte per pixel mode, we can use width % 4 here, since
	// it is equal to 4 - (width * 3) % 4. (4 - (width * Bpp) % 4, is the
	// usual way of computing the padding bytes required).
	const uint linePaddingSize = width % 4;
	const uint lineSize        = width * 3 + linePaddingSize;

	// Allocate memory for screenshot
	uint8 *pixels = new uint8[lineSize * height];

	// Get pixel data from OpenGL buffer
	GLCALL(glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels));

	// BMP stores as BGR. Since we can't assume that GL_BGR is supported we
	// will swap the components from the RGB we read to BGR on our own.
	for (uint y = height; y-- > 0;) {
		uint8 *line = pixels + y * lineSize;

		for (uint x = width; x > 0; --x, line += 3) {
			SWAP(line[0], line[2]);
		}
	}

	// Open file
	Common::DumpFile out;
	out.open(filename);

	// Write BMP header
	out.writeByte('B');
	out.writeByte('M');
	out.writeUint32LE(height * lineSize + 54);
	out.writeUint32LE(0);
	out.writeUint32LE(54);
	out.writeUint32LE(40);
	out.writeUint32LE(width);
	out.writeUint32LE(height);
	out.writeUint16LE(1);
	out.writeUint16LE(24);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);
	out.writeUint32LE(0);

	// Write pixel data to BMP
	out.write(pixels, lineSize * height);

	// Free allocated memory
	delete[] pixels;
}

// This function performs multipass rendering using shaders.
// TODO: check if extensions are available, test OpenGL ES, clean/split function,
//       generate buffers on gfxmode init, use x and y (instead of implicit 0,0)
void OpenGLGraphicsManager::drawTexture(Texture *texture, GLshort x, GLshort y, GLshort w, GLshort h, const ShaderInfo *info) {
	float outputw, outputh, inputw, inputh, texw, texh,
	origInputw, origInputh, origTexw, origTexh;
	origInputw = inputw = texture->getWidth();
	origInputh = inputh = texture->getHeight();
	origTexw = texw = texture->getTextureWidth();
	origTexh = texh = texture->getTextureHeight();
	bool implicitPass = false;
	GLuint currentTexture = texture->getName();
	GLuint origTexture = currentTexture;
	GLuint fbo, outputtex;

	texture->updateTexture();

	for (uint i = 0; i < info->passes.size(); ++i) {
		bool lastPass = (i == info->passes.size() - 1);

		const ShaderPass &p = info->passes[i];

		switch (p.xScaleMethod) {
		case ShaderPass::kFixed:
			outputw = p.xScale;
			break;
		case ShaderPass::kInput:
			outputw = inputw * p.xScale;
			break;
		case ShaderPass::kOutput:
			outputw = w * p.xScale;
			break;
		case ShaderPass::kNotSet:
			outputw = inputw;
		}

		if (lastPass) {
			if (p.xScaleMethod == ShaderPass::kNotSet) {
				outputw = w;
			} else {
				implicitPass = true;
			}
		}

		switch (p.yScaleMethod) {
		case ShaderPass::kFixed:
			outputh = p.yScale;
			break;
		case ShaderPass::kInput:
			outputh = inputh * p.yScale;
			break;
		case ShaderPass::kOutput:
			outputh = h * p.yScale;
			break;
		case ShaderPass::kNotSet:
			outputh = inputh;
		}
		// ChecShaderPass::k if last pass
		if (lastPass) {
			if (p.yScaleMethod == ShaderPass::kNotSet) {
				outputh = h;
			} else {
				implicitPass = true;
			}
		}

		if (!lastPass || implicitPass) {
			glGenTextures(1, &outputtex);
			glBindTexture(GL_TEXTURE_2D, outputtex);
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, p.filter));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p.filter));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGB,
				outputw, outputh,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glGenFramebuffersEXT(1, &fbo);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
			glFramebufferTexture2DEXT(
				GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
				GL_TEXTURE_2D, outputtex, 0);
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
				error("Framebuffer creation failed with %x", status);
			}
			glPushMatrix();
			glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);
			glLoadIdentity();
			glClear(GL_COLOR_BUFFER_BIT);
			glViewport(0,0,outputw, outputh);
		}
		glDisable(GL_BLEND);

		// Set up current Texture
		GLCALL(glActiveTexture(GL_TEXTURE1));
		GLCALL(glBindTexture(GL_TEXTURE_2D, currentTexture));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, p.filter));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p.filter));

		// Set up orig Texture
		GLCALL(glActiveTexture(GL_TEXTURE0));
		GLCALL(glBindTexture(GL_TEXTURE_2D, origTexture));

		glUseProgram(p.program);

		glUniform1i(p.textureLoc, 1);
		glUniform1i(p.frameCountLoc, _frameCount);
		glUniform2f(p.inputSizeLoc, inputw, inputh);
		glUniform2f(p.outputSizeLoc, outputw, outputh);
		glUniform2f(p.textureSizeLoc, texw, texh);

		// Use non-standard uniforms
		glUniform2f(p.origInputSizeLoc, origInputw, origInputh);
		glUniform2f(p.origTextureSizeLoc, origTexw, origTexh);
		glUniform1i(p.origTextureLoc, 0);

		const GLfloat vertices[] = {
			0, 0,
			outputw, 0,
			0, outputh,
			outputw, outputh
		};
		const GLfloat texCoords[] = {
			0, 0,
			inputw/texw, 0,
			0, inputh/texh,
			inputw/texw, inputh/texh,
		};
		GLCALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
		GLCALL(glVertexPointer(2, GL_FLOAT, 0, vertices));

		GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		glUseProgram(0);

		inputw = outputw;
		inputh = outputh;
		texw = outputw;
		texh = outputh;
		if (i)
			glDeleteTextures(1, &currentTexture);
		if (!lastPass || implicitPass) {
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			glDeleteFramebuffersEXT(1, &fbo);
			glPopMatrix();
			glPopAttrib();
		}
		currentTexture = outputtex;
	}
	if (implicitPass) {
		const GLshort vertices[] = {
			0, 0,
			w, 0,
			0, h,
			w, h
		};
		const GLfloat texCoords[] = {
			0, 0,
			inputw/texw, 0,
			0, inputh/texh,
			inputw/texw, inputh/texh,
		};
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		GLCALL(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
		GLCALL(glVertexPointer(2, GL_SHORT, 0, vertices));
		GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		glDeleteTextures(1, &currentTexture);
	}
	glEnable(GL_BLEND);

	_frameCount++;
}

void OpenGLGraphicsManager::drawTexture(Texture *texture, GLshort x, GLshort y, GLshort w, GLshort h) {
	// First update any potentional changes.
	texture->updateTexture();

	// Set the texture.
	GLCALL(glBindTexture(GL_TEXTURE_2D, texture->getName()));

	// Calculate the texture rect that will be drawn.
	const GLfloat texWidth = texture->getDrawWidth();
	const GLfloat texHeight = texture->getDrawHeight();
	const GLfloat texcoords[4*2] = {
		0,        0,
		texWidth, 0,
		0,        texHeight,
		texWidth, texHeight
	};
	GLCALL(glTexCoordPointer(2, GL_FLOAT, 0, texcoords));

	// Calculate the screen rect where the texture will be drawn.
	const GLfloat vertices[4*2] = {
		x,     y,
		x + w, y,
		x,     y + h,
		x + w, y + h
	};
	GLCALL(glVertexPointer(2, GL_FLOAT, 0, vertices));

	// Draw the texture to the screen buffer.
	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

} // End of namespace OpenGL
