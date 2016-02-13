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

#ifndef COMET_COMET_GUI_H
#define COMET_COMET_GUI_H

#include "comet/comet.h"
#include "comet/task.h"
#include "graphics/surface.h"

namespace Comet {

enum GuiPageIdent {
	kGuiInventory,
	kGuiCommandBar,
	kGuiJournal,
	kGuiTownMap,
	kGuiMainMenu,
	kGuiOptionsMenu,
	kGuiPuzzle,
	kGuiSaveMenu,
	kGuiLoadMenu
};

class Gui;

class GuiPage : public CometTaskBase {
public:
	GuiPage(CometEngine *vm);
	virtual ~GuiPage() {};
	virtual void enter();
	virtual void leave();
	virtual void draw() {};
	virtual int getResult() { return 0; }
protected:
	void drawIcon(int elementIndex, int x = 0, int y = 0);
	void drawAnimatedIcon(uint frameListIndex, int x, int y, uint animFrameCounter);
	void drawAnimatedInventoryIcon(uint frameListIndex, int x, int y, uint animFrameCounter);
	void drawAnimatedIconSprite(AnimationResource *animation, uint frameListIndex, int x, int y, uint animFrameCounter);
};

class GuiInventory : public GuiPage {
public:
	GuiInventory(CometEngine *vm) : GuiPage(vm) {};
	virtual void enter();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive() { return _inventoryStatus == 0; }
	virtual int getResult() { return _inventoryStatus; }
protected:
	int _inventoryStatus;
	int _inventoryAction;
	int _mouseSelectedItem;
	Common::Array<uint16> _items;
	uint _firstItem, _currentItem, _animFrameCounter;
	byte _selectionColor;
	void drawInventory();
};

class GuiJournal : public GuiPage {
public:
	GuiJournal(CometEngine *vm) : GuiPage(vm) {};
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive() { return _bookStatus == 0; }
	virtual int getResult() { return _bookStatus; }
protected:
	int _currPageNumber, _pageNumber, _pageCount, _talkPageNumber;
	int _bookStatus;
	int _bookAction;
	int _mouseSelectedRect;
	bool _firstUpdate;
	void drawBookPage(byte fontColor);
	void bookTurnPageNext();
	void bookTurnPagePrev();
	void bookTurnPageTextFadeInEffect();
	void bookTurnPageTextFadeOutEffect();
};

class GuiCommandBar : public GuiPage {
public:
	GuiCommandBar(CometEngine *vm) : GuiPage(vm), _commandBarSelectedItem(0) {};
	virtual void enter();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive() { return _commandBarStatus == 0; }
	virtual int getResult() { return _commandBarStatus; }
	virtual void draw();
protected:
	int _commandBarStatus;
	int _commandBarAction;
	int _commandBarSelectedItem;
	uint _animFrameCounter;
	void selectNextItem();
	void selectPrevItem();
	void drawCommandBar(int selectedItem);
	bool isMuseum() const { return _vm->getGameID() == GID_MUSEUM; }
};

class GuiTownMap : public GuiPage {
public:
	GuiTownMap(CometEngine *vm) : GuiPage(vm) {};
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive() { return _mapStatus == 0; }
	virtual int getResult() { return _mapStatus; }
protected:
	int _mapRectX1, _mapRectX2, _mapRectY1, _mapRectY2;
	int _mapStatus;
	int _mapAction;
	uint16 _sceneBitMaskStatus;
	uint16 _sceneStatus1, _sceneStatus2, _sceneStatus3, _sceneStatus4;
	int16 _cursorX, _cursorY;
	int16 _prevCursorX, _prevCursorY;
	int16 _locationNumber;
	int16 _currMapLocation, _selectedMapLocation;
};

class GuiMainMenu : public GuiPage {
public:
	GuiMainMenu(CometEngine *vm) : GuiPage(vm) {};
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive() { return _mainMenuStatus == 0; }
	virtual int getResult() { return _mainMenuStatus; }
	virtual void draw();
protected:
	int _mainMenuStatus;
	int _mainMenuAction;
	int _mouseSelectedItem;
	int _mainMenuSelectedItem;
	void drawMainMenu(int selectedItem);
};

class GuiOptionsMenu : public GuiPage {
public:
	GuiOptionsMenu(CometEngine *vm) : GuiPage(vm) {};
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive() { return _optionsMenuStatus == 0; }
	virtual int getResult() { return _optionsMenuStatus; }
protected:
	int _optionsMenuStatus;
	int _optionsMenuAction;
	int _musicVolumeDiv, _currMusicVolumeDiv, _digiVolumeDiv, _textSpeed, _gameSpeed, _language;
	uint _animFrameCounter;
	int16 _mouseSliderPos;
	int _mouseSelectedItem;
	int _optionsMenuSelectedItem;
	int _optionIncr;
	void drawOptionsMenu();
};

class GuiPuzzle : public GuiPage {
public:
	GuiPuzzle(CometEngine *vm) : GuiPage(vm) {};
	virtual void enter();
	virtual void leave();
	virtual void update();
	virtual void handleEvent(Common::Event &event);
	virtual bool isActive() { return _puzzleStatus == 0; }
	virtual int getResult() { return _puzzleStatus; }
protected:
	int _puzzleStatus;
	int _puzzleCursorX, _puzzleCursorY;
	uint16 _puzzleTiles[6][6];
	AnimationResource *_puzzleSprite;
	int _puzzleTableRow, _puzzleTableColumn;
	Graphics::Surface *_fingerBackground;
	int16 _prevFingerX, _prevFingerY;
	bool _isTileMoving;
	void initPuzzle();
	void loadFingerCursor();
	void drawFinger();
	void drawField();
	void drawTile(int columnIndex, int rowIndex, int xOffs, int yOffs);
	void moveCurrentTile();
	void updateCurrentTile();
	void selectTileByMouse(int mouseX, int mouseY);
	void moveTileColumnUp();
	void moveTileColumnDown();
	void moveTileRowLeft();
	void moveTileRowRight();
	bool isPuzzleSolved();
	void playTileMoveSound();
};

#if 0 // TODO
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
#endif

class Gui {
public:
	Gui(CometEngine *vm);
	~Gui();
	bool isActive() { return _currPage != 0 && !_stack.empty(); }
	void onEnterPage(GuiPage *page);
	void onLeavePage(GuiPage *page);
	GuiPage *getGuiPage(GuiPageIdent page);
	int getLastResult() const { return _lastResult; }
protected:
	CometEngine *_vm;
	GuiPage *_currPage;
	Common::Array<GuiPage*> _stack;
	byte *_gameScreen;
	GuiInventory *_guiInventory;
	GuiCommandBar *_guiCommandBar;
	GuiJournal *_guiJournal;
	GuiTownMap *_guiTownMap;
	GuiMainMenu *_guiMainMenu;
	GuiOptionsMenu *_guiOptionsMenu;
	GuiPuzzle *_guiPuzzle;
	int _lastResult;
//	GuiSaveLoadMenu *_guiSaveLoadMenu;
};

}

#endif
