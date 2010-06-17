#ifndef COMET_ANIMATION_H
#define COMET_ANIMATION_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"

#include "comet/comet.h"

namespace Comet {

enum AnimationCommandType {
	kActElement			= 0,
	kActCelSprite		= 1,
	kActNop0			= 2,
	kActNop1			= 3,
	kActFilledPolygon	= 4,
	kActRectangle		= 5,
	kActPolygon			= 6,
	kActPixels			= 7,
	kActPolygon1		= 8,	// unused in Comet? / Alias for kActPolygon
	kActPolygon2		= 9,	// unused in Comet? / Alias for kActPolygon
	kActCelRle			= 10
};

struct Point;

struct AnimationCommand {
	byte cmd;
	byte arg1, arg2;
	Common::Array<Point> points;
};

struct AnimationElement {
	byte width, height, flags;
	Common::Array<AnimationCommand*> commands;
	~AnimationElement();
};

struct AnimationCel {
	uint16 flags;
	uint16 width, height;
	uint16 dataSize;
	byte *data;
};

struct AnimationFrame {
	uint16 elementIndex;
	uint16 flags;
	int16 xOffs, yOffs;
};

struct AnimationFrameList {
	byte priority;
	Common::Array<AnimationFrame*> frames;
	~AnimationFrameList();
};

struct InterpolatedAnimationCommand {
	byte _cmd;
	byte _aarg1, _aarg2, _barg1, _barg2;
	Common::Array<Point> _points;
	InterpolatedAnimationCommand(byte cmd, byte aarg1, byte aarg2, byte barg1, byte barg2);
};

struct InterpolatedAnimationElement {
	Common::Array<InterpolatedAnimationCommand*> commands;
	~InterpolatedAnimationElement();
};

class Animation {
public:
	Animation();
	~Animation();
	void load(Common::SeekableReadStream &sourceS, uint dataSize);
	//AnimationCel *getCel(int index) { return _cels[index]; }
	int16 getCelWidth(int16 celIndex) const { return _cels[celIndex]->width; }
	int16 getCelHeight(int16 celIndex) const { return _cels[celIndex]->height; }
//protected://all public while in progress
	typedef Common::Array<uint32> OffsetArray;
	Common::Array<AnimationElement*> _elements;
	Common::Array<AnimationCel*> _cels;
	Common::Array<AnimationFrameList*> _anims;
	// TODO: What is section 4 used for? Special palette?
	void free();
	void loadOffsets(Common::SeekableReadStream &sourceS, OffsetArray &offsets);
	AnimationElement *loadAnimationElement(Common::SeekableReadStream &sourceS);
	AnimationCommand *loadAnimationCommand(Common::SeekableReadStream &sourceS, bool ptAsByte);
	AnimationFrameList *loadAnimationFrameList(Common::SeekableReadStream &sourceS);
};

} // End of namespace Comet

#endif
