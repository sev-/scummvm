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

#include "gui/animation/Drawable.h"

Drawable::Drawable() :
	_bitmap(NULL), _positionX(0), _positionY(0), _width(0), _height(0), _alpha(1),
	_usingSnapshot(false), _shouldCenter(false) {
	_displayRatio = 1.0;
}

Drawable::~Drawable() {
	if (_usingSnapshot) {
		delete _bitmap;
	}
}

void Drawable::updateAnimation(long currentTime) {
	if (_animation.get() != NULL) {
		_animation->update(this, currentTime);
	}
}

bool Drawable::isAnimationFinished() {
	if (_animation.get() != NULL) {
		return _animation->isFinished();
	}

	return false;
}

float Drawable::getAlpha() const {
	return _alpha;
}

void Drawable::setAlpha(float alpha) {
	_alpha = alpha;
}

AnimationPtr Drawable::getAnimation() const {
	return _animation;
}

void Drawable::setAnimation(AnimationPtr animation) {
	_animation = animation;
}

Graphics::TransparentSurface *Drawable::getBitmap() const {
	return _bitmap;
}

void Drawable::setBitmap(Graphics::TransparentSurface *bitmap) {
	_bitmap = bitmap;
}

float Drawable::getPositionX() const {
	return _positionX;
}

void Drawable::setPositionX(float positionX) {
	_positionX = positionX;
}

float Drawable::getPositionY() const {
	return _positionY;
}

void Drawable::setPositionY(float positionY) {
	_positionY = positionY;
}

float Drawable::getWidth() const {
	return _width;
}

void Drawable::setWidth(float width) {
	_width = width;
}

float Drawable::getHeight() const {

	if (_height == 0) {
		return getWidth() * _bitmap->getRatio() * _displayRatio;
	}

	return _height;
}

void Drawable::setHeight(float height) {
	_height = height;
}

void Drawable::setDisplayRatio(float ratio) {
	_displayRatio = ratio;
}
