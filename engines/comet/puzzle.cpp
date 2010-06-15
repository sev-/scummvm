#include "comet/comet.h"
#include "comet/animation.h"
#include "comet/animationmgr.h"
#include "comet/font.h"
#include "comet/screen.h"

namespace Comet {

int CometEngine::runPuzzle() {

#define PUZZLE_CHEAT
#ifdef PUZZLE_CHEAT
	static const uint16 puzzleInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4, 8,13, 0},
		{0, 1, 5, 9,14, 0},
		{0, 2, 6,10,15, 0},
		{0, 3, 7,11,12, 0},
		{0, 0, 0, 0, 0, 0}};
#else
	static const uint16 puzzleInitialTiles[6][6] = {
		{0, 0, 0, 0, 0, 0},
		{0, 0, 4,10,15, 0},
		{0, 5,11,13,12, 0},
		{0, 8, 1, 2, 6, 0},
		{0, 3, 7, 9,14, 0},
		{0, 0, 0, 0, 0, 0}};
#endif
	static const RectItem puzzleTileRects[] = {
		{118, 44, 142, 59, 0},
		{143, 44, 167, 59, 1},
		{168, 44, 192, 59, 2},
		{193, 44, 217, 59, 3},
		{217, 59, 231, 83, 4},
		{217, 84, 231, 108, 5},
		{217, 109, 231, 133, 6},
		{217, 134, 231, 158, 7},
		{118, 158, 142, 171, 8},
		{143, 158, 167, 171, 9},
		{168, 158, 192, 171, 10},
		{193, 158, 217, 171, 11},
		{103, 59, 118, 83, 12},
		{103, 84, 118, 108, 13},
		{103, 109, 118, 133, 14},
		{103, 134, 118, 158, 15},
		{103, 44, 118, 59, 16},
		{217, 44, 231, 59, 17},
		{217, 158, 231, 171, 18},
		{103, 158, 118, 171, 19},
		{119, 60, 216, 157, 20}
	};

	static const struct { int col, row; } rectToColRow[] = {
		{1, 0}, {2, 0}, {3, 0}, {4, 0}, 
		{5, 1}, {5, 2}, {5, 3}, {5, 4}, 
		{1, 5}, {2, 5}, {3, 5}, {4, 5}, 
		{0, 1}, {0, 2}, {0, 3}, {0, 4}, 
		{0, 0}, {5, 0}, {5, 5}, {0, 5} 
	};

	int puzzleStatus = 0;

	_puzzleSprite = _animationMan->loadAnimationResource("A07.PAK", 24);

	// Initialize the puzzle state
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 6; j++)
			_puzzleTiles[i][j] = puzzleInitialTiles[i][j];

	_puzzleCursorX = 0;
	_puzzleCursorY = 0;

	while (puzzleStatus == 0) {

		int selectedTile;

		handleEvents();

		_puzzleCursorX = CLIP(_mouseX, 103, 231);
		_puzzleCursorY = CLIP(_mouseY, 44, 171);

		if (_mouseX != _puzzleCursorX || _mouseY != _puzzleCursorY)
			_system->warpMouse(_puzzleCursorX, _puzzleCursorY);

		selectedTile = findRect(puzzleTileRects, _mouseX, _mouseY, 21, -1);
		if (selectedTile >= 0) {
			if (selectedTile >= 0 && selectedTile < 20) {
				_puzzleTableColumn = rectToColRow[selectedTile].col;
				_puzzleTableRow = rectToColRow[selectedTile].row;
			} else if (selectedTile == 20) {
				_puzzleTableColumn = (_puzzleCursorX - 119) / 24 + 1;
				_puzzleTableRow = (_puzzleCursorY - 60) / 24 + 1; 
			} else {
				_puzzleTableColumn = 0;
				_puzzleTableRow = 0;
			}
		}

		puzzleDrawField();
		_screen->update();
		_system->delayMillis(40); // TODO

		if (_keyScancode != Common::KEYCODE_INVALID) {
			
			bool selectionChanged = false;

			switch (_keyScancode) {
			case Common::KEYCODE_UP:
				if (_puzzleTableRow > 0) {
					_puzzleTableRow--;
					selectionChanged = true;
				}
				break;
			case Common::KEYCODE_DOWN:
				if (_puzzleTableRow < 5) {
					_puzzleTableRow++;
					selectionChanged = true;
				}
				break;
			case Common::KEYCODE_LEFT:
				if (_puzzleTableColumn > 0) {
					_puzzleTableColumn--;
					selectionChanged = true;
				}
				break;
			case Common::KEYCODE_RIGHT:
				if (_puzzleTableColumn < 5) {
					_puzzleTableColumn++;
					selectionChanged = true;
				}
				break;
			default:
				break;			
			}						
			
			if (selectionChanged) {
				selectedTile = 20;
				if (_puzzleTableColumn == 0 && _puzzleTableRow == 0)
					selectedTile = 16;
				else if (_puzzleTableColumn == 5 && _puzzleTableRow == 0)
					selectedTile = 17;
				else if (_puzzleTableColumn == 5 && _puzzleTableRow == 5)
					selectedTile = 18;
				else if (_puzzleTableColumn == 0 && _puzzleTableRow == 5)
					selectedTile = 19;
				else if (_puzzleTableColumn > 0 && _puzzleTableColumn < 5 && _puzzleTableRow == 0)
					selectedTile = _puzzleTableColumn - 1;
				else if (_puzzleTableColumn > 0 && _puzzleTableColumn < 5 && _puzzleTableRow == 5)
					selectedTile = _puzzleTableColumn + 7;
				else if (_puzzleTableRow > 0 && _puzzleTableRow < 5 && _puzzleTableColumn == 0)
					selectedTile = _puzzleTableRow + 11;
				else if (_puzzleTableRow > 0 && _puzzleTableRow < 5 && _puzzleTableColumn == 5)
					selectedTile = _puzzleTableRow + 3;

				if (selectedTile != 20) {
					_puzzleCursorX = (puzzleTileRects[selectedTile].x + puzzleTileRects[selectedTile].x2) / 2;
					_puzzleCursorY = (puzzleTileRects[selectedTile].y + puzzleTileRects[selectedTile].y2) / 2;
				} else {
					_puzzleCursorX = (_puzzleTableColumn - 1) * 24 + 130;
					_puzzleCursorY = (_puzzleTableRow - 1) * 24 + 71;
				}
				
				// Mouse warp to selected tile
				_system->warpMouse(_puzzleCursorX, _puzzleCursorY);

			}

		}

		if (_keyScancode == Common::KEYCODE_ESCAPE || _rightButton) {
			puzzleStatus = 1;
		} else if (_keyScancode == Common::KEYCODE_RETURN || _leftButton) {
			if (_puzzleTableColumn == 0 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileRow(_puzzleTableRow, -1);
			} else if (_puzzleTableColumn == 5 && _puzzleTableRow >= 1 && _puzzleTableRow <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileRow(_puzzleTableRow, 1);
			} else if (_puzzleTableRow == 0 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileColumn(_puzzleTableColumn, -1);
			} else if (_puzzleTableRow == 5 && _puzzleTableColumn >= 1 && _puzzleTableColumn <= 4) {
				// TODO: playSampleFlag, play sample
				puzzleMoveTileColumn(_puzzleTableColumn, 1);
			}
			if (puzzleTestIsSolved())
				puzzleStatus = 2;
		} else {
			waitForKeys();
		}				
			
	}		

	delete _puzzleSprite;

	return puzzleStatus == 2 ? 2 : 0;

}

void CometEngine::puzzleDrawFinger() {
	_screen->drawAnimationElement(_puzzleSprite, 18, _puzzleCursorX, _puzzleCursorY);
}

void CometEngine::puzzleDrawField() {
	memcpy(_sceneBackground, _screen->getScreen(), 320 * 200);
	_screen->drawAnimationElement(_puzzleSprite, 17, 0, 0);
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++) {
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++) {
			puzzleDrawTile(columnIndex, rowIndex, 0, 0);		
		}
	}
	puzzleDrawFinger();
}

void CometEngine::puzzleDrawTile(int columnIndex, int rowIndex, int xOffs, int yOffs) {
	_screen->drawAnimationElement(_puzzleSprite, _puzzleTiles[columnIndex][rowIndex], 
		119 + (columnIndex - 1) * 24 + xOffs, 60 + (rowIndex - 1) * 24 + yOffs);
}

void CometEngine::puzzleMoveTileColumn(int columnIndex, int direction) {
	if (direction < 0) {
		_puzzleTiles[columnIndex][5] = _puzzleTiles[columnIndex][1];
		for (int yOffs = 0; yOffs < 24; yOffs += 2) {
			//_screen->setClipY(60, 156);//TODO: Clipping bug, crashes!
			for (int rowIndex = 1; rowIndex <= 5; rowIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, 0, -yOffs);				
			}
			//_screen->setClipY(0, 199);//s.a.
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int rowIndex = 0; rowIndex <= 4; rowIndex++) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex + 1];
		}
		_puzzleTiles[columnIndex][5] = _puzzleTiles[columnIndex][1];
	} else {
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
		for (int yOffs = 0; yOffs < 24; yOffs += 2) {
			//_screen->setClipY(60, 156);//TODO: Clipping bug, crashes!
			for (int rowIndex = 0; rowIndex <= 4; rowIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, 0, yOffs);				
			}
			//_screen->setClipY(0, 199);//s.a.
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int rowIndex = 5; rowIndex >= 1; rowIndex--) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex][rowIndex - 1];
		}
		_puzzleTiles[columnIndex][0] = _puzzleTiles[columnIndex][4];
	}
}

void CometEngine::puzzleMoveTileRow(int rowIndex, int direction) {
	if (direction < 0) {
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_screen->setClipX(120, 215);
			for (int columnIndex = 1; columnIndex <= 5; columnIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, -xOffs, 0);				
			}
			_screen->setClipX(0, 319);
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int columnIndex = 0; columnIndex <= 4; columnIndex++) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex + 1][rowIndex];
		}
		_puzzleTiles[5][rowIndex] = _puzzleTiles[1][rowIndex];
	} else {
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
		for (int xOffs = 0; xOffs < 24; xOffs += 2) {
			_screen->setClipX(120, 215);
			for (int columnIndex = 0; columnIndex <= 4; columnIndex++) {
				puzzleDrawTile(columnIndex, rowIndex, xOffs, 0);				
			}
			_screen->setClipX(0, 319);
			puzzleDrawFinger();
			_screen->update();
			_system->delayMillis(40); // TODO
		}
		for (int columnIndex = 5; columnIndex >= 1; columnIndex--) {
			_puzzleTiles[columnIndex][rowIndex] = _puzzleTiles[columnIndex - 1][rowIndex];
		}
		_puzzleTiles[0][rowIndex] = _puzzleTiles[4][rowIndex];
	}
}

bool CometEngine::puzzleTestIsSolved() {
	int matchingTiles = 0;
	for (int columnIndex = 1; columnIndex <= 4; columnIndex++) {
		for (int rowIndex = 1; rowIndex <= 4; rowIndex++) {
			if (_puzzleTiles[columnIndex][rowIndex] == (rowIndex - 1) * 4 + (columnIndex - 1))
				matchingTiles++;					
		}
	}
	return matchingTiles == 16;
}

} // End of namespace Comet
