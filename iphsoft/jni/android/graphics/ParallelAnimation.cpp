/*
 * ParallelAnimation.cpp
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#include "ParallelAnimation.h"

ParallelAnimation::ParallelAnimation() {
	// TODO Auto-generated constructor stub

}

ParallelAnimation::~ParallelAnimation() {
	// TODO Auto-generated destructor stub
}

void ParallelAnimation::addAnimation(AnimationPtr animation) {
	mAnimations.push_back(animation);
}

void ParallelAnimation::update(Drawable& drawable, long currentTime) {

	for (AnimationPtr anim : mAnimations) {
		anim->update(drawable, currentTime);
		if (anim->isFinished()) {
			finishAnimation();
		}
	}
}

void ParallelAnimation::start(long currentTime) {

	Animation::start(currentTime);

	for (AnimationPtr anim : mAnimations) {
		anim->start(currentTime);
	}
}

void ParallelAnimation::setDuration(long duration) {
	Animation::setDuration(duration);

	for (AnimationPtr anim : mAnimations) {
		anim->setDuration(duration);
	}
}

