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

#include "base/main.h"
#include "graphics/surface.h"

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"
#include "common/tokenizer.h"

#include "backends/platform/android/texture.h"
#include "backends/platform/android/android.h"
#include "backends/platform/android/jni.h"

#include "backends/platform/android/AndroidPortAdditions.h"

// Supported GL extensions
static bool npot_supported = false;

static inline GLfixed xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}

template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * CHAR_BIT; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

void GLESBaseTexture::initGLExtensions() {
	const char *ext_string = reinterpret_cast<const char *>(glGetString(
			GL_EXTENSIONS));
	const char *renderer = reinterpret_cast<const char *>(glGetString(
			GL_RENDERER));

	LOGI("Extensions: %s", ext_string);
	LOGI("Renderer: %s", renderer);

	Common::StringTokenizer tokenizer(ext_string, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		if (token == "GL_ARB_texture_non_power_of_two")
			npot_supported = true;
	}

	LOGD("initGLExtensions:: %s", glGetString(GL_VERSION));
}

GLESBaseTexture::GLESBaseTexture(GLenum glFormat, GLenum glType,
		Graphics::PixelFormat pixelFormat)
		:
				_glFormat(glFormat),
				_glType(glType),
				_glFilter(GL_NEAREST),
				_texture_name(0),
				_surface(),
				_texture_width(0),
				_texture_height(0),
				mScaledTextureWidth(0),
				mScaledTextureHeight(0),
				_draw_rect(),
				_all_dirty(false),
				_dirty_rect(),
				_pixelFormat(pixelFormat),
				mIsGameTexture(false),
				mDefaultProgram(0),
				mFramebuffer(0),
				mScaledTexture(0),

				_palettePixelFormat() {
	GLCALL(glGenTextures(1, &_texture_name));
	GLCALL(glActiveTexture(GL_TEXTURE0));

	AndroidPortAdditions::instance()->initGLESResources();
}

GLESBaseTexture::~GLESBaseTexture() {
	release();
}

void GLESBaseTexture::release() {
	if (_texture_name) {
		LOGD("Destroying texture %u", _texture_name);

		GLCALL(glDeleteTextures(1, &_texture_name));
		_texture_name = 0;
	}

	if (mFramebuffer) {
		GLCALL(glDeleteFramebuffers(1, &mFramebuffer));
		mFramebuffer = 0;
	}

	if (mScaledTexture) {
		GLCALL(glDeleteTextures(1, &mScaledTexture));
		mScaledTexture = 0;
	}
}

void GLESBaseTexture::reinit() {
	GLCALL(glGenTextures(1, &_texture_name));

	initSize();

	setDirty();
}

void GLESBaseTexture::initSize() {

	// TODO
	//	setLinearFilter(true);

	// Allocate room for the texture now, but pixel data gets uploaded
	// later (perhaps with multiple TexSubImage2D operations).
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
	GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	LOGD("GLESBaseTexture::initSize(): %d %d", _texture_width, _texture_height);

	GLCALL(
			glTexImage2D(GL_TEXTURE_2D, 0, _glFormat, _texture_width, _texture_height, 0, _glFormat, _glType, 0));

	AndroidPortAdditions::instance()->setGameTextureInfo(_texture_name,
			_texture_width, _texture_height);

}

void GLESBaseTexture::setLinearFilter(bool value) {
	if (value)
		_glFilter = GL_LINEAR;
	else
		_glFilter = GL_NEAREST;

	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
}

void GLESBaseTexture::allocBuffer(GLuint w, GLuint h) {
	_surface.w = w;
	_surface.h = h;
	_surface.format = _pixelFormat;

	if (w == _texture_width && h == _texture_height)
		return;

	initTextureSize();
}

void GLESBaseTexture::initFramebuffer(GLuint w, GLuint h) {
	LOGD("GLESBaseTexture::initFramebuffer: %d %d", w, h);

	if (mFramebuffer != 0) {
		return;
	}

	// Generate framebuffer and target texture
	GLCALL(glGenFramebuffers(1, &mFramebuffer));
	GLCALL(glGenTextures(1, &mScaledTexture));

	GLCALL(glBindTexture(GL_TEXTURE_2D, mScaledTexture));

	// The minimum framebuffer size should also match the LQ shader scaling factor
	float lqScalingFactor = AndroidPortAdditions::instance()
			->getLQShaderScalingFactor();
	w = MAX(w, (GLuint) (lqScalingFactor * GAME_SCREEN_WIDTH));
	h = MAX(h, (GLuint) (lqScalingFactor * GAME_SCREEN_HEIGHT));

	// Limit the max resolution
	w = MIN(w, (GLuint)AndroidPortAdditions::instance()->getShaderScalingMaxResolutionW());
	h = MIN(h, (GLuint)AndroidPortAdditions::instance()->getShaderScalingMaxResolutionH());

	// Init the target texture size
	if (npot_supported) {
		mScaledTextureWidth = w;
		mScaledTextureHeight = h;
	} else {
		mScaledTextureWidth = nextHigher2(w);
		mScaledTextureHeight = nextHigher2(h);
	}

	GLCALL(
			glTexImage2D(GL_TEXTURE_2D, 0, _glFormat, mScaledTextureWidth, mScaledTextureHeight, 0, _glFormat, _glType, 0));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	// Attach the texture to the framebuffer
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer));
	GLCALL(
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mScaledTexture, 0));

	// check for framebuffer completion
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOGE("Unable to create framebuffer");
	}

	LOGD("GLESBaseTexture::initFramebuffer: scaled texture size: %d %d",
			mScaledTextureWidth, mScaledTextureHeight);
}

void GLESBaseTexture::initTextureSize() {
	int scalingFactor = AndroidPortAdditions::instance()->getScalingFactor();
	if (AndroidPortAdditions::instance()->getScalingOption() != SCALING_OPTION_SOFT) {
		scalingFactor = 1;
	}

	if (npot_supported) {
		_texture_width = _surface.w * scalingFactor;
		_texture_height = _surface.h * scalingFactor;
	} else {
		_texture_width = nextHigher2(_surface.w * scalingFactor);
		_texture_height = nextHigher2(_surface.h * scalingFactor);
	}

	initSize();
}

void GLESBaseTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {

//	LOGD("GLESBaseTexture::drawTexture %d %d %d %d", x ,y ,w ,h);

	ShaderProgram* shader;

	int scalingFactor = AndroidPortAdditions::instance()->getScalingFactor();
	int scalingOption = AndroidPortAdditions::instance()->getScalingOption();
	if (scalingOption == SCALING_OPTION_NONE) {
		scalingFactor = 1;
		shader = AndroidPortAdditions::instance()->getDefaultShaderProgram();
	} else if (scalingOption == SCALING_OPTION_SOFT) {
		shader = AndroidPortAdditions::instance()->getDefaultShaderProgram();
	} else if (scalingOption == SCALING_OPTION_SHADER
			|| scalingOption == SCALING_OPTION_LQ_SHADER) {
		scalingFactor = 1;
		shader = AndroidPortAdditions::instance()->getScalerShaderProgram();
	} else {
		LOGE("GLESBaseTexture::drawTexture: invalid scaling option");
		return;
	}

	GLfloat dirtyRectLeft, dirtyRectTop, dirtyRectWidth, dirtyRectHeight;
	if (scalingOption == SCALING_OPTION_SHADER
			|| scalingOption == SCALING_OPTION_LQ_SHADER) {
		dirtyRectLeft = _dirty_rect.left / (float) _surface.w;
		dirtyRectTop = _dirty_rect.top / (float) _surface.h;
		dirtyRectWidth = _dirty_rect.width() / (float) _surface.w;
		dirtyRectHeight = _dirty_rect.height() / (float) _surface.h;
	} else {
		dirtyRectLeft = 0;
		dirtyRectTop = 0;
		dirtyRectWidth = 1;
		dirtyRectHeight = 1;
	}

	const GLfloat tex_width = _surface.w * scalingFactor
			/ (GLfloat) _texture_width;
	const GLfloat tex_height = _surface.h * scalingFactor
			/ (GLfloat) _texture_height;

	GLfloat texRectX = dirtyRectLeft * tex_width;
	GLfloat texRectY = dirtyRectTop * tex_height;
	GLfloat texRectW = dirtyRectWidth * tex_width;
	GLfloat texRectH = dirtyRectHeight * tex_height;

	const GLfloat texcoords[] = { texRectX, texRectY, texRectX + texRectW,
			texRectY, texRectX, texRectY + texRectH, texRectX + texRectW,
			texRectY + texRectH, };

	GLfloat vX = dirtyRectLeft * 2.0 - 1.0;
	GLfloat vY = dirtyRectTop * (-2.0) + 1.0;
	GLfloat vW = dirtyRectWidth * 2.0;
	GLfloat vH = dirtyRectHeight * 2.0;

	const GLfloat vertices[] = { vX, vY, vX + vW, vY, vX, vY - vH, vX + vW, vY
			- vH };

	//LOGD("GLESBaseTexture::drawTexture texcoords %f %f %f %f", texRectX ,texRectY ,texRectW ,texRectH);
	//LOGD("GLESBaseTexture::drawTexture vertices %f %f %f %f", vX ,vY ,vW ,vH);

	// Bind the source texture
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

	// Bind the framebuffer
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer));

	uint16 maxResW = AndroidPortAdditions::instance()->getShaderScalingMaxResolutionW();
	uint16 maxResH = AndroidPortAdditions::instance()->getShaderScalingMaxResolutionH();

	if (scalingOption != SCALING_OPTION_LQ_SHADER) {
		// Hard limit on HQ hardware scaling resolution - render to a limited part of the framebuffer

		if (w > maxResW
				|| h > maxResH) {
			GLCALL(
					glViewport(0, 0, MIN(w, (GLshort)maxResW), MIN(h, (GLshort)maxResH)));
		}
	} else {
		// limit on LQ hardware scaling resolution - render according to a whole scale factor
		float lqScaleFactor = AndroidPortAdditions::instance()
				->getLQShaderScalingFactor();
		uint16 limitWidth = MIN((int)(GAME_SCREEN_WIDTH * lqScaleFactor),
				(int)maxResW);
		uint16 limitHeight = MIN((int)(GAME_SCREEN_HEIGHT * lqScaleFactor),
				(int)maxResH);

		GLCALL( glViewport(0, 0, limitWidth, limitHeight));

	}

	// Use the program
	GLCALL(glUseProgram(shader->mProgramHandle));

	// Set uniforms
	GLCALL(glUniform1i(shader->mTextureUniformHandle, 0));
	GLCALL(
			glUniform2f(shader->mTextureSizeUniformHandle, _texture_width, _texture_height));
	GLCALL(
			glUniform2f(shader->mTextureFractUniformHandle, 1 / (GLfloat)_texture_width, 1 / (GLfloat)_texture_height));

	GLCALL(
			glUniform2f(shader->mInputSizeUniformHandle, _surface.w, _surface.h));
	GLCALL(glUniform2f(shader->mOutputSizeUniformHandle, w, h));
	GLCALL(glUniform1f(shader->mAlphaFactorUniformHandle, 1.0));

	// Pass the position attributes
	GLCALL(
			glVertexAttribPointer(shader->mPositionAttributeHandle, 2, GL_FLOAT, false, 0, vertices));

	// Pass the texture attributes
	GLCALL(
			glVertexAttribPointer(shader->mTexCoordAttributeHandle, 2, GL_FLOAT, false, 0, texcoords));

	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	clearDirty();
}

void GLESBaseTexture::flushFramebuffer(GLshort x, GLshort y, GLshort w,
		GLshort h) {

//	LOGD("GLESBaseTexture::flushFramebuffer %d %d %d %d", x ,y ,w ,h);
	int scalingOption = AndroidPortAdditions::instance()->getScalingOption();

	uint16 maxResW = AndroidPortAdditions::instance()->getShaderScalingMaxResolutionW();
	uint16 maxResH = AndroidPortAdditions::instance()->getShaderScalingMaxResolutionH();

	// Hard limit on hardware scaling resolution - render only the needed part of the framebuffer (our limit)
	GLshort framebufferTextureW = w;
	GLshort framebufferTextureH = h;
	bool hardLimitResolution = false;
	if (scalingOption != SCALING_OPTION_LQ_SHADER) {
		if (w > maxResW
				|| h > maxResH) {

			framebufferTextureW = MIN(w,
					(GLshort) maxResW);
			framebufferTextureH = MIN(h,
					(GLshort) maxResH);

			hardLimitResolution = true;

		}
	} else  {
		// limit on LQ hardware scaling resolution - render according to a whole scale factor
		float lqScaleFactor = AndroidPortAdditions::instance()
				->getLQShaderScalingFactor();
		framebufferTextureW = MIN((int)(GAME_SCREEN_WIDTH * lqScaleFactor),
				(int)maxResW);
		framebufferTextureH = MIN((int)(GAME_SCREEN_HEIGHT * lqScaleFactor),
				(int)maxResH);

		hardLimitResolution = true;
	}

	const GLfloat tex_width = framebufferTextureW
			/ (GLfloat) mScaledTextureWidth;
	const GLfloat tex_height = framebufferTextureH
			/ (GLfloat) mScaledTextureHeight;

//	LOGD("GLESBaseTexture::flushFramebuffer: w h  %f %f", tex_width, tex_height);

	const GLfloat texcoords[] = { 0, 0, tex_width, 0, 0, tex_height, tex_width,
			tex_height, };

	const GLfloat vertices[] = { -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0 };

	// Restore viewport back to normal (if we limited resolution)
	if (hardLimitResolution) {
		GLCALL(glViewport(0, 0, w, h));
	}

	// Bind the source texture (scaled)
	GLCALL(glBindTexture(GL_TEXTURE_2D, mScaledTexture));

	// Use the program
	ShaderProgram* shader = AndroidPortAdditions::instance()
			->getDefaultShaderProgram();
	GLCALL(glUseProgram(shader->mProgramHandle));

	// Set uniforms
	GLCALL(glUniform1i(shader->mTextureUniformHandle, 0));
	GLCALL(
			glUniform2f(shader->mTextureSizeUniformHandle, mScaledTextureWidth, mScaledTextureHeight));
	GLCALL(glUniform2f(shader->mInputSizeUniformHandle, w, h));
	GLCALL(glUniform2f(shader->mOutputSizeUniformHandle, w, h));
	GLCALL(glUniform1f(shader->mAlphaFactorUniformHandle, 1.0));

	// Pass the position attributes
	GLCALL(
			glVertexAttribPointer(shader->mPositionAttributeHandle, 2, GL_FLOAT, false, 0, vertices));

	// Pass the texture attributes
	GLCALL(
			glVertexAttribPointer(shader->mTexCoordAttributeHandle, 2, GL_FLOAT, false, 0, texcoords));

	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
}

const Graphics::PixelFormat &GLESBaseTexture::getPixelFormat() const {
	return _pixelFormat;
}

GLESFakePaletteTexture::GLESFakePaletteTexture(GLenum glFormat, GLenum glType,
		Graphics::PixelFormat pixelFormat)
		:
				GLESBaseTexture(glFormat, glType, pixelFormat),
				_palette(0),
				_pixels(0),
				_oldPixels(0),
				_buf(0),
				_bufMemory(0),
				mFirstFrame(true) {
	_palettePixelFormat = pixelFormat;
	_fake_format = Graphics::PixelFormat::createFormatCLUT8();

	_palette = new uint16[256];
	assert(_palette);

	memset(_palette, 0, 256 * 2);
}

GLESFakePaletteTexture::~GLESFakePaletteTexture() {
	delete[] _bufMemory;
	delete[] _pixels;
	delete[] _oldPixels;
	delete[] _palette;
}

void GLESFakePaletteTexture::initSize() {
	GLESBaseTexture::initSize();

	mFirstFrame = true;
}

void GLESFakePaletteTexture::allocBuffer(GLuint w, GLuint h) {

	GLuint oldw = _surface.w;
	GLuint oldh = _surface.h;

	GLESBaseTexture::allocBuffer(w, h);

	_surface.format = Graphics::PixelFormat::createFormatCLUT8();
	_surface.pitch = w;

	if (_surface.w == oldw && _surface.h == oldh) {
		fillBuffer(0);
		return;
	}

	delete[] _bufMemory;
	delete[] _pixels;
	delete[] _oldPixels;

	_pixels = new byte[w * h];

	assert(_pixels);

	// fixup surface, for the outside this is a CLUT8 surface
	_surface.pixels = _pixels;

	fillBuffer(0);

	// The pixel buffer memory is initialized with 2 additional extended rows.
	// The actual used buffer is w * h, inside the initialized memory.
	// The reason for that is that the software scaler assumes 1 pixel padding, and might access memory out of bounds.
	// We don't care about the pixel padding content, and we don't want to check bounds during the SW scaler operation in order to optimize performance.
	uint32 bufMemoryLength = w * h + (w + 2) * 2;
	_bufMemory = new uint16[bufMemoryLength];
	memset(_bufMemory, 0, bufMemoryLength);

	// Set the actual buffer to point inside the allocated memory
	_buf = _bufMemory + w + 2;

	_oldPixels = new uint16[w * h];
	assert(_buf);
	assert(_oldPixels);
}

void GLESFakePaletteTexture::fillBuffer(uint32 color) {

	LOGD("GLESFakePaletteTexture::fillBuffer");

	assert(_surface.pixels);
	memset(_surface.pixels, color & 0xff, _surface.pitch * _surface.h);
	setDirty();
}

void GLESFakePaletteTexture::updateBuffer(GLuint x, GLuint y, GLuint w,
		GLuint h, const void *buf, int pitch_buf) {

	LOGD("GLESFakePaletteTexture::updateBuffer");

	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *) buf;
	byte *dst = _pixels + y * _surface.pitch + x;

	do {
		memcpy(dst, src, w);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

void GLESFakePaletteTexture::drawTexture(GLshort x, GLshort y, GLshort w,
		GLshort h) {

//	LOGD("GLESFakePaletteTexture: drawTexture: %d, %d, %d, %d", x, y, w ,h);

	// Solves an Adreno issue:
	// http://stackoverflow.com/questions/5161784/gldepthmaskgl-false-trashes-the-frame-buffer-on-some-gpus
	GLCALL(glClear(GL_DEPTH_BUFFER_BIT));

	if (mIsGameTexture)
		AndroidPortAdditions::instance()->beforeDrawTextureToScreen(surface());

	// If we are in auto-load state, we don't draw anything
	if (AndroidPortAdditions::instance()->isInAutoloadState()) {
		return;
	}

	if (_all_dirty) {
		_dirty_rect.top = 0;
		_dirty_rect.left = 0;
		_dirty_rect.bottom = _surface.h;
		_dirty_rect.right = _surface.w;

		_all_dirty = false;
	}

	if (!_dirty_rect.isEmpty()) {

		int scalingOption =
				AndroidPortAdditions::instance()->getScalingOption();

		// Either use the normal pixels, or the android port's modified pixels if needed
		byte* pixelsToDraw;
		if (mIsGameTexture
				&& AndroidPortAdditions::instance()->shouldUseModifiedGamePixels()) {
			pixelsToDraw = AndroidPortAdditions::instance()
					->getModifiedGamePixels();
		} else {
			pixelsToDraw = (byte*) _surface.pixels;
		}

		//	LOGD("GLESFakePaletteTexture::drawTexture: dirty rect before: %d %d %d %d", _dirty_rect.left, _dirty_rect.top, _dirty_rect.right, _dirty_rect.bottom);

		int16 dwidth = _dirty_rect.width();
		int16 dheight = _dirty_rect.height();

		byte *src = pixelsToDraw + _dirty_rect.top * _surface.pitch
				+ _dirty_rect.left;
		uint16 *dst = _buf;
		uint pitch_delta = _surface.pitch - dwidth;

		for (uint16 j = 0; j < dheight; ++j) {
			for (uint16 i = 0; i < dwidth; ++i)
				*dst++ = _palette[*src++];
			src += pitch_delta;
		}

		if (mFirstFrame
				|| AndroidPortAdditions::instance()->shouldMeasureRenderTime()) {

			// On the first frame (or when doing the shader test, or autoload), we don't calculate the dirty rect
			memcpy(_oldPixels, _buf, _surface.w * _surface.h * 2);
			mFirstFrame = false;
		} else if (scalingOption != SCALING_OPTION_NONE) {
			// Otherwise calculate the dirty part (optimization)
			calculateDirtyRect(_buf);
		}

		// Bind the texture
		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

		if (scalingOption == SCALING_OPTION_SOFT) {
			// Scale with soft scaler
			uint16* scaledOutput = AndroidPortAdditions::instance()->scale(
					(uint8*) _buf, _dirty_rect.left, _dirty_rect.top,
					_dirty_rect.width(), _dirty_rect.height());

			int scalingFactor = AndroidPortAdditions::instance()
					->getScalingFactor();

			// Adjust pointer to Y offset
			//		uint32 outputPitch = _surface.w * scalingFactor;
			//	scaledOutput += _dirty_rect.top * scalingFactor * outputPitch;

			GLCALL(
					glTexSubImage2D(GL_TEXTURE_2D, 0, _dirty_rect.left * scalingFactor, _dirty_rect.top * scalingFactor, _dirty_rect.width() * scalingFactor, _dirty_rect.height() * scalingFactor, _glFormat, _glType, scaledOutput));

		} else {
			GLCALL(
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _surface.w, _surface.h, _glFormat, _glType, _buf));
		}

	}

	// Draw the game texture to the framebuffer if needed
	if (!_dirty_rect.isEmpty()) {
		GLESBaseTexture::drawTexture(x, y, w, h);
	}

	// Bind the screen framebuffer
	GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	// Flush the framebuffer content to the screen
	GLESBaseTexture::flushFramebuffer(x, y, w, h);

	if (mIsGameTexture) {
		AndroidPortAdditions::instance()->onDrawTextureToScreen(x, y, w, h);
	}

}

void GLESFakePaletteTexture::calculateDirtyRect(uint16* pixels) {

	int gameWidth = _surface.w;
	int gameHeight = _surface.h;

	int left = gameWidth, right = 0, top = gameHeight, bottom = 0;

	for (int i = 0; i < gameHeight; ++i) {
		for (int j = 0; j < gameWidth; ++j) {
			// Compare pixels
			uint16 pNew = pixels[i * gameWidth + j];
			uint16 pOld = _oldPixels[i * gameWidth + j];

			if (pNew != pOld) {
				// Copy
				_oldPixels[i * gameWidth + j] = pNew;

				// Update dirty rect
				left = MIN(left, j);
				right = MAX(right, j);
				top = MIN(top, i);
				bottom = MAX(bottom, i);
			}
		}
	}

//	LOGD("GLESFakePaletteTexture::calculateDirtyRect: result: %d %d %d %d", left, top, right, bottom);

	if (right == 0 || bottom == 0) {
		_dirty_rect = Rect();
	} else {
		_dirty_rect = Rect(left, top, right, bottom);

		// Grow the rect by 2 pixels on each side to account for shader sampling (but maintain game screen dimension)
		_dirty_rect.grow(2);
		_dirty_rect.clip(gameWidth, gameHeight);
	}

//	LOGD("GLESFakePaletteTexture::calculateDirtyRect: dirty rect: %d %d %d %d", _dirty_rect.left, _dirty_rect.top, _dirty_rect.right, _dirty_rect.bottom);
}

const Graphics::PixelFormat &GLESFakePaletteTexture::getPixelFormat() const {
	return _fake_format;
}

GLESFakePalette565Texture::GLESFakePalette565Texture()
		:
				GLESFakePaletteTexture(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
						GLES565Texture::pixelFormat()) {
}

GLESFakePalette565Texture::~GLESFakePalette565Texture() {
}

#endif
