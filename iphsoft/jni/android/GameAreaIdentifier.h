/*
 * GameAreaIdentifier.h
 *
 *  Created on: May 23, 2013
 *      Author: omergilad
 */

#ifndef GAMEAREAIDENTIFIER_H_
#define GAMEAREAIDENTIFIER_H_

#include "Constants.h"

enum GameArea
{
	GOBLIN_FORTRESS_WITH_PAPER_ON_FLOOR,
	HEBREW_TITLE,
	OUTSIDE_DRAGON_CAVE_WITH_HOOK,
	BEFORE_PUDDLE,
	PUDDLE_AFTER_WATER,
	PUDDLE_WATER,
	OTHER
};

class GameAreaIdentifier {

public:

	static GameArea identifyGameArea(const char* gamePixels);

	static void dumpScreenLineBytes(const char* ptr, int line);

private:

	static inline bool checkColor(const char* ptr, int x, int y, int color)
	{
		return (ptr[y * GAME_SCREEN_WIDTH + x] == color);
	}

};

#endif /* GAMEAREAIDENTIFIER_H_ */
