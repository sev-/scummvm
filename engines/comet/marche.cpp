#include "comet/comet.h"
#include "comet/pak.h"

#include "comet/animation.h"

namespace Comet {

int CometEngine::findAnimationSlot(int16 animationType, int16 fileIndex) {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].animationType == animationType && _animationSlots[i].fileIndex == fileIndex) {
			return i;
		}
	}
	return -1;
}

int CometEngine::findFreeAnimationSlot() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim == NULL) {
			return i;
		}
	}
	return -1;
}

bool CometEngine::isAnimationSlotUsed(int16 animationSlot) {
	for (int i = 0; i < 11; i++) {
		if (_sceneObjects[i].animationSlot == animationSlot && _sceneObjects[i].life != 0)
			return true;
	}
	return false;
}

void CometEngine::clearMarcheByIndex(int16 animationSlot) {
	for (int i = 1; i < 11; i++) {
		if (_sceneObjects[i].animationSlot == animationSlot) {
			_sceneObjects[i].animationSlot = -1;
			_sceneObjects[i].life = 0;
		}
	}
}

Animation *CometEngine::loadAnimationResource(const char *pakFilename, int fileIndex) {
	debug(0, "CometEngine::loadAnimationResource([%s], %d)", pakFilename, fileIndex);
	Animation *animation = new Animation();
	byte *buffer = loadFromPak(pakFilename, fileIndex);
	int size = getPakSize(pakFilename, fileIndex);
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(buffer, size);
	animation->load(*stream, size);
	delete stream;
	free(buffer);
	debug(0, "CometEngine::loadAnimationResource([%s], %d) ok", pakFilename, fileIndex);
	return animation;
}

Animation *CometEngine::getGlobalAnimationResource(int16 animationType) {
	switch (animationType) {
	case 1:
		return _heroSprite;
	case 2:
		return _sceneObjectsSprite;
	//case 3: //TODO??? returns NULL var (maybe used in Eternam?)
	default:
		warning("CometEngine::getGlobalAnimationResource() Invalid animationType (%d)", animationType);
		return NULL;
	}
}

void CometEngine::purgeUnusedAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim && _animationSlots[i].animationType == 0 && !isAnimationSlotUsed(i)) {
			clearMarcheByIndex(i);
			delete _animationSlots[i].anim;
			_animationSlots[i].anim = NULL;
		}
	}
}

void CometEngine::purgeAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim && _animationSlots[i].animationType == 0 && _sceneObjects[0].animationSlot != i) {
			clearMarcheByIndex(i);
			delete _animationSlots[i].anim;
			_animationSlots[i].anim = NULL;
		}
	}
}

int CometEngine::getAnimationResource(int16 animationType, int16 fileIndex) {

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
		_animationSlots[animationSlot].anim = getGlobalAnimationResource(animationType);
	} else if (!_animationSlots[animationSlot].anim) {
		_animationSlots[animationSlot].anim = loadAnimationResource(AName, fileIndex);
	}

	return animationSlot;

}

void CometEngine::refreshAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].anim && _animationSlots[i].animationType == 0) {
			delete _animationSlots[i].anim;
			_animationSlots[i].anim = NULL;
		}
	}
	restoreAnimationSlots();
}

void CometEngine::restoreAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_animationSlots[i].fileIndex != -1) {
			if (_animationSlots[i].animationType == 0) {
				delete _animationSlots[i].anim;
				_animationSlots[i].anim = loadAnimationResource(AName, _animationSlots[i].fileIndex);
			} else {
				_animationSlots[i].anim = getGlobalAnimationResource(_animationSlots[i].animationType);
			}
		} else {
			_animationSlots[i].anim = NULL;
		}
	}
}

void CometEngine::unloadSceneObjectSprite(SceneObject *sceneObject) {
	if (sceneObject->animationSlot != -1) {
		AnimationSlot *marche = &_animationSlots[sceneObject->animationSlot];
		if (marche->anim && marche->animationType == 0 && !isAnimationSlotUsed(sceneObject->animationSlot)) {
			clearMarcheByIndex(sceneObject->animationSlot);
			delete marche->anim;
			marche->anim = NULL;
		}
	}
}

} // End of namespace Comet
