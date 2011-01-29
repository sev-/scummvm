#ifndef COMET_UNPACK_H
#define COMET_UNPACK_H

#include "common/util.h"
#include "common/stream.h"
#include "common/endian.h"

struct HufNode {
	unsigned short b0;
	unsigned short b1;
	HufNode *jump;
};

class DecompressImplode {
public:
	int decompress(Common::ReadStream *source, int flags, int size, byte *dest);
protected:
	Common::ReadStream *_source;
	byte *_dest;
	byte _bitBuf;
	int _bitBufLeft;
	byte _buffer[32768];
	int _bufferPos;
	int _bytesWritten;
	HufNode _literalTree[256];
	HufNode _distanceTree[64];
	HufNode _lengthTree[64];
	byte readByte();
	int readBit();
	int readBits(int count);
	void putByte(byte value);
	void flush();
	void recreateTree(HufNode *&currentTree, byte &len, int16 *fpos, int *flens, int16 fmax);
	int decodeSFValue(HufNode *currentTree);
	int createTree(HufNode *currentTree);
};

#endif
