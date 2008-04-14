#ifndef COMET_ANIM_H
#define COMET_ANIM_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/endian.h"

#include "comet/comet.h"

namespace Comet {

class Anim {

public:
    Anim(CometEngine *vm);
    ~Anim();
    
    void load(const char *pakFilename, int fileIndex);

    byte *getSection(int section);
    byte *getSubSection(int section, int subSection);
    int getFrameWidth(uint16 index);
    int getFrameHeight(uint16 index);
    void drawFrame(uint16 index, int x, int y, byte *destBuffer);
	void unpackRle(int x, int y, byte *rleData, byte *destBuffer);
    void runSeq1(uint16 index, int x, int y);

        byte *_animData;

private:
    CometEngine *_vm;
    void decompressLine(byte *&sourceBuf, byte *destBuf);
    void skipLine(byte *&sourceBuf);
};

} // End of namespace Comet

#endif
