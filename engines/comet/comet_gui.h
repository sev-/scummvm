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
	void handleCommandBar();
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

class Gui {
public:
	Gui(CometEngine *vm);
	~Gui();
	int runInventory();
	int runCommandBar();
	int runDiary();
	int runTownMap();
	int runMainMenu();
	int runPuzzle();
protected:
	CometEngine *_vm;
	GuiInventory *_guiInventory;
	GuiCommandBar *_guiCommandBar;
	GuiDiary *_guiDiary;
	GuiTownMap *_guiTownMap;
	GuiMainMenu *_guiMainMenu;
	GuiPuzzle *_guiPuzzle;
};

}

#endif
