/*
 * OpenGLESHelper.h
 *
 *  Created on: Apr 18, 2013
 *      Author: omergilad
 */

#ifndef OPENGLESHELPER_H_
#define OPENGLESHELPER_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "backends/platform/android/OpenGLESDebug.h"

class ShaderProgram {

public:

	GLuint mProgramHandle;

	GLint mPositionAttributeHandle;
	GLint mTexCoordAttributeHandle;
	GLint mTextureUniformHandle;
	GLint mTextureSizeUniformHandle;
	GLint mInputSizeUniformHandle;
	GLint mOutputSizeUniformHandle;
	GLint mAlphaFactorUniformHandle;
	GLint mTextureFractUniformHandle;




	GLuint mVertexShader;
	GLuint mFragmentShader;

	virtual ~ShaderProgram();

};

class OpenGLESHelper {

public:

	static GLuint loadShader(GLenum shaderType, const char* pSource);

	static ShaderProgram* createProgram(const char* pVertexSource,
			const char* pFragmentSource);

	static const char DEFAULT_VERTEX_SHADER[];
	static const char DEFAULT_FRAGMENT_SHADER[];

	static const char BLACK_VERTEX_SHADER[];
		static const char BLACK_FRAGMENT_SHADER[];

private:

	static void dumpShaderLog(GLuint shader);
	static void dumpProgramLog(GLuint program);

};

#endif /* OPENGLESHELPER_H_ */
