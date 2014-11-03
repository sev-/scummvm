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

#ifndef BACKENDS_PLATFORM_IPHONE_IPHONE_COMMON_H
#define BACKENDS_PLATFORM_IPHONE_IPHONE_COMMON_H

#include "common/rect.h"
#include "graphics/surface.h"

enum InputEvent {
	kInputMouseDown,
	kInputMouseUp,
	kInputMouseDragged,
	kInputMouseSecondDragged,
	kInputMouseSecondDown,
	kInputMouseSecondUp,
	kInputOrientationChanged,
	kInputKeyPressed,
	kInputApplicationSuspended,
	kInputApplicationResumed,
	kInputSwipe
};

enum ScreenOrientation {
	kScreenOrientationPortrait,
	kScreenOrientationLandscape,
	kScreenOrientationFlippedLandscape
};

enum UIViewSwipeDirection {
	kUIViewSwipeUp = 1,
	kUIViewSwipeDown = 2,
	kUIViewSwipeLeft = 4,
	kUIViewSwipeRight = 8
};

enum GraphicsModes {
	kGraphicsModeLinear = 0,
	kGraphicsModeNone = 1
};

class Texture {
public:
    /**
     * Create a new texture with the specific internal format.
     *
     * @param glIntFormat The internal format to use.
     * @param glFormat    The input format.
     * @param glType      The input type.
     * @param format      The format used for the texture input.
     */
    Texture(GLenum glIntFormat, GLenum glFormat, GLenum glType, const Graphics::PixelFormat &format);
    virtual ~Texture();
    
    /**
     * Destroy the OpenGL texture name.
     */
    void releaseInternalTexture();
    
    /**
     * Create the OpenGL texture name and flag the whole texture as dirty.
     */
    void recreateInternalTexture();
    
    /**
     * Enable or disable linear texture filtering.
     *
     * @param enable true to enable and false to disable.
     */
    void enableLinearFiltering(bool enable);
    
    /**
     * Allocate texture space for the desired dimensions. This wraps any
     * handling of requirements for POT textures.
     *
     * @param width  The desired logical width.
     * @param height The desired logical height.
     */
    virtual void allocate(uint width, uint height);
    
    void copyRectToTexture(uint x, uint y, uint w, uint h, const void *src, uint srcPitch);
    
    void fill(uint32 color);
    
    void flagDirty() { _allDirty = true; }
    bool isDirty() const { return _allDirty || !_dirtyArea.isEmpty(); }
    
    uint getWidth() const { return _userPixelData.w; }
    uint getHeight() const { return _userPixelData.h; }
    GLuint getTextureWidth() const { return _textureData.w; }
    GLuint getTextureHeight() const { return _textureData.h; }
    GLfloat getDrawWidth() { return (GLfloat)_userPixelData.w / _textureData.w; }
    GLfloat getDrawHeight() { return (GLfloat)_userPixelData.h / _textureData.h; }
    
    /**
     * @return The hardware format of the texture data.
     */
    const Graphics::PixelFormat &getHardwareFormat() const { return _format; }
    
    /**
     * @return The logical format of the texture data.
     */
    virtual Graphics::PixelFormat getFormat() const { return _format; }
    
    virtual Graphics::Surface *getSurface() { return &_userPixelData; }
    virtual const Graphics::Surface *getSurface() const { return &_userPixelData; }
    
    /**
     * @return Whether the texture data is using a palette.
     */
    virtual bool hasPalette() const { return false; }
    
    virtual void setPalette(uint start, uint colors, const byte *palData) {}
    
    virtual void *getPalette() { return 0; }
    virtual const void *getPalette() const { return 0; }
    
    /**
     * Query texture related OpenGL information from the context. This only
     * queries the maximum texture size for now.
     */
    static void queryTextureInformation();
    
    /**
     * @return Return the maximum texture dimensions supported.
     */
    static GLint getMaximumTextureSize() { return _maxTextureSize; }
    
    virtual void updateTexture();
    
    Common::Rect getDirtyArea() const;
    
public:
    GLuint getName() { return _glTexture; }
    
private:
    const GLenum _glIntFormat;
    const GLenum _glFormat;
    const GLenum _glType;
    const Graphics::PixelFormat _format;
    
    GLint _glFilter;
    GLuint _glTexture;
    
    Graphics::Surface _textureData;
    Graphics::Surface _userPixelData;
    
    bool _allDirty;
    Common::Rect _dirtyArea;
    void clearDirty() { _allDirty = false; _dirtyArea = Common::Rect(); }
    
    static GLint _maxTextureSize;
};


struct VideoContext {
	VideoContext() : asprectRatioCorrection(), screenWidth(), screenHeight(), overlayVisible(false),
	                 overlayWidth(), overlayHeight(), mouseX(), mouseY(),
	                 mouseHotspotX(), mouseHotspotY(), mouseWidth(), mouseHeight(),
	                 mouseIsVisible(), graphicsMode(kGraphicsModeLinear), shakeOffsetY() {
	}

	// Game screen state
	bool asprectRatioCorrection;
	uint screenWidth, screenHeight;
	Graphics::Surface screenTexture;

	// Overlay state
	bool overlayVisible;
	uint overlayWidth, overlayHeight;
	Graphics::Surface overlayTexture;

	// Mouse cursor state
	uint mouseX, mouseY;
	int mouseHotspotX, mouseHotspotY;
	uint mouseWidth, mouseHeight;
	bool mouseIsVisible;
	Graphics::Surface mouseTexture;

	// Misc state
	GraphicsModes graphicsMode;
	int shakeOffsetY;
};

struct InternalEvent {
	InternalEvent() : type(), value1(), value2() {}
	InternalEvent(InputEvent t, int v1, int v2) : type(t), value1(v1), value2(v2) {}

	InputEvent type;
	int value1, value2;
};

// On the ObjC side
void iPhone_updateScreen();
bool iPhone_fetchEvent(InternalEvent *event);
const char *iPhone_getDocumentsDir();
bool iPhone_isHighResDevice();
const char *iPhone_getMainBundleDirectory();

uint getSizeNextPOT(uint size);

#endif
