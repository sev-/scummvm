/*
 * GameAreaIdentifier.cpp
 *
 *  Created on: May 23, 2013
 *      Author: omergilad
 */

#include "GameAreaIdentifier.h"
#include "loghelper.h"

#define LOG_COLOR(gamePixels, x, y) LOGD("pixel %d %d: %d", x, y, gamePixels[y * GAME_SCREEN_WIDTH + x])

#define GOBLIN_FORTRESS_P1 0, 10, 0
#define GOBLIN_FORTRESS_P2 45, 10, 1
#define GOBLIN_FORTRESS_P3 75, 10, 7
#define GOBLIN_FORTRESS_P4 110, 10, 6
#define GOBLIN_FORTRESS_P5 150, 10, 2
#define GOBLIN_FORTRESS_P6 250, 10, 0

#define GOBLIN_FORTRESS_PAPER_P1 255, 104, 66
#define GOBLIN_FORTRESS_PAPER_P2 256, 104, 67
#define GOBLIN_FORTRESS_PAPER_P3 255, 105, 65
#define GOBLIN_FORTRESS_PAPER_P4 256, 105, 65

#define HEBREW_TITLE_P1 25, 50, 0
#define HEBREW_TITLE_P2 50, 50, 145
#define HEBREW_TITLE_P3 90, 50, 116
#define HEBREW_TITLE_P4 110, 50, 94
#define HEBREW_TITLE_P5 135, 50, 49
#define HEBREW_TITLE_P6 170, 50, 242
#define HEBREW_TITLE_P7 200, 50, 0
#define HEBREW_TITLE_P8 227, 50, 116
#define HEBREW_TITLE_P9 290, 50, 0

#define OUTSIDE_DRAGON_CAVE_P1 312, 12, 0x67
#define OUTSIDE_DRAGON_CAVE_P2 291, 23, 0x61
#define OUTSIDE_DRAGON_CAVE_P3 300, 13, 0x07
#define OUTSIDE_DRAGON_CAVE_P4 288, 21, 0x0d
#define OUTSIDE_DRAGON_CAVE_P5 276, 21, 0x37
#define OUTSIDE_DRAGON_CAVE_P6 232, 18, 0x38
#define OUTSIDE_DRAGON_CAVE_P7 190, 21, 0x0c
#define OUTSIDE_DRAGON_CAVE_P8 110, 20, 0x0f
#define OUTSIDE_DRAGON_CAVE_P9 12, 7, 0x3d

#define BEFORE_PUDDLE_P1 6, 14, 0x03
#define BEFORE_PUDDLE_P2 5, 47, 0x3a
#define BEFORE_PUDDLE_P3 66, 27, 0x1f
#define BEFORE_PUDDLE_P4 135, 8, 0x1f
#define BEFORE_PUDDLE_P5 182, 15, 0x32
#define BEFORE_PUDDLE_P6 208, 8, 0x3c
#define BEFORE_PUDDLE_P7 235, 15, 0x13
#define BEFORE_PUDDLE_P8 264, 26, 0x34
#define BEFORE_PUDDLE_P9 301, 17, 0x1e

#define PUDDLE_AFTER_WATER_P1 3, 11, 0x07
#define PUDDLE_AFTER_WATER_P2 12, 11, 0x8c
#define PUDDLE_AFTER_WATER_P3 21, 15, 0x06
#define PUDDLE_AFTER_WATER_P4 32, 15, 0x84
#define PUDDLE_AFTER_WATER_P5 48, 15, 0x57
#define PUDDLE_AFTER_WATER_P6 85, 10, 0x5b
#define PUDDLE_AFTER_WATER_P7 243, 8, 0x08
#define PUDDLE_AFTER_WATER_P8 267, 27, 0x3d
#define PUDDLE_AFTER_WATER_P9 297, 2, 0x3c

#define PUDDLE_WATER_P1 138, 88, 0x46
#define PUDDLE_WATER_P2 140, 95, 0x49
#define PUDDLE_WATER_P3 139, 110, 0x4b
#define PUDDLE_WATER_P4 138, 123, 0x4a

GameArea GameAreaIdentifier::identifyGameArea(const char* gamePixels) {

	// Check for goblin fortress with paper on floor
	if (checkColor(gamePixels, GOBLIN_FORTRESS_P1)
			&& checkColor(gamePixels, GOBLIN_FORTRESS_P2)
			&& checkColor(gamePixels, GOBLIN_FORTRESS_P3)
			&& checkColor(gamePixels, GOBLIN_FORTRESS_P4)
			&& checkColor(gamePixels, GOBLIN_FORTRESS_P5)
			&& checkColor(gamePixels, GOBLIN_FORTRESS_P6)) {
		// We are in goblin fortress, check paper
		if (checkColor(gamePixels, GOBLIN_FORTRESS_PAPER_P1)
				&& checkColor(gamePixels, GOBLIN_FORTRESS_PAPER_P2)
				&& checkColor(gamePixels, GOBLIN_FORTRESS_PAPER_P3)
				&& checkColor(gamePixels, GOBLIN_FORTRESS_PAPER_P4)) {
			return GOBLIN_FORTRESS_WITH_PAPER_ON_FLOOR;
		}
	}

	// Check for hebrew title
	if (checkColor(gamePixels, HEBREW_TITLE_P1)
			&& checkColor(gamePixels, HEBREW_TITLE_P2)
			&& checkColor(gamePixels, HEBREW_TITLE_P3)
			&& checkColor(gamePixels, HEBREW_TITLE_P4)
			&& checkColor(gamePixels, HEBREW_TITLE_P5)
			&& checkColor(gamePixels, HEBREW_TITLE_P6)
			&& checkColor(gamePixels, HEBREW_TITLE_P7)
			&& checkColor(gamePixels, HEBREW_TITLE_P8)
			&& checkColor(gamePixels, HEBREW_TITLE_P9)) {
		return HEBREW_TITLE;
	}

	// Check for outside dragon cave
	if (checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P1)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P2)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P3)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P4)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P5)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P6)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P7)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P8)
			&& checkColor(gamePixels, OUTSIDE_DRAGON_CAVE_P9)) {
		return OUTSIDE_DRAGON_CAVE_WITH_HOOK;
	}

	// Check for before puddle
	if (checkColor(gamePixels, BEFORE_PUDDLE_P1)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P2)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P3)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P4)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P5)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P6)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P7)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P8)
			&& checkColor(gamePixels, BEFORE_PUDDLE_P9)) {
		return BEFORE_PUDDLE;
	}

	// Check for puddle
	if (checkColor(gamePixels, PUDDLE_AFTER_WATER_P1)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P2)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P3)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P4)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P5)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P6)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P7)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P8)
			&& checkColor(gamePixels, PUDDLE_AFTER_WATER_P9)) {

		// Check for water
		if (checkColor(gamePixels, PUDDLE_WATER_P1)
				&& checkColor(gamePixels, PUDDLE_WATER_P2)
				&& checkColor(gamePixels, PUDDLE_WATER_P3)
				&& checkColor(gamePixels, PUDDLE_WATER_P4)) {
			return PUDDLE_WATER;
		} else {
			return PUDDLE_AFTER_WATER;
		}
	}

	return OTHER;
}

void GameAreaIdentifier::dumpScreenLineBytes(const char* ptr, int line) {
	// Get the row pointer
	const char* rowPtr = ptr + line * GAME_SCREEN_WIDTH;

	for (int i = 0; i < GAME_SCREEN_WIDTH; ++i) {
		LOGD("line %d: pixel %d: %d", line, i, rowPtr[i]);
	}
}
