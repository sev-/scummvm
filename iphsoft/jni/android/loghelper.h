/*
 * loghelper.h
 *
 *  Created on: Apr 18, 2013
 *      Author: omergilad
 */

#ifndef LOGHELPER_H_
#define LOGHELPER_H_

#include <string.h>
#include <android/log.h>


extern const char *android_log_tag;

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, android_log_tag, fmt, ## args)
#define LOGD(fmt, args...)// _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define LOGI(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define LOGW(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define LOGE(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

// Override assert()
#ifndef NDEBUG
#define assert(e) assertFunc(e, #e);
#else
#define assert(e) {}
#endif

// Override printf
#define printf(...) LOGD(__VA_ARGS__);

inline void assertFunc(bool condition, const char* condStr)
{
	if (!condition)
	{
		__android_log_assert(condStr, android_log_tag, "assertion failure: %s %d", __FILE__, __LINE__);
	}
}

inline void log_lines(const char* str)
{
	char line[1024];
	const char* ptr = str;

	while (*ptr != '\0')
	{
		++ptr;

		if (*ptr == '\n' || *ptr == '\0')
		{
			int length = ptr - str;
			if (length > 1023)
			{
				length = 1023;
			}
			memcpy(line, str, length);
			line[length] = '\0';

			LOGD("%s", line);

			str = ptr + 1;
		}
	}
}


#endif /* LOGHELPER_H_ */
