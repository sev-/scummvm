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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* This code is based on Nine Patch code by Matthew Leverton 
   taken from https://github.com/konforce/Allegro-Nine-Patch

   Copyright (C) 2011 Matthew Leverton

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */


#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "graphics/nine_patch.h"

namespace Graphics {

~NinePatchSide::NinePatchSide() {
	for (uint i = 0; i < _m.size(); i++)
		delete _m[i];

	_m.clear();
}


bool NinePatchSide::init(Graphics::TransparentSurface *bmp, bool vertical) {
	const int len = vertical ? bmp->h : bmp->w;
	int i, s, t, n, z;
	uint32 c;

	_m.clear();
	
	for (i = 1, s = -1, t = 0, n = 0, z = -1; i < len; ++i) {
		int zz;
		byte r, g, b, a;
		uint32 *color = vertical ? bmp->getBasePtr(0, i) : bmp->getBasePtr(i, 0);
		bmp->format.colorToARGB(*color, &a, &r, &g, &b);
		
		if (i == len - 1)
			zz = -1;
		else if (r == 0 && g == 0 && b == 0 && a == 255)
			zz = 0;
		else if (a == 0 || r + g + b + a == 255 * 4)
			zz = 1;
		else
			return false;
			
		if (z != zz) {
			if (s != -1) {
				NinePatchMark *mrk = new NinePatchMark;

				mrk->offset = s;
				mrk->length = i - s;
				if (z == 0) {
					mrk->ratio = 1;
					t += mrk->length;
				} else {
					mrk->ratio = 0;
				}
				_m->push_back(mrk);
			}
			s = i;
			z = zz;
		}	
	}
	
	_fix = len - 2 - t;
	for (i = 0; i < _m.size(); ++i) {
		if (_m[i].ratio)
			_m[i].ratio = _m[i].length / (float)t;
	}
	
	return true;
}

void NinePatchSide::calcOffsets(int len) {
	int i, j;
	int dest_offset = 0;
	int remaining_stretch = len - _fix;
	
	for (i = 0, j = 0; i < _m.size(); ++i) {
		_m[i].dest_offset = dest_offset;
		if (_m[i].ratio == 0) {
			_m[i].dest_length = _m[i].length;
		} else {
			_m[i].dest_length = (len - _fix) * _m[i].ratio;
			remaining_stretch -= _m[i].dest_length;
			j = i;
		}

		dest_offset += _m[i].dest_length;
	}

	if (remaining_stretch) {
		_m[j].dest_length += remaining_stretch;
		if (j + 1 < _m.size())
			_m[j + 1].dest_offset += remaining_stretch;
	}
}

NinePatchBitmap::NinePatchBitmap(ALLEGRO_BITMAP *bmp, bool owns_bitmap) {
	int i;
	NinePatchBitmap *p9;
	byte r, g, b, a;
	
	_bmp = bmp;
	_destroy_bmp = owns_bitmap;
	_h.m = NULL;
	_v.m = NULL;
	_cached_dw = 0;
	_cached_dh = 0;
	_width = bmp->w - 2;
	_height = bmp->h - 2;
			
	if (_width <= 0 || _height <= 0)
		goto bad_bitmap;
	
	/* make sure all four corners are transparent */
#define _check_pixel(x, y) \
	bmp->format.colorToARGB(bmp->getBasePtr(x, y), &a, &r, &g, &b)); \

	if (a != 0 && r + g + b + a != 4) goto bad_bitmap;
	
	_check_pixel(0,0);
	_check_pixel(bmp->w - 1, 0);
	_check_pixel(0, bmp->h - 1);
	_check_pixel(bmp->w - 1, bmp->h - 1);
#undef _check_pixel

	_padding.top = _padding.right = _padding.bottom = _padding.left = -1;
	
	i = 1;
	while (i < al_get_bitmap_width(bmp)) {
		bmp->format.colorToARGB(bmp->getBasePtr(i, bmp->h - 1), &a, &r, &g, &b));
		
		if (r + g + b == 0 && a == 1) {
			if (_padding.left == -1)
				_padding.left = i - 1;
			else if (_padding.right != -1)
				goto bad_bitmap;
		} else if (a == 0 || r + g + b + a == 4) {
			if (_padding.left != -1 && _padding.right == -1)
				_padding.right = al_get_bitmap_width(bmp) - i - 1;
		}
		++i;
	}
	
	i = 1;
	while (i < al_get_bitmap_height(bmp)) {
		bmp->format.colorToARGB(bmp->getBasePtr(bmp->w - 1, i), &a, &r, &g, &b));
		
		if (r + g + b == 0 && a == 1) {
			if (_padding.top == -1)
				_padding.top = i - 1;
			else if (_padding.bottom != -1)
				goto bad_bitmap;
		} else if (a == 0 || r + g + b + a == 4) {
			if (_padding.top != -1 && _padding.bottom == -1)
				_padding.bottom = bmp->h - i - 1;
		}
		++i;
	}
	
	if (!_h.init(bmp, false) || !_v.init(bmp, true)) {
bad_bitmap:
		if (_h.m) al_free(_h.m);
		if (_v.m) al_free(_v.m);

		return NULL;
	}
}

void NinePatchBitmap::draw(int dx, int dy, int dw, int dh) {
	int i, j;
	
	/* don't draw bitmaps that are smaller than the fixed area */
	if (dw < _h.fix || dh < _v.fix)
		return;

	/* if the bitmap is the same size as the origin, then draw it as-is */		
	if (dw == _width && dh == _height) {
		al_draw_bitmap_region(_bmp, 1, 1, dw, dh, dx, dy, 0);
		return;
	}
		
	/* only recalculate the offsets if they have changed since the last draw */
	if (_cached_dw != dw || _cached_dh != dh) {
		_h.calcOffsets(dw);
		_v.calcOffsets(dh);
		
		_cached_dw = dw;
		_cached_dh = dh;
	}
	
	/* draw each region */
	for (i = 0; i < _v.count; ++i) {
		for (j = 0; j < _h.count; ++j) {
			al_draw_scaled_bitmap(_bmp,
				_h.m[j].offset, _v.m[i].offset,
				_h.m[j].length, _v.m[i].length,
				dx + _h.m[j].dest_offset, dy + _v.m[i].dest_offset,
				_h.m[j].dest_length, _v.m[i].dest_length,
				0
			);
		}
	}
}

ALLEGRO_BITMAP *NinePatch::createBitmap(int w, int h) {
	ALLEGRO_BITMAP *bmp = al_create_bitmap(w, h);
	ALLEGRO_STATE s;
	
	if (!bmp)
		return NULL;
	
	al_store_state(&s, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(bmp);
	al_clear_to_color(al_map_rgba(0,0,0,0));
	draw(0, 0, w, h);
	al_restore_state(&s);
	
	return bmp;
}

NinePatchBitmap::NinePatchBitmap(const char *filename) {
	ALLEGRO_BITMAP *bmp = al_load_bitmap(filename);
	
	return bmp ? create_NinePatchBitmap(bmp, true) : NULL;
}

void NinePatchBitmap::~NinePatchBitmap() {
	if (_destroy_bmp)
		al_destroy_bitmap(_bmp);
}

} // end of namespace Graphics

