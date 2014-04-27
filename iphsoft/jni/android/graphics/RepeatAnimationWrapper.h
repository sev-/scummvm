/*
 * RepeatAnimationWrapper.h
 *
 *  Created on: Jul 7, 2013
 *      Author: omergilad
 */

#ifndef REPEATANIMATIONWRAPPER_H_
#define REPEATANIMATIONWRAPPER_H_

#include "Animation.h"


class RepeatAnimationWrapper: public Animation {
public:

	/**
	 * Animation - animation to repeat
	 *
	 * timesToRepeat - 0 means infinite
	 */
	RepeatAnimationWrapper(AnimationPtr animation, uint16 timesToRepeat);
	virtual ~RepeatAnimationWrapper();

	virtual void update(Drawable& drawable, long currentTime);

	/**
	 * Set start time in millis
	 */
	virtual void start(long currentTime);

private:

	uint16 mTimesToRepeat;
	uint16 mRepeatCount;

	AnimationPtr mAnimation;

};

#endif /* REPEATANIMATIONWRAPPER_H_ */
