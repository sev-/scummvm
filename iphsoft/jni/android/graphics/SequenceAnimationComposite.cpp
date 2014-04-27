/*
 * SequenceAnimation.cpp
 *
 *  Created on: Jul 6, 2013
 *      Author: omergilad
 */

#include "SequenceAnimationComposite.h"

SequenceAnimationComposite::SequenceAnimationComposite() {
}

SequenceAnimationComposite::~SequenceAnimationComposite() {

//	LOGD("SequenceAnimationComposite::~SequenceAnimationComposite: ");
}

void SequenceAnimationComposite::start(long currentTime) {

	Animation::start(currentTime);

	// The first animation in the sequence should a start time equal to this sequence
	if (mSequence.size() >= 1) {
		mSequence[0]->start(currentTime);
	}

	// Set the index to 0
	mIndex = 0;
}

void SequenceAnimationComposite::addAnimation(AnimationPtr animation) {

	mSequence.push_back(animation);
}

void SequenceAnimationComposite::update(Drawable& drawable, long currentTime) {

	uint16 sequenceSize = mSequence.size();

	// Check index bounds
	if (mIndex >= sequenceSize) {
		return;
	}

	// Get the current animation in the sequence
	AnimationPtr anim = mSequence[mIndex];

	// Update the drawable
	anim->update(drawable, currentTime);

	// Check if the current animation is finished
	if (anim->isFinished()) {

		// Increase the index - move to the next animation
		++mIndex;

		if (mIndex >= sequenceSize) {
			// Finished the sequence

			finishAnimation();
		} else {
			// Set the start time for the next animation

			mSequence[mIndex]->start(currentTime);

		}

	}
}

