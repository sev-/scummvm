/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GUI_ANIMATION_DRAWABLE_H
#define GUI_ANIMATION_DRAWABLE_H

#include "common/ptr.h"
#include "graphics/transparent_surface.h"

#include "gui/animation/Animation.h"


class Animation;
typedef Common::SharedPtr<Animation> AnimationPtr;


class Drawable {

public:

	Drawable();

	virtual ~Drawable();

	float getAlpha() const;

	void setAlpha(float alpha);

	AnimationPtr getAnimation() const;

	void setAnimation(AnimationPtr animation);

	Graphics::TransparentSurface *getBitmap() const;

	void setBitmap(Graphics::TransparentSurface *bitmap);

	float getPositionX() const;

	void setPositionX(float positionX);

	float getPositionY() const;

	void setPositionY(float positionY);

	virtual float getWidth() const;

	void setWidth(float width);

	virtual float getHeight() const;

	void setHeight(float width);

	void updateAnimation(long currentTime);

	bool isAnimationFinished();

	void setDisplayRatio(float ratio);

	inline bool shouldCenter() const {
		return _shouldCenter;
	}

	void setShouldCenter(bool shouldCenter) {
		_shouldCenter = shouldCenter;
	}

protected:

	bool _usingSnapshot;

private:

	Graphics::TransparentSurface *_bitmap;
	float _positionX;
	float _positionY;
	float _width;
	float _height;
	float _alpha;
	bool _shouldCenter;
	AnimationPtr _animation;

	float _displayRatio;

};

typedef Common::SharedPtr<Drawable> DrawablePtr;

#endif /* GUI_ANIMATION_DRAWABLE_H */
