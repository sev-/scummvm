#include "comet/unpack.h"

// Based on gunzip.c by Pasi Ojala from http://www.cs.tut.fi/~albert/Dev/gunzip/

byte DecompressImplode::readByte() {
	return _source->readByte();
}

int DecompressImplode::readBit() {
	int bit;
	if (_bitBufLeft == 0) {
		_bitBuf = readByte();
		_bitBufLeft = 8;
	}
	bit = _bitBuf & 1;
	_bitBuf >>= 1;
	_bitBufLeft--;
	return bit;
}

int DecompressImplode::readBits(int count) {
	int res = 0, pos = 0;
	while (count--)
		res |= readBit() << (pos++);
	return res;
}

void DecompressImplode::putByte(byte value) {
	_bytesWritten++;
	_buffer[_bufferPos++] = value;
	if (_bufferPos == 0x8000)
		flush();
}

void DecompressImplode::flush() {
	memcpy(_dest, _buffer, _bufferPos);
	_dest += _bufferPos;
	_bufferPos = 0;
}

void DecompressImplode::recreateTree(HufNode *&currentTree, byte &len, short *fpos, int *flens, short fmax) {
	HufNode *curplace = currentTree;
	if (len == 17)
		error("DecompressImplode::recreateTree() Invalid huffman tree");
	currentTree++;
	len++;
	while (1) {
		if (fpos[len] >= fmax) {
			curplace->b0 = 0x8000;
			recreateTree(currentTree, len, fpos, flens, fmax);
			break;
		}
		if (flens[fpos[len]] == len) {
			curplace->b0 = fpos[len]++;
			break;
		}
		fpos[len]++;
	}
	while (1) {
		if (fpos[len] >= fmax) {
			curplace->b1 = 0x8000;
			curplace->jump = currentTree;
			recreateTree(currentTree, len, fpos, flens, fmax);
			break;
		}
		if (flens[fpos[len]] == len) {
			curplace->b1 = fpos[len]++;
			curplace->jump = NULL;
			break;
		}
		fpos[len]++;
	}
	len--;
}

int DecompressImplode::decodeSFValue(HufNode *currentTree) {
	HufNode *currNode = currentTree;
	while (1) {
		if (!readBit()) {
			if (!(currNode->b1 & 0x8000))
				return currNode->b1;
			currNode = currNode->jump;
		} else {
			if (!(currNode->b0 & 0x8000))
				return currNode->b0;
			currNode++;
		}
	}
	return -1;
}

int DecompressImplode::createTree(HufNode *currentTree) {
	byte len = 0;
	short fpos[17] = {0,};
	int lengths[256];
	int lengthsCount = 0;
	int treeBytes = readByte() + 1;
	for (int i = 0; i < treeBytes; i++) {
		int a = readByte();
		int bitValues = ((a >> 4) & 0x0F) + 1;
		int bitLength = (a & 0x0F) + 1;
		while (bitValues--)
			lengths[lengthsCount++] = bitLength;
	}
	recreateTree(currentTree, len, fpos, lengths, lengthsCount);
	return 0;
}

int DecompressImplode::decompress(Common::ReadStream *source, int flags, int size, byte *dest) {
	int minMatchLen = flags & 4 ? 3 : 2;
	int distBits = flags & 2 ? 7 : 6;
	memset(_buffer, 0, 32768);
	_bufferPos = 0;
	_bitBufLeft = 0;
	_bytesWritten = 0;
	_source = source;
	_dest = dest;
	if (flags & 4)
		createTree(_literalTree);
	createTree(_lengthTree);
	createTree(_distanceTree);
	while (_bytesWritten < size) {
		if (readBit()) {
			putByte(flags & 4 ? decodeSFValue(_literalTree) : readBits(8));
		} else {
			int distance = (readBits(distBits) | (decodeSFValue(_distanceTree) << distBits)) + 1;
			int len = decodeSFValue(_lengthTree);
			if (len == 63)
				len += readBits(8);
			len += minMatchLen;
			while (len--)
				putByte(_buffer[(_bufferPos - distance) & 0x7FFF]);
		}
	}
	flush();
	return 0;
}
