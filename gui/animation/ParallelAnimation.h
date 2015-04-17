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

#ifndef GUI_ANIMATION_PARALLELANIMATION_H
#define GUI_ANIMATION_PARALLELANIMATION_H

#include "gui/animation/Animation.h"
#include "common/array.h"

class ParallelAnimation: public Animation {
public:
	ParallelAnimation() {}
	virtual ~ParallelAnimation() {}

	virtual void addAnimation(AnimationPtr animation);

	virtual void update(Drawable *drawable, long currentTime);

	virtual void start(long currentTime);

	virtual void setDuration(long duration);

private:

	Common::Array<AnimationPtr> _animations;
};

#endif /* GUI_ANIMATION_PARALLELANIMATION_H */
