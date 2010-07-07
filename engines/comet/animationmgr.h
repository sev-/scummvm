#ifndef COMET_ANIMATIONMGR_H
#define COMET_ANIMATIONMGR_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"

#include "comet/comet.h"

namespace Comet {

const uint kAnimationSlotCount = 20;

enum {
	kAnimationTypeLocal = 0,
	kAnimationTypeHero  = 1,
	kAnimationTypeScene	= 2
};

#if 0
struct AnimationSlot {
	int16 animationType;
	int16 fileIndex;
	Animation *anim;
};
#endif

class AnimationManager {
public:
	AnimationManager(CometEngine *vm);
	~AnimationManager();
	AnimationResource *loadAnimationResource(const char *pakFilename, int fileIndex);
	void purgeUnusedAnimationSlots();
	void purgeAnimationSlots();
	int getAnimationResource(int16 animationType, int16 fileIndex);
	void refreshAnimationSlots();
	void restoreAnimationSlots();
	AnimationSlot *getAnimationSlot(uint index) { return &_animationSlots[index]; }
	AnimationResource *getAnimation(uint index) { return _animationSlots[index].anim; }
	void saveState(Common::WriteStream *out);
	void loadState(Common::ReadStream *in);
//protected: again temporary...
	CometEngine *_vm;
	AnimationSlot _animationSlots[kAnimationSlotCount];
	int findAnimationSlot(int16 animationType, int16 fileIndex);
	int findFreeAnimationSlot();
};

} // End of namespace Comet

#endif
