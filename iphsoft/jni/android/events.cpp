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

#include "common/events.h"
#include "common/error.h"
#include "backends/platform/android/android.h"
#include "backends/platform/android/jni.h"

#include "engine.h"

#include "backends/platform/android/AndroidPortAdditions.h"

// $ANDROID_NDK/platforms/android-9/arch-arm/usr/include/android/keycodes.h
// http://android.git.kernel.org/?p=platform/frameworks/base.git;a=blob;f=libs/ui/Input.cpp
// http://android.git.kernel.org/?p=platform/frameworks/base.git;a=blob;f=core/java/android/view/KeyEvent.java

#include "event_types.h"

// floating point. use sparingly
template<class T>
static inline T scalef(T in, float numerator, float denominator) {
	return static_cast<float>(in) * numerator / denominator;
}

static const int kQueuedInputEventDelay = 50;

void OSystem_Android::setupKeymapper() {
#ifdef ENABLE_KEYMAPPER
	using namespace Common;

	Keymapper *mapper = getEventManager()->getKeymapper();

	HardwareInputSet *inputSet = new HardwareInputSet();

	keySet->addHardwareInput(
			new HardwareInput("n", KeyState(KEYCODE_n), "n (vk)"));

	mapper->registerHardwareInputSet(inputSet);

	Keymap *globalMap = new Keymap(kGlobalKeymapName);
	Action *act;

	act = new Action(globalMap, "VIRT", "Display keyboard");
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, 0));

	mapper->addGlobalKeymap(globalMap);

	mapper->pushKeymap(kGlobalKeymapName);
#endif
}

void OSystem_Android::warpMouse(int x, int y) {
	ENTER("%d, %d", x, y);

	Common::Event e;

	e.type = Common::EVENT_MOUSEMOVE;
	e.mouse.x = x;
	e.mouse.y = y;

	clipMouse(e.mouse);

	lockMutex(_event_queue_lock);
	_event_queue.push(e);
	unlockMutex(_event_queue_lock);
}

void OSystem_Android::clipMouse(Common::Point &p) {
	const GLESBaseTexture *tex;

	tex = _game_texture;

	p.x = CLIP(p.x, int16(0), int16(tex->width() - 1));
	p.y = CLIP(p.y, int16(0), int16(tex->height() - 1));
}

void OSystem_Android::scaleMouse(Common::Point &p, int x, int y,
		bool deductDrawRect, bool touchpadMode) {
	const GLESBaseTexture *tex;

	tex = _game_texture;

	const Common::Rect &r = tex->getDrawRect();

	if (touchpadMode) {
		x = x * 100 / _touchpad_scale;
		y = y * 100 / _touchpad_scale;
	}

	if (deductDrawRect) {
		x -= r.left;
		y -= r.top;
	}

	p.x = scalef(x, tex->width(), r.width());
	p.y = scalef(y, tex->height(), r.height());
}

void OSystem_Android::updateEventScale() {
	const GLESBaseTexture *tex;

	tex = _game_texture;

	_eventScaleY = 100 * 480 / tex->height();
	_eventScaleX = 100 * 640 / tex->width();
}

void OSystem_Android::pushEvent(int type, int arg1, int arg2, int arg3,
		int arg4, int arg5) {
	Common::Event e;

	switch (type) {

	case JE_DOWN:

		AndroidPortAdditions::instance()->onDownEvent(arg1, arg2);

		_touch_pt_down = getEventManager()->getMousePos();
		_touch_pt_scroll.x = -1;
		_touch_pt_scroll.y = -1;
		break;

	case JE_UP:

		LOGD("OSystem_Android::pushEvent: UP: %d %d", arg1, arg2);

		AndroidPortAdditions::instance()->onUpEvent(arg1, arg2);

		break;

	case JE_LONG_CLICK:

		AndroidPortAdditions::instance()->onLongClickEvent(arg1, arg2);

		break;

	case JE_SCROLL:

		if (AndroidPortAdditions::instance()->onScrollEvent(arg3, arg4)) {
			return;
		}

		e.type = Common::EVENT_MOUSEMOVE;

		if (AndroidPortAdditions::instance()->getTouchpadMode()) {
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg3;
				_touch_pt_scroll.y = arg4;
				return;
			}

			scaleMouse(e.mouse, arg3 - _touch_pt_scroll.x,
					arg4 - _touch_pt_scroll.y, false, true);
			e.mouse += _touch_pt_down;
			clipMouse(e.mouse);
		} else {
			//		AndroidPortAdditions::instance()->translateTouchCoordinates(arg3, arg4);

			//		scaleMouse(e.mouse, arg3, arg4);
			//		clipMouse(e.mouse);

			// Scroll event in touch control mode is handled inside AndroidPortAdditions
			return;

		}

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_FLING:

		if (AndroidPortAdditions::instance()->onFlingEvent(arg1, arg2, arg3, arg4)) {
			return;
		}

		e.type = Common::EVENT_MOUSEMOVE;

		if (AndroidPortAdditions::instance()->getTouchpadMode()) {
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg3;
				_touch_pt_scroll.y = arg4;
				return;
			}

			scaleMouse(e.mouse, arg3 - _touch_pt_scroll.x,
					arg4 - _touch_pt_scroll.y, false, true);
			e.mouse += _touch_pt_down;
			clipMouse(e.mouse);
		} else {
			//		AndroidPortAdditions::instance()->translateTouchCoordinates(arg3, arg4);

			//		scaleMouse(e.mouse, arg3, arg4);
			//		clipMouse(e.mouse);

			// Scroll event in touch control mode is handled inside AndroidPortAdditions
			return;

		}

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	case JE_TAP:

		if (AndroidPortAdditions::instance()->onTapEvent(arg1, arg2))
			return;

		if (_fingersDown > 0) {
			_fingersDown = 0;
			return;
		}

		e.type = Common::EVENT_MOUSEMOVE;

		if (AndroidPortAdditions::instance()->getTouchpadMode()) {
			e.mouse = getEventManager()->getMousePos();

			AndroidPortAdditions::instance()->onMouseClick(e.mouse.x,
					e.mouse.y);
		} else {
			return;

		}

		{
			Common::EventType down, up;

			// Commented out the section below to allow only left mouse clicks

			/*	if (arg3 > 1000) {
			 down = Common::EVENT_MBUTTONDOWN;
			 up = Common::EVENT_MBUTTONUP;
			 } else if (arg3 > 500) {
			 down = Common::EVENT_RBUTTONDOWN;
			 up = Common::EVENT_RBUTTONUP;
			 } else {*/
			down = Common::EVENT_LBUTTONDOWN;
			up = Common::EVENT_LBUTTONUP;
			//	}

			lockMutex(_event_queue_lock);

			if (_queuedEventTime)
				_event_queue.push(_queuedEvent);

			if (!AndroidPortAdditions::instance()->getTouchpadMode())
				_event_queue.push(e);

			e.type = down;
			_event_queue.push(e);

			e.type = up;
			_queuedEvent = e;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;

			unlockMutex(_event_queue_lock);
		}

		return;

	case JE_DOUBLE_TAP:

		if (AndroidPortAdditions::instance()->onTapEvent(arg1, arg2, true))
			return;

		e.type = Common::EVENT_MOUSEMOVE;

		if (AndroidPortAdditions::instance()->getTouchpadMode()) {
			e.mouse = getEventManager()->getMousePos();

			AndroidPortAdditions::instance()->onMouseClick(e.mouse.x,
					e.mouse.y);

		} else {
			return;

		}

		{
			Common::EventType dptype = Common::EVENT_INVALID;

			switch (arg3) {
			case JACTION_DOWN:
				dptype = Common::EVENT_LBUTTONDOWN;
				_touch_pt_dt.x = -1;
				_touch_pt_dt.y = -1;
				break;
			case JACTION_UP:
				dptype = Common::EVENT_LBUTTONUP;
				break;
				// held and moved
			case JACTION_MULTIPLE:
				if (_touch_pt_dt.x == -1 && _touch_pt_dt.y == -1) {
					_touch_pt_dt.x = arg1;
					_touch_pt_dt.y = arg2;
					return;
				}

				dptype = Common::EVENT_MOUSEMOVE;

				if (AndroidPortAdditions::instance()->getTouchpadMode()) {
					scaleMouse(e.mouse, arg1 - _touch_pt_dt.x,
							arg2 - _touch_pt_dt.y, false, true);
					e.mouse += _touch_pt_down;

					clipMouse(e.mouse);
				}

				break;
			default:
				LOGE("unhandled jaction on double tap: %d", arg3);
				return;
			}

			lockMutex(_event_queue_lock);
			_event_queue.push(e);
			e.type = dptype;
			_event_queue.push(e);
			unlockMutex(_event_queue_lock);
		}

		return;

	case JE_QUIT:
		e.type = Common::EVENT_QUIT;

		lockMutex(_event_queue_lock);
		_event_queue.push(e);
		unlockMutex(_event_queue_lock);

		return;

	default:
		LOGE("unknown jevent type: %d", type);

		break;
	}
}

bool OSystem_Android::pollEvent(Common::Event &event) {
	//ENTER();

	if (pthread_self() == _main_thread) {
		if (_screen_changeid != JNI::surface_changeid) {
			if (JNI::egl_surface_width > 0 && JNI::egl_surface_height > 0) {
				// surface changed
				JNI::deinitSurface();
				initSurface();
				initViewport();
				updateScreenRect();
				updateEventScale();

				// double buffered, flip twice
				clearScreen(kClearUpdate, 2);

				event.type = Common::EVENT_SCREEN_CHANGED;

				return true;
			} else {
				// surface lost
				deinitSurface();
			}
		}

		if (JNI::pause) {
			deinitSurface();

			LOGD("main thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("main thread woke up");
		}
	}

	lockMutex(_event_queue_lock);

	if (_queuedEventTime && (getMillis() > _queuedEventTime)) {
		event = _queuedEvent;
		_queuedEventTime = 0;
		unlockMutex(_event_queue_lock);
		return true;
	}

	if (_event_queue.empty()) {
		unlockMutex(_event_queue_lock);
		return false;
	}

	event = _event_queue.pop();

	unlockMutex(_event_queue_lock);

	if (event.type == Common::EVENT_MOUSEMOVE) {
		const Common::Point &m = getEventManager()->getMousePos();

		if (m != event.mouse)
			_force_redraw = true;
	}

	return true;
}

void OSystem_Android::forceEvent(Common::Event e) {
	lockMutex(_event_queue_lock);
	_event_queue.push(e);
	unlockMutex(_event_queue_lock);
}

void OSystem_Android::pushClick(uint32 x, uint32 y) {
	LOGD("OSystem_Android::pushClick: %d %d", x, y);

	Common::Event e;
	e.type = Common::EVENT_MOUSEMOVE;

	e.mouse.x = x;
	e.mouse.y = y;

	Common::EventType down, up;

	down = Common::EVENT_LBUTTONDOWN;
	up = Common::EVENT_LBUTTONUP;

	lockMutex(_event_queue_lock);

	if (_queuedEventTime)
		_event_queue.push(_queuedEvent);

	_event_queue.push(e);

	e.type = down;
	_event_queue.push(e);

	e.type = up;
	_queuedEvent = e;
	_queuedEventTime = getMillis() + kQueuedInputEventDelay;

	unlockMutex(_event_queue_lock);
}

#endif
