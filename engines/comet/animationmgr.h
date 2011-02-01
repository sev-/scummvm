/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef COMET_ANIMATIONMGR_H
#define COMET_ANIMATIONMGR_H

#include "comet/comet.h"

namespace Comet {

const uint kAnimationSlotCount = 20;

enum {
	kAnimationTypeLocal = 0,
	kAnimationTypeHero  = 1,
	kAnimationTypeScene = 2
};

class AnimationResource;

struct AnimationSlot {
	int16 animationType;
	int16 fileIndex;
	AnimationResource *anim;
};

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

private:
	CometEngine *_vm;
	AnimationSlot _animationSlots[kAnimationSlotCount];

	int findAnimationSlot(int16 animationType, int16 fileIndex);
	int findFreeAnimationSlot();
};

} // End of namespace Comet

#endif
