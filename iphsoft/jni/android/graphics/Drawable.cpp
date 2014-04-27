/*
 * Drawable.cpp
 *
 *  Created on: Jun 23, 2013
 *      Author: omergilad
 */

#include "Drawable.h"



Drawable::Drawable()
		: mBitmap(NULL), mPositionX(0), mPositionY(0), mWidth(0), mAlpha(1), mAnimation(NULL) {
}

void Drawable::updateAnimation(long currentTime) {
	if (mAnimation != NULL) {
		mAnimation->update(*this, currentTime);
	}
}

bool Drawable::isAnimationFinished()
{
	if (mAnimation != NULL)
	{
		return mAnimation->isFinished();
	}

	return false;
}


float Drawable::getAlpha() const {
	return mAlpha;
}

void Drawable::setAlpha(float alpha) {
	mAlpha = alpha;
}

AnimationPtr Drawable::getAnimation() const {
	return mAnimation;
}

void Drawable::setAnimation(AnimationPtr animation) {
	mAnimation = animation;
}

AndroidBitmap* Drawable::getBitmap() const {
	return mBitmap;
}

void Drawable::setBitmap(AndroidBitmap* bitmap) {
	mBitmap = bitmap;
}

float Drawable::getPositionX() const {
	return mPositionX;
}

void Drawable::setPositionX(float positionX) {
	mPositionX = positionX;
}

float Drawable::getPositionY() const {
	return mPositionY;
}

void Drawable::setPositionY(float positionY) {
	mPositionY = positionY;
}

float Drawable::getWidth() const {
	return mWidth;
}

void Drawable::setWidth(float width) {
	mWidth = width;
}

float Drawable::getHeightByRatio()
{
	return mWidth * mBitmap->ratio * sDisplayRatio;
}

float Drawable::sDisplayRatio = 0;

void Drawable::setDisplayRatio(float ratio)
{
	sDisplayRatio = ratio;
}

