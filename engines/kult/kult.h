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
 * $URL: https://svn.scummvm.org:4444/svn/kult/kult.h $
 * $Id: kult.h 126 2011-04-30 00:09:02Z digitall $
 *
 */

#ifndef KULT_H
#define KULT_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/textconsole.h"

#include "engines/engine.h"

#include "graphics/surface.h"

namespace Kult {

struct KultGameDescription;
class ResourceData;
class SpriteResource;

class KultEngine : public ::Engine {
protected:
	Common::Error run();
public:
	KultEngine(OSystem *syst, const KultGameDescription *gameDesc);
	virtual ~KultEngine();

	virtual bool hasFeature(EngineFeature f) const;
	virtual void syncSoundSettings();

	Common::RandomSource *_rnd;
	const KultGameDescription *_gameDescription;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	const Common::String& getTargetName() const { return _targetName; }

	void updateInput();

protected:

	Common::KeyState _keyState;
	int16 _mouseX, _mouseY;
	bool _leftButtonDown, _rightButtonDown;

	Graphics::Surface *_frontScreen, *_backScreen;
	
	SpriteResource *_SPRIT;
	SpriteResource *_PUZZL;
	SpriteResource *_PERSO;
	ResourceData *_LUTIN;
	ResourceData *_ICONE;
	ResourceData *_MURSM;

	void loadBackground(const char *filename);	
	void drawSurface(Graphics::Surface *sourceSurface, Graphics::Surface *destSurface, int x, int y, bool flipX = false, bool flipY = false);
	Graphics::Surface *buildLutin(uint index);
	Graphics::Surface *buildIcone(uint index);
	Graphics::Surface *buildMursm(uint index);

};

} // End of namespace Kult

#endif /* KULT_H */
