/*
 * WaitForConditionAnimation.cpp
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#include "WaitForConditionAnimation.h"


WaitForConditionAnimation::WaitForConditionAnimation() {

}


WaitForConditionAnimation::~WaitForConditionAnimation() {
}

ConditionPtr WaitForConditionAnimation::getCondition() const {
	return mCondition;
}

void WaitForConditionAnimation::setCondition(ConditionPtr condition) {
	mCondition = condition;
}


void WaitForConditionAnimation::update(Drawable& drawable, long currentTime) {

	// Check the condition - if it has been met, finish.
	if (mCondition != NULL && mCondition->evaluate())
	{
		finishAnimation();
	}
}


