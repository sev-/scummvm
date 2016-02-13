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

#include "comet/task.h"
#include "comet/actor.h"
#include "comet/animationmgr.h"
#include "comet/comet.h"
#include "comet/comet_gui.h"
#include "comet/dialog.h"
#include "comet/input.h"
#include "comet/resource.h"
#include "comet/scene.h"
#include "comet/screen.h"
#include "comet/script.h"
#include "comet/talktext.h"
#include "common/keyboard.h"

namespace Comet {

// TaskMan

TaskMan::TaskMan() {
	memset(_funcCallStack, 0, sizeof(_funcCallStack));
	_callLevel = 0;
}

void TaskMan::newTask(TaskBase *task) {
	_tasks.push(task);
}

void TaskMan::enterTask(TaskBase *task) {
	_tasks.push(task);
	currentTask()->enter();
	currentTask()->update();
}

void TaskMan::leaveCurrentTask() {
	currentTask()->leave();
	_tasks.pop();
}

void TaskMan::update() {
	while (isActive()) {
		currentTask()->update();
		if (currentTask()->isTerminated()) {
			leaveCurrentTask();
		} else
			break;
	}
	//while (!_tasks.empty() && (result = _tasks.top()->update()) == -1);
}

void TaskMan::handleEvent(Common::Event &event) {
	currentTask()->handleEvent(event);
}

uint32 TaskMan::getUpdateTicks() {
	return currentTask()->getUpdateTicks();
}

int TaskMan::getResumeLineNum() {
	return _funcCallStack[_callLevel];
}

void TaskMan::setResumeLineNum(int resumeLineNum) {
	_funcCallStack[_callLevel] = resumeLineNum;
}

// TODO Make this a singleton
TaskMan g_taskMan;

// CometTaskBase

CometTaskBase::CometTaskBase(CometEngine *vm)
	: TaskBase(), _vm(vm) {
}

// PaletteFadeTask

PaletteFadeTask::PaletteFadeTask(CometEngine *vm, int fadeStep)
	: CometTaskBase(vm), _fadeStep(fadeStep) {
}

void PaletteFadeTask::enter() {
	_currValue = _fadeStep < 0 ? 255 : 0;
}

void PaletteFadeTask::update() {
	debug("PaletteFadeTask::update()");
	_vm->_screen->setFadePalette(_currValue);
	_currValue += _fadeStep;
}

// PaletteFadeInTask

PaletteFadeInTask::PaletteFadeInTask(CometEngine *vm, int fadeStep)
	: PaletteFadeTask(vm, fadeStep) {
}

void PaletteFadeInTask::enter() {
	PaletteFadeTask::enter();
	_vm->_screen->setFadePalette(0);
	_vm->_system->copyRectToScreen(_vm->_screen->getPixels(), 320, 0, 0, 320, 200);
}

void PaletteFadeInTask::leave() {
	_vm->_screen->setFadePalette(255);
	_vm->_screen->_fadeType = kFadeNone;
	_vm->_screen->_palFlag = false;
}

bool PaletteFadeInTask::isActive() {
	return _currValue < 255;
}

// PaletteFadeOutTask

PaletteFadeOutTask::PaletteFadeOutTask(CometEngine *vm, int fadeStep)
	: PaletteFadeTask(vm, fadeStep) {
}

void PaletteFadeOutTask::enter() {
	PaletteFadeTask::enter();
	_vm->_screen->setFadePalette(255);
}

void PaletteFadeOutTask::leave() {
	_vm->_screen->setFadePalette(0);
	_vm->_screen->_fadeType = kFadeNone;
	_vm->_screen->_palFlag = true;
}

bool PaletteFadeOutTask::isActive() {
	return _currValue > 0;
}

// ScreenTransitionEffectTask

ScreenTransitionEffectTask::ScreenTransitionEffectTask(CometEngine *vm)
	: CometTaskBase(vm), _currColumn(0) {
}

void ScreenTransitionEffectTask::enter() {
	_currColumn = 0;
	_workScreen = new byte[64000];
	Graphics::Surface *currentScreen = _vm->_system->lockScreen();
	memcpy(_workScreen, currentScreen->getPixels(), 320 * 200);
	_vm->_system->unlockScreen();
}

void ScreenTransitionEffectTask::leave() {
	delete[] _workScreen;
}

void ScreenTransitionEffectTask::update() {
	debug("ScreenTransitionEffectTask::update()");
	byte *sourceBuf = (byte*)_vm->_screen->getBasePtr(_currColumn, 0);
	byte *destBuf = _workScreen + _currColumn;
	for (int x = 0; x < 320 * 200 / 6; x++) {
		*destBuf = *sourceBuf;
		sourceBuf += 6;
		destBuf += 6;
	}
	_vm->_system->copyRectToScreen(_workScreen, 320, 0, 0, 320, 200);
	_vm->_system->updateScreen();
	++_currColumn;
}

bool ScreenTransitionEffectTask::isActive() {
	return _currColumn < 7;
}

// ScreenScrollEffectTask

ScreenScrollEffectTask::ScreenScrollEffectTask(CometEngine *vm, byte *newScreen, int scrollDirection)
	: CometTaskBase(vm), _copyOfs(0), _newScreen(newScreen), _scrollDirection(scrollDirection) {
}

void ScreenScrollEffectTask::enter() {
	_copyOfs = 0;
	Graphics::Surface *vgaScreen = _vm->_system->lockScreen();
	memcpy(_vm->_screen->getPixels(), vgaScreen->getPixels(), 320 * 200);
	_vm->_system->unlockScreen();
}

void ScreenScrollEffectTask::leave() {
}

void ScreenScrollEffectTask::update() {
	const int kScrollStripWidth = 40;
	Graphics::Surface *screen = _vm->_screen;
	if (_scrollDirection < 0) {
		for (int y = 0; y < 200; y++) {
			memmove(screen->getBasePtr(0, y), screen->getBasePtr(kScrollStripWidth, y), 320 - kScrollStripWidth);
			memcpy(screen->getBasePtr(320 - kScrollStripWidth, y), &_newScreen[y * 320 + _copyOfs], kScrollStripWidth);
		}
	} else {
		for (int y = 0; y < 200; y++) {
			memmove(screen->getBasePtr(kScrollStripWidth, y), screen->getBasePtr(0, y), 320 - kScrollStripWidth);
			memcpy(screen->getBasePtr(0, y), &_newScreen[y * 320 + (320 - kScrollStripWidth - _copyOfs)], kScrollStripWidth);
		}
	}
	_vm->_system->copyRectToScreen(screen->getPixels(), 320, 0, 0, 320, 200);
	_copyOfs += kScrollStripWidth;
}

bool ScreenScrollEffectTask::isActive() {
	return _copyOfs < 320;
}

// PauseTask

PauseTask::PauseTask(CometEngine *vm)
	: CometTaskBase(vm) {
}

void PauseTask::enter() {
	debug("PauseTask::enter()");
	static const byte* const pauseText = (const byte*)"Game Paused";
	const int kPauseTextX = (320 - _vm->_screen->getTextWidth(pauseText)) / 2;
	const int kPauseTextY = 180;
	_vm->_talkText->stopVoice();
	_vm->_screen->drawTextOutlined(kPauseTextX, kPauseTextY, pauseText, 95, 80);
	_vm->_screen->update();
}

void PauseTask::handleEvent(Common::Event &event) {
	debug("PauseTask::handleEvent()");
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		terminate();
		break;
	default:
		break;
	}
}

// CutsceneTask

CutsceneTask::CutsceneTask(CometEngine *vm, int fileIndex, int frameListIndex, int backgroundIndex, int loopCount, int soundFramesCount, byte *soundFramesData)
	: CometTaskBase(vm), _fileIndex(fileIndex), _frameListIndex(frameListIndex), _backgroundIndex(backgroundIndex), _loopCount(loopCount),
	_soundFramesCount(soundFramesCount), _soundFramesData(soundFramesData)
{
}

void CutsceneTask::enter() {

	_vm->_talkText->stopVoice();
	_vm->_talkText->stopText();

	_palStatus = 0;
	_cutsceneSprite = _vm->_animationMan->loadAnimationResource(_vm->_animPakName.c_str(), _fileIndex);
	_frameList = _cutsceneSprite->getFrameList(_frameListIndex);
	_animFrameCount = _frameList->getFrameCount();

	if (_backgroundIndex == 0) {
		_vm->_screen->clear();
	} else if (_backgroundIndex < 0) {
		_vm->_screen->drawAnimationElement(_cutsceneSprite, -_backgroundIndex, 0, 0);
	} else if (_backgroundIndex < 32000) {
		_vm->_screen->drawAnimationElement(_vm->_sceneDecorationSprite, _backgroundIndex, 0, 0);
	} else if (_backgroundIndex == 32000) {
		// TODO: Grab vga screen to work screen
	}

	_vm->_screen->copyToScreen(_vm->_tempScreen);

	if (_soundFramesCount > 0) {
		int sampleIndex = _soundFramesData[0];
		debug("  sampleIndex = %d", sampleIndex);
		// TODO: Load the sample
	}

}

void CutsceneTask::leave() {
	delete _cutsceneSprite;
	if (_palStatus > 0)
		_vm->_screen->setFullPalette(_vm->_screenPalette);
	if (_vm->_talkText->isActive())
		_vm->_talkText->resetTextValues();
	// CHECKME: If this is neccessary (screen is copied in main loop anyways)
	_vm->_screen->copyFromScreenResource(_vm->_sceneBackgroundResource);
}

void CutsceneTask::update() {
	TASK_BODY_BEGIN

	for (_loopIndex = 0; _loopIndex < _loopCount; ++_loopIndex) {
		debug("_loopIndex: %d; _loopCount: %d", _loopIndex, _loopCount);
		_workSoundFramesData = _soundFramesData;
		_workSoundFramesCount = _soundFramesCount;
		_animSoundFrameIndex = 0;
		_interpolationStep = 0;

		if (_soundFramesCount > 0) {
			_workSoundFramesData++;
			_animSoundFrameIndex = *_workSoundFramesData++;
			_workSoundFramesData++;
		}

		_animFrameIndex = 0;
		while (_animFrameIndex < _animFrameCount) {
			debug("_animFrameIndex: %d; _animFrameCount: %d", _animFrameIndex, _animFrameCount);
			_vm->_screen->copyFromScreen(_vm->_tempScreen);
			_interpolationStep = _vm->_screen->drawAnimation(_cutsceneSprite, _frameList, _animFrameIndex, _interpolationStep, 0, 0, _animFrameCount);
			TASK_AWAIT(_vm->_talkText->updateTextDialog);
			if (_palStatus == 1) {
				// TODO: Set the anim palette
				_palStatus = 2;
			}
			if (_workSoundFramesCount > 0 && _animFrameIndex == _animSoundFrameIndex) {
				// TODO: Play the anim sound
				_workSoundFramesCount--;
				if (_workSoundFramesCount > 0) {
					// NOTE: Load the next sample; unused in Comet (only max. one sample per cutscene)
				}
			}
			_vm->_screen->update();
			TASK_YIELD
			// TODO Obsolete checkPauseGame();
#if 0 // TODO
			if (_input->getKeyCode() == Common::KEYCODE_ESCAPE) {
				// TODO: yesNoDialog();
			} else if (_input->getKeyCode() == Common::KEYCODE_RETURN) {
				_animFrameIndex = _animFrameCount;
				_loopIndex = _loopCount;
				if (_talkText->isActive())
					_talkText->stopText();
			}
#endif
			if (_interpolationStep == 0)
				_animFrameIndex++;
		}
	}
	
	terminate();

    TASK_BODY_END
}

void CutsceneTask::handleEvent(Common::Event &event) {
}

// CometIntroTask

CometIntroTask::CometIntroTask(CometEngine *vm)
	: CometTaskBase(vm) {
}

void CometIntroTask::update() {
	TASK_BODY_BEGIN
	
	debug("CometIntroTask::update()");
	if (_vm->_currentSceneNumber == 0 && _vm->_currentModuleNumber == 0) {
		_vm->_endIntroLoop = true;
		return;
	}
	_vm->updateMouseCursor();
	if (_vm->_moduleNumber != _vm->_currentModuleNumber)
		_vm->updateModuleNumber();
	if (_vm->_sceneNumber != _vm->_currentSceneNumber)
		_vm->updateSceneNumber();
	_vm->_screen->copyFromScreenResource(_vm->_sceneBackgroundResource);
	TASK_AWAIT(_vm->_script->runAllScripts);
	_vm->_actors->updateAnimations();
	_vm->_actors->updateMovement();
	_vm->buildSpriteDrawQueue();
	_vm->drawSpriteQueue();
	_vm->_talkText->update();
	_vm->updateScreen();
	_vm->updateHeroLife();
	_vm->_verbs.clear();

	TASK_BODY_END
}

void CometIntroTask::handleEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
			debug("endIntroLoop");
			_vm->_endIntroLoop = true;
			break;
		case Common::KEYCODE_RETURN:
			_vm->_talkText->stopText();
			break;
		case Common::KEYCODE_p:
			g_taskMan.enterTask(new PauseTask(_vm));
			break;
		default:
			break;
		}
	default:
		break;
	}
}

bool CometIntroTask::isActive() {
	return !_vm->_endIntroLoop;
}

// CometGameTask

CometGameTask::CometGameTask(CometEngine *vm)
	: CometTaskBase(vm) {
}

void CometGameTask::update() {
	TASK_BODY_BEGIN
	
	debug("CometGameTask::update()");
	/* TODO
	if (_currentModuleNumber == 7 && _currentSceneNumber == 1 && _paletteStatus == 0) {
		memcpy(_backupPalette, _gamePalette, 768);
		memcpy(_gamePalette, _flashbakPal, 768);
		memcpy(_screenPalette, _flashbakPal, 768);
		_screen->clear();
		_screen->setFullPalette(_gamePalette);
		_paletteStatus = 1;
	} else if (_currentModuleNumber == 2 && _currentSceneNumber == 22 && _paletteStatus == 1) {
		memcpy(_gamePalette, _backupPalette, 768);
		memcpy(_screenPalette, _backupPalette, 768);
		_screen->clear();
		_screen->setFullPalette(_gamePalette);
		_paletteStatus = 0;
	}

	if (_scriptVars[9] == 1) {
		_scriptVars[9] = _gui->run(kGuiPuzzle);
		loadSceneBackground();
	}
	*/

	_vm->_input->updateArrowDirection();
	_vm->updateMouseCursor();
	++_vm->_gameLoopCounter;
	++_vm->_textColorFlag;
	if (_vm->_moduleNumber != _vm->_currentModuleNumber)
		_vm->updateModuleNumber();
	if (_vm->_sceneNumber != _vm->_currentSceneNumber)
		_vm->updateSceneNumber();
	_vm->_screen->copyFromScreenResource(_vm->_sceneBackgroundResource);
	if (_vm->_verbs.isLookRequested())
		_vm->lookAtItemInSight(true);
	if (_vm->_verbs.isGetRequested())
		_vm->getItemInSight();
	_vm->_input->_scriptKeybFlag = _vm->_input->isButtonPressed() ? 1 : 0;
	_vm->updateWalkToDirection(_vm->_input->getWalkDirection());
	TASK_AWAIT(_vm->_script->runAllScripts);
	if (_vm->_loadgameRequested) {
		// TODO:
		//	while (savegame_load() == 0);
		//	savegame_load
		_vm->_loadgameRequested = false;
	} else {
		_vm->_scene->drawExits();
		_vm->_actors->updateAnimations();
		_vm->_actors->updateMovement();
		_vm->buildSpriteDrawQueue();
		_vm->lookAtItemInSight(false);
		_vm->drawSpriteQueue();
		TASK_AWAIT(_vm->_talkText->update);
		// TODO: Make vars for indices
		if (_vm->_scriptVars[11] < 100 && _vm->_scriptVars[10] == 1)
			_vm->drawTextIllsmouth();
		_vm->updateScreen();
		_vm->updateHeroLife();
		_vm->_loadgameRequested = false;
		_vm->_verbs.clear();
		_vm->checkCurrentInventoryItem();
	}
	
	TASK_BODY_END
}

void CometGameTask::handleEvent(Common::Event &event) {
	_vm->_input->handleEvent(event);
	// TODO Handle dialog input
	
	if (_vm->_dialog->isRunning()) {
		_vm->_dialog->handleEvent(event);
	}
	
	if (!_vm->_dialog->isRunning() && _vm->_currentModuleNumber != 3 && _vm->_actors->getActor(0)->_value6 != 4 && !_vm->_screen->_palFlag && !_vm->_talkText->isActive()) {
		// TODO Extract
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_t:
				_vm->_verbs.requestTalk();
				break;
			case Common::KEYCODE_g:
				_vm->_verbs.requestGet();
				break;
			case Common::KEYCODE_l:
				_vm->_verbs.requestLook();
				break;
			case Common::KEYCODE_o:
				// TODO Usually this 
				g_taskMan.enterTask(_vm->_gui->getGuiPage(kGuiInventory));
				// DEBUG
				//g_taskMan.enterTask(_vm->_gui->getGuiPage(kGuiOptionsMenu));
				break;
			case Common::KEYCODE_u:
				_vm->useCurrentInventoryItem();
				break;
			case Common::KEYCODE_d:
				// TODO
				g_taskMan.enterTask(_vm->_gui->getGuiPage(kGuiMainMenu));
				break;
			case Common::KEYCODE_m:
				if (_vm->canShowMap()) {
					g_taskMan.enterTask(_vm->_gui->getGuiPage(kGuiTownMap));
				}
				break;
			case Common::KEYCODE_i:
				g_taskMan.enterTask(_vm->_gui->getGuiPage(kGuiJournal));
				break;
			case Common::KEYCODE_x: // DEBUG
				g_taskMan.enterTask(_vm->_gui->getGuiPage(kGuiPuzzle));
				break;
			case Common::KEYCODE_p:
				// TODO
				g_taskMan.enterTask(new PauseTask(_vm));
				break;
			case Common::KEYCODE_RETURN:
				_vm->_talkText->stopText();
				break;
			case Common::KEYCODE_TAB:			
				g_taskMan.enterTask(_vm->_gui->getGuiPage(kGuiCommandBar));
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			// TODO _gui->run(kGuiCommandBar);
			break;
		case Common::EVENT_RBUTTONDOWN:
			_vm->_talkText->stopText();
			break;
		default:
			break;
		}
	} else {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_RETURN:
				_vm->_talkText->stopText();
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_RBUTTONDOWN:
			_vm->_talkText->stopText();
			break;
		default:
			break;
		}
	}
#if 0 // TODO Reimplement these
		// Debugging keys
		switch (_input->getKeyCode()) {
		case Common::KEYCODE_F7:
			savegame("comet.000", "Quicksave");
			_input->waitForKeys();
			break;
		case Common::KEYCODE_F9:
			loadgame("comet.000");
			_input->waitForKeys();
			break;
		case Common::KEYCODE_KP_PLUS:
			_sceneNumber++;
			debug("## New _sceneNumber = %d", _sceneNumber);
			_input->waitForKeys();
			break;
		case Common::KEYCODE_KP_MINUS:
			if (_sceneNumber > 0) {
				debug("## New _sceneNumber = %d", _sceneNumber);
				_sceneNumber--;
			}
			_input->waitForKeys();
			break;
		case Common::KEYCODE_KP_MULTIPLY:
			_moduleNumber++;
			_sceneNumber = 0;
			debug("## New _moduleNumber = %d", _moduleNumber);
			_input->waitForKeys();
			break;
		case Common::KEYCODE_KP_DIVIDE:
			if (_moduleNumber > 0) {
				_moduleNumber--;
				_sceneNumber = 0;
				debug("## New _moduleNumber = %d", _moduleNumber);
			}
			_input->waitForKeys();
			break;
		default:
			break;
		}
#endif	
}

bool CometGameTask::isActive() {
	return true;
}

} // End of namespace Comet
