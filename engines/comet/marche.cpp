#include "comet/comet.h"
#include "comet/pak.h"

#include "comet/animation.h"

namespace Comet {

int CometEngine::findAnimationSlot(int16 animationType, int16 fileIndex) {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].animationType == animationType && _marcheItems[i].fileIndex == fileIndex) {
			return i;
		}
	}
	return -1;
}

int CometEngine::findFreeAnimationSlot() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim == NULL) {
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

	/*
	char fn[256];
	snprintf(fn, 256, "%c%06d.0", *pakFilename, fileIndex);
	FILE *d = fopen(fn, "wb");
	fwrite(buffer, size, 1, d);
	fclose(d);
	*/

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
	//case 3: //TODO??? returns NULL var
	default:
		warning("CometEngine::getGlobalAnimationResource() Invalid animationType (%d)", animationType);
		return NULL;
	}
}

void CometEngine::purgeUnusedAnimationSlots() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim && _marcheItems[i].animationType == 0 && !isAnimationSlotUsed(i)) {
			clearMarcheByIndex(i);
			delete _marcheItems[i].anim;
			_marcheItems[i].anim = NULL;
		}
	}
}

void CometEngine::freeMarche() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim && _marcheItems[i].animationType == 0 && _sceneObjects[0].animationSlot != i) {
			clearMarcheByIndex(i);
			delete _marcheItems[i].anim;
			_marcheItems[i].anim = NULL;
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

	_marcheItems[animationSlot].animationType = animationType;
	_marcheItems[animationSlot].fileIndex = fileIndex;

	// Possible workaround for the memory leak bug
	#if 0
	if (/*_marcheItems[animationSlot].animationType == 0 && */_marcheItems[animationSlot].anim) {
		//warning("CometEngine::freeMarche() _marcheItems[%d].anim not NULL", animationSlot);
		delete _marcheItems[animationSlot].anim;
		_marcheItems[animationSlot].anim = NULL;
	}
	#endif

	if (animationType != 0) {
		_marcheItems[animationSlot].anim = getGlobalAnimationResource(animationType);
	} else if (_marcheItems[animationSlot].anim == NULL) {
		_marcheItems[animationSlot].anim = loadAnimationResource(AName, fileIndex);
	}

	return animationSlot;

}

void CometEngine::freeMarcheAnims() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim && _marcheItems[i].animationType == 0) {
			delete _marcheItems[i].anim;
			_marcheItems[i].anim = NULL;
		}
	}
	loadAllMarche();
}

void CometEngine::loadAllMarche() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].fileIndex != -1) {
			if (_marcheItems[i].animationType == 0) {
				delete _marcheItems[i].anim;
				_marcheItems[i].anim = loadAnimationResource(AName, _marcheItems[i].fileIndex);
			} else {
				_marcheItems[i].anim = getGlobalAnimationResource(_marcheItems[i].animationType);
			}
		} else {
			_marcheItems[i].anim = NULL;
		}
	}
}

void CometEngine::unloadSceneObjectSprite(SceneObject *sceneObject) {
	if (sceneObject->animationSlot != -1) {
		AnimationSlot *marche = &_marcheItems[sceneObject->animationSlot];
		if (marche->anim && marche->animationType == 0 && !isAnimationSlotUsed(sceneObject->animationSlot)) {
			clearMarcheByIndex(sceneObject->animationSlot);
			delete marche->anim;
			marche->anim = NULL;
		}
	}
}

} // End of namespace Comet
