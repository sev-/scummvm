/*
 * SequenceAnimation.h
 *
 *  Created on: Jul 6, 2013
 *      Author: omergilad
 */

#ifndef SEQUENCEANIMATION_H_
#define SEQUENCEANIMATION_H_

#include "Animation.h"
#include <vector>

using std::vector;


class SequenceAnimationComposite: public Animation {
public:
	SequenceAnimationComposite();
	virtual ~SequenceAnimationComposite();

	virtual void addAnimation(AnimationPtr animation);

	virtual void update(Drawable& drawable, long currentTime);

	virtual void start(long currentTime);

private:

	uint16 mIndex;
	vector<AnimationPtr> mSequence;
};

#endif /* SEQUENCEANIMATION_H_ */
