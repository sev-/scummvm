#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/pak.h"
#include "comet/screen.h"

#include "comet/animation.h"

namespace Comet {

AnimationElement::~AnimationElement() {
	for (Common::Array<AnimationCommand*>::iterator iter = commands.begin(); iter != commands.end(); iter++)
		delete (*iter);
}

AnimationFrameList::~AnimationFrameList() {
	for (Common::Array<AnimationFrame*>::iterator iter = frames.begin(); iter != frames.end(); iter++)
		delete (*iter);
}

Animation::Animation() {
}

Animation::~Animation() {
	free();
}

void Animation::load(Common::SeekableReadStream &sourceS, uint dataSize) {

	free();

	OffsetArray sectionOffsets, offsets;
	
	loadOffsets(sourceS, sectionOffsets);

	if (sectionOffsets.size() < 4)
		error("Animation::load() Unexpected section count");

	// Load animation elements
	sourceS.seek(sectionOffsets[0]);
	loadOffsets(sourceS, offsets);
	for (uint i = 0; i < offsets.size(); i++) {
		sourceS.seek(sectionOffsets[0] + offsets[i] - 2);
		AnimationElement *animationElement = loadAnimationElement(sourceS);
		_elements.push_back(animationElement);
	}
	
	// Load animation cels
	sourceS.seek(sectionOffsets[1]);
	loadOffsets(sourceS, offsets);
	offsets.push_back(sectionOffsets[2] - sectionOffsets[1]);
	for (uint i = 0; i < offsets.size() - 1; i++) {
		sourceS.seek(sectionOffsets[1] + offsets[i] - 2);
		AnimationCel *animationCel = new AnimationCel();
		animationCel->flags = sourceS.readUint16LE();
		animationCel->width = sourceS.readByte() * 16;
		animationCel->height = sourceS.readByte();
		animationCel->dataSize = offsets[i + 1] - offsets[i] - 2;
		animationCel->data = new byte[animationCel->dataSize];
		sourceS.read(animationCel->data, animationCel->dataSize);
		debug(0, "Animation::load() cel width = %d; height = %d; dataSize = %d", animationCel->width, animationCel->height, animationCel->dataSize);
		_cels.push_back(animationCel);
	}

	// Load animation frames
	sourceS.seek(sectionOffsets[2]);
	loadOffsets(sourceS, offsets);
	for (uint i = 0; i < offsets.size(); i++) {
		sourceS.seek(sectionOffsets[2] + offsets[i]);
		AnimationFrameList *animationFrameList = loadAnimationFrameList(sourceS);
		_anims.push_back(animationFrameList);
	}

	// TODO: Load section 4 data

}

void Animation::free() {

	for (Common::Array<AnimationElement*>::iterator iter = _elements.begin(); iter != _elements.end(); iter++)
		delete (*iter);

	for (Common::Array<AnimationCel*>::iterator iter = _cels.begin(); iter != _cels.end(); iter++)
		delete (*iter);

	for (Common::Array<AnimationFrameList*>::iterator iter = _anims.begin(); iter != _anims.end(); iter++)
		delete (*iter);

}

void Animation::loadOffsets(Common::SeekableReadStream &sourceS, OffsetArray &offsets) {
	offsets.clear();
	uint32 offset = sourceS.readUint32LE();
	uint count = offset / 4;
	debug(0, "Animation::loadOffsets() count = %d", count);
	while (count--) {
		offsets.push_back(offset);
		offset = sourceS.readUint32LE();
	}
}

AnimationElement *Animation::loadAnimationElement(Common::SeekableReadStream &sourceS) {
	AnimationElement *animationElement = new AnimationElement();
	animationElement->width = sourceS.readByte();
	animationElement->height = sourceS.readByte();
	animationElement->flags = sourceS.readByte();
	byte cmdCount = sourceS.readByte();
	debug(0, "Animation::loadAnimationElement() cmdCount = %d", cmdCount);
	while (cmdCount--) {
		AnimationCommand *animationCommand = loadAnimationCommand(sourceS, animationElement->flags & 0x10);
		animationElement->commands.push_back(animationCommand);
	}
	return animationElement;
}

AnimationCommand *Animation::loadAnimationCommand(Common::SeekableReadStream &sourceS, bool ptAsByte) {
	AnimationCommand *animationCommand = new AnimationCommand();
	animationCommand->cmd = sourceS.readByte();
	byte pointsCount = sourceS.readByte();
	animationCommand->arg1 = sourceS.readByte();
	animationCommand->arg2 = sourceS.readByte();
	debug(0, "Animation::loadAnimationCommand() cmd = %d; pointsCount = %d; arg1 = %d; arg2 = %d",
		animationCommand->cmd, pointsCount, animationCommand->arg1, animationCommand->arg2);
	while (pointsCount--) {
		Point pt;
		if (ptAsByte) {
			pt.x = (int8)sourceS.readByte();
			pt.y = (int8)sourceS.readByte();
		} else {
			pt.x = (int16)sourceS.readUint16LE();
			pt.y = (int16)sourceS.readUint16LE();
		}
		debug(0, "Animation::loadAnimationCommand()	 x = %d; y = %d", pt.x, pt.y);
		animationCommand->points.push_back(pt);
	}
	return animationCommand;
}

AnimationFrameList *Animation::loadAnimationFrameList(Common::SeekableReadStream &sourceS) {
	AnimationFrameList *animationFrameList = new AnimationFrameList();
	animationFrameList->priority = sourceS.readByte();
	byte frameCount = sourceS.readByte();
	debug(0, "Animation::loadAnimationFrameList() frameCount = %d", frameCount);
	while (frameCount--) {
		AnimationFrame *animationFrame = new AnimationFrame();
		animationFrame->elementIndex = sourceS.readUint16LE();
		animationFrame->flags = sourceS.readUint16LE();
		animationFrame->xOffs = (int16)sourceS.readUint16LE();
		animationFrame->yOffs = (int16)sourceS.readUint16LE();
		debug(0, "Animation::loadAnimationFrameList() elementIndex = %d; flags = %04X; xOffs = %d; yOffs = %d",
			animationFrame->elementIndex, animationFrame->flags, animationFrame->xOffs, animationFrame->yOffs);
		animationFrameList->frames.push_back(animationFrame);
	}
	return animationFrameList;
}

} // End of namespace Comet
