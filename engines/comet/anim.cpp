#include "graphics/primitives.h"

#include "comet/comet.h"
#include "comet/anim.h"
#include "comet/pak.h"
#include "comet/screen.h"

namespace Comet {

//DEBUG
const char *commandNames[] = {
	"drawSub",
	"drawFrame",
	"skip",
	"skip",
	"drawFilledPolygon",
	"drawRect",
	"drawPolygon",
	"drawPixels",
	"drawPolygon",
	"drawPolygon",
	"drawRle"
};

Anim::Anim(CometEngine *vm) : _vm(vm), _animData(NULL) {
}

Anim::~Anim() {
	free(_animData);
}

void Anim::load(const char *pakFilename, int fileIndex) {
	free(_animData);
	_animData = loadFromPak(pakFilename, fileIndex);
}

byte *Anim::getSection(int section) {
	return _animData + READ_LE_UINT32(_animData + section * 4);
}

byte *Anim::getSubSection(int section, int subSection) {
	byte *secData = getSection(section);
	return secData + READ_LE_UINT32(secData + subSection * 4);
}

int Anim::getFrameWidth(uint16 index) {
	byte *frame = getSubSection(1, index & 0xFFF);
	return frame[0] * 16;
}

int Anim::getFrameHeight(uint16 index) {
	byte *frame = getSubSection(1, index & 0xFFF);
	return frame[1];
}

void Anim::drawFrame(uint16 index, int x, int y, byte *destBuffer) {

	byte *frame = getSubSection(1, index & 0xFFF);

	int width = *frame++ * 16;
	int lineWidth = width;
	int height = *frame++;
	int skipX = 0;

	index ^= READ_LE_UINT16(frame - 4);

	y -= height;
	y++;

	if (x + width >= 320)
		width = 320 - x;

	if (y + height >= 200)
		height = 200 - y;

	if (y < 0) {
		if (y + height - 1 < 0)
			return;

		height -= -y;

		while (y < 0) {
			skipLine(frame);
			y++;
		}

	}

	if (x < 0) {
		if (x + width - 1 < 0)
			return;
		skipX = -x;
		x = 0;
	}

	if (x >= 320 || y >= 200)
		return;

	byte line[320];

	byte *dest = destBuffer + x + y * 320;
	while (height--) {

		memset(line, 0, lineWidth);

		decompressLine(frame, line);

		if (index & 0x8000) {
			for (int xc = skipX; xc < width; xc++) {
				if (line[lineWidth-xc-1] != 0)
					dest[xc-skipX] = line[lineWidth-xc-1];
			}
		} else {
			for (int xc = skipX; xc < width; xc++) {
				if (line[xc] != 0)
					dest[xc-skipX] = line[xc];
			}
		}

		dest += 320;
	}
	
}

// TODO: Clean up this messy function
void Anim::unpackRle(int x, int y, byte *rleData, byte *destBuffer) {

	int line = 0;
	byte *offsets[200];
	byte bh = 0, bl = 0;
	byte cl = 0, dh = 0, dl = 0;
	bool doMemset = false;

	for (int i = 0; i < 200; i++)
		offsets[i] = destBuffer + x + (y + i) * 320;

	do {

		byte flags = *rleData++;
		byte count = *rleData++;

		if (flags & 0x80)
			line = *rleData++;

		byte pixel = *rleData++;
		dh = 1;

		do {

			byte cmd = *rleData++;

			dl = cmd;
			
			cmd = ((cmd & 0xC0) | (flags & 0x3E)) >> 1;
			
			//debug(2, "cmd = %d", cmd); fflush(stdout);

			switch (cmd) {

			case 0:
			case 1:
			case 8:
			case 9:
			case 22:
			case 23:
				doMemset = (cmd % 2 == 0) ? true : false;	// 0, 8, 22 and 1, 9, 23
				for (int i = 0; i < 6; i++) {
					cl = dh + (dl & 1);
					dl >>= 1;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 11:
			case 13:
			case 15:
			case 35:
			case 57:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl -= (dl & 1);
					dl >>= 1;
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 32:
			case 40:
			case 54:
				for (int i = 0; i < 6; i++) {
					cl = dh - (dl & 1);
					dl >>= 1;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 2:
			case 4:
			case 6:
			case 24:
			case 26:
			case 42:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl += (dl & 1);
					dl >>= 1;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 48:
			case 50:
			case 52:
				bh = dh - (dl & 1);
				dl >>= 1;
				bl = dh - (dl & 1);
				dl >>= 1;
				dh = dh - (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					memset(offsets[line], pixel, bh);
					offsets[line++] += bh;
					memset(offsets[line], pixel, bl);
					offsets[line++] += bl;
					memset(offsets[line], pixel, dh);
					offsets[line++] += dh;
				}
				break;

			case 3:
			case 5:
			case 7:
			case 25:
			case 27:
			case 43:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl += (dl & 1);
					dl >>= 1;
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 10:
			case 12:
			case 14:
			case 34:
			case 56:
				cl = dh;
				for (int i = 0; i < 6; i++) {
					cl -= (dl & 1);
					dl >>= 1;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 16:
			case 18:
			case 20:
				bh = dh + (dl & 1);
				dl >>= 1;
				bl = dh + (dl & 1);
				dl >>= 1;
				dh = dh + (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					memset(offsets[line], pixel, bh);
					offsets[line++] += bh;
					memset(offsets[line], pixel, bl);
					offsets[line++] += bl;
					memset(offsets[line], pixel, dh);
					offsets[line++] += dh;
				}
				break;

			case 17:
			case 19:
			case 21:
				bh = dh + (dl & 1);
				dl >>= 1;
				bl = dh + (dl & 1);
				dl >>= 1;
				dh = dh + (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					offsets[line++] += bh;
					offsets[line++] += bl;
					offsets[line++] += dh;
				}
				break;

			case 28:
			case 60:
			case 92:
			case 124:
				do {
					memset(offsets[line], pixel, count & 7);
					offsets[line++] += count & 7;
					count >>= 3;
				} while (count & 7);
				rleData--;
				count = 1;
				break;

			case 30:
			case 36:
			case 38:
			case 64:
			case 66:
			case 78:
			case 80:
				for (int i = 0; i < 3; i++) {
					cl = dh + (dl & 3);
					dl >>= 2;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 96:
			case 98:
			case 100:
			case 102:
			case 104:
			case 106:
			case 108:
			case 110:
			case 112:
			case 114:
			case 116:
			case 118:
			case 120:
			case 122:
			case 126:
				dh = dl & 0x3F;
				memset(offsets[line], pixel, dh);
				offsets[line++] += dh;
				break;

			case 44:
			case 46:
			case 70:
			case 72:
			case 74:
			case 82:
				for (int i = 0; i < 3; i++) {
					cl = dh - (dl & 3);
					dl >>= 2;
					memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 49:
			case 51:
			case 53:
				bh = dh - (dl & 1);
				dl >>= 1;
				bl = dh - (dl & 1);
				dl >>= 1;
				dh = dh - (dl & 1);
				dl >>= 1;
				dl &= 7;
				dl++;
				while (dl--) {
					offsets[line++] += bh;
					offsets[line++] += bl;
					offsets[line++] += dh;
				}
				break;

			case 33:
			case 41:
			case 55:
				for (int i = 0; i < 6; i++) {
					cl = dh - (dl & 1);
					dl >>= 1;
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 58:
			case 59:
			case 86:
			case 87:
			case 88:
			case 89:
				doMemset = (cmd % 2 == 0) ? true : false;	// 58, 86, 88 and 59, 87, 89
				for (int i = 0; i < 3; i++) {
					cl = dh + (dl & 3) - 1;
					dl >>= 2;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 76:
			case 77:
			case 84:
			case 85:
			case 94:
			case 95:
				doMemset = (cmd % 2 == 0) ? true : false;	// 76, 84, 94 and 77, 85, 95
				for (int i = 0; i < 2; i++) {
					cl = dh + (dl & 7);
					dl >>= 3;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 31:
			case 37:
			case 39:
			case 65:
			case 67:
			case 79:
			case 81:
				for (int i = 0; i < 3; i++) {
					cl = dh + (dl & 3);
					dl >>= 2;
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 62:
			case 63:
			case 68:
			case 69:
			case 90:
			case 91:
				doMemset = (cmd % 2 == 0) ? true : false;	// 62, 68, 90 and 63, 69, 91
				for (int i = 0; i < 2; i++) {
					cl = dh - (dl & 7);
					dl >>= 3;
					if (doMemset)
						memset(offsets[line], pixel, cl);
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 45:
			case 47:
			case 71:
			case 73:
			case 75:
			case 83:
				for (int i = 0; i < 3; i++) {
					cl = dh - (dl & 3);
					dl >>= 2;
					offsets[line++] += cl;
					dh = cl;
				}
				break;

			case 97:
			case 99:
			case 101:
			case 103:
			case 105:
			case 107:
			case 109:
			case 111:
			case 113:
			case 115:
			case 117:
			case 119:
			case 121:
			case 123:
			case 127:
				dh = dl & 0x3F;
				offsets[line++] += dh;
				break;

			case 29:
			case 61:
			case 93:
			case 125:
				do {
					offsets[line++] += count & 7;
					count >>= 3;
				} while (count & 7);
				rleData--;
				count = 1;
				break;

			default:
				error("Anim::unpackRle() Unknown RLE command %d", cmd);
				
			}

		} while (--count > 0);

	} while (*rleData != 0);

}

void Anim::runSeq1(uint16 index, int x, int y) {

	PointArray args;

	byte *seq = getSubSection(0, index & 0xFFF);
	byte flags = *seq++ | ((index >> 8) & 0xA0);
	byte count = *seq++;

	/*
	debug(2, "Anim::runSeq1()  flags = %02X; x = %d; y = %d", flags, x, y);
	*/

	while (count--) {

		byte command = seq[0];
		byte argCount = seq[1];
		byte arg1 = seq[2];
		byte arg2 = seq[3];
		seq += 4;
		
		//debug(2, "runSeq1()  cmd = %d; argCount = %d; arg1 = %02X; arg2 = %02X", command, argCount, arg1, arg2);

		args.clear();

		for (int i = 0; i < argCount; i++) {

			int16 ax, ay;

			if (flags & 0x10) {
				ax = (int8)seq[0];
				ay = (int8)seq[1];
				seq += 2;
			} else {
				ax = (int16)READ_LE_UINT16(seq);
				ay = (int16)READ_LE_UINT16(seq + 2);
				seq += 4;
			}

			if (flags & 0x80)
				ax = -ax;

			if (flags & 0x20)
				ay = -ay;

			//debug(2, "arg: x = %d; y = %d", ax + x, ay + y);

			args.push_back(Common::Point(ax + x, ay + y));

		}

		// TODO: Array

		switch (command) {

		case 0:
			{
				uint16 tempIndex = (arg2 << 8) | arg1;
				runSeq1((flags << 8) | tempIndex, args[0].x, args[0].y);
			}
			break;

		case 1:
			{
				uint16 tempIndex = (arg2 << 8) | arg1;
				if (flags & 0x80)
					args[0].x -= getFrameWidth(tempIndex);
				if (flags & 0x20)
					args[0].y += getFrameHeight(tempIndex);
				drawFrame((flags << 8) | tempIndex, args[0].x, args[0].y, _vm->_screen->getScreen());
			}
			break;

		case 4:
			{
				// _vm->_screen->filledPolygonColor(args, arg2);
				if (arg1 != 0xFF) {
					args.push_back(args[0]);
					arg2 = arg1;
					for (uint i = 0; i < args.size() - 1; i++) {
						Graphics::drawLine(args[i].x, args[i].y, args[i+1].x, args[i+1].y,
							arg2, plotProc, _vm->_screen->getScreen());
					}
				}
			}
			break;

		case 5:
			{
				_vm->_screen->fillRect(args[0].x, args[0].y, args[1].x, args[1].y, arg2);
				if (arg1 != 0xFF)
					_vm->_screen->frameRect(args[0].x, args[0].y, args[1].x, args[1].y, arg1);
			}
			break;

		case 6:
			{
				for (uint i = 0; i < args.size() - 1; i++) {
					_vm->_screen->line(args[i].x, args[i].y, args[i+1].x, args[i+1].y, arg2);
					argCount--;
				}
			}
			break;

		case 7:
			{
				for (uint i = 0; i < args.size(); i++) {
					int pointX = args[i].x;
					int pointY = args[i].y;
					_vm->_screen->putPixel(pointX, pointY, arg2);
				}
			}
			break;
			
		case 10:
			{
				byte *rleData = getSubSection(1, (arg2 << 8) | arg1);
				unpackRle(args[0].x, args[0].y - rleData[1] + 1, rleData + 2, _vm->_screen->getScreen());
			}
			break;

		default:
			error("Anim::runSeq1() Unknown command %d", command);
			
		}

	}

}

void Anim::decompressLine(byte *&sourceBuf, byte *destBuf) {
	byte chunks = *sourceBuf++;
	while (chunks--) {
		byte skip = sourceBuf[0];
		int count = sourceBuf[1] * 4 + sourceBuf[2];
		sourceBuf += 3;
		destBuf += skip;
		memcpy(destBuf, sourceBuf, count);
		destBuf += count;
		sourceBuf += count;
	}
	memset(destBuf, 0, *sourceBuf++);
}

void Anim::skipLine(byte *&sourceBuf) {
	byte chunks = *sourceBuf++;
	while (chunks--) {
		int count = sourceBuf[1] * 4 + sourceBuf[2];
		sourceBuf += 3 + count;
	}
	sourceBuf++;
}

} // End of namespace Comet
