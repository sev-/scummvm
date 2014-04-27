/*
 * OpenGLESHelper.cpp
 *
 *  Created on: Apr 18, 2013
 *      Author: omergilad
 */

#include <stdio.h>
#include <stdlib.h>

#include "OpenGLESHelper.h"

#include "backends/platform/android/loghelper.h"
#include "backends/platform/android/AndroidPortUtils.h"

#ifdef ANDROID_DEBUG_GL
static const char *getGlErrStr(GLenum error) {
	switch (error) {
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
//		case GL_STACK_OVERFLOW:
//		return "GL_STACK_OVERFLOW";
//		case GL_STACK_UNDERFLOW:
//		return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	}

	static char buf[40];
	snprintf(buf, sizeof(buf), "(Unknown GL error code %d)", error);

	return buf;
}

void checkGlError(const char *expr, const char *file, int line) {
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		LOGE("GL ERROR: %s on %s (%s:%d)",
				getGlErrStr(error), expr, file, line);
}

unsigned long gPreviousFrameTime = 0;
float gAverageFrameRate = 0.0;
unsigned int gCounter = 0;

void checkFrameRate() {
	unsigned long currentTime = AndroidPortUtils::getTimeOfDayMillis();

	if (gPreviousFrameTime == 0) {
		// Only for first frame
		gPreviousFrameTime = currentTime;
		return;
	}

	float lastFrameRate = (float) 1000 / (currentTime - gPreviousFrameTime);
	gPreviousFrameTime = currentTime;
	gAverageFrameRate = 0.9 * gAverageFrameRate + 0.1 * lastFrameRate;

	if (gCounter++ == 10) {
		LOGD("average frame rate: %f", gAverageFrameRate);
		gCounter = 0;
	}
}

#endif

const char OpenGLESHelper::DEFAULT_VERTEX_SHADER[] =
		"precision highp float;\n"
		"attribute vec4 vPosition;\n"
				"attribute vec2 a_TexCoordinate;\n"
				"varying vec2 v_TexCoordinate;\n"
				"void main() {\n"
				"	v_TexCoordinate = a_TexCoordinate;\n"
				"  gl_Position = vPosition;\n"
				"}\n";

const char OpenGLESHelper::DEFAULT_FRAGMENT_SHADER[] =
		"precision highp float;\n"
				"uniform sampler2D rubyTexture;\n"
				"uniform float alphaFactor;\n"
				"varying vec2 v_TexCoordinate;\n"
				" "
				"void main() {\n"
				""
				"  gl_FragColor = texture2D(rubyTexture, v_TexCoordinate);\n"
				"  gl_FragColor.a *= alphaFactor;\n"
				"}\n";

const char OpenGLESHelper::BLACK_VERTEX_SHADER[] = "attribute vec4 vPosition;\n"
		"void main() {\n"
		"  gl_Position = vPosition;\n"
		"}\n";

const char OpenGLESHelper::BLACK_FRAGMENT_SHADER[] =
		"precision mediump float;\n"
				"void main() {\n"
				"  gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
				"}\n";

GLuint OpenGLESHelper::loadShader(GLenum shaderType, const char* pSource) {

	LOGD("OpenGLESHelper::loadShader: type %d", shaderType);

#ifdef ANDROID_DEBUG_GL
	LOGD("OpenGLESHelper::loadShader: source:");
	log_lines(pSource);
#endif

	GLuint shader;
	GLCALL(shader = glCreateShader(shaderType));
	if (shader) {
		GLCALL(glShaderSource(shader, 1, &pSource, NULL));
		GLCALL(glCompileShader(shader));
		GLint compiled = 0;
		GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled));
		if (!compiled) {
			LOGE("Could not compile shader");

#ifdef ANDROID_DEBUG_GL
			dumpShaderLog(shader);
#endif

			GLCALL(glDeleteShader(shader));
			shader = 0;
		}
#ifdef ANDROID_DEBUG_GL
		else {
			dumpShaderLog(shader);
		}
#endif

	}

	return shader;
}
void OpenGLESHelper::dumpShaderLog(GLuint shader) {
	GLint infoLen = 0;
	GLCALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen));

	// Workaround
	if (infoLen == 0)
		infoLen = 4096;

	if (infoLen) {
		char* buf = (char*) malloc(infoLen);
		if (buf) {
			GLCALL(glGetShaderInfoLog(shader, infoLen, NULL, buf));
			LOGI("Log for shader %d:\n%s\n", shader, buf);
			free(buf);
		}

	}
}

void OpenGLESHelper::dumpProgramLog(GLuint program) {

	GLint infoLen = 0;
	GLCALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen));

	// Workaround
//	if (infoLen == 0)
//		infoLen = 4096;

	if (infoLen) {
		char* buf = (char*) malloc(infoLen);
		if (buf) {
			GLCALL(glGetProgramInfoLog(program, infoLen, NULL, buf));
			LOGI("Log for Program %d:\n%s\n", program, buf);
			free(buf);
		}
	}
}

ShaderProgram* OpenGLESHelper::createProgram(const char* pVertexSource,
		const char* pFragmentSource) {
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
		return 0;
	}

	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) {
		return 0;
	}

	GLuint programId;
	GLCALL(programId = glCreateProgram());
	if (programId) {
		GLCALL(glAttachShader(programId, vertexShader));
		GLCALL(glAttachShader(programId, pixelShader));
		GLCALL(glLinkProgram(programId));
		GLint linkStatus = GL_FALSE;
		GLCALL(glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus));
		if (linkStatus != GL_TRUE) {
			LOGE("Could not link program, retrieving log...");

#ifdef ANDROID_DEBUG_GL
			dumpProgramLog(programId);
#endif

			GLCALL(glDeleteProgram(programId));
			return 0;
		}
#ifdef ANDROID_DEBUG_GL
		else {
			dumpProgramLog(programId);
		}
#endif
	} else {
		return 0;
	}

	ShaderProgram* program = new ShaderProgram;
	program->mProgramHandle = programId;

	GLCALL(
			program->mPositionAttributeHandle = glGetAttribLocation(programId, "vPosition"));
	GLCALL(
			program->mTexCoordAttributeHandle = glGetAttribLocation(programId, "a_TexCoordinate"));
	GLCALL(
			program->mTextureUniformHandle = glGetUniformLocation(programId, "rubyTexture"));
	GLCALL(
			program->mTextureSizeUniformHandle = glGetUniformLocation(programId, "rubyTextureSize"));
	GLCALL(
			program->mInputSizeUniformHandle = glGetUniformLocation(programId, "rubyInputSize"));
	GLCALL(
			program->mOutputSizeUniformHandle = glGetUniformLocation(programId, "rubyOutputSize"));
	GLCALL(
			program->mAlphaFactorUniformHandle = glGetUniformLocation(programId, "alphaFactor"));
	GLCALL(
			program->mTextureFractUniformHandle = glGetUniformLocation(programId, "rubyTextureFract"));

	return program;
}

ShaderProgram::~ShaderProgram() {

	GLCALL(glDeleteShader(mVertexShader));
	GLCALL(glDeleteShader(mFragmentShader));

	GLCALL(glDeleteProgram(mProgramHandle));
}
