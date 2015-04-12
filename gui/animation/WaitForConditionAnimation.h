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

#ifndef GUI_ANIMATION_WAITFORCONDITIONANIMATION_H
#define GUI_ANIMATION_WAITFORCONDITIONANIMATION_H

#include "gui/animation/Animation.h"

class Condition {

public:
	virtual ~Condition() {}

	virtual bool evaluate() = 0;
};

typedef Common::SharedPtr<Condition> ConditionPtr;

/**
 * Used for delaying the animation sequence until a certain condition has been met
 */
class WaitForConditionAnimation: public Animation {
public:

	WaitForConditionAnimation() {}
	virtual ~WaitForConditionAnimation() {}

	virtual void update(Drawable *drawable, long currentTime);

	ConditionPtr getCondition() const;
	void setCondition(ConditionPtr condition);

private:
	ConditionPtr _condition;
};

#endif /* GUI_ANIMATION_WAITFORCONDITIONANIMATION_H */
