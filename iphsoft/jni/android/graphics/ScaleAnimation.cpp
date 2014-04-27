/*
 * ScaleAnimation.cpp
 *
 *  Created on: Jul 7, 2013
 *      Author: omergilad
 */

#include "ScaleAnimation.h"

ScaleAnimation::ScaleAnimation() {
	// TODO Auto-generated constructor stub

}

ScaleAnimation::~ScaleAnimation() {
	// TODO Auto-generated destructor stub
}

float ScaleAnimation::getEndWidth() const {
	return mEndWidth;
}

void ScaleAnimation::setEndWidth(float endWidth) {
	mEndWidth = endWidth;
}

float ScaleAnimation::getStartWidth() const {
	return mStartWidth;
}

void ScaleAnimation::setStartWidth(float startWidth) {
	mStartWidth = startWidth;
}

float ScaleAnimation::getCenterX() const {
	return mCenterX;
}

void ScaleAnimation::setCenterX(float centerX) {
	mCenterX = centerX;
}

float ScaleAnimation::getCenterY() const {
	return mCenterY;
}

void ScaleAnimation::setCenterY(float centerY) {
	mCenterY = centerY;
}

void ScaleAnimation::updateInternal(Drawable& drawable, float interpolation) {
	// Calculate width based on interpolation
	float width = mStartWidth * (1 - interpolation) + mEndWidth * interpolation;
	drawable.setWidth(width);

	// Center X value
	drawable.setPositionX(mCenterX - width / 2);

	// Calculate height based on ratio
	float height = drawable.getHeightByRatio();

	// Center Y value
	drawable.setPositionY(mCenterY - height / 2);

}

