#include "comet/comet.h"
#include "comet/pak.h"

#include "comet/animation.h"

namespace Comet {

int CometEngine::findMarcheItem(int marcheNumber, int fileIndex) {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].marcheNumber == marcheNumber && _marcheItems[i].fileIndex == fileIndex) {
			return i;
		}
	}
	return -1;
}

int CometEngine::findFreeMarcheSlot() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim == NULL) {
			return i;
		}
	}
	return -1;
}

bool CometEngine::isMarcheLoaded(int marcheIndex) {
	for (int i = 0; i < 11; i++) {
		if (_sceneObjects[i].marcheIndex == marcheIndex && _sceneObjects[i].flag != 0)
			return true;
	}
	return false;
}

void CometEngine::clearMarcheByIndex(int marcheIndex) {
	for (int i = 1; i < 11; i++) {
		if (_sceneObjects[i].marcheIndex == marcheIndex) {
			_sceneObjects[i].marcheIndex = -1;
			_sceneObjects[i].flag = 0;
		}
	}
}

Animation *CometEngine::loadMarcheData(const char *pakFilename, int fileIndex) {

	Animation *animation = new Animation();

	byte *buffer = loadFromPak(pakFilename, fileIndex);
	int size = getPakSize(pakFilename, fileIndex);
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(buffer, size);
	animation->load(*stream, size);
	delete stream;

	char fn[256];
	snprintf(fn, 256, "%c%06d.0", *pakFilename, fileIndex);
	FILE *d = fopen(fn, "wb");
	fwrite(buffer, size, 1, d);
	fclose(d);

	free(buffer);

	return animation;

}

Animation *CometEngine::getMarcheAnim(int marcheNumber) {
	switch (marcheNumber) {
	case 1:
		return _heroSprite;
	case 2:
		return _sceneObjectsSprite;
	//case 3: //TODO??? returns NULL var
	default:
		warning("CometEngine::getMarcheAnim() Invalid marcheNumber (%d)", marcheNumber);
		return NULL;
	}
}

void CometEngine::freeAllMarche() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim && _marcheItems[i].marcheNumber == 0 && !isMarcheLoaded(i)) {
			clearMarcheByIndex(i);
			delete _marcheItems[i].anim;
			_marcheItems[i].anim = NULL;
		}
	}
}

void CometEngine::freeMarche() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim && _marcheItems[i].marcheNumber == 0 && _sceneObjects[0].marcheIndex != i) {
			clearMarcheByIndex(i);
			delete _marcheItems[i].anim;
			_marcheItems[i].anim = NULL;
		}
	}
}

int CometEngine::loadMarche(int marcheNumber, int fileIndex) {

	int marcheIndex = findMarcheItem(marcheNumber, fileIndex);

	if (marcheIndex == -1) {
		marcheIndex = findFreeMarcheSlot();
		if (marcheIndex == -1) {
			freeAllMarche();
			marcheIndex = findFreeMarcheSlot();
		}
	}

	_marcheItems[marcheIndex].marcheNumber = marcheNumber;
	_marcheItems[marcheIndex].fileIndex = fileIndex;

	// Possible workaround for the memory leak bug
	if (_marcheItems[marcheIndex].anim) {
		//warning("CometEngine::freeMarche() _marcheItems[%d].anim not NULL", marcheIndex);
		delete _marcheItems[marcheIndex].anim;
		_marcheItems[marcheIndex].anim = NULL;
	}

	if (marcheNumber != 0) {
		_marcheItems[marcheIndex].anim = getMarcheAnim(marcheNumber);
	} else if (_marcheItems[marcheIndex].anim == NULL) {
		_marcheItems[marcheIndex].anim = loadMarcheData(AName, fileIndex);
	}

	return marcheIndex;

}

void CometEngine::freeMarcheAnims() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].anim && _marcheItems[i].marcheNumber == 0) {
			delete _marcheItems[i].anim;
			_marcheItems[i].anim = NULL;
		}
	}
	loadAllMarche();
}

void CometEngine::loadAllMarche() {
	for (int i = 0; i < 20; i++) {
		if (_marcheItems[i].marcheNumber == 0 && _marcheItems[i].anim != NULL) {
			delete _marcheItems[i].anim; // CHECKME: Workaround for memory/freezing bug
			_marcheItems[i].anim = loadMarcheData(AName, _marcheItems[i].fileIndex);
		} else {
			_marcheItems[i].anim = getMarcheAnim(_marcheItems[i].marcheNumber);
		}
	}
}

void CometEngine::unloadSceneObjectSprite(SceneObject *sceneObject) {
	if (sceneObject->marcheIndex != -1) {
		MarcheItem *marche = &_marcheItems[sceneObject->marcheIndex];
		if (marche->anim && marche->marcheNumber == 0 && !isMarcheLoaded(sceneObject->marcheIndex)) {
			clearMarcheByIndex(sceneObject->marcheIndex);
			delete marche->anim;
			marche->anim = NULL;
		}
	}
}

} // End of namespace Comet
