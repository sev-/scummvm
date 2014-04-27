/*
 * Animation.h
 *
 *  Created on: Jun 23, 2013
 *      Author: omergilad
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <memory>

//class Animation;
//typedef std::shared_ptr<Animation> AnimationPtr;

#include "Drawable.h"
#include "Interpolator.h"


struct Drawable;

class Animation {
public:
	Animation();
	virtual ~Animation() = 0;

	virtual void update(Drawable& drawable, long currentTime);

	/**
	 * Set start time in millis
	 */
	virtual void start(long currentTime);

	/**
	 * Set duration in millis
	 */
	virtual void setDuration(long duration);

	virtual bool isFinished() const;

	bool isFinishOnEnd() const;

	void setFinishOnEnd(bool finishOnEnd);

	InterpolatorPtr getInterpolator() const;
	void setInterpolator(InterpolatorPtr interpolator);

protected:

	void finishAnimation();

	virtual void updateInternal(Drawable& drawable, float interpolation);

	long mStartTime;
	long mDuration;
	bool mFinished;
	bool mFinishOnEnd;
	InterpolatorPtr mInterpolator;

};

typedef std::shared_ptr<Animation> AnimationPtr;

#endif /* ANIMATION_H_ */
