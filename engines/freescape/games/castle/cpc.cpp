/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void CastleEngine::initCPC() {
	_viewArea = Common::Rect(40, 33 - 2, 280, 152);
	_soundIndexShoot = 5;
	_soundIndexCollide = -1;
	_soundIndexFallen = -1;
	_soundIndexStepUp = -1;
	_soundIndexStepDown = -1;
	_soundIndexMenu = -1;
	_soundIndexStart = 6;
	_soundIndexAreaChange = 7;
}


byte kCPCPaletteCastleTitleData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0xff},
	{0xff, 0xff, 0x00},
	{0xff, 0x00, 0x00},
};

byte kCPCPaletteCastleBorderData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x80, 0x80, 0x80},
	{0x00, 0x80, 0x00},
	{0xff, 0xff, 0xff},
};

// Data for the mountains background. This is not included in the original game for some reason
// but all the other releases have it. This is coming from the ZX Spectrum version.
byte mountainsData[288] {
	0x06, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b,
	0x00, 0x00, 0x38, 0x01, 0xa0, 0x00, 0x00, 0x00,
	0x00, 0x1e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x17,
	0x80, 0x00, 0x7c, 0x03, 0xd0, 0x00, 0x00, 0x01,
	0x00, 0x3f, 0x5c, 0x80, 0x0, 0x18, 0x0, 0x23,
	0xc0, 0x00, 0xfe, 0x0d, 0xfe, 0x6, 0x00, 0x02,
	0x80, 0x7e, 0xbe, 0xc0, 0x0, 0x3c, 0x0, 0x47,
	0xe0, 0x1, 0x57, 0x56, 0xff, 0xf, 0x80, 0x5,
	0x40, 0xf5, 0xfb, 0x40, 0x0, 0x76, 0x0, 0x93,
	0xd0, 0xa, 0xab, 0xab, 0xff, 0xaf, 0xc3, 0x2a,
	0xa1, 0xeb, 0xfe, 0xa8, 0x0, 0xde, 0x0, 0x21,
	0xa8, 0x75, 0x55, 0x41, 0xff, 0xd6, 0xef, 0xd5,
	0x53, 0x57, 0xfc, 0x14, 0x1, 0xb7, 0x7, 0x42,
	0xd5, 0xea, 0xaa, 0x92, 0xfb, 0xeb, 0xab, 0xea,
	0xaa, 0xae, 0xfa, 0x4a, 0x82, 0xea, 0xbe, 0x97,
	0xab, 0xd5, 0x55, 0x25, 0xdd, 0x75, 0x45, 0xf5,
	0x55, 0x7d, 0xdd, 0x25, 0x55, 0xd5, 0x54, 0x2f,
	0xf7, 0xaa, 0xaa, 0x53, 0xea, 0xa8, 0x13, 0xfa,
	0xaa, 0xea, 0xbe, 0x42, 0xab, 0xaa, 0xa9, 0x5f,
	0xdd, 0xd5, 0x55, 0x7, 0x55, 0x2, 0x45, 0xfd,
	0x51, 0x55, 0x57, 0x15, 0x57, 0xd5, 0x52, 0xaf,
	0xee, 0xfa, 0xaa, 0x2b, 0xaa, 0x80, 0x8b, 0xfe,
	0xaa, 0xaa, 0xae, 0xaa, 0xbe, 0xaa, 0xa4, 0x5a,
	0xb5, 0x5d, 0x5c, 0x56, 0xd5, 0x29, 0x1f, 0xff,
	0x55, 0x55, 0x5b, 0x55, 0x7d, 0x55, 0x9, 0xb5,
	0x5a, 0xaf, 0xba, 0xad, 0xaa, 0x92, 0x3e, 0xbf,
	0xea, 0xaa, 0xaf, 0xab, 0xea, 0xaa, 0x2, 0x5a,
	0xf5, 0x55, 0xfd, 0x57, 0x55, 0x5, 0x5f, 0x57,
	0xfd, 0x55, 0x55, 0x57, 0x55, 0x50, 0x15, 0xaf,
	0xba, 0xaa, 0xfe, 0xae, 0xfa, 0xaa, 0xbe, 0xaa,
	0xbf, 0xaa, 0xaa, 0xaa, 0xaa, 0x82, 0xaa, 0x55,
	0x55, 0x55, 0x5f, 0xd5, 0xfd, 0x55, 0x55, 0x55,
	0x5f, 0xfd, 0x55, 0x55, 0x55, 0x55, 0x55, 0xea,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};



void CastleEngine::loadAssetsCPCFullGame() {
	Common::File file;
	uint8 r, g, b;
	Common::Array<Graphics::ManagedSurface *> chars;

    file.open("CMLOAD.BIN");

	if (!file.isOpen())
		error("Failed to open CMLOAD.BIN");

	_title = readCPCImage(&file, true);
	_title->setPalette((byte*)&kCPCPaletteCastleTitleData, 0, 4);

    file.close();
	file.open("CMSCR.BIN");

	if (!file.isOpen())
		error("Failed to open CMSCR.BIN");

	_border = readCPCImage(&file, true);
	_border->setPalette((byte*)&kCPCPaletteCastleBorderData, 0, 4);

    file.close();
	file.open("CM.BIN");

	if (!file.isOpen())
		error("Failed to open TECODE.BIN/TE2.BI2");

	loadMessagesVariableSize(&file, 0x16c6, 71);
	switch (_language) {
		/*case Common::ES_ESP:
			loadRiddles(&file, 0x1470 - 4 - 2 - 9 * 2, 9);
			loadMessagesVariableSize(&file, 0xf3d, 71);
			load8bitBinary(&file, 0x6aab - 2, 16);
			loadSpeakerFxZX(&file, 0xca0, 0xcdc);

			file.seek(0x1218 + 16);
			for (int i = 0; i < 90; i++) {
				Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
				surface->create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
				chars.push_back(loadFrame(&file, surface, 1, 8, 1));
			}
			_font = Font(chars);
			_font.setCharWidth(9);
			_fontLoaded = true;

			break;*/
		case Common::EN_ANY:
			loadRiddles(&file, 0x1b75 - 2 - 9 * 2, 9);
			load8bitBinary(&file, 0x791a, 16);

			file.seek(0x2724);
			for (int i = 0; i < 90; i++) {
				Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
				surface->create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
				chars.push_back(loadFrame(&file, surface, 1, 8, 1));
			}
			_font = Font(chars);
			_font.setCharWidth(9);
			_fontLoaded = true;

			break;
		default:
			error("Language not supported");
			break;
	}

	loadColorPalette();

	int backgroundWidth = 16;
	int backgroundHeight = 18;
	Graphics::ManagedSurface *background = new Graphics::ManagedSurface();
	background->create(backgroundWidth * 8, backgroundHeight, _gfx->_texturePixelFormat);
	background->fillRect(Common::Rect(0, 0, backgroundWidth * 8, backgroundHeight), 0);
	Common::MemoryReadStream mountainsStream(mountainsData, sizeof(mountainsData));

	_gfx->readFromPalette(11, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_background = loadFrame(&mountainsStream, background, backgroundWidth, backgroundHeight, front);

	// CPC UI Sprites - located at different offsets than ZX Spectrum!
	// CPC uses Mode 1 format (4 pixels per byte, 2 bits per pixel).
	// Sprite pixel values 0-3 are CPC ink numbers that map to the border palette.
	uint32 cpcPalette[4];
	for (int i = 0; i < 4; i++) {
		cpcPalette[i] = _gfx->_texturePixelFormat.ARGBToColor(0xFF,
			kCPCPaletteCastleBorderData[i][0],
			kCPCPaletteCastleBorderData[i][1],
			kCPCPaletteCastleBorderData[i][2]);
	}

	// Keys Border: CPC offset 0x2362 (8x14 px, 1 frame - matches ZX key_sprite)
	_keysBorderFrames.push_back(loadFrameWithHeaderCPC(&file, 0x2362, cpcPalette));

	// Spirit Meter Background: CPC offset 0x2383 (64x8 px - matches ZX spirit_meter_bg)
	_spiritsMeterIndicatorBackgroundFrame = loadFrameWithHeaderCPC(&file, 0x2383, cpcPalette);

	// Spirit Meter Indicator: CPC offset 0x2408 (16x8 px - matches ZX spirit_meter_indicator)
	_spiritsMeterIndicatorFrame = loadFrameWithHeaderCPC(&file, 0x2408, cpcPalette);

	// Strength Background: CPC offset 0x242D (68x15 px - matches ZX strength_bg)
	_strenghtBackgroundFrame = loadFrameWithHeaderCPC(&file, 0x242D, cpcPalette);

	// Strength Bar: CPC offset 0x2531 (68x3 px - matches ZX strength_bar)
	_strenghtBarFrame = loadFrameWithHeaderCPC(&file, 0x2531, cpcPalette);

	// Strength Weights: CPC offset 0x2569 (4x15 px, 4 frames - matches ZX weight_sprite w=1,h=15)
	_strenghtWeightsFrames = loadFramesWithHeaderCPC(&file, 0x2569, 4, cpcPalette);

	// Flag Animation: CPC offset 0x2654 (16x9 px, 4 frames)
	_flagFrames = loadFramesWithHeaderCPC(&file, 0x2654, 4, cpcPalette);

	// Note: Riddle frames are not loaded from external files for CPC.
	// The _riddleTopFrame, _riddleBackgroundFrame, and _riddleBottomFrame
	// will remain nullptr (initialized in constructor).
	// The drawRiddle function handles nullptr gracefully.

	for (auto &it : _areaMap) {
		it._value->addStructure(_areaMap[255]);

		it._value->addObjectFromArea(164, _areaMap[255]);
		it._value->addObjectFromArea(174, _areaMap[255]);
		it._value->addObjectFromArea(175, _areaMap[255]);
		for (int16 id = 136; id < 140; id++) {
			it._value->addObjectFromArea(id, _areaMap[255]);
		}

		it._value->addObjectFromArea(195, _areaMap[255]);
		for (int16 id = 214; id < 228; id++) {
			it._value->addObjectFromArea(id, _areaMap[255]);
		}
	}
	// Discard some global conditions
	// It is unclear why they hide/unhide objects that formed the spirits
	for (int i = 0; i < 3; i++) {
		debugC(kFreescapeDebugParser, "Discarding condition %s", _conditionSources[0].c_str());
		_conditions.remove_at(0);
		_conditionSources.remove_at(0);
	}
}

void CastleEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = _gfx->_paperColor;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 1;

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect backRect(97, 181, 232, 190);
	surface->fillRect(backRect, back);

	Common::String message;
	int deadline = -1;
	getLatestMessages(message, deadline);
	if (deadline > 0 && deadline <= _countdown) {
		drawStringInSurface(message, 97, 182, front, back, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_gameStateControl == kFreescapeGameStatePlaying) {
			drawStringInSurface(_currentArea->_name, 97, 182, front, back, surface);
		}
	}

	// Draw collected keys
	if (!_keysBorderFrames.empty()) {
		for (int k = 0; k < int(_keysCollected.size()); k++) {
			surface->copyRectToSurface((const Graphics::Surface)*_keysBorderFrames[0], 76 - k * 4, 179, Common::Rect(0, 0, _keysBorderFrames[0]->w, _keysBorderFrames[0]->h));
		}
	}

	// Draw energy meter (strength)
	drawEnergyMeter(surface, Common::Point(38, 158));

	// Draw spirit meter
	uint32 blackColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0, 0, 0);
	if (_spiritsMeterIndicatorBackgroundFrame) {
		surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorBackgroundFrame, 136, 162, Common::Rect(0, 0, _spiritsMeterIndicatorBackgroundFrame->w, _spiritsMeterIndicatorBackgroundFrame->h));
		if (_spiritsMeterIndicatorFrame) {
			surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 131 + _spiritsMeterPosition, 161, Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h), blackColor);
		}
	} else {
		uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0xff, 0x00);
		surface->fillRect(Common::Rect(152, 162, 216, 170), green);
		if (_spiritsMeterIndicatorFrame) {
			surface->copyRectToSurface((const Graphics::Surface)*_spiritsMeterIndicatorFrame, 131 + _spiritsMeterPosition, 161, Common::Rect(0, 0, _spiritsMeterIndicatorFrame->w, _spiritsMeterIndicatorFrame->h));
		}
	}

	// Draw animated flag
	if (!_flagFrames.empty()) {
		int ticks = g_system->getMillis() / 20;
		int flagFrameIndex = (ticks / 10) % 4;
		surface->copyRectToSurface(*_flagFrames[flagFrameIndex], 285, 5, Common::Rect(0, 0, _flagFrames[flagFrameIndex]->w, _flagFrames[flagFrameIndex]->h));
	}

}

} // End of namespace Freescape
