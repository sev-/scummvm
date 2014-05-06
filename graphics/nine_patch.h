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

#ifndef GRAPHICS_NINE_PATCH_H
#define GRAPHICS_NINE_PATCH_H

#include <allegro5/allegro.h>

namespace Graphics {

struct NINE_PATCH_BITMAP;

struct NINE_PATCH_PADDING {
	int top, right, bottom, left;
};

NINE_PATCH_BITMAP *create_nine_patch_bitmap(ALLEGRO_BITMAP *bmp, bool owns_bitmap);
NINE_PATCH_BITMAP *load_nine_patch_bitmap(const char *filename);
void draw_nine_patch_bitmap(NINE_PATCH_BITMAP *p9, int dx, int dy, int dw, int dh);
ALLEGRO_BITMAP *create_bitmap_from_nine_patch(NINE_PATCH_BITMAP *p9, int w, int h);

int get_nine_patch_bitmap_width(const NINE_PATCH_BITMAP *p9);
int get_nine_patch_bitmap_height(const NINE_PATCH_BITMAP *p9);

int get_nine_patch_bitmap_min_width(const NINE_PATCH_BITMAP *p9);
int get_nine_patch_bitmap_min_height(const NINE_PATCH_BITMAP *p9);

ALLEGRO_BITMAP *get_nine_patch_bitmap_source(const NINE_PATCH_BITMAP *p9);
NINE_PATCH_PADDING get_nine_patch_padding(const NINE_PATCH_BITMAP *p9);

void destroy_nine_patch_bitmap(NINE_PATCH_BITMAP *p9);

} // end of namespace Graphics

#endif // GRAPHICS_NINE_PATCH_H
