/*
 * ParallelAnimation.h
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#ifndef PARALLELANIMATION_H_
#define PARALLELANIMATION_H_

#include <vector>
#include "Animation.h"

using std::vector;

class ParallelAnimation: public Animation {
public:
	ParallelAnimation();
	virtual ~ParallelAnimation();

	virtual void addAnimation(AnimationPtr animation);

	virtual void update(Drawable& drawable, long currentTime);

	virtual void start(long currentTime);

	virtual void setDuration(long duration);

private:

	vector<AnimationPtr> mAnimations;
};

#endif /* PARALLELANIMATION_H_ */
