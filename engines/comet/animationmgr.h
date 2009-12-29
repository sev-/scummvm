#ifndef COMET_ANIMATION_H
#define COMET_ANIMATION_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"

#include "comet/comet.h"

namespace Comet {

const int kAnimationSlotCount = 20;

enum {
	kAnimationTypeLocal = 0,
	kAnimationTypeHero  = 1,
	kAnimationTypeScene	= 2
};

class AnimationManager {
public:
	AnimationManager();
	~AnimationManager();
protected:
    AnimationSlot _animationSlots[kAnimationSlotCount];
};

} // End of namespace Comet

#endif
