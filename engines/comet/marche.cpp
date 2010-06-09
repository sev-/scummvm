#include "comet/comet.h"
#include "comet/pak.h"

#include "comet/animation.h"
#include "comet/animationmgr.h"

namespace Comet {

bool CometEngine::isAnimationSlotUsed(int16 animationSlot) {
	for (int i = 0; i < 11; i++) {
		if (_actors[i].animationSlot == animationSlot && _actors[i].life != 0)
			return true;
	}
	return false;
}

void CometEngine::clearMarcheByIndex(int16 animationSlot) {
	for (int i = 1; i < 11; i++) {
		if (_actors[i].animationSlot == animationSlot) {
			_actors[i].animationSlot = -1;
			_actors[i].life = 0;
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

void CometEngine::unloadSceneObjectSprite(Actor *actor) {
	if (actor->animationSlot != -1) {
		AnimationSlot *animationSlot = _animationMan->getAnimationSlot(actor->animationSlot);
		if (animationSlot->anim && animationSlot->animationType == 0 && !isAnimationSlotUsed(actor->animationSlot)) {
			clearMarcheByIndex(actor->animationSlot);
			delete animationSlot->anim;
			animationSlot->anim = NULL;
		}
	}
}

} // End of namespace Comet
