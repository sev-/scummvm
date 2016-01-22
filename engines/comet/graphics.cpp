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
 * $URL$
 * $Id$
 *
 */

#include "comet/graphics.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/primitives.h"

namespace Comet {

static void plotDottedPoint(int x, int y, int color, void *data) {
	DottedLineData *dottedLineData = (DottedLineData*)data;
	Graphics::Surface *s = dottedLineData->_surface;
	if (x >= 0 && x < s->w && y >= 0 && y < s->h) {
		if ((++dottedLineData->_dotCounter) & 2)
			*(byte*)s->getBasePtr(x, y) = color;
	}
}

// CometSurface

CometSurface::CometSurface() {
	create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	setClipRect(0, 0, 320, 200);
}

CometSurface::~CometSurface() {
	free();
}

void CometSurface::grabRect(Graphics::Surface *surface, int x, int y) {
	for (int yc = 0; yc < surface->h; yc++)
		memcpy(surface->getBasePtr(0, yc), getBasePtr(x, y + yc), surface->w);
}

void CometSurface::putRect(Graphics::Surface *surface, int x, int y) {
	for (int yc = 0; yc < surface->h; yc++)
		memcpy(getBasePtr(x, y + yc), surface->getBasePtr(0, yc), surface->w);
}

void CometSurface::clear() {
	Graphics::Surface::fillRect(Common::Rect(0, 0, 320, 200), 0);
}

void CometSurface::putPixel(int x, int y, byte color) {
	if (x >= 0 && x < 320 && y >= 0 && y < 200)
		*(byte*)getBasePtr(x, y) = color;
}

void CometSurface::hLine(int x, int y, int x2, byte color) {	
	Graphics::Surface::hLine(x, y, x2, color);
}

void CometSurface::drawLine(int x1, int y1, int x2, int y2, byte color) {
	Graphics::Surface::drawLine(x1, y1, x2, y2, color);
}

void CometSurface::drawDottedLine(int x1, int y1, int x2, int y2, int color) {
	DottedLineData dottedLineData = {this, 1};
	Graphics::drawLine(x1, y1, x2, y2, color, plotDottedPoint, &dottedLineData);
}

void CometSurface::fillRect(int x1, int y1, int x2, int y2, byte color) {
	Graphics::Surface::fillRect(Common::Rect(x1, y1, x2 + 1, y2 + 1), color);
}

void CometSurface::frameRect(int x1, int y1, int x2, int y2, byte color) {
	Graphics::Surface::frameRect(Common::Rect(x1, y1, x2 + 1, y2 + 1), color);
}

void CometSurface::drawAnimationCelSprite(AnimationCel &cel, int16 x, int16 y, byte flags) {
	byte *frameData = cel.data;

	int width = cel.width;
	int lineWidth = width;
	int height = cel.height;
	int skipX = 0;

	flags ^= (cel.flags >> 8);

	y -= height;
	y++;

	if (x + width >= _clipX2)
		width = _clipX2 - x;

	if (y + height >= _clipY2)
		height = _clipY2 - y;

	if (y < _clipY1) {
		if (y + height - 1 < _clipY1)
			return;
		height -= _clipY1 - y;
		while (y < _clipY1) {
			// Skip the clipped RLE-compressed line
			byte chunks = *frameData++;
			while (chunks--)
				frameData += 3 + frameData[1] * 4 + frameData[2];
			frameData++;
			y++;
		}
	}

	if (x < _clipX1) {
		if (x + width - 1 < _clipX1)
			return;
		skipX = _clipX1 - x;
		x = _clipX1;
	}

	if (x >= _clipX2 || y >= _clipY2)
		return;

	byte *screenDestPtr = (byte*)getBasePtr(x, y);
	byte lineBuffer[320];

	while (height--) {

		memset(lineBuffer, 0, lineWidth);

		// Decompress the current pixel row
		byte chunks = *frameData++;
		byte *lineBufferPtr = lineBuffer;
		while (chunks--) {
			byte skip = frameData[0];
			int count = frameData[1] * 4 + frameData[2];
			frameData += 3;
			lineBufferPtr += skip;
			memcpy(lineBufferPtr, frameData, count);
			lineBufferPtr += count;
			frameData += count;
		}
		frameData++;

		// Draw the decompressed pixels
		if (flags & 0x80) {
			for (int xc = skipX; xc < width; xc++)
				if (lineBuffer[lineWidth-xc-1] != 0)
					screenDestPtr[xc-skipX] = lineBuffer[lineWidth-xc-1];
		} else {
			for (int xc = skipX; xc < width; xc++)
				if (lineBuffer[xc] != 0)
					screenDestPtr[xc-skipX] = lineBuffer[xc];
		}

		screenDestPtr += pitch;
	}

}

void CometSurface::drawAnimationCelRle(AnimationCel &cel, int16 x, int16 y) {
	int line = 0;
	byte *offsets[200];
	byte bh = 0, bl = 0;
	byte cl = 0, dh = 0, dl = 0;
	bool doMemset = false;
	byte *rleData = cel.data;

	for (int yc = 0; yc < 200; yc++)
		offsets[yc] = (byte*)getBasePtr(x, y + yc);

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

			switch (cmd) {

			case 0:
			case 1:
			case 8:
			case 9:
			case 22:
			case 23:
				doMemset = (cmd % 2 == 0) ? true : false;	// 0, 8, 22 and 1, 9, 23
				for (int i = 0; i < 6; ++i) {
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
				for (int i = 0; i < 6; ++i) {
					cl -= (dl & 1);
					dl >>= 1;
					offsets[line++] += cl;
				}
				dh = cl;
				break;

			case 32:
			case 40:
			case 54:
				for (int i = 0; i < 6; ++i) {
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
				for (int i = 0; i < 6; ++i) {
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
				for (int i = 0; i < 6; ++i) {
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
				for (int i = 0; i < 6; ++i) {
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
				for (int i = 0; i < 3; ++i) {
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
				for (int i = 0; i < 3; ++i) {
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
				for (int i = 0; i < 6; ++i) {
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
				for (int i = 0; i < 3; ++i) {
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
				for (int i = 0; i < 2; ++i) {
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
				for (int i = 0; i < 3; ++i) {
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
				for (int i = 0; i < 2; ++i) {
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
				for (int i = 0; i < 3; ++i) {
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
				error("CometSurface::drawAnimationCelRle() Unknown RLE command %d", cmd);

			}

		} while (--count > 0);

	} while (*rleData != 0);

}

void CometSurface::drawFilledPolygon(Common::Array<Common::Point> &poly, byte fillColor, byte outlineColor) {
	drawFilledPolygonBody(poly, fillColor);
	if (outlineColor != 0xFF) {
		poly.push_back(poly[0]);
		drawPolygon(poly, outlineColor);
	}				
}

void CometSurface::drawPolygon(Common::Array<Common::Point> &poly, byte color) {
	for (uint i = 0; i < poly.size() - 1; ++i)
		drawLine(poly[i].x, poly[i].y, poly[i + 1].x, poly[i + 1].y, color);
}

void CometSurface::drawRectangle(Common::Array<Common::Point> &points, byte fillColor, byte outlineColor) {
	fillRect(points[0].x, points[0].y, points[1].x, points[1].y, fillColor);
	if (outlineColor != 0xFF)
		frameRect(points[0].x, points[0].y, points[1].x, points[1].y, outlineColor);
}

void CometSurface::drawPixels(Common::Array<Common::Point> &points, byte color) {
	for (uint i = 0; i < points.size(); ++i)
		putPixel(points[i].x, points[i].y, color);
}

void CometSurface::setClipRect(int clipX1, int clipY1, int clipX2, int clipY2) {
	_clipX1 = clipX1;
	_clipY1 = clipY1;
	_clipX2 = clipX2;
	_clipY2 = clipY2;
}

void CometSurface::setClipX(int clipX1, int clipX2) {
	_clipX1 = clipX1;
	_clipX2 = clipX2;
}

void CometSurface::setClipY(int clipY1, int clipY2) {
	_clipY1 = clipY1;
	_clipY2 = clipY2;
}

void CometSurface::clipPolygonLeft(Common::Array<Common::Point> **poly, int16 clipLeft) {
	Common::Array<Common::Point> *points = *poly; 
	Common::Array<Common::Point> *clipPoints = new Common::Array<Common::Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currX < clipLeft) {
			doClip = newX >= clipLeft;
		} else {
			clipPoints->push_back(Common::Point(currX, currY));
			doClip = newX < clipLeft;
		}
		if (doClip) {
			if (newX <= currX) {
				SWAP(newX, currX);
				SWAP(newY, currY);
			}
			clipPoints->push_back(Common::Point(
				clipLeft,
				-(currX - clipLeft) * (newY - currY) / (newX - currX) + currY));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void CometSurface::clipPolygonRight(Common::Array<Common::Point> **poly, int16 clipRight) {
	Common::Array<Common::Point> *points = *poly; 
	Common::Array<Common::Point> *clipPoints = new Common::Array<Common::Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currX > clipRight) {
			doClip = newX <= clipRight;
		} else {
			clipPoints->push_back(Common::Point(currX, currY));
			doClip = newX > clipRight;
		}
		if (doClip) {
			if (newX <= currX) {
				SWAP(newX, currX);
				SWAP(newY, currY);
			}
			clipPoints->push_back(Common::Point(
				clipRight,
				-(currX - clipRight) * (newY - currY) / (newX - currX) + currY));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void CometSurface::clipPolygonTop(Common::Array<Common::Point> **poly, int16 clipTop) {
	Common::Array<Common::Point> *points = *poly; 
	Common::Array<Common::Point> *clipPoints = new Common::Array<Common::Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currY < clipTop) {
			doClip = newY >= clipTop;
		} else {
			clipPoints->push_back(Common::Point(currX, currY));
			doClip = newY < clipTop;
		}
		if (doClip) {
			clipPoints->push_back(Common::Point(
				-(currY - clipTop) * (newX - currX) / (newY - currY) + currX, 
				clipTop));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void CometSurface::clipPolygonBottom(Common::Array<Common::Point> **poly, int16 clipBottom) {
	Common::Array<Common::Point> *points = *poly; 
	Common::Array<Common::Point> *clipPoints = new Common::Array<Common::Point>();
	int16 currX, currY;
	currX = (*points)[0].x;
	currY = (*points)[0].y;
	for (uint ptIndex = 1; ptIndex < points->size(); ptIndex++) {
		int16 newX, newY;
		bool doClip = false;
		newX = (*points)[ptIndex].x;
		newY = (*points)[ptIndex].y;
		if (currY > clipBottom) {
			doClip = newY <= clipBottom;
		} else {
			clipPoints->push_back(Common::Point(currX, currY));
			doClip = newY > clipBottom;
		}
		if (doClip) {
			clipPoints->push_back(Common::Point(
				(-(currY - clipBottom)) * (newX - currX) / (newY - currY) + currX, 
				clipBottom));
		}
		currX = (*points)[ptIndex].x;
		currY = (*points)[ptIndex].y;
	}
	if (clipPoints->size() > 0)
		clipPoints->push_back((*clipPoints)[0]);
	delete points;
	*poly = clipPoints;	
}

void CometSurface::drawFilledPolygonBody(Common::Array<Common::Point> &poly, byte color) {

	Common::Array<Common::Point> *workPoints = new Common::Array<Common::Point>();
	int16 x1buffer[200], x2buffer[200];
	int16 minX = 32767, maxX = -32767;
	int16 minY = 32767, maxY = -32767;
	int16 currX, currY;
	int16 *x1dst, *x2dst;
	bool clipped = false;

	workPoints->reserve(poly.size() + 1);
	for (uint i = 0; i < poly.size(); ++i) {
		Common::Point pt = poly[i];
		if (pt.x < minX) minX = pt.x;
		if (pt.x > maxX) maxX = pt.x;
		if (pt.y < minY) minY = pt.y;
		if (pt.y > maxY) maxY = pt.y;
		workPoints->push_back(pt);
	}
	workPoints->push_back((*workPoints)[0]);
					  
	if (minY == maxY) {
		delete workPoints;
		return;
	}

	if (minX < _clipX1 && workPoints->size() > 2) {
		clipPolygonLeft(&workPoints, _clipX1);
		clipped = true;
	}

	if (maxX >= _clipX2 && workPoints->size() > 2) {
		clipPolygonRight(&workPoints, _clipX2 - 1);
		clipped = true;
	}

	if (minY < _clipY1 && workPoints->size() > 2) {
		clipPolygonTop(&workPoints, _clipY1);
		clipped = true;
	}
	
	if (maxY >= _clipY2 && workPoints->size() > 2) {
		clipPolygonBottom(&workPoints, _clipY2 - 1);
		clipped = true;
	}

	if (workPoints->size() <= 2) {
		delete workPoints;
		return;
	}

	if (clipped) {
		// Recalc min/max y
		minX = 32767;
		maxX = -32767;
		minY = 32767;
		maxY = -32767;
		for (uint i = 0; i < workPoints->size(); ++i) {
			Common::Point pt = (*workPoints)[i];
			if (pt.x < minX) minX = pt.x;
			if (pt.x > maxX) maxX = pt.x;
			if (pt.y < minY) minY = pt.y;
			if (pt.y > maxY) maxY = pt.y;
		}
	}

	x1dst = x1buffer;
	x2dst = x2buffer;
	currX = (*workPoints)[0].x;
	currY = (*workPoints)[0].y;
	for (uint i = 1; i < workPoints->size(); ++i) {
		int16 newX, newY, sax, sdx;
		newX = (*workPoints)[i].x;
		newY = (*workPoints)[i].y;
		sdx = newX;
		sax = newY;
		if (newY != currY) {
			int16 err;
			if (currY == minY || currY == maxY)
				SWAP(x1dst, x2dst);
			if (currX >= newX) {
				SWAP(currX, newX);
				SWAP(currY, newY);
			}
			int16 *di = x2dst + currY;
			int dir = 1;
			if (currY >= newY) {
				dir = -1;
				SWAP(currY, newY);
			}
			SWAP(currY, newY);
			currY -= newY;
			*di = currX;
			di += dir;
			newX -= currX;
			err = currY / 2;
			for (int y = 0; y < currY; y++) {
				err += newX;
				while (err >= currY) {
					err -= currY;
					currX++;
				}
				*di = currX;
				di += dir;
			}
		}
		currX = sdx;
		currY = sax;
	}

	int16 *x1src = x1dst + minY;
	int16 *x2src = x2dst + minY;
	int16 rowCount = maxY - minY + 1;
	do {
		int16 x1 = *x1src++;
		int16 x2 = *x2src++;
		if (x1 > x2) SWAP(x1, x2);
		hLine(x1, minY, x2, color);
		minY++;
	} while (--rowCount);

	delete workPoints;
	
}

} // End of namespace Comet
