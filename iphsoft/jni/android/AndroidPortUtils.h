/*
 * AndroidPortUtils.h
 *
 *  Created on: Jan 18, 2013
 *      Author: omergilad
 */

#ifndef ANDROIDPORTUTILS_H_
#define ANDROIDPORTUTILS_H_


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

#define FORBIDDEN_SYMBOL_ALLOW_ALL


#include "backends/platform/android/android.h"
#include <sys/time.h>



class AndroidPortUtils {

public:

	static void dumpBytesToFile(const byte* bytes, uint32 length, const char* filename);

	static inline uint64 getTimeOfDayMillis()
	{
		struct timeval tim;
		gettimeofday(&tim, NULL);
		return (tim.tv_sec * 1000 + tim.tv_usec / 1000);
	}

private:
	AndroidPortUtils();
	virtual ~AndroidPortUtils();
};

#endif /* ANDROIDPORTUTILS_H_ */
