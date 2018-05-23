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

#include "illusions/illusions.h"
#include "illusions/actor.h"
#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/menusystem_duckman.h"

namespace Illusions {

// DuckmanMenuSystem

DuckmanMenuSystem::DuckmanMenuSystem(IllusionsEngine_Duckman *vm)
	: BaseMenuSystem(vm), _vm(vm) {
	clearMenus();
}

DuckmanMenuSystem::~DuckmanMenuSystem() {
	freeMenus();
}

void DuckmanMenuSystem::runMenu(MenuChoiceOffsets menuChoiceOffsets, int16 *menuChoiceOffset,
	uint32 menuId, uint32 duration, uint timeOutMenuChoiceIndex, uint32 menuCallerThreadId) {
	
	debug(0, "DuckmanMenuSystem::runMenu(%08X)", menuId);

	setTimeOutDuration(duration, timeOutMenuChoiceIndex);
	setMenuCallerThreadId(menuCallerThreadId);
	setMenuChoiceOffsets(menuChoiceOffsets, menuChoiceOffset);

	int rootMenuId = convertRootMenuId(menuId | 0x180000);
	BaseMenu *rootMenu = getMenuById(rootMenuId);
	openMenu(rootMenu);

}

void DuckmanMenuSystem::clearMenus() {
	for (int i = 0; i < kDuckmanLastMenuIndex; ++i)
		_menus[i] = 0;
}

void DuckmanMenuSystem::freeMenus() {
	for (int i = 0; i < kDuckmanLastMenuIndex; ++i)
		delete _menus[i];
}

BaseMenu *DuckmanMenuSystem::getMenuById(int menuId) {
	if (!_menus[menuId])
		_menus[menuId] = createMenuById(menuId);
	return _menus[menuId];
}

BaseMenu *DuckmanMenuSystem::createMenuById(int menuId) {
	switch (menuId) {
	case kDuckmanMainMenu:
		return createMainMenu();
	case kDuckmanPauseMenu:
		return createPauseMenu();
	case kDuckmanQueryRestartMenu:
		return createQueryRestartMenu();
	case kDuckmanQueryQuitMenu:
		return createQueryQuitMenu();
	case kDuckmanOptionsMenu:
		return createOptionsMenu();
	default:
		error("DuckmanMenuSystem::createMenuById() Invalid menu id %d", menuId);
	}
}

BaseMenu *DuckmanMenuSystem::createMainMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 0);
	menu->addMenuItem(new MenuActionReturnChoice(this, "Start New Game", 11));
	menu->addMenuItem(new MenuActionLoadGame(this, "Load Saved Game", 1));
	menu->addMenuItem(new MenuActionEnterMenu(this, "Options", kDuckmanOptionsMenu));
	menu->addMenuItem(new MenuActionEnterQueryMenu(this, "Quit Game", kDuckmanQueryQuitMenu, 12));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createLoadGameMenu() {
	return 0; // TODO
}

BaseMenu *DuckmanMenuSystem::createOptionsMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 0);
	menu->addText("              GAME OPTIONS");
	menu->addText("--------------------------------------");
	menu->addMenuItem(new MenuSliderItem(this, "SFX Volume     @@", kSliderSFXVolume, 16, 16, 119, 0, 203, 12));
	menu->addMenuItem(new MenuSliderItem(this, "Music Volume  @@@", kSliderMusicVolume, 16, 16, 119, 0, 203, 12));
	menu->addMenuItem(new MenuSliderItem(this, "Speech Volume ", kSliderSpeechVolume, 16, 16, 119, 0, 203, 12));
	menu->addMenuItem(new MenuSliderItem(this, "Text Duration @@@", kSliderTextDuration, 16, 16, 119, 0, 203, 12));
	menu->addMenuItem(new MenuActionRestoreDefaultSettings(this, "Restore Defaults"));
	menu->addMenuItem(new MenuActionLeaveMenu(this, "Back"));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createPauseMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 1);
	menu->addText("   Game Paused");
	menu->addText("-------------------");
	menu->addMenuItem(new MenuActionReturnChoice(this, "Resume", 21));
	menu->addMenuItem(new MenuActionLoadGame(this, "Load Game", 1));
	// TODO menu->addMenuItem(new MenuActionSaveGame(this, "Save Game", 11)));
	menu->addMenuItem(new MenuActionEnterQueryMenu(this, "Restart Game", kDuckmanQueryRestartMenu, 22));
	menu->addMenuItem(new MenuActionEnterMenu(this, "Options", kDuckmanOptionsMenu));
	menu->addMenuItem(new MenuActionEnterQueryMenu(this, "Quit Game", kDuckmanQueryQuitMenu, 23));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createQueryRestartMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 2);
	menu->addText("Do you really want to restart?");
	menu->addText("-------------------------------");
	menu->addMenuItem(new MenuActionReturnChoice(this, "Yes, let's try again", getQueryConfirmationChoiceIndex()));
	menu->addMenuItem(new MenuActionLeaveMenu(this, "No, just kidding"));
	return menu;
}

BaseMenu *DuckmanMenuSystem::createQueryQuitMenu() {
	BaseMenu *menu = new BaseMenu(this, 0x00120003, 12, 17, 11, 27, 2);
	menu->addText("Do you really want to quit?");
	menu->addText("-------------------------------");
	menu->addMenuItem(new MenuActionReturnChoice(this, "Yes, I'm outta here", getQueryConfirmationChoiceIndex()));
	menu->addMenuItem(new MenuActionLeaveMenu(this, "No, just kidding"));
	return menu;
}

int DuckmanMenuSystem::convertRootMenuId(uint32 menuId) {
	switch (menuId) {
	case 0x180001:
		return kDuckmanMainMenu;
	case 0x180002:
		return kDuckmanPauseMenu;
	/* Debug menus, not implemented
	case 0x180003:
	case 0x180004:
	*/
	/* TODO
	case 0x180005:
	case 0x180006:
	case 0x180007:
	*/
	/* Another pause menu, never called in original
	case 0x180008:
	*/
	default:
		error("DuckmanMenuSystem() Menu ID %08X not found", menuId);
	}
}

bool DuckmanMenuSystem::initMenuCursor() {
	bool cursorInitialVisibleFlag = false;
	Control *cursorControl = _vm->getObjectControl(0x40004);
	if (cursorControl) {
		if (cursorControl->_flags & 1)
			cursorInitialVisibleFlag = false;
		cursorControl->appearActor();
	} else {
		Common::Point pos = _vm->getNamedPointPosition(0x70001);
		_vm->_controls->placeActor(0x50001, pos, 0x60001, 0x40004, 0);
		cursorControl = _vm->getObjectControl(0x40004);
	}
	return cursorInitialVisibleFlag;
}

int DuckmanMenuSystem::getGameState() {
	return _vm->_cursor._gameState;
}

void DuckmanMenuSystem::setMenuCursorNum(int cursorNum) {
	Control *mouseCursor = _vm->getObjectControl(0x40004);
	_vm->setCursorActorIndex(5, cursorNum, 0);
	mouseCursor->startSequenceActor(0x60001, 2, 0);
}

void DuckmanMenuSystem::setGameState(int gameState) {
	_vm->_cursor._gameState = gameState;
}

} // End of namespace Illusions
