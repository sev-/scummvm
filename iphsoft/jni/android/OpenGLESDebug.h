/*
 * OpenGLESDebug.h
 *
 *  Created on: Apr 18, 2013
 *      Author: omergilad
 */

#ifndef OPENGLESDEBUG_H_
#define OPENGLESDEBUG_H_





// #define ANDROID_DEBUG_GL
// #define ANDROID_DEBUG_GL_CALLS
// #define ANDROID_DEBUG_GL_MEASURE_RENDER_TIME


// #define ANDROID_DEBUG_GL_THREAD


#ifdef ANDROID_DEBUG_GL
extern void checkGlError(const char *expr, const char *file, int line);

#ifdef ANDROID_DEBUG_GL_CALLS
#define GLCALLLOG(x, before) \
	do { \
		if (before) \
			LOGD("calling '%s' (%s:%d)", x, __FILE__, __LINE__); \
		else \
			LOGD("returned from '%s' (%s:%d)", x, __FILE__, __LINE__); \
	} while (false)
#else
#define GLCALLLOG(x, before) do {  } while (false)
#endif

#define GLCALL(x) \
	do { \
		GLCALLLOG(#x, true); \
		(x); \
		GLCALLLOG(#x, false); \
		checkGlError(#x, __FILE__, __LINE__); \
	} while (false)



#else
#define GLCALL(x) do { (x); } while (false)
#endif

#ifdef ANDROID_DEBUG_GL_THREAD
#define GLTHREADCHECK \
	do { \
		assert(pthread_self() == _main_thread); \
	} while (false)
#else
#define GLTHREADCHECK do {  } while (false)
#endif



#ifdef ANDROID_DEBUG_GL
extern void checkFrameRate();

#define CHECK_FRAME_RATE checkFrameRate();
#else
#define CHECK_FRAME_RATE do {  } while (false)
#endif

#endif /* OPENGLESDEBUG_H_ */
