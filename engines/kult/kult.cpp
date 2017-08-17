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
 * $URL: https://svn.scummvm.org:4444/svn/kult/kult.cpp $
 * $Id: kult.cpp 127 2011-05-04 04:52:04Z digitall $
 *
 */

#include "common/EventRecorder.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "common/error.h"
#include "common/textconsole.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "engines/util.h"

#include "audio/mixer.h"

#include "kult/kult.h"
#include "kult/resource.h"
#include "kult/script.h"

namespace Kult {

/* EGA Color Table */
byte egaColorTable[256 * 3] = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0xaa,
    0x00, 0xaa, 0x00,
    0x00, 0xaa, 0xaa,
    0xaa, 0x00, 0x00,
    0xaa, 0x00, 0xaa,
    0xaa, 0x55, 0x00,
    0xaa, 0xaa, 0xaa,
    0x00, 0x00, 0x00,
    0x55, 0x55, 0xff,
    0x55, 0xff, 0x55,
    0x55, 0xff, 0xff,
    0xff, 0x55, 0x55,
    0xff, 0x55, 0xff,
    0xff, 0xff, 0x55,
    0xff, 0xff, 0xff,
};

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

KultEngine::KultEngine(OSystem *syst, const KultGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

    _rnd = new Common::RandomSource("kult");
}

KultEngine::~KultEngine() {
	delete _rnd;
}

void KultEngine::syncSoundSettings() {
	/*
	_music->setVolume(ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	*/
}

Common::Error KultEngine::run() {
	// Initialize backend
	_system->beginGFXTransaction();
	initCommonGFX(true);
	_system->initSize(320, 200);
	_system->endGFXTransaction();

	_mouseX = 0;
	_mouseY = 0;
	_leftButtonDown = false;
	_rightButtonDown = false;

	syncSoundSettings();

	CursorMan.showMouse(true);

    _frontScreen = new Graphics::Surface();
    _frontScreen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_backScreen = new Graphics::Surface();
	_backScreen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

    _system->getPaletteManager()->setPalette(egaColorTable, 0, 256);

    _SPRIT = new SpriteResource();
	_SPRIT->appendFromFile("sprit.ega");
    
    _PUZZL = new SpriteResource();
	_PUZZL->appendFromFile("puzzl.ega");
	_PUZZL->appendFromFile("puzz1.ega");
    
    _PERSO = new SpriteResource();
	_PERSO->appendFromFile("perso.ega");

	//loadBackground("fond.ega");
    
	_LUTIN = new ResourceData(1);
	_LUTIN->loadFromFile("lutin.bin");

	_ICONE = new ResourceData(1);
	_ICONE->loadFromFile("icone.bin");

	_MURSM = new ResourceData(1);
	_MURSM->loadFromFile("mursm.bin");

	Script *_script = new Script(this);
	_script->loadFromFile("KULTEGA.BIN");
	
	_script->gotoScript(0);
	_script->runScript();

#if 0
	debug("_LUTIN->getCount() = %d", _LUTIN->getCount());
	
    //debug("_PUZZL->getCount() = %d", _PUZZL->getCount());
    
    int i = 0;
   
	memset(_frontScreen->pixels, 0, _frontScreen->pitch * _frontScreen->h);   
   
#if 1
	while (!shouldQuit()) {
		updateInput();

		#if 0
    	drawSurface(_PUZZL->getSprite(i), _frontScreen, 20, 20);
    	i++;
    	if (i >= _PUZZL->getSpriteCount())
    		i = 0;
		#endif    		

		#if 0
		memset(_frontScreen->pixels, 0, _frontScreen->pitch * _frontScreen->h);
		Graphics::Surface *lutin = buildLutin(i);
        drawSurface(lutin, _frontScreen, 20, 20);
        delete lutin;
    	i++;
    	if (i >= _LUTIN->getCount())
    		i = 0;
		#endif    		

		#if 0
		memset(_frontScreen->pixels, 0, _frontScreen->pitch * _frontScreen->h);
		Graphics::Surface *icone = buildIcone(i);
        drawSurface(icone, _frontScreen, 20, 20);
        delete icone;
    	i++;
    	if (i >= _ICONE->getCount())
    		i = 0;
		#endif    		

		#if 1
		memset(_frontScreen->pixels, 0, _frontScreen->pitch * _frontScreen->h);
		Graphics::Surface *mursm = buildMursm(i);
        drawSurface(mursm, _frontScreen, 20, 20);
        delete mursm;
    	i++;
    	debug("_MURSM->getCount() = %d", _MURSM->getCount());
    	if (i >= _MURSM->getCount())
    		i = 0;
		#endif    		

		_system->copyRectToScreen((const byte *)_frontScreen->getBasePtr(0, 0),
			320, 0, 0, 320, 200);
		_system->updateScreen();
		_system->delayMillis(250);
	}
#endif

#endif

    delete _frontScreen;
	delete _backScreen;

	return Common::kNoError;
}

/*
		_system->copyRectToScreen((const byte *)_screen->_frontScreen + _cameraHeight * 640,
			640, 0, _cameraHeight, 640, _guiHeight);
	_system->updateScreen();
*/			

void KultEngine::updateInput() {

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(event)) {
	switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyState = event.kbd;
			break;
		case Common::EVENT_KEYUP:
			_keyState.reset();
			break;
		case Common::EVENT_QUIT:
			quitGame();
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_leftButtonDown = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			_rightButtonDown = false;
			break;
		default:
			break;
		}
	}

}

void KultEngine::loadBackground(const char *filename) {
	byte *dstPixels = (byte*)_frontScreen->pixels;
	Common::File fd;
	if (!fd.open(filename))
		error("KultEngine::loadBackground() Could not open %s", filename);
	byte *planarData = new byte[32000];
	fd.read(planarData, 32000);
	// Unpack planar EGA data
    for (uint i = 0; i < 8000; i++) {
    	for (int bit = 0; bit < 8; bit++) {
    		*dstPixels++ = (((planarData[i] << bit) & 0x80) >> 7) | 
    			(((planarData[i + 8000] << bit) & 0x80) >> 6) | 
				(((planarData[i + 16000] << bit) & 0x80) >> 5) | 
				(((planarData[i + 24000] << bit) & 0x80) >> 4);
		}
    }
    delete[] planarData;
}

void KultEngine::drawSurface(Graphics::Surface *sourceSurface, Graphics::Surface *destSurface, int x, int y, bool flipX, bool flipY) {
	byte *srcPixels = (byte*)sourceSurface->pixels;
	byte *dstPixels = (byte*)destSurface->getBasePtr(x, y);
	int srcIncr = 1, srcAdd = 0;
	int dstPitch = destSurface->pitch;
	int w = sourceSurface->w;
	int h = sourceSurface->h;

	//debug("x = %d; y = %d; sourceSurface->w = %d; sourceSurface->h = %d; destSurface->w = %d; destSurface->h = %d", x, y, sourceSurface->w, sourceSurface->h, destSurface->w, destSurface->h);

	if (x + w > destSurface->w)
		w = destSurface->w - x;

	if (y + h > destSurface->h)
		h = destSurface->h - y;

	if (flipX) {
		srcIncr = -1;
		srcAdd = sourceSurface->w - 1;
	}

    //debug("w = %d; h = %d", w, h);

	if (flipY) {
		dstPixels = (byte*)destSurface->getBasePtr(x, y + h - 1);
        dstPitch = -destSurface->pitch;
	}

	while (h--) {
		byte *src = srcPixels + srcAdd;
		for (int xc = 0; xc < w; xc++) {
			if (*src != 0)
				dstPixels[xc] = *src;
			src += srcIncr;				
		}
		srcPixels += sourceSurface->pitch;
		dstPixels += dstPitch;
	}

}

Graphics::Surface *KultEngine::buildLutin(uint index) {
	/*
		LUTIN item format:
		byte width			width of sprite
		byte height			height of sprite
		repeat until end of data:
			byte spriteIndex	index of SPRIT to draw
			uint16 offset		& 0x8000: flipped; & 0x7FFF: dest offset in bytes
	*/
	Common::MemoryReadStream *lutinData = _LUTIN->getAsStream(index);
	uint width = lutinData->readByte() * 4;
	uint height = lutinData->readByte();
	Graphics::Surface *destSurface = new Graphics::Surface();
	destSurface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	debug("width = %d; height = %d", width, height);
	while (lutinData->pos() < lutinData->size()) {
		byte spriteIndex = lutinData->readByte();
		uint16 offset = lutinData->readUint16LE();
		int x = ((offset & 0x7FFF) * 4) % width;
		int y = ((offset & 0x7FFF) * 4) / width;
		debug("spriteIndex = %d; offset = %04X; x = %d; y = %d", spriteIndex, offset, x, y);
		// TODO: Flip surface
		drawSurface(_SPRIT->getSprite(spriteIndex), destSurface, x, y);
	}
	delete lutinData;
	return destSurface;
}

Graphics::Surface *KultEngine::buildIcone(uint index) {
	/*
		ICONE item format:
		byte type
		repeat until end of data:
			
	*/
	static const struct { byte height, width, vColor, bColor, hColor1, hColor2; } iconeTypes[] = {
		{65, 16, 15, 3, 15, 15},
		{70, 16, 15, 3, 15, 15},
		{65, 17, 15, 3, 15, 15},
		{75, 17, 15, 3, 15, 15},
		{85, 16, 15, 3, 15, 15},
		{47, 13, 15, 0, 15, 15},
		{65, 18, 15, 3, 15, 15},
		{38, 11, 15, 0, 15, 15},
		{27, 34, 0, 0, 0, 0},
		{9, 11, 0, 0, 0, 0}
	};

    Common::MemoryReadStream *iconeData = _ICONE->getAsStream(index);
    byte type = iconeData->readByte();
    
    debug("type = %d", type);

	Graphics::Surface *destSurface = new Graphics::Surface();
	destSurface->create(iconeTypes[type].width * 4, iconeTypes[type].height, Graphics::PixelFormat::createFormatCLUT8());
	
	destSurface->hLine(0, 0, destSurface->w - 1, iconeTypes[type].vColor);
	destSurface->hLine(0, destSurface->h - 1, destSurface->w - 1, iconeTypes[type].vColor);
	destSurface->vLine(0, 1, destSurface->h - 2, iconeTypes[type].hColor1);
	destSurface->vLine(destSurface->w - 1, 1, destSurface->h - 2, iconeTypes[type].hColor2);
	destSurface->fillRect(Common::Rect(1, 1, destSurface->w - 1, destSurface->h - 1), iconeTypes[type].bColor);

	while (iconeData->pos() < iconeData->size()) {
		byte spriteIndex = iconeData->readByte();
		uint16 offset = iconeData->readUint16LE();
		int x = ((offset & 0x3FFF) * 4) % destSurface->w;
		int y = ((offset & 0x3FFF) * 4) / destSurface->w;
		debug("spriteIndex = %d; offset = %04X; x = %d; y = %d", spriteIndex, offset, x, y);
		// TODO: Flip surface
		drawSurface(_PERSO->getSprite(spriteIndex), destSurface, x, y, offset & 0x4000, offset & 0x8000);
	}	
	
	delete iconeData;
	return destSurface;
}

Graphics::Surface *KultEngine::buildMursm(uint index) {

	/*
		MURSM item format:
			
	*/

    Common::MemoryReadStream *mursmData = _MURSM->getAsStream(index);
    
	Graphics::Surface *destSurface = new Graphics::Surface();
	destSurface->create(80, 44, Graphics::PixelFormat::createFormatCLUT8());
	
	while (mursmData->pos() < mursmData->size()) {
		byte spriteIndex = mursmData->readByte();
		uint16 offset = mursmData->readUint16LE();
		int x = ((offset & 0x3FFF) * 4) % destSurface->w;
		int y = ((offset & 0x3FFF) * 4) / destSurface->w;
		debug("spriteIndex = %d; offset = %04X; x = %d; y = %d", spriteIndex, offset, x, y);
		// TODO: Flip surface
		drawSurface(_PUZZL->getSprite(spriteIndex & 0x7F), destSurface, x, y, spriteIndex & 0x80, false);
	}	

	debug("ok");
	
	delete mursmData;
	return destSurface;

}

} // End of namespace Kult
