/*
 * AlphaAnimation.cpp
 *
 *  Created on: Jun 23, 2013
 *      Author: omergilad
 */

#include "AlphaAnimation.h"

AlphaAnimation::AlphaAnimation() {
	// TODO Auto-generated constructor stub

}

AlphaAnimation::~AlphaAnimation() {
	// TODO Auto-generated destructor stub
}

float AlphaAnimation::getEndAlpha() const {
	return mEndAlpha;
}

void AlphaAnimation::setEndAlpha(float endAlpha) {
	mEndAlpha = endAlpha;
}

float AlphaAnimation::getStartAlpha() const {
	return mStartAlpha;
}

void AlphaAnimation::setStartAlpha(float startAlpha) {
	mStartAlpha = startAlpha;
}

void AlphaAnimation::updateInternal(Drawable& drawable, float interpolation) {

	// Calculate alpha value based on properties and interpolation
	drawable.setAlpha(
			mStartAlpha * (1 - interpolation) + mEndAlpha * interpolation);
}

