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

#include "gui/animation/ScaleAnimation.h"
#include "gui/animation/Drawable.h"

ScaleAnimation::ScaleAnimation() : _endWidth(0), _endWidthFactor(0) {
}

float ScaleAnimation::getEndWidth() const {
	return _endWidth;
}

void ScaleAnimation::setEndWidth(float endWidth) {
	_endWidth = endWidth;
}

float ScaleAnimation::getEndWidthFactor() const {
	return _endWidthFactor;
}

void ScaleAnimation::setEndWidthFactor(float endWidthFactor) {
	_endWidthFactor = endWidthFactor;
}

float ScaleAnimation::getStartWidth() const {
	return _startWidth;
}

void ScaleAnimation::setStartWidth(float startWidth) {
	_startWidth = startWidth;
}

void ScaleAnimation::updateInternal(Drawable *drawable, float interpolation) {

	// If start width was set as 0 -> use the current width as the start dimension
	if (_startWidth == 0) {
		_startWidth = drawable->getWidth();
	}

	// If end width was set as 0 - multiply the start width by the given factor
	if (_endWidth == 0) {
		_endWidth = _startWidth * _endWidthFactor;
	}

	// Calculate width based on interpolation
	float width = _startWidth * (1 - interpolation) + _endWidth * interpolation;
	drawable->setWidth(width);
}
