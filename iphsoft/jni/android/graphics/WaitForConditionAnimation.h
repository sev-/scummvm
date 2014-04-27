/*
 * WaitForConditionAnimation.h
 *
 *  Created on: Jul 9, 2013
 *      Author: omergilad
 */

#ifndef WAITFORCONDITIONANIMATION_H_
#define WAITFORCONDITIONANIMATION_H_

#include "Animation.h"

class Condition {

public:
	virtual ~Condition() {}

	virtual bool evaluate() = 0;
};

typedef std::shared_ptr<Condition> ConditionPtr;

/**
 * Used for delaying the animation sequence until a certain condition has been met
 */
class WaitForConditionAnimation: public Animation {
public:

	WaitForConditionAnimation();
	virtual ~WaitForConditionAnimation();

	virtual void update(Drawable& drawable, long currentTime);

	ConditionPtr getCondition() const;
	void setCondition(ConditionPtr condition);

private:
	ConditionPtr mCondition;
};

#endif /* WAITFORCONDITIONANIMATION_H_ */
