/*
 * AlphaAnimation.h
 *
 *  Created on: Jun 23, 2013
 *      Author: omergilad
 */

#ifndef ALPHAANIMATION_H_
#define ALPHAANIMATION_H_

#include "Animation.h"


class AlphaAnimation: public Animation {
public:

	AlphaAnimation();
	virtual ~AlphaAnimation();

	float getEndAlpha() const;

	void setEndAlpha(float endAlpha);

	float getStartAlpha() const;

	void setStartAlpha(float startAlpha);

protected:

	virtual void updateInternal(Drawable& drawable, float interpolation);

	float mStartAlpha;
	float mEndAlpha;
};

#endif /* ALPHAANIMATION_H_ */
