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

#ifndef BACKENDS_MUTEX_ABSTRACT_H
#define BACKENDS_MUTEX_ABSTRACT_H

#include "common/system.h"

#if defined(USE_NULL_DRIVER)
#define CREATE_MUTEX()
#define LOCK_MUTEX(mutex)
#define UNLOCK_MUTEX(mutex)
#define DELETE_MUTEX(mutex)

#elif defined(__ANDROID__) || defined(IPHONE)

#include <pthread.h>

inline OSystem::MutexRef CREATE_MUTEX() {
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_t *mutex = new pthread_mutex_t;

	if (pthread_mutex_init(mutex, &attr) != 0) {
		warning("pthread_mutex_init() failed");
		delete mutex;
		return NULL;
	}

	return (OSystem::MutexRef)mutex;
}

#define LOCK_MUTEX(mutex) \
	if (pthread_mutex_lock((pthread_mutex_t *)mutex) != 0) \
		warning("pthread_mutex_lock() failed");

#define UNLOCK_MUTEX(mutex) \
	if (pthread_mutex_unlock((pthread_mutex_t *)mutex) != 0) \
		warning("pthread_mutex_unlock() failed");

void DELETE_MUTEX(OSystem::MutexRef mutex) {
	pthread_mutex_t *m = (pthread_mutex_t *)mutex;

	if (pthread_mutex_destroy(m) != 0)
		warning("pthread_mutex_destroy() failed");
	else
		delete m;
}

#elif defined(SDL_BACKEND)

#include "backends/platform/sdl/sdl-sys.h"

/**
 * Create a new mutex.
 * @return the newly created mutex, or 0 if an error occurred.
 */
#define CREATE_MUTEX() (OSystem::MutexRef) SDL_CreateMutex()

/**
 * Lock the given mutex.
 *
 * @note ScummVM code assumes that the mutex implementation supports
 * recursive locking. That is, a thread may lock a mutex twice w/o
 * deadlocking. In case of a multilock, the mutex has to be unlocked
 * as many times as it was locked befored it really becomes unlocked.
 *
 * @param mutex	the mutex to lock.
 */
#define LOCK_MUTEX(mutex) SDL_mutexP((SDL_mutex *)mutex);

/**
 * Unlock the given mutex.
 * @param mutex	the mutex to unlock.
 */
#define UNLOCK_MUTEX(mutex)  SDL_mutexV((SDL_mutex *)mutex);

/**
 * Delete the given mutex. Make sure the mutex is unlocked before you delete it.
 * If you delete a locked mutex, the behavior is undefined, in particular, your
 * program may crash.
 * @param mutex	the mutex to delete.
 */
#define DELETE_MUTEX(mutex) SDL_DestroyMutex((SDL_mutex *)mutex);

#endif

#endif
