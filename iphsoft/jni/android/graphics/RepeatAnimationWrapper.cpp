/*
 * RepeatAnimationWrapper.cpp
 *
 *  Created on: Jul 7, 2013
 *      Author: omergilad
 */

#include "RepeatAnimationWrapper.h"

RepeatAnimationWrapper::RepeatAnimationWrapper(AnimationPtr animation,
		uint16 timesToRepeat)
		: mAnimation(animation), mTimesToRepeat(timesToRepeat) {
}

RepeatAnimationWrapper::~RepeatAnimationWrapper() {

	//LOGD("RepeatAnimationWrapper::~RepeatAnimationWrapper: ");
}

void RepeatAnimationWrapper::update(Drawable& drawable, long currentTime) {

	// Update wrapped animation
	mAnimation->update(drawable, currentTime);

	// If the animation is finished, increase the repeat count and restart it if needed
	if (mAnimation->isFinished()) {
		++mRepeatCount;
		if (mTimesToRepeat > 0 && mRepeatCount >= mTimesToRepeat) {
			finishAnimation();
		} else {
			mAnimation->start(currentTime);
		}
	}
}

void RepeatAnimationWrapper::start(long currentTime) {

	Animation::start(currentTime);
	mRepeatCount = 0;

	// Start wrapped animation
	mAnimation->start(currentTime);
}

