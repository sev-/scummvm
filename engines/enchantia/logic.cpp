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

#include "enchantia/enchantia.h"

namespace Enchantia {

void EnchantiaEngine::handleSceneInit(SceneInitItem &sceneInitItem, int16 sceneLink) {
	// Special scene init handling for the ice desert
	int16 x = -1;
	switch (sceneLink) {
	case 75:
		x = (actorSprite().x + _cameraStripX - 183) * 1021 / 969 + 79;
		if (x >= 805 && x < 959)
			x = 805;
		break;
	case 76:
		x = (actorSprite().x + _cameraStripX - 79) * 953 / 1037 + 183;
		break;
	case 77:
		x = (actorSprite().x + _cameraStripX - 142) * 980 / 914 + 110;
		break;
	case 78:
		x = (actorSprite().x + _cameraStripX - 110) * 898 / 996 + 142;
		break;
	case 79:
		x = (actorSprite().x + _cameraStripX - 146) * 956 / 913 + 99;
		break;
	case 80:
		x = (actorSprite().x + _cameraStripX - 99) * 897 / 972 + 146;
		break;
	}
	if (x >= 0) {
		sceneInitItem.cameraStripNum = MAX(0, x - 144) / 32;
		sceneInitItem.x = x - sceneInitItem.cameraStripNum * 32;
	}
}

void EnchantiaEngine::updateScene() {

	static const Point scene72FirePoints[] = {
		{202, 74}, {207, 76}, {212, 78}, {217, 80},
		{222, 82}, {227, 84}, {232, 86}, {237, 87},
		{242, 88}, {247, 89}, {252, 90}};

	static const Point points30[] = {
		{40, 25}, {42, 24}, {43, 52}, {42, 43},
		{43, 34}, {42, 25}};
		
	static const Point beamPoints56[] = {
		{253, 13}, {253, 25}, {261, 19}, {263, 25},
		{263, 13}, {255, 19}};

	static const uint16 spriteDef30[] = {0xAF37, 0xAF40, 0xAF49};
	static const uint16 bird30SpriteDefs[] = {0xB125, 0xB12E};
	static const uint16 moveX30[] = {0xAFE8, 0xAFEA};
	static const uint16 bird30MoveX1[] = {0xB163, 0xB165, 0xB167, 0xB169};
	static const uint16 bird30MoveX2[] = {0xB173, 0xB175, 0xB177, 0xB179};
	static const uint16 spriteDef35[] = {0xB26C, 0xB275, 0xB27E};
	static const uint16 cloud38MoveX[] = {0xB2E0, 0xB2E2};
	static const uint16 bats65MoveX1[] = {0x15C2, 0x15C4, 0x15C6, 0x15C8};
	static const uint16 bats65MoveX2[] = {0x15CF, 0x15D1, 0x15D3, 0x15D5};
	
	SpriteDef *spriteDef;
	SpriteTemplate *spriteTemplate;
	Sprite *sprite;
	int16 deltaX = 0, x = 0, y = 0;
	uint spriteIndex = 0;

	switch (_sceneIndex) {
	
	case kSceneDungeonHall:
		if (_cameraStripNum == 24 && (_sprites[7].status & 2)) {
			_sprites[7].status &= ~2;
			_sprites[7].anim.codeId = 0xADD3;
		}
		if (_cameraStripNum == 0 && _flgCanRunBoxClick && actorSprite().x < 90) {
			removeInventoryItem(idKey);
			queueWalk(55, 173);
			_currWalkInfo = &_walkInfos[0];
			startActorWalk(_currWalkInfo);
			_flgCanRunBoxClick = false;
			_flgCanRunMenu = false;
		}
		break;
		
	case kSceneUnderwater:
		{
			bool insert1 = false;
			_queryBoxX1 = _cameraStripX + _sprites[17].x - 11;
			_queryBoxX2 = _queryBoxX1 + 42;
			_queryBoxY2 = 199;
			if (_spriteResourceType == 5 && actorSprite().anim.index <= 12) {
				insert1 = true;
			} else if (_spriteResourceType != 10 && --_sceneCountdown1 == 0) {
				_sceneCountdown1 = getRandom(50) + 1;
				playSound(17, actorSprite());
				insert1 = true;
			}
			if (insert1) {
				static const uint16 spriteDefs[3] = {0xADDF, 0xADE8, 0xADF1};
				uint count = 2 + getRandom(4);
				for (uint i = 0; i < count; i++) {
					spriteDef = getSpriteDef(spriteDefs[getRandom(3)]);
					spriteDef->setPos((getRandom(24) - 12 + actorSprite().width) / 2 + actorSprite().x,
						getRandom(6) + 10 + actorSprite().y);
					sprite = insertSprite(spriteDef);
					if (sprite)
						sprite->moveX.index = getRandom(12);
				}
			}
			if (_sprites[3].x < -400) {
				_sprites[3].setPos(actorSprite().x + 100, -14);
				_sprites[3].setCodeSync(0xAE69, 0xAE6E, 0xAE79, 1, 1);
				_sprites[6].setPos(actorSprite().x + 97, -12);
				_sprites[6].setCodeSync(0xAEB0, 0xAEB2, 0xAEED, 1, 1);
			}
			if (_cameraStripNum >= 10 && _cameraStripNum <= 15 && _sprites[15].status == 3 &&
				getSceneSpriteDef(3, 13)->status != 0) {
				_sprites[15].status = 1;
				_sprites[15].setPos(-144, 100);
				_sprites[15].setCodeSync(0xAF2D, kNone, 0xAF32, 1, 2);
			}
		}
		break;
	
	case kSceneCaveMonster:
		if (_sprites[7].status != 0 && _sprites[7].anim.codeId == 0 && _flgCanRunBoxClick) {
			// Monster
			_sprites[7].setPos(121 + _sprites[8].x, 25 + _sprites[8].y);
			_sprites[7].setCodeSync(0x1211, 0x124B, 0x1280, 1, 2);
			if (_sprites[10].status == 1) {
				// Wire
				getSceneSpriteDef(5, 5)->status = 0;
				getSceneSpriteDef(5, 10)->status = 1;
				_sprites[7].anim.codeId = 0x12B5;
				_sprites[12].anim.set(0x1A2C, 0, 53, 2);
				_sprites[11].setCodeSync(0x12EE, kNone, 0x12F7, 47, 2);
			}
		}
		break;

	case kSceneCaveRocks:
		// Shake the screen
		if (actorSprite().x + _cameraStripX >= 832 && actorSprite().x + _cameraStripX <= 896) {
			if (_screenShakeStatus == 0)
				_screenShakeStatus = 1;
		} else
			_screenShakeStatus = 0;
		if (getRandom(3) != 0) {
			// Insert falling rock
			spriteDef = getSpriteDef(0x1208);
			spriteDef->frameIndex = (2 + getRandom(4)) | (getRandom(2) << 7);
			spriteDef->x = getRandom(63) + 832 - _cameraStripX;
			insertSprite(spriteDef);
		}
		break;
		
	case kSceneCaveLake:
		if (_screenShakeStatus != 0 && getRandom(2) == 0) {
			spriteDef = getSpriteDef(0x12FE);
			spriteDef->setPos(getRandom(200) + 50, getRandom(10) - 20);
			insertSprite(spriteDef);
		}
		break;
		
	case kSceneBandit:
		if (_sprites[2].status != 0) {
			_queryBoxX2 = 228;
			_queryBoxY2 = 199;
		}
		break;
		
	case kSceneFountain:
		if (_sprites[3].status == 1) {
			_queryBoxX2 = 319;
			_queryBoxY1 = 190;
			_queryBoxY2 = 199;
		}
		break;
		
	case kSceneTown20:
		_queryBoxX2 = 60;
		if (_sprites[3].frameIndex != 23)
			_queryBoxY2 = _sprites[3].y + _sprites[3].height;
		else
			_queryBoxY2 = 199;
		break;
		
	case kSceneCostumeShop:
		if (_sprites[2].status) {
			_queryBoxX1 = _sprites[2].x;
			_queryBoxX2 = _sprites[2].x2();
			_queryBoxY1 = _sprites[2].y + _sprites[2].height - 5;
			_queryBoxY2 = _sprites[2].y + _sprites[2].height;
			if ((_flags1 & 4) && _sprites[2].anim.codeId != 0x1383) {
				if (_sprites[2].anim.codeId == 0) {
					getSceneSpriteDef(24, 0)->status = 0;
					_sprites[2].anim.set(0x1383, 0, 5, 2);
				} else if (_sprites[2].status == 1) {
					spriteDef = getSpriteDef(0x138A);
					for (uint i = 0; i < 15; i++) {
						spriteDef->setPos(actorSprite().x + 4 + getRandom(24),
							actorSprite().y + 1 + getRandom(47));
						spriteDef->frameIndex = 17 + getRandom(4);
						insertSprite(spriteDef);
					}
				}
			}
		}
		break;
		
	case kSceneChangingRoom:
		if (_sprites[2].frameIndex == 12) {
			// Snowflakes
			//
			spriteDef = getSpriteDef(0x13AE);
			spriteTemplate = getSpriteTemplate(spriteDef->templateId);
			spriteTemplate->animListTicks = 1;
			insertSprite(spriteDef);
			spriteTemplate->animListTicks = 5;
			insertSprite(spriteDef);
			spriteTemplate->animListTicks = 9;
			insertSprite(spriteDef);
			//
			spriteDef = getSpriteDef(0x13B7);
			spriteTemplate = getSpriteTemplate(spriteDef->templateId);
			spriteTemplate->animListTicks = 1;
			insertSprite(spriteDef);
			spriteTemplate->animListTicks = 4;
			insertSprite(spriteDef);
			//
			spriteDef = getSpriteDef(0x13C0);
			spriteTemplate = getSpriteTemplate(spriteDef->templateId);
			spriteTemplate->animListTicks = 1;
			insertSprite(spriteDef);
			spriteTemplate->animListTicks = 4;
			insertSprite(spriteDef);
		}
		if (_spriteResourceType == 19) {
			spriteDef = getSpriteDef(0x149D);
			if (actorSprite().frameIndex == 0) {
				_scene25Value = 0;
			} else if (actorSprite().frameIndex == 3) {
				_scene25Value = 1;
			}
			if (actorSprite().frameIndex == 2 && _scene25Value == 1) {
				for (uint i = 0; i < 6; i++) {
					spriteDef->setPos(_sprites[4].x + getRandom(_sprites[4].width),
						_sprites[4].y + getRandom(_sprites[4].height));
					spriteDef->frameIndex = 15 + getRandom(3);
					insertSprite(spriteDef);
				}
			}
			if ((actorSprite().frameIndex == 1 && _scene25Value == 0) ||
				(actorSprite().frameIndex == 2 && _scene25Value == 1)){
				for (uint i = 0; i < 10; i++) {
					spriteDef->setPos(actorSprite().x + getRandom(19),
						actorSprite().y + getRandom(43));
					spriteDef->frameIndex = 15 + getRandom(3);
					insertSprite(spriteDef);
				}
				for (uint i = 0; i < 6; i++) {
					spriteDef->setPos(_sprites[3].x + getRandom(_sprites[3].width),
						_sprites[3].y + getRandom(_sprites[3].height));
					spriteDef->frameIndex = 15 + getRandom(3);
					insertSprite(spriteDef);
				}
			}
		}
		break;
		
	case kSceneCliff:
		// Scene: The cliff edge
		if (_spriteResourceType == 20 && actorSprite().frameIndex == 90 && _sprites[2].anim.codeId != 0xB0ED)
			_sprites[2].setCodeSync(0xB0ED, 0xB0F6, 0xB0FD, 2, 2);
		// Check collision with the big rock
		if (_sprites[5].x - actorSprite().x >= 0 && _sprites[5].x - actorSprite().x <= 10 && _flgCanRunBoxClick) {
			_sprites[5].setCodeSync(0xB0E5, 0xB0E8, kNone, 1, 1);
			updateFunc411h();
			return;
		}
		if (!(_sprites[13].status & 2)) {
			if (_sprites[13].x + _sprites[13].width <= 0) {
				_sprites[13].status |= 2;
			} else {
				if (_sprites[13].y - 50 < 0)
					_sprites[13].y += 4;
				else if (_sprites[13].y - 50 > 0)
					_sprites[13].y -= 4;
				if (actorSprite().anim.codeId != 0xB02D && !_flgCanRunBoxClick) {
					actorSprite().setPos(_sprites[13].x + points30[_sprites[13].frameIndex - 22].x,
						_sprites[13].y + points30[_sprites[13].frameIndex - 22].y);
					if (actorSprite().x + _cameraStripX <= 175) {
						actorSprite().setPos(175 - _cameraStripX, 79);
						actorSprite().setCodeSync(0xB02D, kNone, 0xB032, 1, 2);
						return;
					}
				}
			}
		}
		if (_sprites[8].status & 2) {
			_queryBoxX1 = 714;
			_queryBoxX2 = 825;
			_queryBoxY2 = 199;
		}
		if (_sprites[14].status) {
			if (_sprites[14].anim.codeId != 0xB0A8) {
				if (_sprites[14].x - actorSprite().x >= 0 && _sprites[14].x - actorSprite().x <= 16) {
					_sprites[14].setCodeSync(0xB0A8, 0xB0D0, 0xB0D7, 1, 1);
				}
			} else if (_sprites[14].y - actorSprite().y >= 0 && _sprites[14].y - actorSprite().y <= actorSprite().height &&
				_sprites[14].x - actorSprite().x - actorSprite().width + 4 <= 0 && _flgCanRunBoxClick) {
				updateFunc411h();
				return;
			}
		}
		if (--_sceneCountdown1 >= 180) { // CHECKME Weird
			_sceneCountdown1 = 180;
			_screenShakeStatus = 0;
		}
		if (_sceneCountdown1 <= 50) {
			if (_cameraStripNum >= 21 && _cameraStripNum <= 40) {
				if (getRandom(3) == 0) {
					playSound(6, actorSprite());
				}
				if (_screenShakeStatus == 0)
					_screenShakeStatus = 1;
			}
			if (_sceneCountdown1 & 1) {
				for (uint i = 0; i < 3; i++) {
					spriteDef = getSpriteDef(spriteDef30[getRandom(2)]);
					spriteTemplate = getSpriteTemplate(spriteDef->templateId);
					spriteDef->setPos(getRandom(260) + 1000 - _cameraStripX, getRandom(30) - 15);
					spriteTemplate->moveXListId = moveX30[getRandom(2)];
					insertSprite(spriteDef);
				}
			}
		}
		if (_sprites[15].status && _sprites[15].anim.codeId != 0xB104 && _sprites[15].anim.codeId != 0xB11C &&
			ABS(_sprites[15].x - actorSprite().x - 4) <= 3) {
			_sprites[15].y = actorSprite().y - 89;
			_sprites[15].setCodeSync(0xB104, kNone, 0xB10F, 1, 1);
			if (_sprites[4].id == -1 && _sprites[4].status == 1) {
				addInventoryItem(94);
				_sprites[4].anim.set(0x1A2E, 0, 12, 1);
				_sprites[15].anim.codeId = 0xB11C;
				_sprites[15].moveY.codeId = 0xB120;
				actorSprite().setCodeSync(0xAFEC, 0xAFF2, kNone, 8, 2);
			}
		}
		if (_queryBoxX1 == 0 && _sprites[6].y == 57) {
			_queryBoxX1 = 1386;
			_queryBoxX2 = 1443;
			_queryBoxY2 = 199;
		}
		// Insert a bird
		if (getRandom(10) == 0) {
			uint16 spriteDefId = bird30SpriteDefs[getRandom(2)];
			spriteDef = getSpriteDef(spriteDefId);
			spriteTemplate = getSpriteTemplate(spriteDef->templateId);
			spriteDef->y = getRandom(150) + 25;
			if (spriteDefId == bird30SpriteDefs[0])
				spriteTemplate->moveXListId = bird30MoveX1[getRandom(4)];
			else
				spriteTemplate->moveXListId = bird30MoveX2[getRandom(4)];
			insertSprite(spriteDef);
		}
		if (_sprites[2].status && actorSprite().x2() - 10 - _sprites[2].x >= 0 &&
			_flgCanRunBoxClick) {
			updateFunc411h();
			return;
		}
		if (_sprites[22].status) {
			if (_sprites[22].anim.codeId != 0xB0A8) {
				if (_sprites[22].x - actorSprite().x >= 0 && _sprites[22].x - actorSprite().x <= 14) {
					_sprites[22].setCodeSync(0xB0A8, 0xB0D0, 0xB0D7, 1, 1);
					return;
				}
			} else if (_sprites[22].y - actorSprite().y >= 0 && _sprites[22].y - actorSprite().y <= actorSprite().height &&
				_sprites[22].x - actorSprite().x - actorSprite().width + 4 <= 0 && _flgCanRunBoxClick) {
				updateFunc411h();
				return;
			}
		}
		break;
		
	case kSceneCars:
		if (actorSprite().x + _cameraStripX <= 250) {
			if (_sprites[2].status == 1) {
				_queryBoxX1 = _cameraStripX + _sprites[2].x;
				_queryBoxX2 = _cameraStripX + _sprites[2].x + _sprites[2].width;
				_queryBoxY2 = _sprites[2].y + _sprites[2].height;
			}
		} else if (_sprites[4].status == 1) {
			_queryBoxX1 = _cameraStripX + _sprites[4].x;
			_queryBoxX2 = _cameraStripX + _sprites[4].x + _sprites[4].width;
			_queryBoxY1 = _sprites[4].y + _sprites[4].height - 15;
			_queryBoxY2 = _sprites[4].y + _sprites[4].height;
		}
		if (_sprites[3].status != 1) {
			if (_cameraStripNum <= 8 && hasInventoryItem(121) && hasInventoryItem(119) &&
				hasInventoryItem(120) && hasInventoryItem(115)) {
				_sprites[3].setCodeSync(0xB181, 0xB18D, 0xB197, 1, 2);
			}
		} else if (_sprites[3].y + _sprites[3].height - 5 - actorSprite().y - actorSprite().height <= 0) {
			_queryBoxX1 = _cameraStripX + _sprites[3].x;
			_queryBoxX2 = _cameraStripX + _sprites[3].x + _sprites[3].width / 2 - 4;
			_queryBoxY2 = _sprites[3].y + _sprites[3].height - 1;
		} else {
			_queryBoxX1 = _cameraStripX + _sprites[3].x; 
			_queryBoxX2 = _cameraStripX + _sprites[3].x + _sprites[3].width;
			_queryBoxY1 = _sprites[3].y + _sprites[3].height - 10;
			_queryBoxY2 = 199;
		}
		break;
		
	case kSceneShipWreck:
		if (_sprites[2].frameIndex != 19) {
			_queryBoxY2 = _sprites[2].y + _sprites[2].height;
			_queryBoxX1 = _cameraStripX + _sprites[2].x;
			_queryBoxX2 = _cameraStripX + _sprites[2].x + _sprites[2].width;
		}
		break;

	case kSceneRemotesDump:
		if (_sprites[2].anim.codeId == 0 && actorSprite().x >= 65 && actorSprite().x <= 200 &&
			actorSprite().y >= 110) {
			_sprites[2].setCodeSync(0xB19C, 0xB1A8, 0xB1B2, 1, 1);
		}
		break;
		
	case kSceneBand:
		if (_sprites[2].status == 0 && (hasInventoryItem(111) || hasInventoryItem(112)) &&
			(actorSprite().x + _cameraStripX < 320 || actorSprite().x + _cameraStripX >= 600)) {
			if (actorSprite().x + _cameraStripX < 320) {
				_sprites[2].x = 320 - _cameraStripX;
				getSceneSpriteDef(35, 0)->x = 320 - _cameraStripX + 129 + _cameraStripX;
				getSceneSpriteDef(35, 0)->frameIndex = 159;
				_sprites[2].setCodeSync(0xB227, 0xB245, kNone, 1, 1);
			} else if (actorSprite().x + _cameraStripX >= 600) {
				_sprites[2].x = 600 - _cameraStripX;
				getSceneSpriteDef(35, 0)->x = 600 - _cameraStripX - 129 + _cameraStripX;
				getSceneSpriteDef(35, 0)->frameIndex = 31;
				_sprites[2].setCodeSync(0xB1BC, 0xB1DA, kNone, 1, 1);
			}
			removeInventoryItem(idLetter);
			removeInventoryItem(idLetterStamp);
			_sprites[2].status = 3;
			_sprites[2].y = actorSprite().y + 30;
			getSceneSpriteDef(35, 0)->y = actorSprite().y + 30 + 6;
			getSceneSpriteDef(35, 0)->status = 1;
			_sprites[2].moveX.set(0xB201, 0, 1, 1);
		}
		{
			// Insert falling lava rocks
			int16 value = getRandom(3);
			spriteDef = getSpriteDef(spriteDef35[value]);
			spriteDef->frameIndex = value * 3 + getRandom(3) + 51;
			spriteDef->x = getRandom(280) + 40 - _cameraStripX;
			value = (value + 1) * 15;
			spriteDef->y = getRandom(value) - value;
			insertSprite(spriteDef);
		}
		break;
		
	case kSceneInsideWreck:
		if (_sprites[2].frameIndex == 2 && _sprites[3].frameIndex == 2) {
			_queryBoxX1 = 151;
			_queryBoxY1 = 159;
			_queryBoxX2 = 186;
			_queryBoxY2 = 165;
		} else if ((_sprites[2].frameIndex == 2 && _sprites[3].frameIndex != 2) ||
			(_sprites[2].frameIndex != 2 && _sprites[3].frameIndex == 2)) {
			_queryBoxX1 = 154;
			_queryBoxY1 = 135;
			_queryBoxX2 = 182;
			_queryBoxY2 = 159;
		}
		break;
		
	case kSceneCloud:
		deltaX = (_actorXTable[9] - 150) / 10;
		actorSprite().x -= deltaX;
		if (actorSprite().anim.codeId != 0x9808 && _spriteResourceType == 255) {
			if (actorSprite().frameIndex <= 8 && deltaX >= 4) {
				_walkDistance = 0;
				_currWalkInfo = NULL;
				actorSprite().changeCodeSync(0, 0, 0);
				actorSprite().frameIndex = 0;
			} else if (actorSprite().frameIndex >= 128 && deltaX <= -4) {
				_walkDistance = 0;
				_currWalkInfo = NULL;
				actorSprite().changeCodeSync(0, 0, 0);
				actorSprite().frameIndex = 128;
			}
		}
		deltaX = (_actorXTable[2] - 150) / 10;
		_sprites[3].x -= deltaX;
		_sprites[4].x -= deltaX;
		_sprites[5].x -= deltaX;
		_sprites[6].x -= deltaX;
		for (uint cloudIndex = 3; cloudIndex <= 6; cloudIndex++) {
			sprite = &_sprites[cloudIndex];
			if (sprite->x > 480 || sprite->x <= -160) {
				if (sprite->x > 480)
					sprite->x -= 600;
				else if (sprite->x <= -160)
					sprite->x += 600;
				sprite->frameIndex = getRandom(3) + 4;
				sprite->y = getRandom(50) - 5 + (cloudIndex - 3) * 50;
				sprite->moveX.set(cloud38MoveX[getRandom(2)], 0, 1, 1);
			}
		}
		break;
		
	case kSceneElectro:
		if (_sprites[3].y >= -70) {
			// Update the gate blocking rectangle
			_queryBoxX1 = (actorSprite().y - 127) / 2 + 92;
			_queryBoxX2 = (actorSprite().y - 127) / 2 + 92 + 50;
			_queryBoxY2 = 199;
		}
		if (_flgCanRunBoxClick &&
			((actorSprite().x < 160 && (actorSprite().y - 102) * 45 / 49 + 72 <= actorSprite().x) ||
			(actorSprite().x >= 160 && (actorSprite().y - 102) * 69 / 49 + 177 >= actorSprite().x))) {
			// Player gets shocked if he walks on the electric floor
			if (actorSprite().x < 160 && (actorSprite().y - 102) * 45 / 49 + 72 <= actorSprite().x)
				actorSprite().moveX.set(0x0E36, 0, 1, 1);
			else
				actorSprite().moveX.set(0x0E26, 0, 1, 1);
			_flgWalkBoxEnabled = false;
			_flgCanRunMenu = false;
			_flgCanRunBoxClick = false;
			_walkDistance = 0;
			_currWalkInfo = NULL;
			actorSprite().setCodeSync(0xB2E4, kNone, 0x0E46, 1, 1);
		}
		if (_sprites[9].status == 1 && actorSprite().x >= 160) {
			_queryBoxX1 = 259;
			_queryBoxX2 = 319;
			_queryBoxY1 = 163;
			_queryBoxY2 = 175;
		}
		break;
		
	case kSceneIceDesert40:
		if (_sprites[2].status == 1 && _flgCanRunBoxClick && actorSprite().y >= 72 &&
			ABS(actorSprite().x + _cameraStripX - 165) <= 5 && actorSprite().y <= 100) {
			_currWalkInfo = NULL;
			_flgCanRunBoxClick = false;
			_flgCanRunMenu = false;
			_walkDistance = 0;
			_flgWalkBoxEnabled = false;
			actorSprite().setCodeSync(0xB2FC, 0, 0xB30F, 1, 1);
		}
		break;
		
	case kSceneIceDesert41:
		if (!(_flags1 & 0x80) && _sprites[2].status && _sprites[2].frameIndex == 17 &&
			ABS(actorSprite().x - _sprites[2].x - 20) < 120)
			// Fishing eskimo collapses
			_sprites[2].setCodeSync(0xB32A, 0xB338, 0xB33C, 1, 2);
		if (_sprites[3].frameIndex == 39 && actorSprite().frameIndex == 28 &&
			actorSprite().anim.codeId == 0) {
			// Catching the fish
			actorSprite().anim.codeId = 0xB321;
			actorSprite().anim.index = 0;
			actorSprite().anim.ticks = 2;
			actorSprite().moveX.codeId = 0xB327;
			actorSprite().moveX.index = 0;
			actorSprite().moveX.ticks = 2;
			_sprites[3].setCodeSync(0xB344, 0xB352, 0xB35E, 2, 2);
			_sprites[4].anim.set(0x1A2C, 0, 2, 2);
		}
		if (_sprites[2].status == 1) {
			// Update the eskimo blocking rectangle
			if (_sprites[2].frameIndex == 17 && _sprites[2].anim.codeId != 0xB32A) {
				_queryBoxX1 = _cameraStripX + _sprites[2].x;
				_queryBoxX2 = _cameraStripX + _sprites[2].x + _sprites[2].width - 8;
				_queryBoxY1 = _sprites[2].y + _sprites[2].height - 8 - 10;
				_queryBoxY2 = _sprites[2].y + _sprites[2].height - 8;
			} else {
				_queryBoxX1 = _cameraStripX + _sprites[2].x;
				_queryBoxX2 = _cameraStripX + _sprites[2].x + _sprites[2].width - 8;
				_queryBoxY1 = _sprites[2].y + _sprites[2].height - 2 - 16;
				_queryBoxY2 = _sprites[2].y + _sprites[2].height - 2;
			}
		}
		break;
		
	case kSceneIceDesert42:
		if (actorSprite().x + _cameraStripX <= 900) {
			if (_sprites[5].status == 1) {
				if (_sprites[5].frameIndex == 36) {
					_queryBoxX1 = _cameraStripX + _sprites[5].x;
					_queryBoxX2 = _cameraStripX + _sprites[5].x + _sprites[5].width;
					_queryBoxY1 = _sprites[5].y + _sprites[5].height - 6;
					_queryBoxY2 = _sprites[5].y + _sprites[5].height;
				} else {
					_queryBoxX1 = _cameraStripX + _sprites[5].x;
					_queryBoxX2 = _cameraStripX + _sprites[5].x + _sprites[5].width - 4;
					_queryBoxY1 = _sprites[5].y + 6;
					_queryBoxY2 = _sprites[5].y + _sprites[5].height;
				}
			}
		} else if (_sprites[2].status == 1) {
			_queryBoxX1 = _cameraStripX + _sprites[2].x;
			_queryBoxX2 = _cameraStripX + _sprites[2].x + _sprites[2].width + _sprites[3].width;
			_queryBoxY1 = 142;
			_queryBoxY2 = 165;
		}
		break;

	case kSceneIceDesert43:
		if (!(_flags2 & 1) && _sprites[2].x >= 0 && _sprites[2].x2() <= 320) {
			_flags2 |= 1;
			_sprites[2].setCodeSync(0xB36A, 0xB377, 0xB381, 10, 2);
			_sprites[4].setCodeSync(0xB38B, 0xB390, 0xB393, 12, 2);
		}
		if (_sprites[2].status == 1) {
			_queryBoxX1 = _cameraStripX + _sprites[2].x;
			if (_sprites[2].frameIndex != 32) {
				_queryBoxX2 = _cameraStripX + _sprites[2].x + _sprites[2].width - 8;
				_queryBoxY1 = _sprites[2].y + _sprites[2].height - 12;
				_queryBoxY2 = _sprites[2].y + _sprites[2].height;
			} else {
				_queryBoxX2 = _cameraStripX + _sprites[2].x + _sprites[2].width - 12;
				_queryBoxY1 = _sprites[2].y + 16;
				_queryBoxY2 = _sprites[2].y + _sprites[2].height;
			}
		}
		break;

	case kSceneBoat:
		if (_sprites[6].frameIndex == 6)
			_scrollBorderRight = 0;
		break;

	case kSceneIceCastle:
		// Penguins
		if (_sprites[12].x >= 600) {
			_sprites[9].setPos(-20, 152);
			_sprites[9].anim.index = 0;
			_sprites[9].moveX.index = 0;
			_sprites[9].moveY.index = 0;
			_sprites[10].setPos(-30, 153);
			_sprites[10].anim.index = 0;
			_sprites[10].moveX.index = 0;
			_sprites[10].moveY.index = 0;
			_sprites[11].setPos(-40, 155);
			_sprites[11].anim.index = 0;
			_sprites[11].moveX.index = 0;
			_sprites[11].moveY.index = 0;
			_sprites[12].setPos(-50, 157);
			_sprites[12].anim.index = 0;
			_sprites[12].moveX.index = 0;
			_sprites[12].moveY.index = 0;
		}
		break;

	case kSceneIceCastleHall:
		if (_sprites[8].y < 30) {
			_sprites[8].y = 30;
			_sprites[8].moveY.codeId = 0;
		} else if (_sprites[8].y > 89) {
			_sprites[8].y = 89;
			_sprites[8].moveY.codeId = 0;
		}
		break;
		
	case kSceneIceCastleBeam:
		if (_sprites[11].status) {
			if (_sprites[16].status == 1) {
				// The beam destroys the door mechanism
				spriteDef = getSpriteDef(0xB396);
				spriteTemplate = getSpriteTemplate(0xB39F);
				_sprites[11].status = 0;
				_sprites[12].anim.set(0x1A2E, 0, 18, 1);
				_sprites[13].anim.set(0x1A2E, 0, 18, 1);
				_sprites[14].anim.set(0x1A2E, 0, 18, 1);
				_sprites[15].anim.set(0x1A2E, 0, 18, 1);
				_sprites[16].anim.set(0x1A2E, 0, 18, 1);
				// Make these items non-pickupable
				getSceneItemInfo(idMagnifier).flags &= ~2;
				getSceneItemInfo(idIceCube).flags &= ~2;
				getSceneItemInfo(idIcicle).flags &= ~2;
				getSceneItemInfo(idGlass).flags &= ~2;
				for (uint i = 0; i < 6; i++) {
					spriteDef->x = beamPoints56[i].x;
					spriteDef->y = beamPoints56[i].y;
					spriteTemplate->animListTicks = (4 + i) * 3;
					spriteTemplate->moveXListTicks = (4 + i) * 3;
					spriteTemplate->moveYListTicks = (4 + i) * 3;
					insertSprite(spriteDef); 
				}
			} else {
				_sprites[13].status = 3;
				_sprites[14].status = 3;
				_sprites[15].status = 3;
				_sprites[11].x = 257;
				_sprites[11].y = 163;
				if (_sprites[6].status == 1 && _sprites[6].frameIndex == 30) {
					_sprites[13].status = 1;
					_sprites[11].x -= 183;
					_sprites[11].y -= 10;
					if (_sprites[7].status == 1 && _sprites[7].frameIndex == 27) {
						_sprites[14].status = 1;
						_sprites[11].x -= 40;
						_sprites[11].y += 35;
						if (_sprites[8].status == 1 && _sprites[8].frameIndex == 34) {
							_sprites[15].status = 1;
							_sprites[11].x += 239;
							if (_sprites[9].status == 1 && _sprites[9].frameIndex == 14) {
								_sprites[16].status = 1;
							}
						}
					}
				}
			}
		}
		break;
		
	case kSceneIceCastleWitch:
		_collectItemIgnoreActorPosition = true;
		if (actorSprite().y >= 125) {
			if (actorSprite().x >= 197) {
				if (_flgCanRunBoxClick) {
					_flgCanRunBoxClick = false;
					_flgCanRunMenu = false;
					_flgWalkBoxEnabled = false;
					queueWalk(212, 144);
					actorSprite().anim.codeId = 0;
					actorSprite().moveX.codeId = 0;
					actorSprite().moveY.codeId = 0;
					queueActor(0xB479, 0xB487, 0xB490);
					queueAnim(6, 17, 2, 0xB499, 0xB4AB, 0xB4BC);
					queueAnim(2, 1, 2, 0xB3EA, 0, 0);
					_currWalkInfo = &_walkInfos[0];
					startActorWalk(_currWalkInfo);
				}
			} else {
				if (_spriteResourceType == 24 && actorSprite().frameIndex == 47) {
					// The player is in an ice block, skip back to the left
					if (actorSprite().x <= 40)
						actorSprite().setCodeSync(0xB3E0, kNone, 0xB3E5, 1, 2);
					else
						actorSprite().x -= MIN(10, (actorSprite().x - 37) / 4);
				} else if (actorSprite().x > 40 && _sprites[5].anim.codeId == 0) {
					// The witch casts a bolt
					_sprites[2].anim.set(0xB3E7, 0, 7, 2);
					_sprites[3].setCodeSync(0xB3EC, 0xB3FB, 0xB40A, 1, 2);
					_sprites[5].setCodeSync(0xB429, 0xB445, 0xB460, 9, 1);
					_sprites[4].setCodeSync(0xB419, 0xB41F, 0xB424, 11, 2);
				}
				if (actorSprite().x > 40 && _sprites[5].status == 1 &&
					(_spriteResourceType != 9 || actorSprite().frameIndex < 130 || actorSprite().frameIndex > 131) &&
					actorSprite().x2() - _sprites[5].x > 0 &&
					actorSprite().x2() - _sprites[5].x - actorSprite().width - _sprites[5].width < 0 &&
					actorSprite().frameIndex != 47) {
					// The bolt hit the player, freeze into an ice block
					_flgCanRunBoxClick = false;
					_flgCanRunMenu = false;
					_flgWalkBoxEnabled = false;
					_walkDistance = 0;
					_currWalkInfo = NULL;
					actorSprite().y = 130;
					actorSprite().setCodeSync(0xB3DA, 0, 0, 1, 2);
				}
			}
		}
		break;
		
	case kSceneIceCastle58:
		if (_sprites[2].status == 3 && actorSprite().y <= 130 && _flgCanRunBoxClick) {
			getSceneSpriteDef(58, 0)->status = 0;
			_flgCanRunBoxClick = false;
			_flgCanRunMenu = false;
			_flgWalkBoxEnabled = false;
			_walkDistance = 0;
			_currWalkInfo = NULL;
			_sprites[2].setPos(actorSprite().x + 128, actorSprite().y - 106);
			actorSprite().setCodeSync(0xB4EE, 0xB4FA, 0xB503, 1, 1);
			_sprites[2].changeCodeSync(0xB4CD, 0xB4D4, 0xB4E1);
		}
		if (actorSprite().status & 2)
			_sprites[2].heightAdd = 200;
		break;
		
	case kSceneTrollBaby:
		if (_sprites[2].status == 1) {
			_queryBoxX1 = 190;
			_queryBoxX2 = 261;
			_queryBoxX2 = 158;
			if (actorSprite().x <= 80 && ABS(actorSprite().y - 122) < 5) {
				if (++_scene63BabySleepCounter >= 0) {
					// Baby falls asleep
					if (_scene63BabySleepCounter > 100)
						_scene63BabySleepCounter = 100;
					if (_sprites[2].anim.codeId != 0xB526) {
						_sprites[2].anim.set(0xB526, 0, 1, 4);
						_sprites[3].anim.set(0xB533, 0, 1, 2);
					}
				}
			} else if (--_scene63BabySleepCounter <= 0) {
				// Baby wakes up
				_scene63BabySleepCounter = -20;
				if (_sprites[2].anim.codeId == 0xB526) {
					_sprites[2].anim.set(0xB531, 0, 1, 4);
					_sprites[3].anim.set(0xB535, 0, 1, 2);
				}
				if (_sprites[3].anim.codeId == 0) {
					if (actorSprite().x < 230 && _sprites[3].frameIndex == 10)
						_sprites[3].anim.set(0xB520, 0, 1, 2);
					else if (actorSprite().x >= 230 && _sprites[3].frameIndex == 8)
						_sprites[3].anim.set(0xB523, 0, 1, 2);
				}
			}
		}
		if (_sprites[9].status == 1 && _sprites[9].frameIndex != 30 &&
			_sprites[9].frameIndex != 36 && _sprites[9].anim.index >= 8) {
			// Insert sparkles
			spriteDef = getSpriteDef(0xB539);
			for (uint i = 0; i < 10; i++) {
				spriteDef->setPos(getRandom(24) + 4 + actorSprite().x,
					getRandom(47) + 1 + actorSprite().y);
				spriteDef->frameIndex = getRandom(4) + 37;
				insertSprite(spriteDef);
			}
		}
		break;
		
	case kSceneEvilQueen:
		if (!(_flags2 & 0xC0) && _flgCanRunMenu) {
			_scrollBorderLeft = 0;
			_scrollBorderRight = 96;
		}
		switch ((_flags2 >> 2) & 3) {
		case 0:
			// The evil queen sends off a ghost
			if (actorSprite().anim.codeId == 0 && _sprites[2].anim.codeId == 0) {
				if (_sprites[8].anim.codeId == 0) {
					_sprites[2].setCodeSync(0x1667, 0x1671, 0x1679, 1, 2);
					_sprites[4].anim.set(0x1681, 0, 5, 2);
					_sprites[5].anim.set(0x1681, 0, 5, 2);
					_sprites[8].setCodeSync(0x16A2, 0x16AD, 0x16C0, 21, 2);
				} else if (_spriteResourceType == 27 && _sprites[8].x <= 120) {
					_flags2 ^= 0x40;
					actorSprite().anim.codeId = 0x16D3;
					actorSprite().anim.index = 0;
					actorSprite().anim.ticks = 13;
					actorSprite().moveX.codeId = 0x16D8;
					actorSprite().moveX.index = 0;
					actorSprite().moveX.ticks = 13;
					_sprites[8].setCodeSync(0x16DA, 0x16E2, 0x16E8, 1, 2);
				} else if (_spriteResourceType != 27 && _sprites[8].x == 85) {
					resetActor();
					actorSprite().changeCodeSync(0x162D, 0x1647, 0x1657);
				}
			}
			break;
		case 1:
			// The evil queen attacks with fire
			if (actorSprite().anim.codeId == 0 && _sprites[2].anim.codeId == 0) {
				_sprites[2].setCodeSync(0x16EE, 0x16F8, 0x1700, 1, 2);
				_sprites[4].anim.set(0x1708, 0, 5, 2);
				_sprites[5].anim.set(0x1708, 0, 5, 2);
			}
			if (_sprites[4].status == 1) {
				spriteDef = getSpriteDef(0x1765);
				uint pointIndex = getRandom(11);
				spriteDef->setPos(scene72FirePoints[pointIndex].x, scene72FirePoints[pointIndex].y);
				insertSprite(spriteDef);
			}
			if (_sprites[2].anim.ticks == 3) {
				resetActor();
				actorSprite().changeCodeSync(0x162D, 0x1647, 0x1657);
			}
			break;
		case 2:
			// Fireballs
			if (actorSprite().anim.codeId == 0) {
				if (_sprites[2].anim.codeId == 0 && _sprites[6].anim.codeId == 0) {
					_sprites[2].setCodeSync(0x17D4, 0x17E1, 0x17EC, 1, 2);
					_sprites[6].setCodeSync(0x17F7, 0x1801, 0x1808, 11, 2);
					_sprites[7].setCodeSync(0x17F7, 0x1801, 0x1808, 11, 2);
				}
				if (_sprites[7].anim.index == 4) {
					if (_sprites[9].status == 1) {
						_flags2 ^= 0x40;
						_sprites[2].setCodeSync(0x1829, 0x183F, 0x1853, 2, 2);
						_sprites[6].setCodeSync(0x180F, 0x1819, 0x1821, 2, 2);
						_sprites[7].setCodeSync(0x180F, 0x1819, 0x1821, 2, 2);
					} else {
						resetActor();
						actorSprite().changeCodeSync(0x162D, 0x1647, 0x1657);
					}
				}
			}
			break;
		default:
			if (_sprites[3].status != 0 && _sprites[2].anim.codeId == 0) {
				_scrollBorderRight = 999;
				resetActor();
				actorSprite().changeCodeSync(0x1867, 0x1897, 0x18C3);
				_sprites[9].anim.set(0x1A2E, 0, 1, 2);
				_sprites[3].anim.set(0x1A2E, 0, 1, 2);
			}
			break;
		}
		break;
		
	case kSceneGraveyard65:
	case kSceneGraveyard77:
		// Insert a bat
		if (getRandom(20) == 0) {
			spriteDef = getSpriteDef(0x159D);
			spriteTemplate = getSpriteTemplate(0x15A6);
			spriteDef->y = getRandom(159) - 5;
			if (getRandom(2) == 0) {
				spriteDef->x = -8;
				spriteDef->frameIndex = 101;
				spriteTemplate->animListId = 0x15BD;
				spriteTemplate->moveXListId = bats65MoveX1[getRandom(4)];
			} else {
				spriteDef->x = 328;
				spriteDef->frameIndex = 229;
				spriteTemplate->animListId = 0x15CA;
				spriteTemplate->moveXListId = bats65MoveX2[getRandom(4)];
			}
			insertSprite(spriteDef);
		}
		spriteIndex = 0;
		if (_sceneIndex == kSceneGraveyard65) {
			if (_sprites[8].status) {
				spriteIndex = 8;
				x = 374;
				y = 105;
			} else if (_sprites[9].status) {
				spriteIndex = 9;
				x = 500;
				y = 71;
			}
		} else {
			if (_sprites[7].status) {
				spriteIndex = 7;
				x = 26;
				y = 140;
			} else if (_sprites[8].status) {
				spriteIndex = 8;
				x = 406;
				y = 97;
			}
		}
		if (spriteIndex > 0) {
			spriteDef = getSpriteDef(0x15D7);
			spriteTemplate = getSpriteTemplate(0x15E0);
			spriteDef->x = x - _cameraStripX;
			if (ABS(spriteDef->x - actorSprite().x) <= 4) {
				_sprites[spriteIndex].status = 0;
				spriteDef->y = y;
				for (uint i = 0; i < 6; i++) {
					spriteTemplate->animListTicks = getRandom(30);
					spriteTemplate->moveXListTicks = spriteTemplate->animListTicks;
					spriteTemplate->moveYListTicks = spriteTemplate->animListTicks;
					insertSprite(spriteDef);
				}
			}
		}

		switch ((_flags2 >> 3) & 7) {

		case 0:
			_queryBoxX1 = 270;
			_queryBoxX2 = 400;
			_queryBoxY2 = 199;
			if (_sprites[2].status == 1 && _sprites[2].anim.codeId != 0x14C4) {
				if (_sprites[2].anim.codeId == 0x14E9) {
					if (_sprites[2].x - actorSprite().x - actorSprite().width <= 0 && _flgCanRunBoxClick) {
						_sprites[2].setCodeSync2(0x14F7, 0x14FC, 0x1501, 1);
						resetActor();
						actorSprite().changeCodeSync(0x14B8, 0x14C1, 0);
					}
				} else if (_sprites[2].anim.codeId == 0) {
					if (actorSprite().x + _cameraStripX <= 300 &&
						ABS(_sprites[2].y + _sprites[2].height - 169) <= 4) {
						// The vampire falls into the grave
						_flags2 |= 8;
						getSceneSpriteDef(65, 0)->templateId = 0x54E1;
						_sprites[2].setCodeSync2(0x14C4, 0x14D5, 0x14DF, 1);
						_sprites[3].anim.set(0x1A2F, 0, 7, 2);
					} else {
						_sprites[2].setCodeSync2(0x14E9, 0x14EF, 0x14F3, 1);
					}
				} else if (_sprites[2].frameIndex >= 39 && _sprites[2].frameIndex <= 41 &&
					_sprites[2].anim.ticks != 1) {
					if (_sprites[2].y + _sprites[2].height - actorSprite().y - actorSprite().height < 0)
						_sprites[2].y += 2;
					else
						_sprites[2].y -= 2;
				}
			}
			break;
			
		case 1:
			if (_sprites[2].status == 1) {
				_queryBoxX1 = _cameraStripX + _sprites[2].x;
				_queryBoxX2 = 320;
				_queryBoxY2 = 199; 
			} else if (actorSprite().x + _cameraStripX >= 270) {
				_sprites[2].x = 322 - _cameraStripX;
				_sprites[2].y = 127;
				_sprites[2].frameIndex = 32;
				_sprites[2].heightAdd = 200;
				_sprites[2].setCodeSync2(0x1506, 0x150C, 0x150F, 1);
				_queryBoxX1 = _cameraStripX + _sprites[2].x;
				_queryBoxX2 = 320;
				_queryBoxY2 = 199; 
			}
			break;
			
		case 2:
			if (_sceneIndex == kSceneGraveyard65 && _sprites[2].status != 1) {
				_sprites[2].x = 447 - _cameraStripX;
				_sprites[2].y = 101;
				_sprites[2].frameIndex = 31;
				_sprites[2].heightAdd = 0;
				_sprites[2].setCodeSync2(0x1515, 0x151A, 0x151F, 1);
			}
			break;

		case 3:
			if (_sceneIndex == kSceneGraveyard77 && _sprites[2].status == 1) {
				_queryBoxX1 = _cameraStripX + _sprites[2].x;
				_queryBoxX2 = 320;
				_queryBoxY2 = 199; 
			}
			break;
			
		case 4:
			if (_sceneIndex == kSceneGraveyard77) {
				if (_sprites[2].status == 1) {
					_queryBoxX1 = _cameraStripX + _sprites[2].x;
					_queryBoxX2 = 320;
					_queryBoxY2 = 199; 
				} else if (actorSprite().x + _cameraStripX >= 120) {
					_sprites[2].x = 203 - _cameraStripX;
					_sprites[2].y = 109;
					_sprites[2].frameIndex = 31;
					_sprites[2].setCodeSync2(0x1515, 0x151A, 0x151F, 1);
					_queryBoxX1 = _cameraStripX + _sprites[2].x;
					_queryBoxX2 = 320;
					_queryBoxY2 = 199; 
				}
			}
			break;
			
		case 5:
			if (_sceneIndex == kSceneGraveyard77) {
				if (_sprites[2].status == 1) {
					_queryBoxX1 = 0;
					_queryBoxX2 = _sprites[2].x + _sprites[2].width + _cameraStripX;
					_queryBoxY2 = 199; 
				} else if (actorSprite().x + _cameraStripX >= 350) {
					_sprites[2].x = 273 - _cameraStripX;
					_sprites[2].y = 93;
					_sprites[2].frameIndex = 31;
					_sprites[2].setCodeSync2(0x1524, 0x152C, 0x1532, 1);
					_queryBoxX1 = 0;
					_queryBoxX2 = _sprites[2].x + _sprites[2].width + _cameraStripX;
					_queryBoxY2 = 199; 
				}
			}
			break;
			
		case 6:
			if (_sceneIndex == kSceneGraveyard77 && _sprites[2].status != 1 &&
				_cameraStripNum >= 8 && hasInventoryItem(idVacuumCleaner)) {
					_sprites[2].x = 458 - _cameraStripX;
					_sprites[2].y = 92;
					_sprites[2].frameIndex = 31;
					_sprites[2].setCodeSync2(0x154D, 0x156B, 0x1581, 1);
					_sprites[6].setCodeSync(0x1597, kNone, 0x159B, 5, 2);
			}
			break;

		}

		break;
		
	default:
		break;
	}
}

void EnchantiaEngine::updateSprites() {

	// TODO Move this somewhere else (unless the sprite handlers are split up)
	static const int8 sprite401XTab00[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 1, 1, 1, 0, 0, -1, -1, 7, 7, 7,
		7, 7, 7, 7, 7, 7};
	static const int8 sprite401YTab00[] = {-5, -5, -5, -5, -5, -5, -5, -5, -5, -5,
		-5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, 7, 16,
		7, 0, 0, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5};
	static const int8 sprite401XTab02[] = {0, 0, 0, 0, 8, 10, 10, 8};
	static const int8 sprite401YTab02[] = {0, 0, 0, 0, 14, 19, 19, 6};
	static const int8 sprite401XTab03[] = {2, 8, 8};
	static const int8 sprite401YTab03[] = {0, 10, 10};
	static const int8 sprite401XTab07[] = {15, 15, 15, 7, 7};
	static const int8 sprite401YTab07[] = {-4, -4, -4, -5, -3};
	static const int8 sprite401XTab08[] = {9, 2, 13, 12, 3, 2, 4, -1, 8, 17, -1};
	static const int8 sprite401YTab08[] = {-3, -5, -5, -5, -5, -5, -5, -5, -6, -6, -6};
	static const int8 sprite401XTab09[] = {2, 20, 20, 11, 4, -1, -2, 2};
	static const int8 sprite401YTab09[] = {4, 0, 0, -1, -2, -2, -2, -2};
	static const int8 sprite401XTab10[] = {3, -2, 2, 0, 14, -2, -1};
	static const int8 sprite401YTab10[] = {-5, -5, -4, -5, -2, -4, -4};
	static const int8 sprite401XTab11[] = {8, 8, -1, -1, 14, 29, 33};
	static const int8 sprite401YTab11[] = {-5, -5, -5, -5, -10, -8, -8};
	
	static const struct { const int8 *xtab, *ytab; } sprite401XYTabs[] = {
		{sprite401XTab00, sprite401YTab00},
		{NULL, NULL},
		{sprite401XTab02, sprite401YTab02},
		{sprite401XTab03, sprite401YTab03},
		{NULL, NULL},
		{NULL, NULL},
		{NULL, NULL},
		{sprite401XTab07, sprite401YTab07},
		{sprite401XTab08, sprite401YTab08},
		{sprite401XTab09, sprite401YTab09},
		{sprite401XTab10, sprite401YTab10},
		{sprite401XTab11, sprite401YTab11}
	};

	_queryBoxDefValue = 0;
	_queryBoxX1 = 0;
	_queryBoxY1 = 0;
	_queryBoxX2 = 0;
	_queryBoxY2 = 0;

	for (uint i = 2; i < kSpriteCount; i++) {
		Sprite *sprite = &_sprites[i];
		if (sprite->status == 1 && sprite->id >= 0x400) {
			switch (sprite->id) {

			case 0x400:
				// No clue what this is
				if (ABS(_sprites[i - 1].x + _sprites[i - 1].width / 2 - actorSprite().x) <= 50)
					sprite->x = _sprites[i - 1].x + 21;
				else if (_sprites[i - 1].x + _sprites[i - 1].width / 2 < actorSprite().x)
					sprite->x = _sprites[i - 1].x + 21 + 1;
				else
					sprite->x = _sprites[i - 1].x + 21 - 1;
				break;
	
			case 0x401:
				// Update the fishbowl helmet sprite
				if (sprite->anim.codeId == 0 && sprite->moveX.codeId == 0 && sprite->moveY.codeId == 0) {
					byte frameIndex = actorSprite().frameIndex;
					byte tableIndex = _spriteResourceType == 255 ? 0 : _spriteResourceType + 1;
					const int8 *xtab = sprite401XYTabs[tableIndex].xtab;
					const int8 *ytab = sprite401XYTabs[tableIndex].ytab;
					if (xtab) {
						if (_spriteResourceType == 255 && (frameIndex & 0x80))
							frameIndex -= 94;
						else if (_spriteResourceType == 2)
							frameIndex &= 0x7F;
						sprite->setPos(actorSprite().x + xtab[frameIndex], actorSprite().y + ytab[frameIndex]);
					}
				}
				break;
	
			case 0x406:
				// Falling rock
				if (sprite->y >= 200) {
					sprite->status = 0;
				} else if (_sceneIndex == kSceneCaveLake && sprite->y >= 145 && getRandom(4) == 0) {
					// Splash
					sprite->id = -1;
					sprite->anim.set(0x131B, 0, 1, 2);
					sprite->moveX.set(0x1322, 0, 1, 2);
					sprite->moveY.codeId = 0;
				}
				break;
			
			case 0x407:
				// Hammer
				if (_flgCanRunBoxClick) {
					if (sprite->anim.codeId == 0) {
						if (actorSprite().x - sprite->x > -24 && actorSprite().x - sprite->x < 24) {
							// Start hammer animation if the player is close
							SpriteDef *spriteDef = getSpriteDef(0x0FED);
							sprite->anim.codeId = 0x1008;
							sprite->anim.index = 0;
							sprite->anim.ticks = 6;
							sprite->moveX.codeId = 0x101F;
							sprite->moveX.index = 0;
							sprite->moveX.ticks = 6;
							sprite->moveY.codeId = 0x1032;
							sprite->moveY.index = 0;
							sprite->moveY.ticks = 6;
							spriteDef->x = sprite->x - 28;
							insertSprite(spriteDef);
						}
					} else if (sprite->anim.index == 5 && actorSprite().x - sprite->x > -16 &&
						actorSprite().x - sprite->x < 16) {
						// Splat if the player was too close
						sprite->id = -1;
						resetActor();
						actorSprite().y = 138;
						actorSprite().changeCodeSync(0x0F94, 0x0FAA, 0x0FB9);
					}
				}
				break;

			case 0x408:
				_queryBoxX1 = _cameraStripX + sprite->x;
				_queryBoxX2 = _cameraStripX + sprite->x + sprite->width;
				_queryBoxY2 = 199;
				if (sprite->x + sprite->width + 64 <= 0)
					sprite->status = 0;
				break;

			case 0x409:
				debug("TODO sprite id %03X", sprite->id);
				break;

			case 0x40A:
				// Check collision with electric eels 
				if (actorSprite().anim.codeId != 0x11F0 && actorSprite().anim.codeId != 0x104A) {
					if (sprite->x - actorSprite().x < 0 && sprite->x - actorSprite().x + sprite->width - 10 > 0) {
						resetActor();
						actorSprite().anim.codeId = 0x11F0;
						actorSprite().anim.initialTicks = 1;
						actorSprite().moveX.codeId = 0x0E26;
						actorSprite().moveX.initialTicks = 1;
						actorSprite().moveY.codeId = 0x0E46;
						actorSprite().moveY.initialTicks = 1;
					} else if (sprite->x - actorSprite().x >= 0 && sprite->x - actorSprite().x - actorSprite().width < 0) {
						resetActor();
						actorSprite().anim.codeId = 0x11F0;
						actorSprite().anim.initialTicks = 1;
						actorSprite().moveX.codeId = 0x0E36;
						actorSprite().moveX.initialTicks = 1;
						actorSprite().moveY.codeId = 0x0E46;
						actorSprite().moveY.initialTicks = 1;
					}
				} 
				break;

			case 0x40B:
				// Riding the turtle
				if (sprite->anim.codeId != 0x1111) {
					if (sprite->y - actorSprite().y >= 26) {
						sprite->y = actorSprite().y + 26;
						sprite->anim.set(0x1111, 0, 19, 2);
						sprite->moveX.set(0x1143, 0, 19, 1);
						sprite->moveY.set(0x1186, 0, 19, 1);
						actorSprite().setCodeSync(0x104A, 0x106A, 0x10B1, 1, 1);
					} else if (sprite->moveY.codeId != 0x110F && sprite->anim.index == 13) {
						sprite->setCodeSync(0x110D, 0, 0x110F, 1, 1);
					}
				}
				break;

			case 0x40C:
				// Update shark 
				if (sprite->x >= 320) {
					sprite->status = 3;
				} else {
					sprite->x += 8;
					if (sprite->x + 96 - 8 >= actorSprite().x) {
						sprite->y--;
						sprite->x += 8;
					} else {
						if (sprite->y - actorSprite().y < -20)
							sprite->y++;
						else if (sprite->y - actorSprite().y > -20)
							sprite->y--;
						if (sprite->anim.codeId != 0x11C6 && actorSprite().x - sprite->x <= 144) {
							if (_spriteResourceType == 8 && actorSprite().x - sprite->x >= 124 &&
								actorSprite().frameIndex >= 1 && actorSprite().frameIndex <= 2) {
								sprite->anim.set(0x11C6, 0, 1, 1);
								sprite->moveY.set(0x11DC, 0, 1, 1);
								getSceneSpriteDef(3, 13)->status = 0;
							} else if ((_spriteResourceType == 8 || _flgCanRunBoxClick) &&
								actorSprite().anim.codeId != 0x10F8) {
								resetActor();
								actorSprite().anim.codeId = 0x10F8;
								actorSprite().anim.initialTicks = 1;
								actorSprite().moveX.codeId = 0x1104;
								actorSprite().moveX.initialTicks = 1;
								actorSprite().moveY.codeId = 0;
								sprite->moveX.set(0x1104, 0, 1, 1);
							}
						}
					}
				}
				break;

			case 0x40D:
				// Draw guards sparkles effect
				if (sprite->anim.codeId != 0) {
					SpriteDef *spriteDef = getSpriteDef(0x132E);
					for (uint j = 0; j < 5; j++) {
						spriteDef->frameIndex = getRandom(4) + 26;
						spriteDef->setPos(sprite->x + getRandom(sprite->width),
							sprite->y + getRandom(sprite->height));
						insertSprite(spriteDef);
					}
				}
				break;

			case 0x40F:
				// Bug guard
				if (_sprites[2].status & 2) {
					// Block the player if he doesn't wear the pig mask
					_queryBoxX2 = sprite->x + sprite->width;
					_queryBoxY2 = 199;
				} else if (sprite->x + sprite->width + 16 >= actorSprite().x &&
					sprite->anim.codeId == 0) {
					// Let the player pass if he wears the pig mask
					sprite->anim.set(0x1324, 0, 1, 2);
					sprite->moveX.set(0x132A, 0, 1, 2);
					queueWalk(15, actorSprite().y + actorSprite().height - 1);
					_currWalkInfo = &_walkInfos[0];
					startActorWalk(_currWalkInfo);
				}
				break;

			case 0x410:
				// Update pig mask
				if (actorSprite().frameIndex < 9) {
					sprite->frameIndex = 0;
					sprite->setPos(actorSprite().x + 8 + 3, actorSprite().y - 7 - 2);
				} else if (actorSprite().frameIndex < 18) {
					sprite->frameIndex = 3;
					sprite->setPos(actorSprite().x + 8, actorSprite().y - 7);
				} else if (actorSprite().frameIndex < 128) {
					sprite->frameIndex = 1;
					sprite->setPos(actorSprite().x + 8, actorSprite().y - 7);
				} else {
					sprite->frameIndex = 2;
					sprite->setPos(actorSprite().x + 8 - 5, actorSprite().y - 7 - 2);
				}
				break;

			case 0x411:
				//debug("TODO sprite id %03X", sprite->id);
				break;

			case 0x412:
				// Check collision with falling lava rock
				if (_flgCanRunBoxClick &&
					sprite->frameIndex == 60 && ABS(sprite->x - actorSprite().x) <= 20 &&
					ABS(sprite->y + sprite->height - actorSprite().y - actorSprite().height) <= 10) {
					SpriteDef *spriteDef = getSpriteDef(0x135E);
					resetActor();
					spriteDef->setPos(actorSprite().x + 4, actorSprite().y + 5);
					insertSprite(spriteDef);
					spriteDef->setPos(actorSprite().x + 12, actorSprite().y + 10);
					insertSprite(spriteDef);
					spriteDef->setPos(actorSprite().x + 8, actorSprite().y + 15);
					insertSprite(spriteDef);
					actorSprite().changeCodeSync(0x1349, 0x1354, 0x1359);
				}
				break;
			
			default:
				// Nothing
				break;
			}
		}
	}
}

void EnchantiaEngine::handleBoxAction(int16 action) {

	switch (action) {
	case 0:
		queueActor(0x93DC, 0x944D, 0x94B9);
		queueWalk(100, 122);
		actionThumbsUp();
		_sprites[0].frameIndex = 0;
		_sprites[0].anim.codeId = 0;
		break;
	case 1:
		queueActor(0x9525, 0x9584, 0x95DC);
		queueWalk(183, 96);
		actionThumbsUp();
		_sprites[0].frameIndex = 0;
		_sprites[0].anim.codeId = 0;
		break;
	case 2:
		_sprites[2].setCodeSync(0x96B5, 0x96BC, kNone, 1, 2);
		_sprites[3].anim.set(0x1A2E, 0, 1, 1);
		queueWalk(4, 190);
		queueActor(0x93A0, 0, 0);
		actionThumbsUp();
		_sprites[0].frameIndex = 0;
		_sprites[0].anim.codeId = 0;
		break;
	case 3:
		resetActor();
		actorSprite().anim.codeId = 0x93C0;
		actorSprite().anim.initialTicks = 1;
		actorSprite().moveX.codeId = 0x93CA;
		actorSprite().moveX.initialTicks = 1;
		actorSprite().moveY.codeId = 0x93D3;
		actorSprite().moveY.initialTicks = 1;
		break;
	case 4:
		_sprites[2].frameIndex = 19;
		queueWalkToSprite(2, 20, 50);
		queueActor(0x93A4, 0x93AE, 0x93B7);
		actionThumbsUp();
		_sprites[0].frameIndex = 0;
		_sprites[0].anim.codeId = 0;
		playSound(7, actorSprite());
		break;
	case 9:
		// Fall into zombie grave
		resetActor();
		if (actorSprite().frameIndex <= 8) {
			actorSprite().changeCodeSync(0x9634, 0x963D, 0x9641);
		} else if (actorSprite().frameIndex <= 17) {
			actorSprite().changeCodeSync(0x9656, 0x965F, 0x9663);
		} else if (actorSprite().frameIndex <= 26) {
			actorSprite().changeCodeSync(0x9667, 0x9670, 0x9674);
		} else {
			actorSprite().changeCodeSync(0x9645, 0x964E, 0x9652);
		}
		actorSprite().anim.initialTicks = 8;
		actorSprite().moveX.initialTicks = 8;
		actorSprite().moveY.initialTicks = 8;
		break;
	case 10:
		// Fall into grave
		// Same as 9 but without initialTicks
		resetActor();
		if (actorSprite().frameIndex <= 8) {
			actorSprite().changeCodeSync(0x9634, 0x963D, 0x9641);
		} else if (actorSprite().frameIndex <= 17) {
			actorSprite().changeCodeSync(0x9656, 0x965F, 0x9663);
		} else if (actorSprite().frameIndex <= 26) {
			actorSprite().changeCodeSync(0x9667, 0x9670, 0x9674);
		} else {
			actorSprite().changeCodeSync(0x9645, 0x964E, 0x9652);
		}
		break;
	case 11:
		resetActor();
		actorSprite().changeCodeSync(0x9678, 0x968C, 0x9697);
		_sprites[2].anim.set(0x1A2F, 0, 1, 50);
		break;
	default:
		debug("EnchantiaEngine::handleBoxAction() %d unhandled", action);
	}

}

bool EnchantiaEngine::handleBoxSceneChange(int16 &type, int16 &param) {
	// Check if the scene can be changed/start an exit animation
	
	bool canChangeScene = false;
	uint16 exitAnimId = 0;
	
	switch (param) {
	
	case 0:
		canChangeScene = _sprites[5].frameIndex == 16;
		break;
		
	case 17:
		canChangeScene = _sprites[2].frameIndex == 13;
		break;
	
	case 35:
		canChangeScene = _sprites[2].status != 1;
		break;
	
	case 36:
		canChangeScene = !(_flags1 & 4);
		break;
	
	case 38:
		canChangeScene = _sprites[2].status != 1;
		break;
	
	case 63:
		canChangeScene = _spriteResourceType == 255;
		break;
	
	case 66:
		canChangeScene = _sprites[6].status == 1;
		break;
	
	case 68:
		canChangeScene = _sprites[3].status == 1;
		break;
	
	case 69:
		canChangeScene = _sprites[4].x == 295;
		break;
	
	case 72:
		canChangeScene = _sprites[25].frameIndex == 68;
		break;
	
	case 74:
		canChangeScene = _sprites[4].status == 1;
		break;
	
	case 83:
		canChangeScene = _sprites[3].status == 0;
		break;
	
	case 84:
		canChangeScene = _sprites[8].y== 30;
		break;
	
	case 91:
		canChangeScene = getSceneSpriteDef(47, 5)->status == 1;
		break;
	
	case 93:
		canChangeScene = getSceneSpriteDef(47, 6)->status == 1;
		break;
	
	case 95:
		canChangeScene = getSceneSpriteDef(47, 7)->status == 1;
		break;
	
	case 97:
		canChangeScene = getSceneSpriteDef(47, 8)->status == 1;
		break;
	
	case 99:
		canChangeScene = getSceneSpriteDef(47, 9)->status == 1;
		break;
	
	case 101:
		canChangeScene = getSceneSpriteDef(47, 10)->status == 1;
		break;
	
	case 103:
		canChangeScene = _spriteResourceType == 24;
		break;
	
	case 116:
		canChangeScene = _sprites[6].frameIndex != 22 && _sprites[4].status == 0;
		break;
		
	case 124:
		canChangeScene = _sprites[3].x + _cameraStripX == 155;
		break;

	case 56:
		exitAnimId = 0x18FD;
		break;		

	case 76:
		exitAnimId = 0x1907;
		break;		

	case 78:
		exitAnimId = 0x1911;
		break;		

	case 80:
		exitAnimId = 0x191B;
		break;
		
	case 70:
		if (getRandom(2))
			param = 52;
		canChangeScene = true;
		break;		

	case 105:
		if (getRandom(2))
			param = 106;
		canChangeScene = true;
		break;		

	case 107:
		if (!hasInventoryItem(idMatchBox))
			param = 108;
		canChangeScene = true;
		break;		

	case 108:
		if (getRandom(2))
			param = 109;
		canChangeScene = true;
		break;		

	default:
		canChangeScene = true;
		break;
		
	}
	
	if (!canChangeScene) {
		type = kBoxTypeNone;
		param = 0;
	}
	
	if (exitAnimId) {
		_flgCanRunBoxClick = false;
		_flgCanRunMenu = false;
		_flgWalkBoxEnabled = false;
		_walkDistance = 0;
		_currWalkInfo = NULL;
		actorSprite().setCodeSync(exitAnimId, 0x1925, 0x192E, 1, 1);
	}
	
	return canChangeScene || exitAnimId;
	
}

bool EnchantiaEngine::performCommand(int cmd, int item1, int item2) {

	#define ACTION1(cmd, item1) ((cmd << 16) | (255 << 8) | item1)
	#define ACTION2(cmd, item1, item2) ((cmd << 16) | (item1 << 8) | item2)
	
	#define UNLOCK(item1, item2) ACTION2(kCmdUnlock, item1, item2)
	#define INSERT(item1, item2) ACTION2(kCmdInsert, item1, item2)
	#define MOVE(item) ACTION1(kCmdMove, item)
	#define EAT(item) ACTION1(kCmdEat, item)
	#define WEAR(item) ACTION1(kCmdWear, item)
	#define THROW(item) ACTION1(kCmdThrow, item)
	#define GIVE(item1, item2) ACTION2(kCmdGive, item1, item2)
	#define COMBINE(item1, item2) ACTION2(kCmdCombine, item1, item2)
	#define TAKE(item) ACTION1(kCmdTake, item)
	#define TALK(item) ACTION1(kCmdTalk, item)
	#define LOOK(item) ACTION1(kCmdLook, item)
	#define FIGHT(item) ACTION1(kCmdFight, item)
	#define JUMP(item) ACTION1(kCmdJump, item)
	
	#define MUDCOMBI(item1, item2) ((item1 << 8) | item2)		

	static const SpriteListItem throwCoin47SpriteList1[] = {
		{26, -19,      0, 0x9E60},
		{16,  -9, 0x9E60, 0x9E60},
		{35,  -9, 0x9E65, 0x9E60},
		{16,   0, 0x9E60,      0},
		{35,   0, 0x9E65,      0},
		{16,   9, 0x9E60, 0x9E65},
		{35,   9, 0x9E65, 0x9E65}
	};

	static const SpriteListItem throwCoin47SpriteList2[] = {
		{-12,  16, 0x9E60,      0},
		{ -4,   0, 0x9E60, 0x9E60},
		{ 17, -10,      0, 0x9E60},
		{ 38,   0, 0x9E65, 0x9E60},
		{ 46,  16, 0x9E65,      0},
		{ 38,  32, 0x9E65, 0x9E65},
		{ 17,  43,      0, 0x9E65},
		{ -4,  32, 0x9E60, 0x9E65},
	};

	static const SpriteListItem eatGarlicSpriteList[] = {
		{144, 158, 0x2978, 0x297B},
		{ 86, 177, 0x297E, 0x2981},
		{ 92, 157, 0x2984, 0x2987},
		{119, 151, 0x298A, 0x298D},
		{134, 178, 0x2990, 0x2993}
	}; 

	static const SpriteListItem fightShovelSpriteList[] = {
		{373, 173, 0x2805, 0x2808},
		{355, 170, 0x280B, 0x280E},
		{336, 176, 0x2811, 0x2814},
		{343, 189, 0x2817, 0x281A}
	};

	static const SpriteListItem fightCrossSpriteList[] = {
		{219,  96, 0x29A3, 0x29A6},
		{229, 134, 0x29A9, 0x29AC},
		{196, 125, 0x29AF, 0x29B2},
		{195, 146, 0x29B5, 0x29B8},
		{230, 113, 0x29BB, 0x29BE},
		{200, 107, 0x29C1, 0x29C4},
		{228, 150, 0x29C7, 0x29CA}
	};

	bool handled = true;
	int16 walkTicks;
	Sprite *sprite;
	SpriteDef *spriteDef;
	SpriteTemplate *spriteTemplate;

	// NOTE This is a really huge switch, maybe rework this somehow (but how?)
	
	switch (ACTION2(cmd, item1, item2)) {

	// Unlock

	case UNLOCK(idKey, idShackles):
	case UNLOCK(idShackles, idKey):
		if (_spriteResourceType == 0) {
			getSceneSpriteDef(0, 9)->status = 0;
			_sprites[11].anim.set(0x1A2E, 0, 3, 1);
			removeInventoryItem(idKey);
			addInventoryItem(idKey);
			actorSprite().changeCodeSync(0x99CB, 0x99E8, 0x99FE);
			_theScore += 5;
			if (actorSprite().frameIndex != 0) {
				actorSprite().anim.index = 3;
				actorSprite().moveX.index = 3;
				actorSprite().moveY.index = 3;
			}
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case UNLOCK(idPaperClip, idKeyHole4):
	case UNLOCK(idKeyHole4, idPaperClip):
		getSceneSpriteDef(0, 3)->frameIndex = 16;
		getSceneSpriteDef(0, 3)->x += 17;
		queueWalk(55, 156);
		queueActor(0x9A2C, 0x9A46, 0);
		queueAnim(5, 21, 2, 0x9A6C, 0x9A73, 0);
		_theScore += 5;
		actionThumbsUp();
		break;
		
	case UNLOCK(idPaperClip, idKeyHole123):
	case UNLOCK(idKeyHole123, idPaperClip):
		removeInventoryItem(idPaperClip);
		removeInventoryItem(idKeyCard);
		_flags1 |= 2;
		getSceneSpriteDef(17, 1)->status = 0;
		getSceneSpriteDef(24, 0)->status = 3;
		getSceneSpriteDef(24, 1)->status = 3;
		getSceneSpriteDef(17, 0)->status = 0;
		getSceneSpriteDef(18, 0)->status = 0;
		getSceneSpriteDef(19, 0)->status = 0;
		getSceneSpriteDef(20, 0)->status = 0;
		getSceneSpriteDef(22, 3)->status = 0;
		getSceneSpriteDef(23, 1)->status = 0;
		getSceneSpriteDef(24, 2)->status = 0;
		getSceneSpriteDef(25, 3)->status = 0;
		getSceneSpriteDef(27, 1)->status = 0;
		queueWalk(264, 166);
		queueActor(0xA4CA, 0xA4DF, 0);
		queueAnim(4, 21, 1, 0xA5F0, 0xA5F4, 0xA5FF);
		_theScore += 5;
		actionThumbsUp();
		break;
		
	// Insert
	
	case INSERT(idKeyCard, idKeyCardLock):
	case INSERT(idKeyCardLock, idKeyCard):
		queueWalk(910 - _cameraStripX, 140);
		queueActor(0xA3A2, 0xA3B8, 0);
		queueAnim(6, 9, 1, 0x1A2C, 0, 0);
		actionThumbsUp();
		break;

	case INSERT(idGun, idHolster):
	case INSERT(idHolster, idGun):
		removeInventoryItem(idGun);
		getSceneSpriteDef(47, 0)->frameIndex = 12;
		getSceneSpriteDef(47, 1)->y = 24;
		getSceneSpriteDef(47, 2)->status = 7;
		getSceneSpriteDef(47, 3)->status = 7;
		_sprites[4].status = 7;
		_sprites[5].status = 7;
		queueWalk(89, 180);
		queueActor(0x96DE, 0x96E7, 0);
		queueAnim(2, 9, 2, 0xABB6, 0, 0);
		queueAnim(3, 9, 2, 0, 0, 0xABC0);
		_theScore += 5;
		actionThumbsUp();
		break;

	case INSERT(idCarJack, idDoor159):
	case INSERT(idDoor159, idCarJack):
		if (_sprites[12].status == 0) {
			removeInventoryItem(idCarJack);
			queueWalk(251, 163);
			queueActor(0xAC5A, 0xAC76, 0xAC8B);
			queueAnim(17, 15, 8, 0, 0, 0xACA0);
			queueAnim(18, 3, 4, 0xACA3, 0xACB0, 0);
			queueAnim(19, 3, 4, 0xACB7, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case INSERT(idMatch, idBabyFoot):
	case INSERT(idBabyFoot, idMatch):
		if (_scene63BabySleepCounter > 0) {
			queueWalk(242, 163);
			queueActor(0xAD53, 0xAD61, 0xAD6C);
			walkTicks = actionThumbsUp();
			if (walkTicks + 6 >= _scene63BabySleepCounter) {
				queueActor(0xAD50, 0, 0);
			} else {
				removeInventoryItem(idMatch);
				queueAnim(4, 7, 1, 0x1A2C, 0, 0);
				_theScore += 5;
			}
		} else
			actionThumbsDown();
		break;

	case INSERT(idCassette, idConsole):
	case INSERT(idConsole, idCassette):
		if (actorSprite().x + 8 - _sprites[7].x - _sprites[7].width >= 0 ||
			actorSprite().y + actorSprite().height - _sprites[7].y - _sprites[7].height >= 0) {
			removeInventoryItem(idCassette);
			getSceneSpriteDef(35, 1)->status = 1;
			getSceneSpriteDef(35, 1)->frameIndex = 71;
			getSceneSpriteDef(35, 1)->y = 7;
			queueWalk(752 - _cameraStripX, 172);
			queueActor(0xA362, 0xA370, 0);
			queueAnim(3, 9, 2, 0xA34F, 0, 0xA355);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case INSERT(idLetterStamp, idPostBox):
	case INSERT(idPostBox, idLetterStamp):
		if (actorSprite().y < 161) {
			queueWalk(427 - _cameraStripX, 174);
			if (_sprites[8].y + _sprites[8].height - actorSprite().y - actorSprite().height >= 0) {
				if (actorSprite().x2() - _sprites[8].x < 0 ||
					actorSprite().x - _sprites[8].x - _sprites[8].width >= 0) {
					// TODO Extend queueWalk
					_walkInfos[1].x = _walkInfos[0].x;
					_walkInfos[1].y = _walkInfos[0].y;
					_walkInfos[0].x = actorSprite().x;
					_walkInfos[0].next = &_walkInfos[1];
				} else {
					_walkInfos[2].x = _walkInfos[0].x;
					_walkInfos[2].y = _walkInfos[0].y;
					_walkInfos[1].x = _sprites[8].x - 16;
					_walkInfos[1].y = _walkInfos[0].y;
					_walkInfos[0].x = _sprites[8].x - 16;
					_walkInfos[0].y = actorSprite().y + 47;
					_walkInfos[0].next = &_walkInfos[1];
					_walkInfos[1].next = &_walkInfos[2];
				}
			}
		}
		removeInventoryItem(idLetterStamp);
		if (getSceneSpriteDef(32, 0)->status)
			getSceneSpriteDef(32, 0)->frameIndex = 4;
		else
			getSceneSpriteDef(35, 5)->status = 1;
		queueActor(0xA326, 0xA335, 0);
		_theScore += 5;
		actionThumbsUp();
		break;

	case INSERT(idHair, idBigNose):
	case INSERT(idBigNose, idHair):
		if (actorSprite().x2() - 6 - _sprites[4].x < 0 ||
			actorSprite().y + actorSprite().height - _sprites[4].y - _sprites[4].height >= 0) {
			removeInventoryItem(idHair);
			getSceneSpriteDef(34, 0)->status = 0;
			queueWalk(332 - _cameraStripX, 141);
			queueActor(0xA1BE, 0xA1CC, 0);
			_sprites[5].status = 0;
			_sprites[6].status = 0;
			getSceneSpriteDef(32, 2)->frameIndex = 29;
			getSceneSpriteDef(32, 2)->x = 55;
			getSceneSpriteDef(32, 2)->y = -39;
			getSceneSpriteDef(32, 3)->status = 0;
			getSceneSpriteDef(32, 4)->status = 0;
			queueAnim(4, 21, 1, 0xA1D6, 0xA1FC, 0xA215);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case INSERT(idMagnetOnString, idHole62):
	case INSERT(idHole62, idMagnetOnString):
		if (_sceneIndex == kSceneCaveHole) {
			queueWalkToSprite(3, 19, 42);
			queueActor(0xA018, 0xA02A, 0);
			actionThumbsUp();
			_flgWalkBoxEnabled = true;
			if (_sprites[4].status) {
				addInventoryItem(idThread);
				getSceneSpriteDef(13, 2)->status = 0;
				_sprites[4].status = 0;
				_theScore += 5;
			}
		} else
			actionThumbsDown();
		break;
		
	case INSERT(idCattleProd, idPlug):
	case INSERT(idPlug, idCattleProd):
		removeInventoryItem(idCattleProd);
		removeInventoryItem(idKey);
		_sprites[18].heightAdd = 100;
		_sprites[18].setCodeSync(0x9B99, kNone, 0x9BA0, 0, 2);
		_sprites[14].status = 3;
		_sprites[14].heightAdd = 100;
		_sprites[14].setCodeSync(0x9BA7, 0x9BB2, 0x9BBD, 0, 2);
		queueWalkToSprite(18, 68, 30);
		_sprites[14].setPos(_sprites[18].x + 68 - 7, _sprites[18].y + 30 - 22);
		queueActor(0x9AEB, 0x9B03, 0x9B12);
		actionThumbsUp();
		_sprites[18].anim.ticks = _walkDistance + 14;
		_sprites[18].moveY.ticks = _walkDistance + 14;
		_sprites[14].anim.ticks = _walkDistance + 14;
		_sprites[14].moveX.ticks = _walkDistance + 14;
		_sprites[14].moveY.ticks = _walkDistance + 14;
		_theScore += 5;
		break;

	// Move		
		
	case MOVE(idCellWall):
		if (_sprites[4].frameIndex == 3) {
			_sprites[4].anim.codeId = 0x9A59;
			_sprites[4].anim.ticks = 10;
			_sprites[4].moveY.codeId = 0x9A64;
			_sprites[4].moveY.ticks = 10;
			getSceneSpriteDef(0, 2)->frameIndex = 10;
			getSceneSpriteDef(0, 2)->y = 27;
			getSceneSpriteDef(0, 7)->frameIndex = 42;
			_sprites[9].status = 1;
			queueWalk(210, 157);
			queueActor(0x9A14, 0x9A26, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case MOVE(idStuckFish):
		if (_sprites[3].status) {
			_sprites[3].anim.codeId = 0x9B21;
			_sprites[3].anim.index = 0;
			_sprites[3].anim.ticks = 15;
			_sprites[3].moveX.codeId = 0x9B26;
			_sprites[3].moveX.index = 0;
			_sprites[3].moveX.ticks = 15;
			_sprites[3].moveY.codeId = 0x9B2B;
			_sprites[3].moveY.index = 0;
			_sprites[3].moveY.ticks = 15;
			queueWalkToSprite(3, -16, 24);
			queueActor(0x9AA1, 0x9AB2, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case MOVE(idButton48):
		if (_sprites[2].frameIndex == 6) {
			_sprites[2].anim.codeId = 0x9DC0;
			queueWalkToSprite(3, -15, 33);
			queueActor(0x9D91, 0x9DB2, 0);
			_sprites[2].anim.ticks = actionThumbsUp() + 12;
			_theScore += 5;
		} else
			actionThumbsDown();
		break;

	case MOVE(idCliffRock):
		if (_sprites[5].y == 76) {
			queueWalk(453 - _cameraStripX, 113);
			queueActor(0xA035, 0xA04D, 0);
			queueAnim(5, 5, 2, 0xA0D2, 0xA0DC, 0xA0ED);
			queueAnim(21, 41, 2, 0xA196, 0xA19A, 0xA19E);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
	
	case MOVE(idButton88):
	case MOVE(idButton89):
	case MOVE(idButton90):
	case MOVE(idButton91):
		{
			uint spriteIndex;
			switch (item2) {
			case idButton88:
				_flags1 ^= 0x08;
				spriteIndex = 9;
				break;
			case idButton89:
				_flags1 ^= 0x10;
				spriteIndex = 10;
				break;
			case idButton90:
				_flags1 ^= 0x20;
				spriteIndex = 11;
				break;
			case idButton91:
				_flags1 ^= 0x40;
				spriteIndex = 12;
				break;
			default:
				_flags1 = 0;
				spriteIndex = 0;
				break;
			}
			queueWalkToSprite(spriteIndex, 4, 19);
			queueActor(0xA082, 0xA08C, 0);
			if (_sprites[spriteIndex].frameIndex == 20)
				queueAnim(spriteIndex, 7, 1, 0xA105, 0, 0);
			else
				queueAnim(spriteIndex, 7, 1, 0xA107, 0, 0);
			if (((_flags1 >> 3) & 0x0F) == 11)
				queueAnim(8, 11, 2, 0xA12C, 0, 0);
			actionThumbsUp();
		}
		break;

	case MOVE(idRemote):
		if (_sprites[3].status == 1 && 
			((actorSprite().x + _cameraStripX >= 735 && actorSprite().y >= 123) ||
			(actorSprite().x - _sprites[7].x - _sprites[7].width >= 0) ||
			(actorSprite().x - _sprites[7].x - _sprites[7].width + 50 >= 0 && actorSprite().y >= 123))) {
			removeInventoryItem(idRemote);
			addInventoryItem(idCassetteMusic);
			getSceneSpriteDef(35, 1)->status = 0;
			queueWalk(752 - _cameraStripX, 172);
			queueActor(0xA379, 0xA390, 0);
			queueAnim(3, 59, 2, 0xA359, 0, 0xA35F);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case MOVE(idGoldFoil):
		if (_sceneIndex == kSceneElectro && _sprites[6].status == 1) {
			removeInventoryItem(idGoldFoil);
			queueWalk(39, 155);
			queueActor(0xA447, 0xA450, 0);
			if (_sprites[6].status == 1) {
				queueAnim(6, 7, 1, 0x1A2E, 0, 0);
				queueAnim(5, 7, 1, 0x1A2C, 0, 0);
				_theScore += 5;
			}
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case MOVE(idButton122):
		queueWalk(22, 155);
		queueActor(0xA456, 0xA467, 0);
		queueAnim(3, 13, 1, 0, 0, 0xA565);
		queueAnim(2, 13, 1, 0xA4EC, 0, 0);
		_theScore += 5;
		actionThumbsUp();
		break;

	case MOVE(idIcicle136):
		_flags2 &= 0xF9;
		_flags2 |= 2;
		queueWalk(_sprites[5].x + 7, 145);
		queueActor(0xAAFA, 0x96E7, 0);
		actionThumbsUp();
		break;

	case MOVE(idIcicle137):
		if (((_flags2 >> 1) & 3) == 1) {
			_flags2 &= 0xF9;
			_flags2 |= 4;
		} else {
			_flags2 &= 0xF9;
		}
		queueWalk(_sprites[6].x + 7, 145);
		queueActor(0xAB03, 0x96E7, 0);
		actionThumbsUp();
		break;

	case MOVE(idIcicle138):
		if (((_flags2 >> 1) & 3) == 2) {
			_flags2 |= 6;
		} else {
			_flags2 &= 0xF9;
		}
		queueWalk(_sprites[7].x + 7, 145);
		queueActor(0xAB0C, 0x96E7, 0);
		actionThumbsUp();
		break;

	case MOVE(idIcicle139):
		if (((_flags2 >> 1) & 3) == 3) {
			queueAnim(2, 9, 1, 0xAA98, 0, 0xAA9C);
			queueAnim(3, 9, 1, 0, 0, 0xAACB);
		} else {
			_flags2 &= 0xF9;
		}
		queueWalk(_sprites[8].x + 7, 145);
		queueActor(0xAB15, 0x96E7, 0);
		actionThumbsUp();
		break;

	case MOVE(idGraveStone):
		if (((_flags2 >> 3) & 7) == 2) {
			_flags2 ^= 8;
			getSceneSpriteDef(77, 0)->status = 1;
			queueWalk(440 - _cameraStripX, 159);
			queueActor(0x96DE, 0x96E7, 0);
			getSceneSpriteDef(65, 5)->frameIndex = 26;
			getSceneSpriteDef(65, 5)->y += 16;
			queueAnim(2, 1, 2, 0x281D, 0x2846, 0x2861);
			queueAnim(7, 7, 2, 0x287C, 0, 0x287F);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case MOVE(idGate):
		queueWalk(180 - _cameraStripX, 171);
		queueActor(0x2A65, 0x2A9B, 0x2ACB);
		queueAnim(2, 7, 2, 0x2AFB, 0, 0x2B01);
		queueAnim(3, 83, 2, 0, 0, 0x2B1A);
		queueAnim(4, 67, 2, 0x2B05, 0, 0x2B10);
		queueAnim(5, 75, 2, 0, 0, 0x2B1A);
		_theScore += 5;
		actionThumbsUp();
		break;

	case MOVE(idBook):
		if (_sprites[3].x + _cameraStripX == 187) {
			queueWalk(142 - _cameraStripX, 159);
			if (actorSprite().y >= 132) {
				// TODO Extend queueWalk
				_walkInfos[1].x = _walkInfos[0].x;
				_walkInfos[1].y = _walkInfos[0].y;
				_walkInfos[0].y = 179;
				_walkInfos[0].next = &_walkInfos[1];
			}
			queueActor(0x2B24, 0x2B34, 0);
			queueAnim(2, 11, 1, 0x1A2C, 0, 0);
			queueAnim(3, 15, 2, 0x2B3D, 0x2B40, 0);
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idMatchBox):
	case MOVE(idMatchBox):
		if (_scene63BabySleepCounter > 0) {
			queueWalk(242, 163);
			queueActor(0xAD53, 0xAD61, 0xAD6C);
			if (actionThumbsUp() + 6 >= _scene63BabySleepCounter) {
				// Walking takes longer than the baby will keep sleeping
				queueActor(0xAD50, 0, 0);
			} else {
				removeInventoryItem(idMatchBox);
				queueAnim(4, 7, 1, 0xAD77, 0, 0);
				queueAnim(6, 45, 1, 0x1A2C, 0, 0);
				queueAnim(3, 17, 2, 0xAD9E, 0, 0);
				queueAnim(2, 19, 2, 0xADA8, 0xADB8, 0xADBC);
				queueAnim(8, 45, 2, 0xADCA, 0, 0);
				queueAnim(7, 43, 2, 0xADC0, 0xADC4, 0xADC7);
				_theScore += 5;
			}
		} else
			actionThumbsDown();
		break;

	case MOVE(idFireExtinguisher):
		if (_sceneIndex == 72 && ((_flags2 >> 2) & 3) == 1) {
			_flags2 ^= 0xC0;
			actorSprite().changeCodeSync(0x2B79, 0x2B85, kNone);
			_sprites[10].anim.set(0x1A2F, 0, 3, 22);
			_sprites[11].setCodeSync(0x2B8C, 0x2B98, 0x2BA0, 9, 2);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	// Eat
	
	case EAT(idMeat):
		if (_spriteResourceType == 255) {
			removeInventoryItem(idMeat);
			if (actorSprite().frameIndex >= 18 && actorSprite().frameIndex <= 26)
				actorSprite().changeCodeSync(0x1A69, 0x1A77, kNone);
			else
				actorSprite().changeCodeSync(0x1A4E, 0x1A5C, kNone);
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case EAT(idGarlic):
		if (((_flags2 >> 3) & 7) == 3) { 
			spriteDef = getSpriteDef(0x275E);
			_flags2 ^= 0x38;
			removeInventoryItem(idGarlic);
			getSceneSpriteDef(77, 0)->status = 3;
			queueWalk(78 - _cameraStripX, 177);
			queueActor(0x2904, 0x2920, 0x2938);
			queueAnim(2, 1, 2, 0x2882, 0x28AE, 0x28CF);
			queueAnim(4, 33, 2, 0x28F0, 0, 0x28FB);
			for (uint i = 0; i < 5; i++) {
				spriteDef->setPos(eatGarlicSpriteList[i].x - _cameraStripX, eatGarlicSpriteList[i].y);
				sprite = insertSprite(spriteDef);
				if (sprite) {
					queueAnim(0, 85, 1, 0x2779, eatGarlicSpriteList[i].moveXListId,
						eatGarlicSpriteList[i].moveYListId);
					_animQueue[_animQueueCount - 1].sprite = sprite;
				}
			}
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	// Wear
	
	case WEAR(idFishBowl):
		if (_sceneIndex == kSceneUnderwater) {
			removeInventoryItem(idFishBowl);
			removeInventoryItem(idGoldfish);
			_sprites[2].setPos(actorSprite().x, actorSprite().y);
			_sprites[2].setCodeSync(0x9A91, 0x9A95, 0x9A9B, 7, 2);
			_sprites[19].setCodeSync(0x9BC8, 0x9BCC, 0x9BCE, 7, 2);
			actorSprite().changeCodeSync(0x9A78, 0x9A87, kNone);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case WEAR(idDress):
		if (_sceneIndex == kSceneChangingRoom) {
			removeInventoryItem(idDress);
			getSceneSpriteDef(25, 2)->status = 1;
			getSceneSpriteDef(25, 2)->y = 2;
			getSceneSpriteDef(25, 0)->status = 1;
			getSceneSpriteDef(25, 0)->frameIndex = 13;
			queueWalk(176, 157);
			queueActor(0x1B8E, 0x1BB3, 0);
			_sprites[3].anim.set(0x1FC7, 0, 0, 2);
			_sprites[4].setCodeSync(0x1FCB, kNone, 0x1FD1, 0, 2);
			_sprites[2].anim.set(0x1FD7, 0, 0, 2);
			_sprites[3].setPos(136, 137);
			walkTicks = actionThumbsUp();
			_sprites[3].anim.ticks = walkTicks + 14;
			_sprites[4].anim.ticks = walkTicks + 14 + 58;
			_sprites[4].moveY.ticks = walkTicks + 14 + 58;
			_sprites[2].anim.ticks = walkTicks + 14 + 58 + 10;
			_theScore += 5;
		} else
			actionThumbsDown();
		break;

	case WEAR(idHelmet):
		if (_sceneIndex == kSceneCaveRocks) {
			removeInventoryItem(idHelmet);
			queueWalk(820 - _cameraStripX, 160);
			queueActor(0x9C4B, 0x9C8D, 0x9CC2);
			_sprites[7].setCodeSync(0x9CF7, 0x9D02, 0x9D2B, 0, 2);
			_sprites[10].setCodeSync(0x1A2C, 0x9D54, 0x9D67, 0, 1);
			_sprites[8].setCodeSync(0x1A2C, 0x9D83, 0x9D7B, 0, 1);
			_sprites[9].setCodeSync(0x1A2C, 0x9D8A, 0x9D7B, 0, 1);
			walkTicks = actionThumbsUp() + 8;
			_sprites[7].setTicksSync(walkTicks);
			_sprites[10].setTicksSync(walkTicks + 32);
			_sprites[8].setTicksSync(walkTicks + 32 + 10);
			_sprites[9].setTicksSync(walkTicks + 32 + 10);
			_theScore += 5;
		} else
			actionThumbsDown();
		break;

	case WEAR(idMudBranchSeaWeed):
		if (_spriteResourceType == 14) {
			removeInventoryItem(idMudBranchSeaWeed);
			removeInventoryItem(idKey);
			removeInventoryItem(idRock27);
			removeInventoryItem(idRock34);
			removeInventoryItem(idRock39);
			removeInventoryItem(idRock41);
			removeInventoryItem(idRock43);
			removeInventoryItem(idRock46);
			removeInventoryItem(idRock28);
			removeInventoryItem(idRock30);
			removeInventoryItem(idRock32);
			removeInventoryItem(idRock35);
			removeInventoryItem(idRock40);
			removeInventoryItem(idRock45);
			removeInventoryItem(idRock29);
			removeInventoryItem(idRock31);
			removeInventoryItem(idRock33);
			removeInventoryItem(idRock38);
			removeInventoryItem(idRock42);
			removeInventoryItem(idRock44);
			actorSprite().changeCodeSync(0x9FF8, kNone, 0xA00A);
			_sprites[2].moveY.set(0xA016, 0, 21, 2);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idGloves):
	case WEAR(idGloves):
		if (_sprites[2].x - actorSprite().x <= 40) {
			removeInventoryItem(idGloves);
			queueWalk(974 - _cameraStripX, 122);
			queueActor(0xA061, 0xA072, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case WEAR(idItem87):
		if (ABS(_sprites[15].x - actorSprite().x) <= 30) {
			removeInventoryItem(idItem87);
			queueWalk(_sprites[15].x + 2, 114);
			queueActor(0xA09A, 0xA09F, 0);
			_sprites[4].setPos(1354 - _cameraStripX, 92);
			_sprites[4].heightAdd = 100;
			_sprites[4].status = 3;
			_sprites[4].anim.set(0x1A2C, 0, 0, 1);
			_theScore += 5;
			_sprites[4].anim.ticks = actionThumbsUp() + 4;
		} else
			actionThumbsDown();
		break;

	case WEAR(idSprayCan):
		removeInventoryItem(idSprayCan);
		_flags1 |= 0x80;
		actorSprite().changeCodeSync(0xA60A, 0xA626, kNone);
		_theScore += 5;
		actionThumbsUp();
		break;

	case WEAR(idSunTanOil):
		if (_spriteResourceType == 23 && actorSprite().frameIndex == 29) {
			removeInventoryItem(idSunTanOil);
			actorSprite().changeCodeSync(0xACF6, 0xAD0E, 0xAD21);
			_sprites[2].setCodeSync(0xAD34, 0xAD3A, 0xAD45, 29, 1);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	// Throw
	
	case THROW(idMarblesBag):
		if (_sceneIndex == kSceneElectro && _sprites[3].y != 0) {
			removeInventoryItem(idMarblesBag);
			queueWalk(103, 177);
			queueActor(0xA46D, 0xA47A, 0);
			queueAnim(7, 7, 2, 0xA5B6, 0xA5C1, 0xA5CA);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case THROW(idTray):
		if (_sceneIndex == kSceneElectro && _sprites[7].status == 1) {
			removeInventoryItem(idTray);
			queueWalk(112, 171);
			queueActor(0xA482, 0xA498, 0xA4B9);
			queueAnim(8, 5, 2, 0xA5E5, 0xA5ED, 0);
			queueAnim(7, 15, 1, 0xA5D3, 0xA5D6, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case THROW(idWoodenPlank):
		if (_sceneIndex == kSceneIceDesert41) {
			removeInventoryItem(idWoodenPlank);
			getSceneSpriteDef(41, 1)->frameIndex = 52;
			getSceneSpriteDef(41, 1)->x = -6;
			getSceneSpriteDef(41, 1)->y = 7;
			getSceneSpriteDef(41, 1)->templateId = 0xA683;
			getSceneSpriteDef(41, 2)->status = 1;
			queueWalk(274 - _cameraStripX, 143);
			queueActor(0xA639, 0xA63D, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case THROW(idDice):
		{
			int16 throw1, throw2, throw3;
			removeInventoryItem(idDice);
			getSceneSpriteDef(47, 9)->status = 0;
			getSceneSpriteDef(47, 7)->status = 0;
			getSceneSpriteDef(47, 5)->status = 0;
			getSceneSpriteDef(47, 10)->status = 0;
			getSceneSpriteDef(47, 8)->status = 0;
			getSceneSpriteDef(47, 6)->status = 0;
			getSceneSpriteDef(48, 4)->status = 0;
			getSceneSpriteDef(48, 2)->status = 0;
			getSceneSpriteDef(48, 5)->status = 0;
			getSceneSpriteDef(48, 3)->status = 0;
			getSceneSpriteDef(48, 1)->status = 0;
			getSceneSpriteDef(49, 6)->status = 0;
			getSceneSpriteDef(49, 7)->status = 0;
			getSceneSpriteDef(49, 5)->status = 0;
			throw1 = getRandom(6);
			_sprites[4].frameIndex = throw1 + 1;
			throw2 = getRandom(100) < 50 ? throw1 : getRandom(6);
			_sprites[5].frameIndex = throw2 + 1;
			throw3 = getRandom(100) < 50 ? throw1 : getRandom(6);
			_sprites[6].frameIndex = throw3 + 1;
			if (throw3 == throw1 && throw3 == throw2) {
				if (throw3 & 1) {
					getSceneSpriteDef(47, 7)->status = 1;
					getSceneSpriteDef(48, 2)->status = 1;
					getSceneSpriteDef(47, 10)->status = 1;
					getSceneSpriteDef(48, 5)->status = 1;
					getSceneSpriteDef(49, 7)->status = 1;
					getSceneSpriteDef(47, 6)->status = 1;
					getSceneSpriteDef(48, 1)->status = 1;
				} else {
					getSceneSpriteDef(47, 9)->status = 1;
					getSceneSpriteDef(48, 4)->status = 1;
					getSceneSpriteDef(49, 6)->status = 1;
					getSceneSpriteDef(47, 5)->status = 1;
					getSceneSpriteDef(47, 8)->status = 1;
					getSceneSpriteDef(48, 3)->status = 1;
					getSceneSpriteDef(49, 5)->status = 1;
				}
			} else {
				int16 throw0 = -1;
				if (throw3 == throw1 || throw3 == throw2)
					throw0 = throw3;
				else if (throw1 == throw2)
					throw0 = throw1;
				switch (throw0) {
				case 0:
					getSceneSpriteDef(47, 9)->status = 1;
					getSceneSpriteDef(48, 4)->status = 1;
					getSceneSpriteDef(49, 6)->status = 1;
					break;
				case 1:
					getSceneSpriteDef(47, 7)->status = 1;
					getSceneSpriteDef(48, 2)->status = 1;
					break;
				case 2:
					getSceneSpriteDef(47, 5)->status = 1;
					break;
				case 3:
					getSceneSpriteDef(47, 10)->status = 1;
					getSceneSpriteDef(48, 5)->status = 1;
					getSceneSpriteDef(49, 7)->status = 1;
					break;
				case 4:
					getSceneSpriteDef(47, 8)->status = 1;
					getSceneSpriteDef(48, 3)->status = 1;
					getSceneSpriteDef(49, 5)->status = 1;
					break;
				case 5:
					getSceneSpriteDef(47, 6)->status = 1;
					getSceneSpriteDef(48, 1)->status = 1;
					break;
				}
			}
			getSceneSpriteDef(46, 0)->status = 1;
			_sprites[2].status = 3;
			queueWalk(232, 159);
			queueActor(0xAB1E, 0xAB33, 0);
			queueAnim(2, 17, 2, 0xAB41, 0xAB4E, 0xAB57);
			queueAnim(3, 31, 50, 0x1A2F, 0, 0);
			queueAnim(4, 31, 50, 0x1A2F, 0, 0);
			queueAnim(5, 31, 50, 0x1A2F, 0, 0);
			queueAnim(6, 31, 50, 0x1A2F, 0, 0);
			queueAnim(8, 17, 1, 0xAB60, 0, 0xAB63);
			actionThumbsUp();
		}
		break;

	case THROW(idComputer):
		if (_sprites[2].frameIndex == 6 &&
			ABS(actorSprite().x - _sprites[2].x) <= 60 &&	
			ABS(actorSprite().y + actorSprite().height - _sprites[2].y - _sprites[2].height) <= 5) {
			removeInventoryItem(idComputer);
			getSceneItemInfo(idMagnet).flags |= 2;
			queueWalkToSprite(2, -1, 39);
			_sprites[4].setPos(_sprites[2].x + 5, _sprites[2].y + 4);
			_sprites[4].setCodeSync(0x9FA1, 0x9FA8, 0x9FBC, 0, 1);
			_sprites[2].heightAdd = 180;
			_sprites[2].setCodeSync(0x9FD0, 0x9FD2, 0x9FD4, 0, 1);
			queueActor(0x9F64, 0x9F7D, 0x9F8F);
			walkTicks = actionThumbsUp() + 8;
			_sprites[4].setTicksSync(walkTicks);
			_sprites[2].setTicksSync(walkTicks + 18);
			_theScore += 5;
		} else
			actionThumbsDown();
		break;
		
	case THROW(idWishCoin):
		if (_sceneIndex == kSceneWishingWell && _sprites[2].status == 0) {
			removeInventoryItem(idWishCoin);
			_collectItemIgnoreActorPosition = true;
			queueActor(0x9DCE, 0x9DE5, 0);
			queueWalk(118, 167);
			_sprites[3].setCodeSync(0x9DF6, 0x9E18, 0x9E2B, 0, 1);
			_sprites[4].anim.set(0x1A2C, 0, 0, 2);
			_sprites[5].anim.set(0x1A2C, 0, 0, 2);
			_sprites[6].anim.set(0x1A2C, 0, 0, 2);
			walkTicks = actionThumbsUp() + 8;
			_sprites[3].setTicksSync(walkTicks);
			walkTicks += 17;
			insertSpriteList(throwCoin47SpriteList1, 7, 0x9E3E, 3, walkTicks);
			walkTicks += 32;
			_sprites[4].anim.ticks = walkTicks;
			insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 4, walkTicks);
			walkTicks += 12;
			_sprites[5].anim.ticks = walkTicks;
			insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 5, walkTicks);
			walkTicks += 12;
			_sprites[6].anim.ticks = walkTicks;
			insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 6, walkTicks);
			_theScore += 5;
		} else
			actionThumbsDown();
		break;

	case THROW(idRock94):
		if (ABS(actorSprite().x + _cameraStripX - 1361) <= 30) {
			spriteDef = getSpriteDef(0xA17B);
			removeInventoryItem(idRock94);
			queueWalk(1368 - _cameraStripX, 113);
			queueActor(0xA0A4, 0xA0AF, 0);
			walkTicks = actionThumbsUp();
			_sprites[15].setPos(1388 - _cameraStripX, 87);
			_sprites[15].status = 3;
			_sprites[15].setCodeSync(0xA136, 0xA146, 0xA152, walkTicks + 6, 1);
			_sprites[6].setCodeSync(kNone, 0xA163, 0xA169, walkTicks + 6 + 4, 1);
			_sprites[23].setCodeSync(0xA1AE, 0xA1B6, 0xA1BA, walkTicks + 6 + 4 + 2 + 2 + 2, 2);
			_sprites[19].setPos(1372 - _cameraStripX, 112);
			_sprites[19].setCodeSync(0xA196, 0xA1A6, 0xA1A2, walkTicks + 6 + 4 + 2 + 2 + 2 + 2 + 6, 2);
			_sprites[20].setPos(1457 - _cameraStripX, 111);
			_sprites[20].setCodeSync(0xA196, 0xA1AA, 0xA1A2, walkTicks + 6 + 4 + 2 + 2 + 2 + 2 + 6, 2);

			spriteDef->setPos(1386 - _cameraStripX, 70);
			sprite = insertSprite(spriteDef);
			sprite->anim.ticks = walkTicks + 6 + 4;
			sprite->moveX.ticks = walkTicks + 6 + 4;
			sprite->moveY.ticks = walkTicks + 6 + 4;

			spriteDef->setPos(1433 - _cameraStripX, 79);
			sprite = insertSprite(spriteDef);
			sprite->anim.ticks = walkTicks + 6 + 4 + 2;
			sprite->moveX.ticks = walkTicks + 6 + 4 + 2;
			sprite->moveY.ticks = walkTicks + 6 + 4 + 2;

			spriteDef->setPos(1406 - _cameraStripX, 80);
			sprite = insertSprite(spriteDef);
			sprite->anim.ticks = walkTicks + 6 + 4 + 2 + 2;
			sprite->moveX.ticks = walkTicks + 6 + 4 + 2 + 2;
			sprite->moveY.ticks = walkTicks + 6 + 4 + 2 + 2;

			spriteDef->setPos(1445 - _cameraStripX, 67);
			sprite = insertSprite(spriteDef);
			sprite->anim.ticks = walkTicks + 6 + 4 + 2 + 2 + 2;
			sprite->moveX.ticks = walkTicks + 6 + 4 + 2 + 2 + 2;
			sprite->moveY.ticks = walkTicks + 6 + 4 + 2 + 2 + 2;

			spriteDef->setPos(1378 - _cameraStripX, 55);
			sprite = insertSprite(spriteDef);
			sprite->anim.ticks = walkTicks + 6 + 4 + 2 + 2 + 2 + 2;
			sprite->moveX.ticks = walkTicks + 6 + 4 + 2 + 2 + 2 + 2;
			sprite->moveY.ticks = walkTicks + 6 + 4 + 2 + 2 + 2 + 2;

			_theScore += 5;
		} else
			actionThumbsDown();
		break;
		
	case THROW(idRope):
		if (ABS(1567 - _cameraStripX - actorSprite().x) <= 40) {
			removeInventoryItem(idRope);
			_sprites[18].id = 96;
			_sprites[18].yAdd = 30;
			queueWalk(1574 - _cameraStripX, 112);
			queueActor(0xA0B9, 0xA0C7, 0);
			queueAnim(18, 9, 1, 0xA179, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case THROW(idItem120):
		if (_sceneIndex == kSceneElectro) {
			removeInventoryItem(idItem120);
			queueWalk(39, 155);
			queueActor(0xA438, 0xA441, 0);
			queueAnim(6, 7, 1, 0x1A2C, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case THROW(idItem131):
		if (_sceneIndex == kSceneBoat) {
			removeInventoryItem(idItem131);
			_scrollBorderLeft = 0;
			_scrollBorderRight = 500;
			queueWalk(102 - _cameraStripX, 177);
			queueActor(0xA993, 0xA9B5, 0xA9D7);
			queueAnim(4, 43, 2, 0xA9F7, 0xAA29, 0xAA41);
			queueAnim(5, 21, 2, 0xA8F4, 0, 0xA906);
			queueAnim(6, 49, 2, 0xAA59, 0xAA6C, 0xAA83);
			queueAnim(7, 27, 2, 0xA8F4, 0, 0xA906);
			queueAnim(8, 51, 46, 0x1A32, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	// Give

	case GIVE(idWorm, idMrFish):
	case GIVE(idMrFish, idWorm):
		removeInventoryItem(idWorm);
		_sprites[12].status = 1;
		_sprites[12].anim.set(0x9B35, 0, 1, 2);
		_sprites[12].moveX.set(0x9B50, 0, 1, 1);
		_sprites[9].anim.set(0x1A2E, 0, 58, 1);
		_sprites[10].anim.set(0x1A2E, 0, 50, 1);
		_sprites[13].status = 0;
		_sprites[7].setCodeSync(kNone, 0x9B85, 0x9B8F, 50, 1);
		_sprites[8].moveX.set(0x9B85, 8, 64, 1);
		_sprites[8].moveY.set(0x9B8F, 8, 64, 1);
		queueWalkToSprite(7, 12, 52);
		queueActor(0x9ABC, 0x9AD6, 0);
		_theScore += 5;
		actionThumbsUp();
		break;

	case GIVE(idShell, idTurtle):
		removeInventoryItem(idShell);
		_scrollBorderLeft = 160;
		_sprites[12].id = 0x40B;
		queueWalkToSprite(16, 38, 40);
		queueActor(0x99C8, 0, 0);
		_theScore += 5;
		actionThumbsUp();
		break;

	case GIVE(idRock27, idRockBasher):
	case GIVE(idRock34, idRockBasher):
	case GIVE(idRock39, idRockBasher):
	case GIVE(idRock41, idRockBasher):
	case GIVE(idRock43, idRockBasher):
	case GIVE(idRock46, idRockBasher):
	case GIVE(idRock28, idRockBasher):
	case GIVE(idRock30, idRockBasher):
	case GIVE(idRock32, idRockBasher):
	case GIVE(idRock35, idRockBasher):
	case GIVE(idRock40, idRockBasher):
	case GIVE(idRock45, idRockBasher):
	case GIVE(idRock29, idRockBasher):
	case GIVE(idRock31, idRockBasher):
	case GIVE(idRock33, idRockBasher):
	case GIVE(idRock38, idRockBasher):
	case GIVE(idRock42, idRockBasher):
	case GIVE(idRock44, idRockBasher):
		{
			uint index;
			removeInventoryItem(item1);
			switch (item1) {
			case idRock27: case idRock34: case idRock39:
			case idRock41: case idRock43: case idRock46:
				index = 0;
				break;
			case idRock28: case idRock30: case idRock32:
			case idRock35: case idRock40: case idRock45:
				index = 1;
				break;
			case idRock29: case idRock31: case idRock33:
			case idRock38: case idRock42: case idRock44:
				index = 2;
				break;
			default:
				index = 0;
				break;
			}
			if (--_rockBasherCounters[index] >= 0) {
				getSceneSpriteDef(10, 1 + index * 4 + _rockBasherCounters[index])->status = 0;
				_sprites[3 + index * 4 + _rockBasherCounters[index]].status = 0;
			}
			queueWalkToSprite(2, -10, 34);
			if (_rockBasherCounters[0] == 0 && _rockBasherCounters[1] == 0 && _rockBasherCounters[2] == 0) {
				addInventoryItem(idString);
				queueActor(0x9EFD, 0x9F0A, 0);
				queueAnim(2, 3, 2, 0x9F10, 0, 0);
			} else {
				queueActor(0x9EE2, 0x9EEF, 0);
				queueAnim(2, 3, 2, 0x9EF5, 0, 0);
			}
			_theScore += 5;
			actionThumbsUp();
		}
		break;

	case GIVE(idMoneyPouch, idFrank):
		removeInventoryItem(idMeat);
		addInventoryItem(idMeat);
		queueWalk(150, 120);
		queueActor(0x96DE, 0x96E7, 0);
		queueAnim(2, 9, 2, 0x224C, 0, 0);
		actionThumbsUp();
		break;

	case GIVE(idMoneyPouch, idSally):
		queueWalk(199, 159);
		queueActor(0x1B2F, 0x1B50, 0x1B6F);
		if ((_flags1 & 1) == 0) {
			queueAnim(2, 25, 2, 0x20EB, 0x2129, 0x2156);
			queueAnim(4, 47, 2, 0x2183, 0x2194, 0x21A2);
		} else if ((_flags1 & 6) == 0) {
			queueAnim(2, 25, 2, 0x2072, 0x20A3, 0x20C7);
			queueAnim(4, 47, 2, 0x21B0, 0x21C1, 0x21CF);
		} else if (((_flags1 >> 1) & 3) == 3) {
			queueAnim(2, 25, 2, 0x1FDD, 0x1FF4, 0x2006);
			queueAnim(4, 47, 2, 0x220A, 0x2211, 0x2218);
		} else if ((_flags1 & 2) == 0) {
			queueAnim(2, 25, 2, 0x2018, 0x203C, 0x2057);
			queueAnim(4, 47, 2, 0x221F, 0x2230, 0x223E);
		} else {
			queueAnim(2, 25, 2, 0x2018, 0x203C, 0x2057);
			queueAnim(4, 47, 2, 0x21DD, 0x21EE, 0x21FC);
		}
		actionThumbsUp();
		break;

	case GIVE(idMoneyPouch, idMage):
		removeInventoryItem(idMeat);
		if ((_flags1 & 1) && (((_flags1 & 2) == 0 && getSceneSpriteDef(17, 1)->status == 1) ||
			((_flags1 & 2) && (_flags1 & 4) == 0)))
			actionThumbsDown();
		else {
			if ((_flags1 & 1) == 0) {
				// Teleport to cliff
				queueActor(0x1DEF, 0x1E20, 0x1E4E);
			} else if ((_flags1 & 2) == 0) {
				// Give pig mask disguise
				getSceneSpriteDef(17, 1)->status = 1;
				getSceneSpriteDef(24, 0)->status = 0;
				getSceneSpriteDef(24, 1)->status = 0;
				getSceneSpriteDef(17, 0)->status = 1;
				getSceneSpriteDef(18, 0)->status = 1;
				getSceneSpriteDef(19, 0)->status = 1;
				getSceneSpriteDef(20, 0)->status = 1;
				getSceneSpriteDef(22, 3)->status = 1;
				getSceneSpriteDef(23, 1)->status = 1;
				getSceneSpriteDef(24, 2)->status = 1;
				getSceneSpriteDef(25, 3)->status = 1;
				getSceneSpriteDef(27, 1)->status = 1;
				queueActor(0x1E7E, 0x1EA7, 0x1ECF);
				queueAnim(5, 61, 2, 0x1A2C, 0, 0);
			} else if (_flags1 & 4) {
				// Teleport into coffin
				queueActor(0x1EF7, 0x1F28, 0x1F56);
			}
			queueWalk(142, 143);
			queueAnim(2, 47, 2, 0x1F84, 0, 0);
			queueAnim(3, 51, 2, 0x1F90, 0, 0);
			queueAnim(4, 52, 2, 0x1F90, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		}
		break;

	case GIVE(idMoneyPouch, idBenn):
		if (_sprites[3].status == 3) {
			_sprites[3].status |= 4;
			getSceneSpriteDef(24, 1)->status |= 4;
			getSceneSpriteDef(24, 1)->templateId = 0x1F9E;
			queueWalk(_sprites[2].x + 16, _sprites[2].y + _sprites[2].height + 5);
			queueActor(0x96DE, 0x96E7, 0);
			queueAnim(2, 7, 2, 0x1FB0, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case GIVE(idCassette, idBoomBoxGuy):
		queueWalk(141 - _cameraStripX, 165);
		queueActor(0x96DE, 0x96E7, 0);
		queueAnim(8, 5, 40, 0x1A2F, 0, 0);
		actionThumbsUp();
		break;

	case GIVE(idCassetteMusic, idBoomBoxGuy):
		removeInventoryItem(idCassetteMusic);
		getSceneSpriteDef(32, 0)->status = 0;
		queueWalk(141 - _cameraStripX, 165);
		queueActor(0x96DE, 0x96E7, 0);
		if (_sprites[2].frameIndex == 4) {
			// Guy gives the keycard and walks off
			addInventoryItem(idKeyCard);
			getSceneItemInfo(idKeyCard).frameIndex = 144;
			queueAnim(2, 13, 2, 0xA22E, 0xA23B, 0xA240);
		} else {
			// Guy just walks off
			queueAnim(2, 13, 2, 0xA245, 0xA265, 0xA27F);
		}
		_theScore += 5;
		actionThumbsUp();
		break;

	case GIVE(idJumpingFish, idEskimo):
		if ((_flags1 & 0x80) && _sprites[2].frameIndex == 17) {
			queueWalk(_sprites[2].x + 23, 171);
			if (_sprites[2].y + _sprites[2].height - 8 - actorSprite().y - actorSprite().height >= 0) {
				if (actorSprite().x2() - _sprites[2].x < 0 ||
					actorSprite().x - _sprites[2].x - _sprites[2].width >= 0) {
					_walkInfos[1].x = _walkInfos[0].x;
					_walkInfos[1].y = _walkInfos[0].y;
					_walkInfos[0].next = &_walkInfos[1];
					_walkInfos[0].x = actorSprite().x;
				} else {
					_walkInfos[2].x = _walkInfos[0].x;
					_walkInfos[1].y = _walkInfos[0].y;
					_walkInfos[2].y = _walkInfos[0].y;
					_walkInfos[0].next = &_walkInfos[1];
					_walkInfos[1].next = &_walkInfos[2];
					_walkInfos[0].x = _sprites[2].x - 16;
					_walkInfos[1].x = _sprites[2].x - 16;
					_walkInfos[0].y = actorSprite().y + 47;
				}
			}
			removeInventoryItem(idJumpingFish);
			getSceneSpriteDef(41, 0)->status = 0;
			getSceneSpriteDef(41, 3)->status = 0;
			queueActor(0xA640, 0xA64B, 0);
			queueAnim(2, 5, 2, 0xA653, 0xA669, 0xA676);
			queueAnim(5, 5, 2, 0x1A2C, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	// Combine
	
	case COMBINE(idThread, idRings):
		removeInventoryItem(idThread);
		getSceneSpriteDef(5, 8)->status = 1;
		_sprites[10].anim.set(0x1A2C, 0, 0, 1);
		queueWalkToSprite(8, 0, 15);
		queueActor(0x9BD6, 0x9C0C, 0x9C2F);
		_sprites[10].anim.ticks = actionThumbsUp() + 36;
		_theScore += 5;
		break;
		
	case COMBINE(idBoard, idBoulder58):
		removeInventoryItem(idBoard);
		_collectItemIgnoreActorPosition = false;
		actorSprite().changeCodeSync(0x9F1D, 0x9F25, 0x9F42);
		getSceneSpriteDef(11, 0)->frameIndex = 6;
		getSceneSpriteDef(11, 0)->x = 124;
		getSceneSpriteDef(11, 0)->y = 121;
		getSpriteTemplate(getSceneSpriteDef(11, 0)->templateId)->heightAdd = 10;
		_sprites[2].setPos(124, 121);
		_sprites[2].heightAdd = 10;
		_sprites[2].anim.set(0x9F5F, 0, 58, 1);
		_theScore += 5;
		actionThumbsUp();
		break;
		
	case COMBINE(idString, idMagnet):
	case COMBINE(idMagnet, idString):
		if (_flgCanRunBoxClick && removeInventoryItem(idMagnet)) {
			removeInventoryItem(idString);
			addInventoryItem(idMagnetOnString);
			actorSprite().changeCodeSync(0x9FD6, 0x9FE3, kNone);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case COMBINE(idStamp, idLetter):
	case COMBINE(idLetter, idStamp):
		if (removeInventoryItem(idLetter)) {
			removeInventoryItem(idStamp);
			addInventoryItem(idLetterStamp);
			getSceneSpriteDef(35, 0)->templateId = 0xA33D;
			_sprites[2].id = 112;
			actorSprite().changeCodeSync(0x96DE, 0x96E7, kNone);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case COMBINE(idDirtySock, idGoldCoins):
	case COMBINE(idGoldCoins, idDirtySock):
		removeInventoryItem(idDirtySock);
		addInventoryItem(idSockWithCoins);
		queueWalk(552 - _cameraStripX, 163);
		queueActor(0xA2C0, 0xA2CF, 0);
		_theScore += 5;
		actionThumbsUp();
		break;

	case COMBINE(idMud, idBranch):
	case COMBINE(idBranch, idMud):
	case COMBINE(idMud, idSeaWeed):
	case COMBINE(idSeaWeed, idMud):
	case COMBINE(idMud, idBranchSeaWeed):
	case COMBINE(idBranchSeaWeed, idMud):
	case COMBINE(idBranch, idSeaWeed):
	case COMBINE(idSeaWeed, idBranch):
	case COMBINE(idBranch, idMudSeaWeed):
	case COMBINE(idMudSeaWeed, idBranch):
	case COMBINE(idSeaWeed, idMudBranch):
	case COMBINE(idMudBranch, idSeaWeed):
		if (_spriteResourceType == 14 || _flgCanRunBoxClick) {
			removeInventoryItem(item1);
			removeInventoryItem(item2);
			switch (MUDCOMBI(item1, item2)) {
			case MUDCOMBI(idMud, idBranch):
			case MUDCOMBI(idBranch, idMud):
				addInventoryItem(idMudBranch);
				break; 
			case MUDCOMBI(idMud, idSeaWeed):
			case MUDCOMBI(idSeaWeed, idMud):
				addInventoryItem(idMudSeaWeed);
				break; 
			case MUDCOMBI(idBranch, idSeaWeed):
			case MUDCOMBI(idSeaWeed, idBranch):
				addInventoryItem(idBranchSeaWeed);
				break; 
			case MUDCOMBI(idMud, idBranchSeaWeed):
			case MUDCOMBI(idBranchSeaWeed, idMud):
			case MUDCOMBI(idBranch, idMudSeaWeed):
			case MUDCOMBI(idMudSeaWeed, idBranch):
			case MUDCOMBI(idSeaWeed, idMudBranch):
			case MUDCOMBI(idMudBranch, idSeaWeed):
				addInventoryItem(idMudBranchSeaWeed);
				break; 
			}			
			if (_spriteResourceType == 255)
				actorSprite().changeCodeSync(0x9FD6, 0x9FE3, kNone);
			else
				actorSprite().anim.codeId = 0x9FED;
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case COMBINE(idWhistle, idMegaphone):
	case COMBINE(idMegaphone, idWhistle):
		if (removeInventoryItem(idWhistle)) {
			removeInventoryItem(idMegaphone);
			addInventoryItem(idWhistleMegaphone);
			actorSprite().changeCodeSync(0x96DE, 0x96E7, kNone);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case COMBINE(idMagnifier, idBeamHole1):
	case COMBINE(idMagnifier, idBeamHole2):
	case COMBINE(idMagnifier, idBeamHole3):
	case COMBINE(idMagnifier, idBeamHole4):
	case COMBINE(idIceCube, idBeamHole1):
	case COMBINE(idIceCube, idBeamHole2):
	case COMBINE(idIceCube, idBeamHole3):
	case COMBINE(idIceCube, idBeamHole4):
	case COMBINE(idIcicle, idBeamHole1):
	case COMBINE(idIcicle, idBeamHole2):
	case COMBINE(idIcicle, idBeamHole3):
	case COMBINE(idIcicle, idBeamHole4):
	case COMBINE(idGlass, idBeamHole1):
	case COMBINE(idGlass, idBeamHole2):
	case COMBINE(idGlass, idBeamHole3):
	case COMBINE(idGlass, idBeamHole4):
		{
			uint spriteIndex;
			byte frameIndex;
			switch (item1) {
			case idMagnifier:
				frameIndex = 30;
				break; 
			case idIceCube:
				frameIndex = 27;
				break; 
			case idIcicle:
				frameIndex = 34;
				break; 
			case idGlass:
				frameIndex = 14;
				break; 
			default:
				frameIndex = 0;
				break;
			}
			switch (item2) {
			case idBeamHole1:
				spriteIndex = 6;
				break;
			case idBeamHole2:
				spriteIndex = 7;
				break;
			case idBeamHole3:
				spriteIndex = 8;
				break;
			case idBeamHole4:
				spriteIndex = 9;
				break;
			default:
				spriteIndex = 0;
				break;
			}
			if (_sprites[spriteIndex].status != 1) {
				removeInventoryItem(item1);
				_sprites[spriteIndex].id = item1;
				_sprites[spriteIndex].frameIndex = frameIndex;
				_sprites[spriteIndex].status = 3;
				queueWalkToSprite(spriteIndex, 0, 7);
				queueActor(0x96CF, 0x96D8, 0);
				queueAnim(spriteIndex, 7, 1, 0xACBE, 0, 0);
				actionThumbsUp();
			} else
				actionThumbsDown();
		}
		break;

	case COMBINE(idRing, idEvilQueen):
		actorSprite().changeCodeSync(0x2BA8, 0x2BC0, 0x2BD2);
		_sprites[2].setCodeSync(0x2BE7, 0x2BF4, 0x2BFF, 17, 2);
		_theScore += 5;
		actionThumbsUp();
		break;

	// Take

	case TAKE(idKey):
		getSceneSpriteDef(0, 9)->status = 0;
		_sprites[11].anim.set(0x1A2E, 0, 3, 1);
		actorSprite().anim.codeId = 0x9723;
		actionThumbsUp();
		break;

	case TAKE(idPaperClip):
		getSceneSpriteDef(0, 7)->status = 0;
		_sprites[9].status = 0;
		queueWalk(200, 157);
		queueActor(0x972C, 0x9738, 0);
		actionThumbsUp();
		break;

	case TAKE(idCoin5):
		defaultTakeItem(0, 5, -2, 6);
		break;

	case TAKE(idFishBowl):
		addInventoryItem(idGoldfish);
		getSceneSpriteDef(1, 4)->status = 0;
		_sprites[6].moveX.set(0x97A2, _sprites[6].moveY.index & 3,
			_sprites[6].anim.ticks, _sprites[6].anim.initialTicks);
		getSceneSpriteDef(1, 0)->status = 0;
		getSceneSpriteDef(1, 1)->status = 0;
		_sprites[2].anim.codeId = 0x1A2E;
		_sprites[2].anim.index = 0;
		_sprites[2].anim.ticks = 6;
		_sprites[3].anim.codeId = 0x1A2E;
		_sprites[3].anim.index = 0;
		_sprites[3].anim.ticks = 6;
		queueWalk(_sprites[2].x + 11, 171);
		queueActor(0x96DE, 0x96E7, 0);
		actionThumbsUp();
		break;
		
	case TAKE(idCoin9):
		getSceneSpriteDef(1, 6)->status = 0;
		_sprites[8].status = 0;
		queueWalkToSprite(8, 0, 44);
		queueActor(0x96DE, 0x96E7, 0);
		actionThumbsUp();
		break;
		
	case TAKE(idJewel10):
		defaultTakeItem(0, 5, -2, 9);
		break;
		
	case TAKE(idJewel11):
		defaultTakeItem(0, 5, -2, 10);
		break;
		
	case TAKE(idJewel12):
		defaultTakeItem(0, 5, -7, 11);
		break;
		
	case TAKE(idJewel13):
		getSceneSpriteDef(_sceneIndex, 12 - 2)->status = 0;
		_sprites[12].status = 0;
		queueWalk(_sprites[12].x + 5, _sprites[12].y + _sprites[12].height + 12);
		queueActor(0x96CF, 0x96D8, 0);
		actionThumbsUp();
		break;
		
	case TAKE(idJewel14):
		defaultTakeItem(0, 5, -7, 13);
		break;
		
	case TAKE(idWorm):
		defaultTakeItem(0, 5, -2, 5);
		break;
		
	case TAKE(idShell):
		if (_sprites[6].anim.codeId == 0 && _sprites[6].moveX.codeId == 0 && _sprites[6].moveY.codeId == 0)
			defaultTakeItem(0, 5, -2, 6);
		else {
			removeInventoryItem(idShell);
			actionThumbsDown();
		}
		break;
		
	case TAKE(idCattleProd):
		defaultTakeItem(0, 5, -7, 14);
		break;
		
	case TAKE(idCoin26):
		defaultTakeItem(0, 5, -2, 11);
		break;
		
	case TAKE(idRock27):
		defaultTakeItem(0, 5, -2, 2);
		break;
		
	case TAKE(idRock28):
		defaultTakeItem(0, 5, -2, 3);
		break;
		
	case TAKE(idRock29):
		defaultTakeItem(0, 5, -2, 4);
		break;
		
	case TAKE(idRock30):
		defaultTakeItem(0, 5, -2, 5);
		break;
		
	case TAKE(idRock31):
		defaultTakeItem(0, 5, -2, 2);
		break;
		
	case TAKE(idRock32):
		defaultTakeItem(0, 5, -2, 3);
		break;
		
	case TAKE(idRock33):
		defaultTakeItem(0, 5, -2, 4);
		break;
		
	case TAKE(idRock34):
		defaultTakeItem(0, 5, -2, 5);
		break;
		
	case TAKE(idRock35):
		defaultTakeItem(0, 5, -2, 6);
		break;
		
	case TAKE(idMud):
		if (hasInventoryItem(idBranch) || hasInventoryItem(idBranchSeaWeed)) {
			// Enable the wishing well
			getSceneSpriteDef(9, 0)->status = 0;
			getSceneSpriteDef(9, 1)->status = 3;
			getSceneSpriteDef(9, 2)->status = 3;
			getSceneSpriteDef(9, 3)->status = 3;
			getSceneSpriteDef(9, 4)->status = 3;
		}
		defaultTakeItem(0, 5, -2, 12);
		break;
		
	case TAKE(idRock38):
		defaultTakeItem(0, 5, -2, 2);
		break;
		
	case TAKE(idRock39):
		defaultTakeItem(0, 5, -2, 3);
		break;
		
	case TAKE(idRock40):
		defaultTakeItem(0, 5, -2, 4);
		break;
		
	case TAKE(idRock41):
		defaultTakeItem(0, 5, -2, 5);
		break;
		
	case TAKE(idRock42):
		defaultTakeItem(0, 5, -2, 6);
		break;
		
	case TAKE(idRock43):
		defaultTakeItem(0, 5, -2, 2);
		break;
		
	case TAKE(idRock44):
		defaultTakeItem(0, 5, -2, 3);
		break;
		
	case TAKE(idRock45):
		defaultTakeItem(0, 5, -2, 4);
		break;
		
	case TAKE(idRock46):
		defaultTakeItem(0, 5, -2, 5);
		break;
		
	case TAKE(idWishCoin):
		defaultTakeItem(0, 5, -2, 6);
		break;
		
	case TAKE(idMoneyPouch51):
		insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 4, 1);
		_sprites[4].setCodeSync(0x97A7, 0x97AE, 0x97B0, 1, 1);
		actorSprite().setCodeSync(0x973D, 0x974B, kNone, 9, 1);
		actionThumbsUp();
		break;

	case TAKE(idWoman):
		insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 5, 1);
		insertSpriteList(throwCoin47SpriteList1, 7, 0x9E3E, 1, 1);
		_sprites[5].status = 0;
		actorSprite().anim.codeId = 0x974E;
		actionThumbsUp();
		break;

	case TAKE(idHelmet):
		if (_sprites[6].frameIndex == 17) {
			// Chose the helmet as wish
			_collectItemIgnoreActorPosition = false;
			removeInventoryItem(idHelmet);
			insertSpriteList(throwCoin47SpriteList1, 7, 0x9E3E, 1, 1);
			_sprites[3].status = 0;
			if (_sprites[4].status) {
				_sprites[4].status = 0;
				insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 4, 1);
			}
			if (_sprites[5].status) {
				_sprites[5].status = 0;
				insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 5, 1);
			}
			insertSpriteList(throwCoin47SpriteList2, 8, 0x9E3E, 6, 1);
			_sprites[6].setCodeSync(0x97BB, 0x97C0, 0x97C2, 1, 1);
			getSceneSpriteDef(9, 4)->status = 1;
			getSceneSpriteDef(9, 4)->frameIndex = 21;
			getSceneSpriteDef(9, 4)->x = 188;
			getSceneSpriteDef(9, 4)->y = 151;
			actionThumbsUp();
		} else {
			// Helmet is on the ground
			getSceneSpriteDef(9, 4)->status = 0;
			defaultTakeItem(0, 5, -2, 6);
		}
		break;

	case TAKE(idSeaWeed):
		_sprites[3].id = idButton48;
		defaultTakeItem(1, 5, -9, 4);
		break;
		
	case TAKE(idBoard):
		getSceneItemInfo(idBoard).flags &= ~2;
		_collectItemIgnoreActorPosition = true;
		_sprites[2].anim.set(0x1A34, 0, 0, 1);
		queueWalkToSprite(2, 5, 2);
		queueActor(0x975A, 0x9765, 0x976B);
		actionThumbsUp();
		_sprites[2].anim.ticks = _walkDistance + 6;
		break;
		
	case TAKE(idMagnet):
		getSceneSpriteDef(11, 1)->status = 0;
		_sprites[3].anim.set(0x1A2E, 0, 6, 1);
		actorSprite().changeCodeSync(0x96ED, 0x96C9, kNone);
		actionThumbsUp();
		break;

	case TAKE(idBranch):
		if (hasInventoryItem(idMud) || hasInventoryItem(idMudSeaWeed)) {
			// Enable the wishing well
			getSceneSpriteDef(9, 0)->status = 0;
			getSceneSpriteDef(9, 1)->status = 3;
			getSceneSpriteDef(9, 2)->status = 3;
			getSceneSpriteDef(9, 3)->status = 3;
			getSceneSpriteDef(9, 4)->status = 3;
		}
		getSceneSpriteDef(14, 4)->status = 3;
		_sprites[6].status = 0;
		queueWalkToSprite(10, 4, 42);
		queueActor(0x1AE1, 0x1AF3, 0);
		actionThumbsUp();
		_flgWalkBoxEnabled = true;
		break;
		
	case TAKE(idComputer):
		getSceneItemInfo(idComputer).flags &= ~2;
		defaultTakeItem(0, 5, -4, 5);
		break;

	case TAKE(idKnife):
		_sprites[2].heightAdd = -25;
		_sprites[3].heightAdd = 0;
		_sprites[3].anim.set(0x1A2E, 0, 0, 1);
		queueWalk(_sprites[3].x + 4, _sprites[3].y + _sprites[3].height);
		queueActor(0x9771, 0x977F, 0);
		_sprites[3].anim.ticks = actionThumbsUp() + 6;
		break;
		
	case TAKE(idMoneyPouch):
		getSceneSpriteDef(21, 1)->status = 0;
		defaultTakeItem(1, 5, 1, _sceneIndex == kSceneTown21 ? 3 : 4);
		break;
		
	case TAKE(idDress):
		_sprites[3].status = 0;
		getSceneSpriteDef(24, 0)->status = 0;
		getSceneSpriteDef(24, 1)->status = 0;
		_sprites[2].anim.set(0x97D7, 0, 0, 2);
		queueWalk(128, 151);
		queueActor(0x96DE, 0x96E7, 0);
		_sprites[2].anim.ticks = actionThumbsUp() + 6;
		break;
		
	case TAKE(idGloves):
		defaultTakeItem(1, 4, 1, 3);
		break;

	case TAKE(idItem87):
		_sprites[4].id = -1;
		defaultTakeItem(1, 5, 2, 4);
		break;
		
	case TAKE(idRope):
		defaultTakeItem(0, 5, -2, 16);
		break;

	case TAKE(idDirtySock):
		defaultTakeItem(0, 15, -4, 11);
		break;

	case TAKE(idPen):
		defaultTakeItem(0, 35, -4, 10);
		break;
		
	case TAKE(idCassette):
		defaultTakeItem(1, 3, 1, 7);
		break;
		
	case TAKE(idRemote):
		if (_sprites[2].status == 0)
			defaultTakeItem(0, 45, -22, 3);
		else {
			removeInventoryItem(idRemote);
			actionThumbsDown();
		}
		break;
		
	case TAKE(idStamp):
		defaultTakeItem(0, 45, -4, 9);
		break;
		
	case TAKE(idKeyCard):
		getSceneItemInfo(idKeyCard).frameIndex &= 144;//CHECKME
		defaultTakeItem(1, 5, 1, 7);
		break;

	case TAKE(idHair):
		defaultTakeItem(0, 45, -12, 9);
		break;
		
	case TAKE(idLetter):
	case TAKE(idLetterStamp):
		defaultTakeItem(1, 7, 1, 2);
		break;
		
	case TAKE(idTray):
		defaultTakeItem(0, 5, -2, 4);
		break;
		
	case TAKE(idPlank117):
	case TAKE(idPlank118):
		// Only one of these planks can be taken
		if ((item2 == idPlank117 && hasInventoryItem(idPlank118)) ||
			(item2 == idPlank118 && hasInventoryItem(idPlank117))) {
			removeInventoryItem(item2);
			actionThumbsDown();
		} else {
			sprite = &_sprites[item2 == idPlank117 ? 2 : 3];
			queueWalk(sprite->x + 35, sprite->y + 2);
			queueActor(0x97EE, 0x97FA, 0x9801);
			sprite->anim.set(0x1A34, 0, 0, 2);
			sprite->anim.ticks = actionThumbsUp() + 8;
		}
		break;
		
	case TAKE(idGoldFoil):
		defaultTakeItem(0, 5, -2, 4);
		break;
		
	case TAKE(idItem120):
		defaultTakeItem(1, 5, 1, 15);
		break;
		
	case TAKE(idMarblesBag):
		getSceneSpriteDef(_sceneIndex, 2 - 2)->status = 0;
		_sprites[2].anim.set(0x1A2E, 0, 0, 1);
		actorSprite().changeCodeSync(0x9808, 0x9814, kNone);
		actionThumbsUp();
		_sprites[2].anim.ticks = _walkDistance + 6;
		break;
		
	case TAKE(idFan):
		defaultTakeItem(1, 5, 1, 9);
		break;
		
	case TAKE(idWoodenPlank):
		getSceneSpriteDef(40, 1)->status = 0;
		queueWalk(533 - _cameraStripX, 146);
		queueActor(0x9789, 0x9796, 0x979C);
		queueAnim(3, 7, 2, 0x1A2E, 0, 0);
		queueAnim(4, 9, 2, 0x97E2, 0x97E6, 0x97EA);
		actionThumbsUp();
		break;
		
	case TAKE(idJumpingFish):
		if (_sprites[3].anim.codeId == 0)
			defaultTakeItem(0, 5, -2, 3);
		else {
			removeInventoryItem(idJumpingFish);
			actionThumbsDown();
		}
		break;
		
	case TAKE(idFishingRod):
		defaultTakeItem(1, 15, -7, 5);
		break;
		
	case TAKE(idSprayCan):
		defaultTakeItem(0, 5, -2, 4);
		break;
		
	case TAKE(idItem131):
		if (hasInventoryItem(idItem131) && getSceneSpriteDef(42, 0)->status == 0 &&
			getSceneSpriteDef(43, 0)->frameIndex != 19)
			getSceneSpriteDef(40, 0)->status = 1;
		defaultTakeItem(0, 5, -2, 7);
		break;
		
	case TAKE(idSnowball):
		defaultTakeItem(0, 5, -2, 3);
		break;
		
	case TAKE(idDice):
		queueAnim(8, 9, 1, 0x9819, 0, 0x981C);
		defaultTakeItem(2, 5, 25, 2);
		break;
		
	case TAKE(idBroom):
		defaultTakeItem(1, 9, 3, 7);
		break;
		
	case TAKE(idSunTanOil):
		if (hasInventoryItem(idIceCube) && hasInventoryItem(idIcicle) &&
			hasInventoryItem(idMegaphone) && hasInventoryItem(idCarJack) &&
			hasInventoryItem(idMagnifier) && hasInventoryItem(idSunTanOil) &&
			hasInventoryItem(idGlass)) {
			removeInventoryItem(idBroom);
			getSceneSpriteDef(49, 2)->status = 1;
		}
		defaultTakeItem(2, 5, 33, 5);
		break;
		
	case TAKE(idWhistle):
		defaultTakeItem(0, 5, -4, 4);
		break;
		
	case TAKE(idCarJack):
		if (hasInventoryItem(idIceCube) && hasInventoryItem(idIcicle) &&
			hasInventoryItem(idMegaphone) && hasInventoryItem(idCarJack) &&
			hasInventoryItem(idMagnifier) && hasInventoryItem(idSunTanOil) &&
			hasInventoryItem(idGlass)) {
			removeInventoryItem(idBroom);
			getSceneSpriteDef(49, 2)->status = 1;
		}
		defaultTakeItem(0, 5, -16, 2);
		break;
		
	case TAKE(idGun):
		defaultTakeItem(0, 5, -4, 2);
		break;
		
	case TAKE(idMegaphone):
		if (hasInventoryItem(idIceCube) && hasInventoryItem(idIcicle) &&
			hasInventoryItem(idMegaphone) && hasInventoryItem(idCarJack) &&
			hasInventoryItem(idMagnifier) && hasInventoryItem(idSunTanOil) &&
			hasInventoryItem(idGlass)) {
			removeInventoryItem(idBroom);
			getSceneSpriteDef(49, 2)->status = 1;
		}
		defaultTakeItem(0, 5, -4, 2);
		break;
		
	case TAKE(idMagnifier):
		if (_sceneIndex == kSceneIceCastleBeam) {
			if (_sprites[6].id == idMagnifier)
				defaultTakeItem(1, 5, 1, 6);
			else if (_sprites[7].id == idMagnifier)
				defaultTakeItem(1, 5, 1, 7);
			else if (_sprites[8].id == idMagnifier)
				defaultTakeItem(1, 5, 1, 8);
			else if (_sprites[9].id == idMagnifier)
				defaultTakeItem(1, 5, 1, 9);
		} else {
			if (hasInventoryItem(idIceCube) && hasInventoryItem(idIcicle) &&
				hasInventoryItem(idMegaphone) && hasInventoryItem(idCarJack) &&
				hasInventoryItem(idMagnifier) && hasInventoryItem(idSunTanOil) &&
				hasInventoryItem(idGlass)) {
				removeInventoryItem(idBroom);
				getSceneSpriteDef(49, 2)->status = 1;
			}
			defaultTakeItem(0, 5, -4, 2);
		}
		break;
		
	case TAKE(idIceCube):
		if (_sceneIndex == kSceneIceCastleBeam) {
			if (_sprites[6].id == idIceCube)
				defaultTakeItem(1, 5, 1, 6);
			else if (_sprites[7].id == idIceCube)
				defaultTakeItem(1, 5, 1, 7);
			else if (_sprites[8].id == idIceCube)
				defaultTakeItem(1, 5, 1, 8);
			else if (_sprites[9].id == idIceCube)
				defaultTakeItem(1, 5, 1, 9);
		} else {
			if (hasInventoryItem(idIceCube) && hasInventoryItem(idIcicle) &&
				hasInventoryItem(idMegaphone) && hasInventoryItem(idCarJack) &&
				hasInventoryItem(idMagnifier) && hasInventoryItem(idSunTanOil) &&
				hasInventoryItem(idGlass)) {
				removeInventoryItem(idBroom);
				getSceneSpriteDef(49, 2)->status = 1;
			}
			defaultTakeItem(0, 5, -5, 2);
		}
		break;
		
	case TAKE(idGlass):
		if (_sceneIndex == kSceneIceCastleBeam) {
			if (_sprites[6].id == idGlass)
				defaultTakeItem(1, 5, 1, 6);
			else if (_sprites[7].id == idGlass)
				defaultTakeItem(1, 5, 1, 7);
			else if (_sprites[8].id == idGlass)
				defaultTakeItem(1, 5, 1, 8);
			else if (_sprites[9].id == idGlass)
				defaultTakeItem(1, 5, 1, 9);
		} else {
			if (hasInventoryItem(idIceCube) && hasInventoryItem(idIcicle) &&
				hasInventoryItem(idMegaphone) && hasInventoryItem(idCarJack) &&
				hasInventoryItem(idMagnifier) && hasInventoryItem(idSunTanOil) &&
				hasInventoryItem(idGlass)) {
				removeInventoryItem(idBroom);
				getSceneSpriteDef(49, 2)->status = 1;
			}
			defaultTakeItem(2, 5, 33, 4);
		}
		break;
		
	case TAKE(idIcicle):
		if (_sceneIndex == kSceneIceCastleBeam) {
			if (_sprites[6].id == idIcicle)
				defaultTakeItem(1, 5, 1, 6);
			else if (_sprites[7].id == idIcicle)
				defaultTakeItem(1, 5, 1, 7);
			else if (_sprites[8].id == idIcicle)
				defaultTakeItem(1, 5, 1, 8);
			else if (_sprites[9].id == idIcicle)
				defaultTakeItem(1, 5, 1, 9);
		} else {
			if (hasInventoryItem(idIceCube) && hasInventoryItem(idIcicle) &&
				hasInventoryItem(idMegaphone) && hasInventoryItem(idCarJack) &&
				hasInventoryItem(idMagnifier) && hasInventoryItem(idSunTanOil) &&
				hasInventoryItem(idGlass)) {
				removeInventoryItem(idBroom);
				getSceneSpriteDef(49, 2)->status = 1;
			}
			defaultTakeItem(0, 5, -2, 2);
		}
		break;
		
	case TAKE(idMatchBox):
		addInventoryItem(162);
		defaultTakeItem(0, 5, -2, 2);
		break;
		
	case TAKE(idFireExtinguisher):
		_flags1 |= 4;
		getSceneSpriteDef(24, 0)->status = 3;
		queueWalk(_sprites[5].x + 2, _sprites[5].y2());
		queueActor(0x985F, 0x9872, 0x9878);
		queueAnim(5, 7, 1, 0x1A2E, 0, 0);
		queueAnim(9, 17, 2, 0x987C, 0, 0);
		actionThumbsUp();
		break;
		
	case TAKE(idBone):
		_sprites[3].status = 0;
		actorSprite().anim.codeId = 0x9858;
		actionThumbsUp();
		break;
		
	case TAKE(idPlate):
		defaultTakeItem(0, 5, -2, 12);
		break;
		
	case TAKE(idShovel):
		defaultTakeItem(0, 5, -2, 13);
		break;
		
	case TAKE(idCymbals):
		defaultTakeItem(0, 5, -2, 14);
		break;
		
	case TAKE(idGarlic):
		defaultTakeItem(0, 5, -2, 11);
		break;
		
	case TAKE(idVacuumCleaner):
		defaultTakeItem(0, 5, -2, 9);
		break;
		
	case TAKE(idCross):
		defaultTakeItem(0, 5, -2, 10);
		break;
		
	case TAKE(idRing):
		// Enable the book in scene 71
		getSceneItemInfo(idBook).flags |= 1;
		defaultTakeItem(1, 5, 1, 2);
		break;

	// Talk
	
	case TALK(idTalkHelp):
		talkCommandHelp();
		break;
		
	case TALK(idTalkHi):
		talkCommandHi();
		break;

	case TALK(idTalkOpenSesame):
		talkCommandOpenSesame();
		break;

	// Look

	case LOOK(idKeyHole8):
		queueWalk(_sprites[4].x, 160);
		queueActor(0x1AA1, 0x1AAC, 0);
		queueAnim(4, 9, 2, 0x1BE0, 0x1BE9, 0x1BF1);
		queueAnim(5, 19, 2, 0x1BD1, 0, 0);
		actionThumbsUp();
		break;
		
	case LOOK(idMud16):
		queueWalkToSprite(4, 4, 0);
		queueActor(0x1A84, 0x1A96, 0);
		queueAnim(5, 7, 2, 0x1BFA, 0, 0);
		actionThumbsUp();
		break;
		
	case LOOK(idHole62):
		queueWalkToSprite(3, 19, 42);
		queueActor(0xA018, 0xA02A, 0);
		actionThumbsUp();
		_flgWalkBoxEnabled = true;
		if (_sprites[4].status) {
			_sprites[2].setPos(actorSprite().x, actorSprite().y - 30);
			_sprites[2].anim.set(0x1DB0, 0, 1, 10);
			_sprites[4].status |= 4;
		}
		break;

	case LOOK(idHole65):
		_sprites[2].setPos(actorSprite().x, actorSprite().y - 30);
		_sprites[2].anim.set(0x1DA8, 0, 1, 10);
		queueWalkToSprite(7, 19, 42);
		queueActor(0x1ACF, 0x1AF3, 0);
		if (_sprites[4].status & 2) {
			_sprites[4].changeCodeSync(0x1CF6, 0x1D67, 0x1CB5);
		}
		actionThumbsUp();
		break;

	case LOOK(idHole66):
		_sprites[2].setPos(actorSprite().x, actorSprite().y - 30);
		_sprites[2].anim.set(0x1DA8, 0, 1, 10);
		queueWalkToSprite(8, 19, 42);
		queueActor(0xA018, 0xA02A, 0);
		actionThumbsUp();
		_flgWalkBoxEnabled = true;
		if (_sprites[3].status & 2) {
			_sprites[3].changeCodeSync(0x1C03, 0x1C74, 0x1CB5);
		}
		break;

	case LOOK(idHole67):
		_sprites[2].setPos(actorSprite().x, actorSprite().y - 30);
		_sprites[2].anim.set(0x1DA8, 0, 1, 10);
		queueWalkToSprite(9, 19, 42);
		queueActor(0xA018, 0xA02A, 0);
		actionThumbsUp();
		_flgWalkBoxEnabled = true;
		if (_sprites[5].status & 2) {
			_sprites[5].changeCodeSync(0x1CF6, 0x1D67, 0x1CB5);
		}
		break;

	case LOOK(idHole68):
		_sprites[2].setPos(actorSprite().x, actorSprite().y - 30);
		_sprites[2].anim.set(0, 0, 1, 10);
		queueWalkToSprite(10, 4, 42);
		queueActor(0x1AE1, 0x1AF3, 0);
		actionThumbsUp();
		_flgWalkBoxEnabled = true;
		if ((_sprites[3].status & 2) || (_sprites[4].status & 2) ||
			(_sprites[5].status & 2) || !(_sprites[6].status & 1)) {
			_sprites[2].anim.codeId = 0x1DA8;
		} else {
			_sprites[2].anim.codeId = 0x1DAC;
			getSceneItemInfo(idBranch).flags |= 1;
			_sprites[6].status |= 4;
		}
		break;

	case LOOK(idWriting):
		if (_sprites[24].status != 1)
			_talkTable[2] = idTalkOpenSesame;
		queueWalk(2080 - _cameraStripX, 113);
		queueActor(0x270D, 0, 0);
		walkTicks = actionThumbsUp();
		_sprites[17].anim.set(0x1A2F, 0, walkTicks + 4, 30);
		break;

	case LOOK(idBowl):
		if (_sprites[2].frameIndex == 9) {
			queueWalk(231, 182);
			queueActor(0xAB9F, 0xABAA, 0xABB0);
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case LOOK(idIceWindow):
		queueWalk(159, 164);
		queueActor(0xABC4, 0, 0);
		queueAnim(3, 3, 2, 0xABCA, 0, 0);
		actionThumbsUp();
		break;

	// Fight
	
	case FIGHT(idCattleProd):
		if (_sprites[15].status == 1) {
			_walkDistance = 0;
			_currWalkInfo = NULL;
			actorSprite().changeCodeSync(0x1AB2, 0x1AC3, 0);
			actionThumbsUp();
			_flgWalkBoxEnabled = true;
		} else
			actionThumbsDown();
		break;
		
	case FIGHT(idKnife):
		spriteDef = getSpriteDef(0x1DC0);
		spriteTemplate = getSpriteTemplate(spriteDef->templateId);
		getSceneSpriteDef(16, 0)->status = 0;
		getSceneSpriteDef(16, 1)->status = 0;
		removeInventoryItem(idKnife);
		_sprites[4].setPos(_sprites[2].x + 18, _sprites[2].y + 4);
		_sprites[4].status &= ~2;
		queueWalkToSprite(2, 74, 25);
		queueActor(0x1AFE, 0x1B13, 0x1B21);
		_sprites[2].setCodeSync(0x1DB4, 0x1DB8, 0x1DBC, 0, 2);
		walkTicks = actionThumbsUp() + 18;
		_sprites[2].anim.ticks = walkTicks;
		_sprites[2].moveX.ticks = walkTicks;
		_sprites[2].moveY.ticks = walkTicks;
		// Insert sparkles
		for (uint i = 0; i < 40; i++) {
			spriteDef->setPos(getRandom(30) + 200, getRandom(10) + 160);
			spriteTemplate->moveYListTicks = spriteTemplate->animListTicks = getRandom(10) + walkTicks;
		}
		break;

	case FIGHT(idSockWithCoins):
		if (_sceneIndex == kSceneShipWreck && ABS(actorSprite().x - _sprites[2].x) <= 40) {
			removeInventoryItem(idSockWithCoins);
			getSceneSpriteDef(33, 0)->frameIndex = 19;
			getSceneSpriteDef(33, 0)->x += -2;
			getSceneSpriteDef(33, 0)->y += 21;
			getSceneSpriteDef(33, 0)->templateId = 0xA314;
			getSceneSpriteDef(33, 1)->status = 1;
			getSceneSpriteDef(33, 1)->y += 24;
			queueWalk(360 - _cameraStripX, 154);
			queueActor(0xA299, 0xA2A8, 0xA2B4);
			_sprites[2].id = -1;
			queueAnim(2, 13, 1, 0xA2DB, 0xA2E6, 0xA2EF);
			queueAnim(3, 33, 1, 0xA2F8, 0, 0xA2FE);
			queueAnim(4, 33, 1, 0xA306, 0, 0xA309);
			queueAnim(5, 33, 1, 0xA30C, 0, 0xA30F);
			queueAnim(6, 33, 1, 0xA312, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case THROW(idSnowball):
	case FIGHT(idSnowball):
		if (_sceneIndex == kSceneIceDesert43) {
			if (hasInventoryItem(idItem131) && getSceneSpriteDef(42, 0)->status == 0 &&
				getSceneSpriteDef(43, 0)->frameIndex != 19)
				getSceneSpriteDef(40, 0)->status = 1;
			removeInventoryItem(idSnowball);
			_scrollBorderRight = 128;
			queueWalk(581 - _cameraStripX, 147);
			if (actorSprite().x - _sprites[2].x > 0) {
				if (actorSprite().y - _sprites[2].y < 0) {
					_walkInfos[0].next = &_walkInfos[1];
					_walkInfos[1].x = _walkInfos[0].x;
					_walkInfos[1].y = _walkInfos[0].y;
					_walkInfos[0].x = _sprites[2].x + _sprites[2].width;
					_walkInfos[0].y = _sprites[2].y + _sprites[2].height - 10;
				} else {
					_walkInfos[0].next = &_walkInfos[1];
					_walkInfos[1].x = _walkInfos[0].x;
					_walkInfos[1].y = _walkInfos[0].y;
					_walkInfos[0].x = _sprites[2].x + _sprites[2].width;
					_walkInfos[0].y = _sprites[2].y + _sprites[2].height + 4;
				}
			}
			queueActor(0xA795, 0xA7AE, 0xA7C3);
			_sprites[2].heightAdd = 2;
			_sprites[2].frameIndex = 32;
			getSceneSpriteDef(43, 0)->x -= 215;
			getSceneSpriteDef(43, 0)->y += 43;
			_sprites[3].x = 616 - _cameraStripX;
			_sprites[3].y = 108;
			_sprites[3].status = 3;
			_sprites[3].heightAdd = 200;
			_sprites[4].x = 501 - _cameraStripX;
			_sprites[4].y = 140;
			_sprites[4].status = 3;
			_sprites[4].frameIndex = 13;
			queueAnim(2, 19, 2, 0xA7ED, 0xA81A, 0xA838);
			queueAnim(3, 11, 1, 0xA7D8, 0xA7E1, 0xA7E7);
			queueAnim(4, 71, 1, 0xA856, 0xA86C, 0xA880);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idFishingRod):
		if (_sceneIndex == kSceneIceDesert42) {
			removeInventoryItem(idFishingRod);
			queueWalk(550 - _cameraStripX, 166);
			queueActor(0xA695, 0xA6B3, 0);
			if (actorSprite().x - _sprites[5].x - _sprites[5].width < 0 &&
				actorSprite().y + actorSprite().height - _sprites[5].y - _sprites[5].height < 0) {
				_walkInfos[0].next = &_walkInfos[1];
				_walkInfos[1].x = _walkInfos[0].x;
				_walkInfos[1].y = _walkInfos[0].y;
				if (actorSprite().x + actorSprite().width - _sprites[5].x < 0)
					_walkInfos[0].x -= 40;
				else
					_walkInfos[0].y -= 15;
			}
			getSceneSpriteDef(42, 3)->status = 0;
			getSceneSpriteDef(42, 4)->status = 1;
			getSceneSpriteDef(42, 5)->status = 1;
			getSceneSpriteDef(42, 5)->frameIndex = 25;
			getSceneSpriteDef(42, 5)->x += 8;
			getSceneSpriteDef(42, 5)->y -= 3;
			queueAnim(5, 51, 8, 0xA6CB, 0xA6EE, 0xA703);
			queueAnim(7, 27, 2, 0xA718, 0xA77F, 0xA783);
			queueAnim(6, 75, 1, 0x1A2C, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idBroom):
		if (_sceneIndex == kSceneIceCastleGun && _sprites[2].status == 1) {
			getSceneSpriteDef(52, 0)->y += 62;
			queueWalk(141, 151);
			queueActor(0xAC1C, 0xAC2A, 0xAC31);
			queueAnim(3, 3, 2, 0xAC38, 0, 0xAC3C);
			queueAnim(2, 9, 1, 0xAC49, 0, 0xAC50);
			_theScore += 5;
			actionThumbsUp();
		} else if (_sceneIndex == kSceneIceCastleIcicle && _sprites[2].status == 1 && _sprites[2].y == 89) {
			getSceneSpriteDef(55, 0)->y += 62;
			queueWalk(157, 159);
			queueActor(0xAC1C, 0xAC2A, 0xAC31);
			queueAnim(3, 3, 2, 0xAC38, 0, 0xAC3C);
			queueAnim(2, 9, 1, 0xAC41, 0, 0xAC50);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case FIGHT(idWhistleMegaphone):
		if (_sprites[2].status == 1) {
			removeInventoryItem(idWhistleMegaphone);
			getSceneSpriteDef(49, 0)->status = 0;
			queueWalk(142, 165);
			queueActor(0xABD6, 0xABE1, 0);
			queueAnim(5, 5, 40, 0x1A32, 0, 0);
			queueAnim(2, 17, 2, 0xABE9, 0, 0);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idBone):
		removeInventoryItem(idBone);
		actorSprite().anim.codeId = 0x2714;
		_sprites[2].setCodeSync(0x2742, 0x2752, 0x2758, 9, 8);
		_theScore += 5;
		actionThumbsUp();
		break;

	case FIGHT(idShovel):
		if (((_flags2 >> 3) & 7) == 1) {
			spriteDef = getSpriteDef(0x275E);
			_flags2 ^= 0x18;
			removeInventoryItem(idShovel);
			queueWalk(312 - _cameraStripX, 178);
			queueActor(0x27B9, 0x27CB, 0x27D8);
			queueAnim(2, 1, 2, 0x277D, 0x2792, 0x279B);
			queueAnim(6, 13, 2, 0x27A4, 0x27AB, 0x27B2);
			for (uint i = 0; i < 4; i++) {
				spriteDef->setPos(fightShovelSpriteList[i].x - _cameraStripX, fightShovelSpriteList[i].y);
				sprite = insertSprite(spriteDef);
				if (sprite) {
					queueAnim(0, 47, 1, 0x2779, fightShovelSpriteList[i].moveXListId,
						fightShovelSpriteList[i].moveYListId);
					_animQueue[_animQueueCount - 1].sprite = sprite;
				}
			}
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idCross):
		if (((_flags2 >> 3) & 7) == 4) {
			spriteDef = getSpriteDef(0x275E);
			_flags2 ^= 0x08;
			removeInventoryItem(idCross);
			queueWalk(167 - _cameraStripX, 165);
			queueActor(0x29CD, 0x29D8, 0x29DE);
			queueAnim(2, 5, 2, 0x2996, 0x299F, 0x29A1);
			queueAnim(5, 3, 18, 0x1A32, 0, 0);
			for (uint i = 0; i < 7; i++) {
				spriteDef->setPos(fightCrossSpriteList[i].x - _cameraStripX, fightCrossSpriteList[i].y);
				sprite = insertSprite(spriteDef);
				if (sprite) {
					queueAnim(0, 19, 1, 0x2779, fightCrossSpriteList[i].moveXListId,
						fightCrossSpriteList[i].moveYListId);
					_animQueue[_animQueueCount - 1].sprite = sprite;
				}
			}
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idCymbals):
		if (((_flags2 >> 3) & 7) == 5) {
			_flags2 ^= 0x18;
			removeInventoryItem(idCymbals);
			queueWalk(312 - _cameraStripX, 143);
			queueActor(0x2A1C, 0x2A2B, 0x2A34);
			queueAnim(2, 9, 2, 0x2A3D, 0x2A4D, 0x2A59);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idVacuumCleaner):
		if (_sceneIndex == kSceneEvilQueen && ((_flags2 >> 2) & 3) == 0) {
			actorSprite().changeCodeSync(0x2B61, 0x2B65, kNone);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case FIGHT(idFan):
		if (_sceneIndex == kSceneEvilQueen && ((_flags2 >> 2) & 3) == 2) {
			actorSprite().changeCodeSync(0x2B67, 0x2B6C, kNone);
			_sprites[9].anim.set(0x2B6F, 0, 3, 1);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	// Jump

	case JUMP(idClam):
		if (_sprites[17].frameIndex == 12) {
			_sprites[17].anim.ticks = 20;
			_sprites[17].moveX.ticks = 20;
			_sprites[17].moveY.ticks = 20;
			queueWalkToSprite(17, 61, 21);
			queueActor(0x225B, 0x226C, 0x2276);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case JUMP(idDown59):
		if (hasInventoryItem(idMagnet) || hasInventoryItem(idMagnetOnString)) {
			actorSprite().changeCodeSync(0x2280, 0x228D, 0x2293);
			getSceneSpriteDef(11, 0)->frameIndex = 7;
			getSpriteTemplate(getSceneSpriteDef(11, 0)->templateId)->heightAdd = -12;
			_sprites[2].heightAdd = -12;
			getSceneSpriteDef(11, 0)->x = 133;
			getSceneSpriteDef(11, 0)->y = 110;
			getSceneSpriteDef(11, 2)->frameIndex = 11;
			getSceneSpriteDef(11, 2)->status &= ~2;
			getSpriteTemplate(getSceneSpriteDef(11, 2)->templateId)->heightAdd = -4;
			_sprites[4].heightAdd = -4;
			getSceneSpriteDef(11, 2)->x = _sprites[4].x;
			getSceneSpriteDef(11, 2)->y = _sprites[4].y;
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case JUMP(idBucket):
		if (_spriteResourceType != 14) {
			_sprites[2].heightAdd = -20;
			queueWalkToSprite(2, -18, 183);
			queueActor(0x2299, 0x22AB, 0x22B1);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case JUMP(idCliffRock):
		if (_sprites[5].y != 76) {
			queueWalk(469 - _cameraStripX, 112);
			queueActor(0x22B8, 0x22D0, 0x22E3);
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case JUMP(idItem96):
		if (ABS(1591 - _cameraStripX - actorSprite().x) <= 30) {
			queueWalk(1598 - _cameraStripX, 113);
			queueActor(0x22F0, 0x2356, 0x23BB);
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;

	case JUMP(idBoat):
		queueWalk(91 - _cameraStripX, 180);
		queueActor(0xA912, 0xA943, 0xA96B);
		queueAnim(2, 15, 2, 0xA8AB, 0xA8C6, 0xA8DD);
		queueAnim(3, 21, 2, 0xA8F4, 0, 0xA906);
		_theScore += 2;
		actionThumbsUp();
		break;
		
	case JUMP(idIceWindow):
		if (_sprites[2].status == 0) {
			queueWalk(171, 165);
			queueActor(0xABFC, 0xAC0A, 0xAC13);
			_theScore += 5;
			actionThumbsUp();
		} else
			actionThumbsDown();
		break;
		
	case JUMP(idUp160):
		_walkDistance = 0;
		_currWalkInfo = NULL;
		actorSprite().changeCodeSync(0xACC2, 0xACD8, 0xACE7);
		actionThumbsUp();
		break;

	default:
		handled = false;
		actionThumbsDown();
		break;
	}

	return handled;
}

void EnchantiaEngine::talkCommandHelp() {

	switch (_sceneIndex) {

	case kSceneDungeon:
		if (getSceneSpriteDef(0, 6)->status) {
			actorSprite().anim.codeId = 0x241A;
			_sprites[8].anim.set2(0x2495, 0, 1);
			_sprites[8].moveX.set2(0x24DC, 0, 42);
			_sprites[8].moveY.set2(0x24F7, 0, 42);
			_sprites[5].anim.codeId = 0x2512;
			_sprites[5].anim.ticks = 40;
			_sprites[5].moveX.codeId = 0x2521;
			_sprites[5].moveX.ticks = 40;
			_sprites[11].anim.codeId = 0x252C;
			_sprites[11].anim.ticks = 84;
			_sprites[11].moveX.codeId = 0x2537;
			_sprites[11].moveX.ticks = 86;
			_sprites[11].moveY.codeId = 0x254B;
			_sprites[11].moveY.ticks = 86;
			getSceneSpriteDef(0, 6)->status = 0;
			getSceneSpriteDef(0, 9)->status = 0;
			actionThumbsUp();
		} else
			playSound(0, actorSprite());
		break;
		
	case kSceneCaveWall:
		if (_sprites[15].status == 3) {
			int16 walkTicks;
			queueWalk(159, 176);
			actorSprite().anim.codeId = 0;
			queueActor(0x262E, 0x263F, 0);
			getSceneSpriteDef(37, 13)->status = 1;
			getSceneSpriteDef(37, 0)->status = 0;
			getSceneSpriteDef(37, 1)->status = 0;
			getSceneSpriteDef(37, 2)->status = 0;
			getSceneSpriteDef(37, 3)->status = 0;
			getSceneSpriteDef(37, 4)->status = 0;
			getSceneSpriteDef(37, 5)->status = 0;
			getSceneSpriteDef(37, 6)->status = 0;
			getSceneSpriteDef(37, 7)->status = 0;
			getSceneSpriteDef(37, 8)->status = 0;
			getSceneSpriteDef(37, 9)->status = 0;
			getSceneSpriteDef(37, 10)->status = 0;
			getSceneSpriteDef(37, 11)->status = 0;
			getSceneSpriteDef(37, 12)->status = 0;
			_sprites[15].anim.set(0x1A2C, 0, 0, 1);
			_sprites[2].anim.codeId = 0x2647;
			_sprites[3].anim.codeId = 0x1A2E;
			_sprites[4].anim.codeId = 0x2647;
			_sprites[5].anim.codeId = 0x2647;
			_sprites[6].anim.codeId = 0x1A2E;
			_sprites[7].anim.codeId = 0x2647;
			_sprites[8].anim.codeId = 0x1A2E;
			_sprites[9].anim.codeId = 0x1A2E;
			_sprites[10].anim.codeId = 0x2647;
			_sprites[11].anim.codeId = 0x1A2E;
			_sprites[12].anim.codeId = 0x1A2E;
			_sprites[13].anim.codeId = 0x1A2E;
			_sprites[14].anim.codeId = 0x2647;
			_sprites[2].moveY.codeId = 0x2658;
			_sprites[3].moveY.codeId = 0x2658;
			_sprites[4].moveY.codeId = 0x2658;
			_sprites[5].moveY.codeId = 0x2663;
			_sprites[6].moveY.codeId = 0x266D;
			_sprites[7].moveY.codeId = 0x2663;
			_sprites[8].moveY.codeId = 0x266D;
			_sprites[9].moveY.codeId = 0x2663;
			walkTicks = actionThumbsUp() + 4;
			_sprites[2].moveY.ticks = walkTicks;
			_sprites[3].moveY.ticks = walkTicks;
			_sprites[4].moveY.ticks = walkTicks;
			_sprites[5].moveY.ticks = walkTicks;
			_sprites[6].moveY.ticks = walkTicks;
			_sprites[7].moveY.ticks = walkTicks;
			_sprites[8].moveY.ticks = walkTicks;
			_sprites[9].moveY.ticks = walkTicks;
			walkTicks += 5;
			_sprites[15].anim.ticks = walkTicks;
			_sprites[10].anim.ticks = walkTicks;
			_sprites[11].anim.ticks = walkTicks;
			_sprites[12].anim.ticks = walkTicks;
			_sprites[13].anim.ticks = walkTicks;
			_sprites[14].anim.ticks = walkTicks;
			walkTicks++;
			_sprites[5].anim.ticks = walkTicks;
			_sprites[7].anim.ticks = walkTicks;
			_sprites[9].anim.ticks = walkTicks;
			walkTicks += 2;
			_sprites[6].anim.ticks = walkTicks;
			_sprites[8].anim.ticks = walkTicks;
			walkTicks++;
			_sprites[3].anim.ticks = walkTicks;
			walkTicks++;
			_sprites[2].anim.ticks = walkTicks;
			_sprites[4].anim.ticks = walkTicks;
		} else
			actionThumbsDown();
		break;
		
	case kSceneIceDesert40:
		if (actorSprite().x + _cameraStripX <= 240 && actorSprite().y <= 72) {
			_scrollBorderLeft = 0;
			removeInventoryItem(idPen);
			removeInventoryItem(idMagnetOnString);
			queueWalk(165 - _cameraStripX, 114);
			queueActor(0x2675, 0x268D, 0x269C);
			queueAnim(5, 11, 8, 0x26AC, 0x26D7, 0x26E5);
			actionThumbsUp();
		} else
			playSound(0, actorSprite());
		break;
		
	default:
		playSound(0, actorSprite());
		break;
		
	}

}

void EnchantiaEngine::talkCommandHi() {

	switch (_sceneIndex) {
	
	case kSceneUnderwater:
		if (ABS(actorSprite().x - _sprites[7].x) < 4)
			_sprites[13].anim.set(0x1A2F, 0, 1, 40);
		break;
		
	case kSceneMageShop:
		if (_sprites[2].status == 1)
			_sprites[6].anim.set(0x1A2F, 0, 1, 40);	
		break;
		
	case kSceneCostumeShop:
		if (_sprites[2].status == 1)
			_sprites[5].anim.set(0x1A2F, 0, 1, 40);
		break;

	case kSceneSallySeeAll:
		_sprites[3].anim.set(0x1A2F, 0, 1, 40);
		break;
		
	case kSceneFranksFood:
		_sprites[3].anim.set(0x1A2F, 0, 1, 40);
		break;
		
	case kSceneBigBird:
		if (_flags1 & 1) {
			queueActor(0x241D, 0x247A, 0x245D);
		} else {
			_flags1 |= 1;
			getSceneSpriteDef(24, 0)->status = 3;
			getSceneSpriteDef(24, 1)->status = 3;
			queueActor(0x241D, 0x2442, 0x245D);
			queueAnim(3, 11, 30, 0x256C, 0, 0);
		}
		queueWalk(123, 186);
		queueAnim(2, 11, 1, 0x2571, 0, 0);
		queueAnim(4, 71, 2, 0x25E0, 0x25E6, 0x25EC);
		queueAnim(5, 71, 2, 0x25E0, 0x25F2, 0x25F8);
		queueAnim(6, 71, 2, 0x25E0, 0x25FE, 0x2604);
		queueAnim(7, 71, 2, 0x25E0, 0x260A, 0x2610);
		queueAnim(8, 71, 2, 0x25E0, 0x2616, 0x261C);
		queueAnim(9, 115, 2, 0x25E0, 0x2622, 0x2628);
		actionThumbsUp();
		return; // This is the only case where no playSound is called directly

	case kSceneCars:
		if (_sprites[2].status == 1 && ABS(_sprites[2].x - actorSprite().x) <= 64)
			_sprites[8].anim.set(0x1A2F, 0, 1, 40);
		break;
		
	case kSceneIceDesert41:
		if (_sprites[2].status == 1 && _sprites[2].frameIndex == 17 &&
			ABS(actorSprite().x - _sprites[2].x) <= 64)
			_sprites[6].anim.set(0x1A2F, 0, 1, 40);
		break;
		
	case kSceneIceDesert42:
		if (getSceneSpriteDef(42, 0)->status && ABS(actorSprite().x - _sprites[2].x) <= 64) {
			getSceneSpriteDef(42, 0)->status = 0;
			getSceneSpriteDef(42, 1)->status = 0;
			_sprites[2].setCodeSync(0x26F3, kNone, 0x2702, 7, 2);
			if (hasInventoryItem(idItem131) && getSceneSpriteDef(42, 0)->status == 0 &&
				getSceneSpriteDef(43, 0)->frameIndex != 19)
				getSceneSpriteDef(40, 0)->status = 1;
		}
		break;
	
	default:
		break;
		
	}

	playSound(1, actorSprite());

}

void EnchantiaEngine::talkCommandOpenSesame() {
	_talkTable[2] = -1;
	_sprites[24].anim.set(0x1A2C, 0, 5, 1);
	_sprites[25].anim.set(0x2561, 0, 1, 4);
	playSound(19, actorSprite());
}

void EnchantiaEngine::defaultTakeItem(uint type, int16 x, int16 y, uint spriteIndex) {
	static const struct { uint16 animId, moveXId, animIdSmall, moveXIdSmall; } moveTable[] = {
		{0x96C0, 0x96C9, 0x96F6, 0x96FF},
		{0x96CF, 0x96D8, 0x9705, 0x970E},
		{0x96DE, 0x96E7, 0x96DE, 0x96E7}};
	Sprite &sprite = _sprites[spriteIndex];
		
	queueWalk(x + sprite.x, y + sprite.y + sprite.height);
	getSceneSpriteDef(_sceneIndex, spriteIndex - 2)->status = 0;
	sprite.anim.set(0x1A2E, 0, 0, 1);
	if (_spriteResourceType != 20)
		queueActor(moveTable[type].animId, moveTable[type].moveXId, 0);
	else
		queueActor(moveTable[type].animIdSmall, moveTable[type].moveXIdSmall, 0);
	sprite.anim.ticks = actionThumbsUp() + 6;
}

void EnchantiaEngine::playIntro() {
	
	static const char *kIntroFilenames[] = {
		"cauldrn.pal", "balcony.pal", "baseball.pal", "cauldrn1.dat", "cauldrn2.dat",
		"cauldrn3.dat", "cauldrn4.dat", "balcony1.dat", "balcony2.dat", "balcony3.dat",
		"balcony4.dat", "balcony5.dat", "basebal1.dat", "basebal2.dat", "basebal3.dat",
		"basebal4.dat", "basebal5.dat", "basebal6.dat", "basebat.dat"
	};
	
	enum IntroCommand {
		CMD_FADE_OUT,
		CMD_LOAD_SCREENS,
		CMD_LOAD_PALETTE,
		CMD_CYCLE_SCREENS,
		CMD_SHOW_SCREEN,
		CMD_DONE
	};
	
	static const int8 kIntroCommands[] = {
		CMD_FADE_OUT, 
		CMD_LOAD_SCREENS, 3, 4, 5, 6, -1,
		CMD_LOAD_PALETTE, 0,
		CMD_CYCLE_SCREENS, 39,
		CMD_FADE_OUT, 
		CMD_LOAD_SCREENS, 7, 8, 9, -1,
		CMD_LOAD_PALETTE, 1,
		CMD_CYCLE_SCREENS, 29,
		CMD_LOAD_SCREENS, 10, 11, -1,
		CMD_SHOW_SCREEN, 0, 50,
		CMD_SHOW_SCREEN, 1, 50,
		CMD_FADE_OUT,
		CMD_LOAD_SCREENS, 12, -1,
		CMD_LOAD_PALETTE, 2,
		CMD_SHOW_SCREEN, 0, 50,
		CMD_LOAD_SCREENS, 13, 14, 15, -1,
		CMD_SHOW_SCREEN, 0, 50,
		CMD_SHOW_SCREEN, 1, 50,
		CMD_SHOW_SCREEN, 2, 50,
		CMD_LOAD_SCREENS, 16, 17, 18, -1,
		CMD_SHOW_SCREEN, 0, 50,
		CMD_SHOW_SCREEN, 1, 50,
		CMD_SHOW_SCREEN, 2, 50,
		CMD_FADE_OUT,
		CMD_DONE
	};
	
	Common::EventManager *eventMan = _system->getEventManager();
	Common::Event event;
	int cycleCounter = 0, screenIndex = -1, waitTicks = 0;
	const int8 *currCmd = kIntroCommands;
	Common::Array<Graphics::Surface*> screens;
	bool done = false;
	
	_isSaveAllowed = false;

	while (!done) {
		int8 cmd;

		while (eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN ||
				event.type == Common::EVENT_LBUTTONDOWN ||
				event.type == Common::EVENT_RBUTTONDOWN)
				done = true;
		}

		if (screenIndex >= 0) {
			_system->copyRectToScreen((const byte*)screens[screenIndex]->getBasePtr(0, 0), 320, 0, 0, 320, 200);
			_system->updateScreen();
			screenIndex = -1;
		}

		if (done)
			cmd = CMD_DONE;
		else if (waitTicks > 0) {
			_system->delayMillis(10);
			waitTicks--;
			continue;
		} else
			cmd = *currCmd++;
		
		switch (cmd) {
		case CMD_DONE:
			done = true;
			// Intentional fall-through to delete the screens
		case CMD_LOAD_SCREENS:
			for (uint i = 0; i < screens.size(); i++)
				delete screens[i];
			screens.clear();
			if (cmd == CMD_DONE)
				break;
			while (*currCmd >= 0)
				screens.push_back(loadBitmap(kIntroFilenames[*currCmd++], 320, 200));
			currCmd++;
			break;
		case CMD_LOAD_PALETTE:
			loadPalette(kIntroFilenames[*currCmd++], _palette);
			setVgaPalette(_palette);
			break;
		case CMD_FADE_OUT:
			palFadeOut();
			break;
		case CMD_CYCLE_SCREENS:
			if (cycleCounter == *currCmd) {
				cycleCounter = 0;
				currCmd++;
			} else {
				screenIndex = cycleCounter % screens.size();
				waitTicks = 16;
				cycleCounter++;
				currCmd--;
			}
			break;
		case CMD_SHOW_SCREEN:
			screenIndex = *currCmd++;
			waitTicks = *currCmd++;
			break;
		default:
			// "This should never happen (tm)"
			error("EnchantiaEngine::playIntro() Unknown cmd %02X", cmd);
			break;
		}
	}
	
}

} // End of namespace Enchantia
