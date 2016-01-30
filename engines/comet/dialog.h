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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMET_DIALOG_H
#define COMET_DIALOG_H

#include "common/util.h"
#include "common/array.h"

#include "comet/comet.h"
#include "comet/script.h"

namespace Comet {

struct DialogItem {
	int16 index;
	byte *text;
	uint16 scriptIp;
	GuiRectangle rect;
};

class Dialog {
public:

	Dialog(CometEngine *vm);
	~Dialog();

	void start(Script *script);
	void stop();
	void update();
	bool isRunning() const { return _isRunning; }
	uint16 getChoiceScriptIp();
	
protected:

	CometEngine *_vm;

	int _selectedItemIndex, _selectedItemIndex2;
	int _introTextIndex, _textX, _textY;
	byte _textColor;
	int _textColorInc;
	byte _frameColor;
	int _frameColorInc;
	bool _isRunning;
	Common::Array<DialogItem> _items;

	void drawTextBubbles();
	
};

}

#endif
