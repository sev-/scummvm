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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMET_TASK_H
#define COMET_TASK_H

#include "common/events.h"
#include "common/stack.h"

namespace Comet {

class AnimationResource;
struct AnimationFrameList;

class TaskBase {
public:
	TaskBase() : _resumeLineNum(0), _terminated(false) {}
	virtual ~TaskBase() {}
	virtual void enter() {}
	virtual void leave() {}
	virtual void update() {}
	virtual void handleEvent(Common::Event &event) {}
	virtual bool isActive() { return true; }
	virtual uint32 getUpdateTicks() const { return 100; }
	bool isTerminated() { return _terminated || !isActive(); }
protected:
	int _resumeLineNum;
	bool _terminated;
	void terminate() { _terminated = true; }
};

class TaskMan {
public:
	TaskMan();
	void newTask(TaskBase *task);
	void enterTask(TaskBase *task);
	void leaveCurrentTask();
	void update();
	void handleEvent(Common::Event &event);
	uint32 getUpdateTicks();
	bool isActive() const { return !_tasks.empty(); }
	TaskBase *currentTask() const { return _tasks.top(); }
	int getResumeLineNum();
	void setResumeLineNum(int resumeLineNum);
	int getCallLevel() const { return _callLevel; }
	void pushFuncCall() {
		++_callLevel;
	}
	bool popFuncCall() {
		int r = _funcCallStack[_callLevel];
		--_callLevel;
		return r != 0;
	}
protected:
	Common::FixedStack<TaskBase*> _tasks;
	int _funcCallStack[16];
	int _callLevel;
};

// TODO Make this a singleton
extern TaskMan g_taskMan;

#define TASK_BODY_BEGIN switch (_resumeLineNum) { case 0:
#define TASK_BODY_END ; } _resumeLineNum = 0;
#define TASK_YIELD _resumeLineNum = __LINE__; return; case __LINE__: ;
#define TASK_AWAIT_TASK(task) _resumeLineNum = __LINE__; return g_taskMan.enterTask(task); case __LINE__:
#define TASK_AWAIT(func)                                                       \
	do {                                                                       \
		debug(0, "TASK_AWAIT " #func);	                                       \
		_resumeLineNum = __LINE__;                                             \
		case __LINE__: {                                                       \
			g_taskMan.pushFuncCall();                                          \
			func();                                                            \
			if (g_taskMan.popFuncCall())                                       \
				return;                                                        \
			debug(0, "  TASK_AWAIT finished");                                 \
		}                                                                      \
	} while (0);

#define FUNC_BODY_BEGIN switch (g_taskMan.getResumeLineNum()) { case 0:
#define FUNC_BODY_END ;} g_taskMan.setResumeLineNum(0);
#define FUNC_YIELD g_taskMan.setResumeLineNum(__LINE__); return; case __LINE__: ;
#define FUNC_AWAIT_TASK(task) g_taskMan.setResumeLineNum(__LINE__); g_taskMan.enterTask(task); return; case __LINE__:
#define FUNC_AWAIT_ARGS(func, args)                                            \
	do {                                                                       \
		g_taskMan.setResumeLineNum(__LINE__);                                  \
		case __LINE__: {                                                       \
			debug(0, "FUNC_AWAIT " #func);	                                   \
			g_taskMan.pushFuncCall();                                          \
			func args;                                                         \
			if (g_taskMan.popFuncCall())                                       \
				return;                                                        \
			debug(0, "  FUNC_AWAIT finished");                                 \
		}                                                                      \
	} while (0);                                                                

#define FUNC_AWAIT(func) FUNC_AWAIT_ARGS(func, ())

// Comet-specific
// TODO move elsewhere?

class CometEngine;

class CometTaskBase : public TaskBase {
public:
	CometTaskBase(CometEngine *vm);
protected:
	CometEngine *_vm;
};

class PaletteFadeTask : public CometTaskBase {
public:
	PaletteFadeTask(CometEngine *vm, int fadeStep);
	virtual void enter();
	virtual void update();
	virtual uint32 getUpdateTicks() const { return 10; }
protected:
	int _currValue;
	int _fadeStep;
}; 

class PaletteFadeInTask : public PaletteFadeTask {
public:
	PaletteFadeInTask(CometEngine *vm, int fadeStep);
	virtual void enter();
	virtual void leave();
	virtual bool isActive();
}; 

class PaletteFadeOutTask : public PaletteFadeTask {
public:
	PaletteFadeOutTask(CometEngine *vm, int fadeStep);
	virtual void enter();
	virtual void leave();
	virtual bool isActive();
}; 

class ScreenTransitionEffectTask : public CometTaskBase {
public:
	ScreenTransitionEffectTask(CometEngine *vm);
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual bool isActive();
protected:
	int _currColumn;
	byte *_workScreen;
}; 

class ScreenScrollEffectTask : public CometTaskBase {
public:
	ScreenScrollEffectTask(CometEngine *vm, byte *newScreen, int scrollDirection);
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual bool isActive();
protected:
	byte *_newScreen;
	int _scrollDirection;
	int _copyOfs;
	byte *_workScreen;
}; 

class PauseTask : public CometTaskBase {
public:
	PauseTask(CometEngine *vm);
	virtual void enter();
	virtual void handleEvent(Common::Event &event);
}; 

class CutsceneTask : public CometTaskBase {
public:
	CutsceneTask(CometEngine *vm, int fileIndex, int frameListIndex, int backgroundIndex, int loopCount, int soundFramesCount, byte *soundFramesData);
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
protected:
	int _fileIndex, _frameListIndex, _backgroundIndex, _loopCount, _soundFramesCount;
	byte *_soundFramesData;

	int _palStatus;
	AnimationResource *_cutsceneSprite;
	AnimationFrameList *_frameList;
	int _animFrameCount;
	
	int _loopIndex;
	byte *_workSoundFramesData;
	int _workSoundFramesCount;
	int _animFrameIndex, _animSoundFrameIndex, _interpolationStep;

}; 

class CometIntroTask : public CometTaskBase {
public:
	CometIntroTask(CometEngine *vm);
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive();
};

class CometGameTask : public CometTaskBase {
public:
	CometGameTask(CometEngine *vm);
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive();
};

} // End of namespace Comet

#endif
