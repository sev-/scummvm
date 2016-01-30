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

#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/actor.h"
#include "comet/screen.h"
#include "comet/resource.h"
#include "comet/animationmgr.h"

namespace Comet {

// AnimationSlot

void AnimationSlot::sync(Common::Serializer &s) {
	s.syncAsUint16LE(animationType);
	s.syncAsUint16LE(fileIndex);
	if (s.isLoading()) {
		anim = 0;
	}
}

// AnimationManager

AnimationManager::AnimationManager(CometEngine *vm) : _vm(vm) {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++) {
		_animationSlots[slotIndex].animationType = -1;
		_animationSlots[slotIndex].fileIndex = -1;
		_animationSlots[slotIndex].anim = NULL;
	}
}

AnimationManager::~AnimationManager() {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++) {
		// Only deallocate "normal" animations here,
		// the GlobalAnimationResources have pointers held by the engine
		// and are deallocated there.
		if (_animationSlots[slotIndex].animationType == 0) {
			delete _animationSlots[slotIndex].anim;
			_animationSlots[slotIndex].anim = NULL;
		}
	}
}

AnimationResource *AnimationManager::loadAnimationResource(const char *pakFilename, int fileIndex) {
	AnimationResource *animation = new AnimationResource();
	_vm->_res->loadFromPak(animation, pakFilename, fileIndex);
	return animation;
}

AnimationResource *AnimationManager::loadAnimationResourceFromRaw(const byte *rawData, uint32 rawDataSize, int maxCount, int index) {
	AnimationResource *animation = new AnimationResource();
	_vm->_res->loadFromRaw(animation, rawData, rawDataSize, maxCount, index);
	return animation;
}

void AnimationManager::purgeUnusedAnimationSlots() {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++)
		if (_animationSlots[slotIndex].anim && _animationSlots[slotIndex].animationType == 0 && !_vm->_actors->isAnimationSlotUsed(slotIndex)) {
			_vm->_actors->clearAnimationSlotByIndex(slotIndex);
			delete _animationSlots[slotIndex].anim;
			_animationSlots[slotIndex].anim = NULL;
		}
}

void AnimationManager::purgeAnimationSlots() {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++)
		if (_animationSlots[slotIndex].anim && _animationSlots[slotIndex].animationType == 0 && _vm->_actors->getActor(0)->_animationSlot != (int)slotIndex) {
			_vm->_actors->clearAnimationSlotByIndex(slotIndex);
			delete _animationSlots[slotIndex].anim;
			_animationSlots[slotIndex].anim = NULL;
		}
}

int AnimationManager::getAnimationResource(int16 animationType, int16 fileIndex) {
	int16 animationSlot = findAnimationSlot(animationType, fileIndex);
	if (animationSlot == -1) {
		animationSlot = findFreeAnimationSlot();
		if (animationSlot == -1) {
			purgeUnusedAnimationSlots();
			animationSlot = findFreeAnimationSlot();
		}
	}
	_animationSlots[animationSlot].animationType = animationType;
	_animationSlots[animationSlot].fileIndex = fileIndex;
	if (animationType != 0)
		_animationSlots[animationSlot].anim = _vm->getGlobalAnimationResource(animationType);
	else if (!_animationSlots[animationSlot].anim)
		_animationSlots[animationSlot].anim = loadAnimationResource(_vm->_animPakName.c_str(), fileIndex);
	return animationSlot;
}

void AnimationManager::refreshAnimationSlots() {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++)
		if (_animationSlots[slotIndex].anim && _animationSlots[slotIndex].animationType == 0) {
			delete _animationSlots[slotIndex].anim;
			_animationSlots[slotIndex].anim = NULL;
		}
	restoreAnimationSlots();
}

void AnimationManager::restoreAnimationSlots() {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++) {
		if (_animationSlots[slotIndex].fileIndex < 0) {
			_animationSlots[slotIndex].anim = NULL;
		} else if (_animationSlots[slotIndex].animationType == 0) {
			delete _animationSlots[slotIndex].anim;
			_animationSlots[slotIndex].anim = loadAnimationResource(_vm->_animPakName.c_str(), _animationSlots[slotIndex].fileIndex);
		} else {
			_animationSlots[slotIndex].anim = _vm->getGlobalAnimationResource(_animationSlots[slotIndex].animationType);
		}
	}
}

void AnimationManager::sync(Common::Serializer &s) {
	for (uint i = 0; i < kAnimationSlotCount; ++i) {
		getAnimationSlot(i)->sync(s);
	}
}

int AnimationManager::findAnimationSlot(int16 animationType, int16 fileIndex) {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++)
		if (_animationSlots[slotIndex].animationType == animationType && _animationSlots[slotIndex].fileIndex == fileIndex)
			return slotIndex;
	return -1;
}

int AnimationManager::findFreeAnimationSlot() {
	for (uint slotIndex = 0; slotIndex < kAnimationSlotCount; slotIndex++)
		if (_animationSlots[slotIndex].anim == NULL)
			return slotIndex;
	return -1;
}

} // End of namespace Comet
