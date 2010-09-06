#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/screen.h"

#include "comet/animationmgr.h"
#include "comet/resource.h"

namespace Comet {

AnimationManager::AnimationManager(CometEngine *vm) : _vm(vm) {
	for (uint i = 0; i < kAnimationSlotCount; i++) {
		_animationSlots[i].animationType = -1;
		_animationSlots[i].fileIndex = -1;
		_animationSlots[i].anim = NULL;
	}
}

AnimationManager::~AnimationManager() {
}

void AnimationManager::saveState(Common::WriteStream *out) {
}

void AnimationManager::loadState(Common::ReadStream *in) {
}

AnimationResource *AnimationManager::loadAnimationResource(const char *pakFilename, int fileIndex) {
	debug(0, "AnimationManager::loadAnimationResource([%s], %d)", pakFilename, fileIndex);
	AnimationResource *animation = new AnimationResource();
	_vm->_res->loadFromPak(animation, pakFilename, fileIndex);
	debug(0, "AnimationManager::loadAnimationResource([%s], %d) ok", pakFilename, fileIndex);
	return animation;
}

void AnimationManager::purgeUnusedAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim && _animationSlots[i].animationType == 0 && !_vm->isAnimationSlotUsed(i)) {
			_vm->clearAnimationSlotByIndex(i);
			delete _animationSlots[i].anim;
			_animationSlots[i].anim = NULL;
		}
	}
}

void AnimationManager::purgeAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim && _animationSlots[i].animationType == 0 && _vm->_actors[0].animationSlot != i) {
			_vm->clearAnimationSlotByIndex(i);
			delete _animationSlots[i].anim;
			_animationSlots[i].anim = NULL;
		}
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

	if (animationType != 0) {
		_animationSlots[animationSlot].anim = _vm->getGlobalAnimationResource(animationType);
	} else if (!_animationSlots[animationSlot].anim) {
		_animationSlots[animationSlot].anim = loadAnimationResource(_vm->AName, fileIndex);
	}

	return animationSlot;

}

void AnimationManager::refreshAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim && _animationSlots[i].animationType == 0) {
			delete _animationSlots[i].anim;
			_animationSlots[i].anim = NULL;
		}
	}
	restoreAnimationSlots();
}

void AnimationManager::restoreAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].fileIndex != -1) {
			if (_animationSlots[i].animationType == 0) {
				delete _animationSlots[i].anim;
				_animationSlots[i].anim = loadAnimationResource(_vm->AName, _animationSlots[i].fileIndex);
			} else {
				_animationSlots[i].anim = _vm->getGlobalAnimationResource(_animationSlots[i].animationType);
			}
		} else {
			_animationSlots[i].anim = NULL;
		}
	}
}

int AnimationManager::findAnimationSlot(int16 animationType, int16 fileIndex) {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].animationType == animationType && _animationSlots[i].fileIndex == fileIndex) {
			return i;
		}
	}
	return -1;
}

int AnimationManager::findFreeAnimationSlot() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim == NULL) {
			return i;
		}
	}
	return -1;
}

} // End of namespace Comet
