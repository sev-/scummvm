/*
 * ScaleAnimation.h
 *
 *  Created on: Jul 7, 2013
 *      Author: omergilad
 */

#ifndef SCALEANIMATION_H_
#define SCALEANIMATION_H_

#include "Animation.h"


class ScaleAnimation: public Animation {
public:
	ScaleAnimation();
	virtual ~ScaleAnimation();

	float getEndWidth() const;
	void setEndWidth(float endWidth);

	float getStartWidth() const;
	void setStartWidth(float startWidth);
	float getCenterX() const;
	void setCenterX(float centerX);
	float getCenterY() const;
	void setCenterY(float centerY);

private:
	virtual void updateInternal(Drawable& drawable, float interpolation);
	float mStartWidth;
	float mEndWidth;
	float mCenterX;
	float mCenterY;
};

#endif /* SCALEANIMATION_H_ */
