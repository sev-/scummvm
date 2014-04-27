/*
 * Drawable.h
 *
 *  Created on: Jun 23, 2013
 *      Author: omergilad
 */

#ifndef DRAWABLE_H_
#define DRAWABLE_H_

#include <memory>
#include <backends/platform/android/AndroidBitmap.h>
#include "Animation.h"


class Animation;
typedef std::shared_ptr<Animation> AnimationPtr;


class Drawable {

public:

	Drawable();

	float getAlpha() const;

	void setAlpha(float alpha);

	AnimationPtr getAnimation() const;

	void setAnimation(AnimationPtr animation);

	AndroidBitmap* getBitmap() const;

	void setBitmap(AndroidBitmap* bitmap);

	float getPositionX() const;

	void setPositionX(float positionX);

	float getPositionY() const;

	void setPositionY(float positionY);

	float getWidth() const;

	void setWidth(float width);

	float getHeightByRatio();

	void updateAnimation(long currentTime);

	bool isAnimationFinished();

	static void setDisplayRatio(float ratio);

private:

	AndroidBitmap* mBitmap;
	float mPositionX;
	float mPositionY;
	float mWidth;
	float mAlpha;
	AnimationPtr mAnimation;

	static float sDisplayRatio;

};

typedef std::shared_ptr<Drawable> DrawablePtr;

#endif /* DRAWABLE_H_ */
