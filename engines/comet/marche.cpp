#include "comet/comet.h"
#include "comet/pak.h"

#include "comet/animation.h"
#include "comet/animationmgr.h"

namespace Comet {

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

void CometEngine::unloadSceneObjectSprite(SceneObject *sceneObject) {
	if (sceneObject->animationSlot != -1) {
		AnimationSlot *animationSlot = _animationMan->getAnimationSlot(sceneObject->animationSlot);
		if (animationSlot->anim && animationSlot->animationType == 0 && !isAnimationSlotUsed(sceneObject->animationSlot)) {
			clearMarcheByIndex(sceneObject->animationSlot);
			delete animationSlot->anim;
			animationSlot->anim = NULL;
		}
	}
}

} // End of namespace Comet
