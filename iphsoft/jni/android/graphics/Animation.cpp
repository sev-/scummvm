/*
 * Animation.cpp
 *
 *  Created on: Jun 23, 2013
 *      Author: omergilad
 */

#include "Animation.h"

Animation::Animation()
		: mStartTime(0), mDuration(0), mFinished(false), mFinishOnEnd(true), mInterpolator(NULL) {

}

Animation::~Animation() {
}

void Animation::start(long currentTime) {
	mFinished = false;
	mStartTime = currentTime;
}
void Animation::setDuration(long duration) {
	mDuration = duration;
}

void Animation::update(Drawable& drawable, long currentTime) {
	float interpolation;
	if (currentTime < mStartTime) {
		// If the start time is in the future, nothing changes - the interpolated value is 0
		interpolation = 0;
	} else if (currentTime > mStartTime + mDuration) {
		// If the animation is finished, the interpolated value is 1 and the animation is marked as finished
		interpolation = 1;
		finishAnimation();
	} else {
		// Calculate the interpolated value
		interpolation = (currentTime - mStartTime) / (float) (mDuration);
	}

	// Activate the interpolator if present
	if (mInterpolator != NULL)
	{
		interpolation = mInterpolator->interpolate(interpolation);
	}

	updateInternal(drawable, interpolation);
}

void Animation::finishAnimation() {
	if (mFinishOnEnd)
	{
		mFinished = true;
	}
}

void Animation::updateInternal(Drawable& drawable, float interpolation) {
	// Default implementation
}

bool Animation::isFinished() const {
	return mFinished;
}

bool Animation::isFinishOnEnd() const {
	return mFinishOnEnd;
}

void Animation::setFinishOnEnd(bool finishOnEnd) {
	mFinishOnEnd = finishOnEnd;
}

InterpolatorPtr Animation::getInterpolator() const {
	return mInterpolator;
}

void Animation::setInterpolator(InterpolatorPtr interpolator) {
	mInterpolator = interpolator;
}


