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
 * $URL$
 * $Id$
 *
 */

#ifndef COMET_COMET_GUI_H
#define COMET_COMET_GUI_H

#include "comet/comet.h"

namespace Comet {

enum GuiPageIdent {
	kGuiInventory,
	kGuiCommandBar,
	kGuiDiary,
	kGuiTownMap,
	kGuiMainMenu,
	kGuiOptionsMenu,
	kGuiPuzzle,
	kGuiSaveMenu,
	kGuiLoadMenu
};

class GuiPage {
public:
	GuiPage(CometEngine *vm) : _vm(vm) {};
	virtual ~GuiPage() {};
	virtual int run() = 0;
	virtual void draw() = 0;
protected:
	CometEngine *_vm;
};

class GuiInventory : public GuiPage {
public:
	GuiInventory(CometEngine *vm) : GuiPage(vm) {};
	int run();
	void draw();
protected:
	byte _selectionColor;
	void drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter);
};

class GuiDiary : public GuiPage {
public:
	GuiDiary(CometEngine *vm) : GuiPage(vm) {};
	int run();
	void draw();
protected:
	int handleReadBook();
	void drawBookPage(int pageTextIndex, int pageTextMaxIndex, byte fontColor);
	void bookTurnPage(bool turnDirection);
	void bookTurnPageTextEffect(bool turnDirection, int pageTextIndex, int pageTextMaxIndex);
};

class GuiCommandBar : public GuiPage {
public:
	GuiCommandBar(CometEngine *vm) : GuiPage(vm), _commandBarSelectedItem(0) {};
	int run();
	void draw();
protected:
	int _commandBarSelectedItem;
	uint _animFrameCounter;
	void drawCommandBar(int selectedItem);
	int handleCommandBar();
};

class GuiTownMap : public GuiPage {
public:
	GuiTownMap(CometEngine *vm) : GuiPage(vm) {};
	int run();
	void draw();
};

class GuiMainMenu : public GuiPage {
public:
	GuiMainMenu(CometEngine *vm) : GuiPage(vm) {};
	int run();
	void draw();
protected:
	int _mainMenuSelectedItem;
	void drawMainMenu(int selectedItem);
};

class GuiOptionsMenu : public GuiPage {
public:
	GuiOptionsMenu(CometEngine *vm) : GuiPage(vm) {};
	int run();
	void draw();
protected:
	int _optionsMenuSelectedItem;
	void drawOptionsMenu(int selectedItem, int musicVolumeDiv, int sampleVolumeDiv, 
		int textSpeed, int gameSpeed, int language, uint animFrameCounter,
		const GuiRectangle *guiRectangles);
};

class GuiPuzzle : public GuiPage {
public:
	GuiPuzzle(CometEngine *vm) : GuiPage(vm) {};
	int run();
	void draw();
protected:
	uint16 _puzzleTiles[6][6];
	AnimationResource *_puzzleSprite;
	int _puzzleTableRow, _puzzleTableColumn;
	int _puzzleCursorX, _puzzleCursorY;
	int runPuzzle();
	void loadFingerCursor();
	void drawField();
	void drawTile(int columnIndex, int rowIndex, int xOffs, int yOffs);
	void moveTileColumn(int columnIndex, int direction);
	void moveTileRow(int rowIndex, int direction);
	bool testIsSolved();
};

struct SavegameItem {
	Common::String description;
	Common::String filename;
};

class GuiSaveLoadMenu : public GuiPage {
public:
	GuiSaveLoadMenu(CometEngine *vm) : GuiPage(vm) {};
	int run();
	void draw();
	void setAsSaveMenu(bool value) { _asSaveMenu = value; }
protected:
	SavegameItem _savegames[10];
	bool _asSaveMenu;
	void drawSaveLoadMenu(int selectedItem);
	void loadSavegamesList();
	int handleEditSavegameDescription(int savegameIndex);
	void drawSavegameDescription(Common::String &description, int savegameIndex);
};

class Gui {
public:
	Gui(CometEngine *vm);
	~Gui();
	int run(GuiPageIdent page);
protected:
	CometEngine *_vm;
	GuiPage *_currPage;
	Common::Array<GuiPage*> _stack;
	byte *_gameScreen;
	GuiInventory *_guiInventory;
	GuiCommandBar *_guiCommandBar;
	GuiDiary *_guiDiary;
	GuiTownMap *_guiTownMap;
	GuiMainMenu *_guiMainMenu;
	GuiOptionsMenu *_guiOptionsMenu;
	GuiPuzzle *_guiPuzzle;
	GuiSaveLoadMenu *_guiSaveLoadMenu;
};

}

#endif
