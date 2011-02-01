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

class GuiInventory {
public:
	GuiInventory(CometEngine *vm);
	~GuiInventory();
	int run();
protected:
	CometEngine *_vm;
	byte _selectionColor;
	void drawInventory(Common::Array<uint16> &items, uint firstItem, uint currentItem, uint animFrameCounter);
};

class GuiDiary {
public:
	GuiDiary(CometEngine *vm);
	~GuiDiary();
	int run();
protected:
	CometEngine *_vm;
	int handleReadBook();
	void drawBookPage(int pageTextIndex, int pageTextMaxIndex, byte fontColor);
	void bookTurnPage(bool turnDirection);
	void bookTurnPageTextEffect(bool turnDirection, int pageTextIndex, int pageTextMaxIndex);
};

class GuiCommandBar {
public:
	GuiCommandBar(CometEngine *vm);
	~GuiCommandBar();
	int run();
protected:
	CometEngine *_vm;
	int _commandBarSelectedItem;
	void drawCommandBar(int selectedItem, int animFrameCounter);
	int handleCommandBar();
};

class GuiTownMap {
public:
	GuiTownMap(CometEngine *vm);
	~GuiTownMap();
	int run();
protected:
	CometEngine *_vm;
};

class GuiMainMenu {
public:
	GuiMainMenu(CometEngine *vm);
	~GuiMainMenu();
	int run();
protected:
	CometEngine *_vm;
	int _mainMenuSelectedItem;
	void drawMainMenu(int selectedItem);
};

class GuiOptionsMenu {
public:
	GuiOptionsMenu(CometEngine *vm);
	~GuiOptionsMenu();
	int run();
protected:
	CometEngine *_vm;
	int _optionsMenuSelectedItem;
	void drawOptionsMenu(int selectedItem, int musicVolumeDiv, int sampleVolumeDiv, 
		int textSpeed, int gameSpeed, int language, uint animFrameCounter,
		const GuiRectangle *guiRectangles);
};

class GuiPuzzle {
public:
	GuiPuzzle(CometEngine *vm);
	~GuiPuzzle();
	int run();
protected:
	CometEngine *_vm;
	uint16 _puzzleTiles[6][6];
	AnimationResource *_puzzleSprite;
	int _puzzleTableRow, _puzzleTableColumn;
	int _puzzleCursorX, _puzzleCursorY;
	int runPuzzle();
	void drawFinger();
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

class GuiSaveLoadMenu {
public:
	GuiSaveLoadMenu(CometEngine *vm);
	~GuiSaveLoadMenu();
	int run(bool asSaveMenu);
protected:
	CometEngine *_vm;
	SavegameItem _savegames[10];
	void drawSaveLoadMenu(int selectedItem, bool loadOrSave);
	void loadSavegamesList();
	int handleEditSavegameDescription(int savegameIndex);
	void drawSavegameDescription(Common::String &description, int savegameIndex);
};

class Gui {
public:
	Gui(CometEngine *vm);
	~Gui();
	int runInventory();
	int runCommandBar();
	int runDiary();
	int runTownMap();
	int runMainMenu();
	int runOptionsMenu();
	int runPuzzle();
	int runSaveMenu();
	int runLoadMenu();
protected:
	CometEngine *_vm;
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
