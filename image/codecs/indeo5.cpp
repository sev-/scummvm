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
 *
 */

#include "common/scummsys.h"

/* Intel Indeo 5 decompressor, derived from ffmpeg.
 *
 * Original copyright note:
 * Indeo Video Interactive v5 compatible decoder
 * Copyright (c) 2009 Maxim Poliakovski
 */

/**
 * @file
 * Indeo Video Interactive version 5 decoder
 *
 * Indeo5 data is usually transported within .avi or .mov files.
 * Known FOURCCs: 'IV50'
 */

#include "common/system.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/bitstream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/yuv_to_rgb.h"

#include "image/codecs/indeo5.h"

namespace Image {

#define BITSTREAM_READER_LE

#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)

/**
 *  Indeo5 frame types.
 */
enum {
	FRAMETYPE_INTRA       = 0,
	FRAMETYPE_INTER       = 1,  ///< non-droppable P-frame
	FRAMETYPE_INTER_SCAL  = 2,  ///< droppable P-frame used in the scalability mode
	FRAMETYPE_INTER_NOREF = 3,  ///< droppable P-frame
	FRAMETYPE_NULL        = 4   ///< empty frame with no data
};

#define IVI5_PIC_SIZE_ESC       15

/**
 *  standard picture dimensions (width, height divided by 4)
 */
static const uint8 ivi5_common_pic_sizes[30] = {
	160, 120, 80, 60, 40, 30, 176, 120, 88, 60, 88, 72, 44, 36, 60, 45, 160, 60,
	176,  60, 20, 15, 22, 18,   0,   0,  0,  0,  0,  0
};


/**
 *  Indeo5 dequantization matrixes consist of two tables: base table
 *  and scale table. The base table defines the dequantization matrix
 *  itself and the scale table tells how this matrix should be scaled
 *  for a particular quant level (0...24).
 *
 *  ivi5_base_quant_bbb_ttt  - base  tables for block size 'bbb' of type 'ttt'
 *  ivi5_scale_quant_bbb_ttt - scale tables for block size 'bbb' of type 'ttt'
 */
static const uint16 ivi5_base_quant_8x8_inter[5][64] = {
	{	0x26, 0x3a, 0x3e, 0x46, 0x4a, 0x4e, 0x52, 0x5a, 0x3a, 0x3e, 0x42, 0x46, 0x4a, 0x4e, 0x56, 0x5e,
		0x3e, 0x42, 0x46, 0x48, 0x4c, 0x52, 0x5a, 0x62, 0x46, 0x46, 0x48, 0x4a, 0x4e, 0x56, 0x5e, 0x66,
		0x4a, 0x4a, 0x4c, 0x4e, 0x52, 0x5a, 0x62, 0x6a, 0x4e, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x66, 0x6e,
		0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x72, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x6e, 0x72, 0x76,
	},
	{	0x26, 0x3a, 0x3e, 0x46, 0x4a, 0x4e, 0x52, 0x5a, 0x3a, 0x3e, 0x42, 0x46, 0x4a, 0x4e, 0x56, 0x5e,
		0x3e, 0x42, 0x46, 0x48, 0x4c, 0x52, 0x5a, 0x62, 0x46, 0x46, 0x48, 0x4a, 0x4e, 0x56, 0x5e, 0x66,
		0x4a, 0x4a, 0x4c, 0x4e, 0x52, 0x5a, 0x62, 0x6a, 0x4e, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x66, 0x6e,
		0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x72, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x6e, 0x72, 0x76,
	},
	{	0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
		0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
		0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
		0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
	},
	{	0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4,
		0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
		0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
	},
	{	0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
		0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
		0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
		0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
	}
};

static const uint16 ivi5_base_quant_8x8_intra[5][64] = {
	{	0x1a, 0x2e, 0x36, 0x42, 0x46, 0x4a, 0x4e, 0x5a, 0x2e, 0x32, 0x3e, 0x42, 0x46, 0x4e, 0x56, 0x6a,
		0x36, 0x3e, 0x3e, 0x44, 0x4a, 0x54, 0x66, 0x72, 0x42, 0x42, 0x44, 0x4a, 0x52, 0x62, 0x6c, 0x7a,
		0x46, 0x46, 0x4a, 0x52, 0x5e, 0x66, 0x72, 0x8e, 0x4a, 0x4e, 0x54, 0x62, 0x66, 0x6e, 0x86, 0xa6,
		0x4e, 0x56, 0x66, 0x6c, 0x72, 0x86, 0x9a, 0xca, 0x5a, 0x6a, 0x72, 0x7a, 0x8e, 0xa6, 0xca, 0xfe,
	},
	{	0x26, 0x3a, 0x3e, 0x46, 0x4a, 0x4e, 0x52, 0x5a, 0x3a, 0x3e, 0x42, 0x46, 0x4a, 0x4e, 0x56, 0x5e,
		0x3e, 0x42, 0x46, 0x48, 0x4c, 0x52, 0x5a, 0x62, 0x46, 0x46, 0x48, 0x4a, 0x4e, 0x56, 0x5e, 0x66,
		0x4a, 0x4a, 0x4c, 0x4e, 0x52, 0x5a, 0x62, 0x6a, 0x4e, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x66, 0x6e,
		0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x72, 0x5a, 0x5e, 0x62, 0x66, 0x6a, 0x6e, 0x72, 0x76,
	},
	{	0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
		0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
		0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
		0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2, 0x4e, 0xaa, 0xf2, 0xd4, 0xde, 0xc2, 0xd6, 0xc2,
	},
	{	0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
		0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4, 0xd4,
		0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xde, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
		0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xd6, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2,
	},
	{	0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
		0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
		0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
		0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e, 0x5e,
	}
};

static const uint16 ivi5_base_quant_4x4_inter[16] = {
	0x1e, 0x3e, 0x4a, 0x52, 0x3e, 0x4a, 0x52, 0x56, 0x4a, 0x52, 0x56, 0x5e, 0x52, 0x56, 0x5e, 0x66
};

static const uint16 ivi5_base_quant_4x4_intra[16] = {
	0x1e, 0x3e, 0x4a, 0x52, 0x3e, 0x4a, 0x52, 0x5e, 0x4a, 0x52, 0x5e, 0x7a, 0x52, 0x5e, 0x7a, 0x92
};


static const uint8 ivi5_scale_quant_8x8_inter[5][24] = {
	{	0x0b, 0x11, 0x13, 0x14, 0x15, 0x16, 0x18, 0x1a, 0x1b, 0x1d, 0x20, 0x22,
		0x23, 0x25, 0x28, 0x2a, 0x2e, 0x32, 0x35, 0x39, 0x3d, 0x41, 0x44, 0x4a,
	},
	{	0x07, 0x14, 0x16, 0x18, 0x1b, 0x1e, 0x22, 0x25, 0x29, 0x2d, 0x31, 0x35,
		0x3a, 0x3f, 0x44, 0x4a, 0x50, 0x56, 0x5c, 0x63, 0x6a, 0x71, 0x78, 0x7e,
	},
	{	0x15, 0x25, 0x28, 0x2d, 0x30, 0x34, 0x3a, 0x3d, 0x42, 0x48, 0x4c, 0x51,
		0x56, 0x5b, 0x60, 0x65, 0x6b, 0x70, 0x76, 0x7c, 0x82, 0x88, 0x8f, 0x97,
	},
	{	0x13, 0x1f, 0x20, 0x22, 0x25, 0x28, 0x2b, 0x2d, 0x30, 0x33, 0x36, 0x39,
		0x3c, 0x3f, 0x42, 0x45, 0x48, 0x4b, 0x4e, 0x52, 0x56, 0x5a, 0x5e, 0x62,
	},
	{	0x3c, 0x52, 0x58, 0x5d, 0x63, 0x68, 0x68, 0x6d, 0x73, 0x78, 0x7c, 0x80,
		0x84, 0x89, 0x8e, 0x93, 0x98, 0x9d, 0xa3, 0xa9, 0xad, 0xb1, 0xb5, 0xba,
	},
};

static const uint8 ivi5_scale_quant_8x8_intra[5][24] = {
	{	0x0b, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x17, 0x18, 0x1a, 0x1c, 0x1e, 0x20,
		0x22, 0x24, 0x27, 0x28, 0x2a, 0x2d, 0x2f, 0x31, 0x34, 0x37, 0x39, 0x3c,
	},
	{	0x01, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1b, 0x1e, 0x22, 0x25, 0x28, 0x2c,
		0x30, 0x34, 0x38, 0x3d, 0x42, 0x47, 0x4c, 0x52, 0x58, 0x5e, 0x65, 0x6c,
	},
	{	0x13, 0x22, 0x27, 0x2a, 0x2d, 0x33, 0x36, 0x3c, 0x41, 0x45, 0x49, 0x4e,
		0x53, 0x58, 0x5d, 0x63, 0x69, 0x6f, 0x75, 0x7c, 0x82, 0x88, 0x8e, 0x95,
	},
	{	0x13, 0x1f, 0x21, 0x24, 0x27, 0x29, 0x2d, 0x2f, 0x34, 0x37, 0x3a, 0x3d,
		0x40, 0x44, 0x48, 0x4c, 0x4f, 0x52, 0x56, 0x5a, 0x5e, 0x62, 0x66, 0x6b,
	},
	{	0x31, 0x42, 0x47, 0x47, 0x4d, 0x52, 0x58, 0x58, 0x5d, 0x63, 0x67, 0x6b,
		0x6f, 0x73, 0x78, 0x7c, 0x80, 0x84, 0x89, 0x8e, 0x93, 0x98, 0x9d, 0xa4,
	}
};

static const uint8 ivi5_scale_quant_4x4_inter[24] = {
	0x0b, 0x0d, 0x0d, 0x0e, 0x11, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
};

static const uint8 ivi5_scale_quant_4x4_intra[24] = {
	0x01, 0x0b, 0x0b, 0x0d, 0x0d, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
};

static const uint8 ff_zigzag_direct[64] = {
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

int ff_ivi_init_planes(IVIPlaneDesc *planes, const IVIPicConfig *cfg);
static void skip_hdr_extension(Common::BitStream *gb);

/**
 *  Decode Indeo5 GOP (Group of pictures) header.
 *  This header is present in key frames only.
 *  It defines parameters for all frames in a GOP.
 *
 *  @param[in,out] ctx    ptr to the decoder context
 *  @return         result code: 0 = OK, -1 = error
 */
int Indeo5Decoder::decode_gop_header() {
	int             result, i, p, tile_size, pic_size_indx, mb_size, blk_size, is_scalable;
	int             quant_mat, blk_size_changed = 0;
	IVIBandDesc     *band, *band1, *band2;
	IVIPicConfig    pic_conf;

	_ctx->gop_flags = _ctx->gb->getBits(8);

	_ctx->gop_hdr_size = (_ctx->gop_flags & 1) ? _ctx->gb->getBits(16) : 0;

	if (_ctx->gop_flags & IVI5_IS_PROTECTED)
		_ctx->lock_word = _ctx->gb->getBits(32);

	tile_size = (_ctx->gop_flags & 0x40) ? 64 << _ctx->gb->getBits(2) : 0;
	if (tile_size > 256) {
		error("Indeo5decoder: Invalid tile size: %d\n", tile_size);
		return -1;
	}

	/* decode number of wavelet bands */
	/* num_levels * 3 + 1 */
	pic_conf.luma_bands   = _ctx->gb->getBits(2) * 3 + 1;
	pic_conf.chroma_bands = _ctx->gb->getBit()   * 3 + 1;
	is_scalable = pic_conf.luma_bands != 1 || pic_conf.chroma_bands != 1;
	if (is_scalable && (pic_conf.luma_bands != 4 || pic_conf.chroma_bands != 1)) {
		error("Indeo5decoder: Scalability: unsupported subdivision! Luma bands: %d, chroma bands: %d\n",
			  pic_conf.luma_bands, pic_conf.chroma_bands);
		return -1;
	}

	pic_size_indx = _ctx->gb->getBits(4);
	if (pic_size_indx == IVI5_PIC_SIZE_ESC) {
		pic_conf.pic_height = _ctx->gb->getBits(13);
		pic_conf.pic_width  = _ctx->gb->getBits(13);
	} else {
		pic_conf.pic_height = ivi5_common_pic_sizes[pic_size_indx * 2 + 1] << 2;
		pic_conf.pic_width  = ivi5_common_pic_sizes[pic_size_indx * 2    ] << 2;
	}

	if (_ctx->gop_flags & 2) {
		error("Indeo5decoder: YV12 picture format is not implemented");
		return -2;
	}

	pic_conf.chroma_height = (pic_conf.pic_height + 3) >> 2;
	pic_conf.chroma_width  = (pic_conf.pic_width  + 3) >> 2;

	if (!tile_size) {
		pic_conf.tile_height = pic_conf.pic_height;
		pic_conf.tile_width  = pic_conf.pic_width;
	} else {
		pic_conf.tile_height = pic_conf.tile_width = tile_size;
	}

	/* check if picture layout was changed and reallocate buffers */
	if (ivi_pic_config_cmp(&pic_conf, &_ctx->pic_conf) || _ctx->gop_invalid) {
		result = ff_ivi_init_planes(_ctx->planes, &pic_conf);
		if (result < 0) {
			error("Indeo5decoder: Couldn't reallocate color planes!\n");
			return result;
		}
		_ctx->pic_conf = pic_conf;
		_ctx->is_scalable = is_scalable;
		blk_size_changed = 1; /* force reallocation of the internal structures */
	}

	for (p = 0; p <= 1; p++) {
		for (i = 0; i < (!p ? pic_conf.luma_bands : pic_conf.chroma_bands); i++) {
			band = &_ctx->planes[p].bands[i];

			band->is_halfpel = _ctx->gb->getBit();

			mb_size  = _ctx->gb->getBit();
			blk_size = 8 >> _ctx->gb->getBit();
			mb_size  = blk_size << !mb_size;

			if (p == 0 && blk_size == 4) {
				error("Indeo5decoder: 4x4 luma blocks are unsupported!");
				return -2;
			}

			blk_size_changed = mb_size != band->mb_size || blk_size != band->blk_size;
			if (blk_size_changed) {
				band->mb_size  = mb_size;
				band->blk_size = blk_size;
			}

			if (_ctx->gb->getBit()) {
				error("Indeo5decoder: Extended transform info");
				return -2;
			}

			/* select transform function and scan pattern according to plane and band number */
			switch ((p << 2) + i) {
			case 0:
				band->inv_transform  = ff_ivi_inverse_slant_8x8;
				band->dc_transform   = ff_ivi_dc_slant_2d;
				band->scan           = ff_zigzag_direct;
				band->transform_size = 8;
				break;

			case 1:
				band->inv_transform  = ff_ivi_row_slant8;
				band->dc_transform   = ff_ivi_dc_row_slant;
				band->scan           = ff_ivi_vertical_scan_8x8;
				band->transform_size = 8;
				break;

			case 2:
				band->inv_transform  = ff_ivi_col_slant8;
				band->dc_transform   = ff_ivi_dc_col_slant;
				band->scan           = ff_ivi_horizontal_scan_8x8;
				band->transform_size = 8;
				break;

			case 3:
				band->inv_transform  = ff_ivi_put_pixels_8x8;
				band->dc_transform   = ff_ivi_put_dc_pixel_8x8;
				band->scan           = ff_ivi_horizontal_scan_8x8;
				band->transform_size = 8;
				break;

			case 4:
				band->inv_transform  = ff_ivi_inverse_slant_4x4;
				band->dc_transform   = ff_ivi_dc_slant_2d;
				band->scan           = ff_ivi_direct_scan_4x4;
				band->transform_size = 4;
				break;
			}

			band->is_2d_trans = band->inv_transform == ff_ivi_inverse_slant_8x8 ||
				band->inv_transform == ff_ivi_inverse_slant_4x4;

			if (band->transform_size != band->blk_size) {
				error("Indeo5decoder: transform and block size mismatch (%d != %d)", band->transform_size, band->blk_size);
				return -1;
			}

			/* select dequant matrix according to plane and band number */
			if (!p) {
				quant_mat = (pic_conf.luma_bands > 1) ? i + 1 : 0;
			} else {
				quant_mat = 5;
			}

			if (band->blk_size == 8) {
				if (quant_mat >= 5) {
					error("Indeo5decoder: quant_mat %d too large!", quant_mat);
					return -1;
				}
				band->intra_base  = &ivi5_base_quant_8x8_intra[quant_mat][0];
				band->inter_base  = &ivi5_base_quant_8x8_inter[quant_mat][0];
				band->intra_scale = &ivi5_scale_quant_8x8_intra[quant_mat][0];
				band->inter_scale = &ivi5_scale_quant_8x8_inter[quant_mat][0];
			} else {
				band->intra_base  = ivi5_base_quant_4x4_intra;
				band->inter_base  = ivi5_base_quant_4x4_inter;
				band->intra_scale = ivi5_scale_quant_4x4_intra;
				band->inter_scale = ivi5_scale_quant_4x4_inter;
			}

			if (_ctx->gb->getBits(2)) {
				error("Indeo5decoder: End marker missing!");
				return -1;
			}
		}
	}

	/* copy chroma parameters into the 2nd chroma plane */
	for (i = 0; i < pic_conf.chroma_bands; i++) {
		band1 = &_ctx->planes[1].bands[i];
		band2 = &_ctx->planes[2].bands[i];

		band2->width         = band1->width;
		band2->height        = band1->height;
		band2->mb_size       = band1->mb_size;
		band2->blk_size      = band1->blk_size;
		band2->is_halfpel    = band1->is_halfpel;
		band2->intra_base    = band1->intra_base;
		band2->inter_base    = band1->inter_base;
		band2->intra_scale   = band1->intra_scale;
		band2->inter_scale   = band1->inter_scale;
		band2->scan          = band1->scan;
		band2->inv_transform = band1->inv_transform;
		band2->dc_transform  = band1->dc_transform;
		band2->is_2d_trans   = band1->is_2d_trans;
		band2->transform_size = band1->transform_size;
	}

	/* reallocate internal structures if needed */
	if (blk_size_changed) {
		result = ff_ivi_init_tiles(_ctx->planes, pic_conf.tile_width,
								   pic_conf.tile_height);
		if (result < 0) {
			error("Indeo5decoder: Couldn't reallocate internal structures!");
			return result;
		}
	}

	if (_ctx->gop_flags & 8) {
		if (_ctx->gb->getBits(3)) {
			error("Indeo5decoder: Alignment bits are not zero!");
			return -1;
		}

		if (_ctx->gb->getBit())
			_ctx->gb->skip(24); /* skip transparency fill color */
	}

	_ctx->gb->align();

	_ctx->gb->skip(23); /* FIXME: unknown meaning */

	/* skip GOP extension if any */
	if (_ctx->gb->getBit()) {
		do {
			i = _ctx->gb->getBits(16);
		} while (i & 0x8000);
	}

	_ctx->gb->align();

	return 0;
}


/**
 *  Skip a header extension.
 *
 *  @param[in,out]  gb  the GetBit context
 */
static void skip_hdr_extension(Common::BitStream *gb) {
	int i, len;

	do {
		len = gb->getBits(8);
		for (i = 0; i < len; i++) gb->skip(8);
	} while (len);
}


/**
 *  Decode Indeo5 picture header.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @return         result code: 0 = OK, -1 = error
 */
int Indeo5Decoder::decode_pic_hdr() {
	int ret;

	if (_ctx->gb->getBits(5) != 0x1F) {
		error("Indeo5decoder: Invalid picture start code!");
		return -1;
	}

	_ctx->prev_frame_type = _ctx->frame_type;
	_ctx->frame_type      = _ctx->gb->getBits(3);
	if (_ctx->frame_type >= 5) {
		error("Indeo5decoder: Invalid frame type: %d", _ctx->frame_type);
		return -1;
	}

	_ctx->frame_num = _ctx->gb->getBits(8);

	if (_ctx->frame_type == FRAMETYPE_INTRA) {
		if ((ret = decode_gop_header()) < 0) {
			error("Indeo5decoder: Invalid GOP header, skipping frames");
			_ctx->gop_invalid = 1;
			return ret;
		}
		_ctx->gop_invalid = 0;
	}

	if (_ctx->frame_type == FRAMETYPE_INTER_SCAL && !_ctx->is_scalable) {
		error("Indeo5decoder: Scalable inter frame in non scalable stream");
		_ctx->frame_type = FRAMETYPE_INTER;
		return -1;
	}

	if (_ctx->frame_type != FRAMETYPE_NULL) {
		_ctx->frame_flags = _ctx->gb->getBits(8);

		_ctx->pic_hdr_size = (_ctx->frame_flags & 1) ? _ctx->gb->getBits(24) : 0;

		_ctx->checksum = (_ctx->frame_flags & 0x10) ? _ctx->gb->getBits(16) : 0;

		/* skip unknown extension if any */
		if (_ctx->frame_flags & 0x20)
			skip_hdr_extension(_ctx->gb); /* XXX: untested */

		/* decode macroblock huffman codebook */
		ret = ff_ivi_dec_huff_desc(_ctx->gb, _ctx->frame_flags & 0x40,
								   IVI_MB_HUFF, &_ctx->mb_vlc);
		if (ret < 0)
			return ret;

		_ctx->gb->skip(3); /* FIXME: unknown meaning! */
	}

	_ctx->gb->align();

	return 0;
}


/**
 *  Decode Indeo5 band header.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @param[in,out]  band   ptr to the band descriptor
 *  @return         result code: 0 = OK, -1 = error
 */
int Indeo5Decoder::decode_band_hdr(IVIBandDesc *band) {
	int         i, ret;
	uint8     band_flags;

	band_flags = _ctx->gb->getBits(8);

	if (band_flags & 1) {
		band->is_empty = 1;
		return 0;
	}

	band->data_size = (_ctx->frame_flags & 0x80) ? _ctx->gb->getBits(24) : 0;

	band->inherit_mv     = band_flags & 2;
	band->inherit_qdelta = band_flags & 8;
	band->qdelta_present = band_flags & 4;
	if (!band->qdelta_present) band->inherit_qdelta = 1;

	/* decode rvmap probability corrections if any */
	band->num_corr = 0; /* there are no corrections */
	if (band_flags & 0x10) {
		band->num_corr = _ctx->gb->getBits(8); /* get number of correction pairs */
		if (band->num_corr > 61) {
			error("Indeo5decoder: Too many corrections: %d", band->num_corr);
			return -1;
		}

		/* read correction pairs */
		for (i = 0; i < band->num_corr * 2; i++)
			band->corr[i] = _ctx->gb->getBits(8);
	}

	/* select appropriate rvmap table for this band */
	band->rvmap_sel = (band_flags & 0x40) ? _ctx->gb->getBits(3) : 8;

	/* decode block huffman codebook */
	ret = ff_ivi_dec_huff_desc(_ctx->gb, band_flags & 0x80, IVI_BLK_HUFF,
							   &band->blk_vlc);
	if (ret < 0)
		return ret;

	band->checksum_present = _ctx->gb->getBit();
	if (band->checksum_present)
		band->checksum = _ctx->gb->getBits(16);

	band->glob_quant = _ctx->gb->getBits(5);

	/* skip unknown extension if any */
	if (band_flags & 0x20) { /* XXX: untested */
		_ctx->gb->align();
		skip_hdr_extension(_ctx->gb);
	}

	_ctx->gb->align();

	return 0;
}


/**
 *  Decode info (block type, cbp, quant delta, motion vector)
 *  for all macroblocks in the current tile.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @param[in,out]  band   ptr to the band descriptor
 *  @param[in,out]  tile   ptr to the tile descriptor
 *  @return         result code: 0 = OK, -1 = error
 */
int Indeo5Decoder::decode_mb_info(IVIBandDesc *band, IVITile *tile) {
	int         x, y, mv_x, mv_y, mv_delta, offs, mb_offset,
		mv_scale, blks_per_mb, s;
	IVIMbInfo   *mb, *ref_mb;
	int         row_offset = band->mb_size * band->pitch;

	mb     = tile->mbs;
	ref_mb = tile->ref_mbs;
	offs   = tile->ypos * band->pitch + tile->xpos;

	if (!ref_mb &&
		((band->qdelta_present && band->inherit_qdelta) || band->inherit_mv))
		return -1;

	if (tile->num_MBs != IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size)) {
		error("Indeo5decoder: Allocated tile size %d mismatches parameters %d",
			  tile->num_MBs, IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size));
		return -1;
	}

	/* scale factor for motion vectors */
	mv_scale = (_ctx->planes[0].bands[0].mb_size >> 3) - (band->mb_size >> 3);
	mv_x = mv_y = 0;

	for (y = tile->ypos; y < (tile->ypos + tile->height); y += band->mb_size) {
		mb_offset = offs;

		for (x = tile->xpos; x < (tile->xpos + tile->width); x += band->mb_size) {
			mb->xpos     = x;
			mb->ypos     = y;
			mb->buf_offs = mb_offset;

			if (_ctx->gb->getBit()) {
				if (_ctx->frame_type == FRAMETYPE_INTRA) {
					error("Indeo5decoder: Empty macroblock in an INTRA picture!");
					return -1;
				}
				mb->type = 1; /* empty macroblocks are always INTER */
				mb->cbp  = 0; /* all blocks are empty */

				mb->q_delta = 0;
				if (!band->plane && !band->band_num && (_ctx->frame_flags & 8)) {
					mb->q_delta = get_vlc2(&_ctx->gb, _ctx->mb_vlc.tab->table,
										   IVI_VLC_BITS, 1);
					mb->q_delta = IVI_TOSIGNED(mb->q_delta);
				}

				mb->mv_x = mb->mv_y = 0; /* no motion vector coded */
				if (band->inherit_mv && ref_mb) {
					/* motion vector inheritance */
					if (mv_scale) {
						mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
						mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
					} else {
						mb->mv_x = ref_mb->mv_x;
						mb->mv_y = ref_mb->mv_y;
					}
				}
			} else {
				if (band->inherit_mv && ref_mb) {
					mb->type = ref_mb->type; /* copy mb_type from corresponding reference mb */
				} else if (_ctx->frame_type == FRAMETYPE_INTRA) {
					mb->type = 0; /* mb_type is always INTRA for intra-frames */
				} else {
					mb->type = _ctx->gb->getBit();
				}

				blks_per_mb = band->mb_size != band->blk_size ? 4 : 1;
				mb->cbp = _ctx->gb->getBits(blks_per_mb);

				mb->q_delta = 0;
				if (band->qdelta_present) {
					if (band->inherit_qdelta) {
						if (ref_mb) mb->q_delta = ref_mb->q_delta;
					} else if (mb->cbp || (!band->plane && !band->band_num &&
										   (_ctx->frame_flags & 8))) {
						mb->q_delta = get_vlc2(&_ctx->gb, _ctx->mb_vlc.tab->table,
											   IVI_VLC_BITS, 1);
						mb->q_delta = IVI_TOSIGNED(mb->q_delta);
					}
				}

				if (!mb->type) {
					mb->mv_x = mb->mv_y = 0; /* there is no motion vector in intra-macroblocks */
				} else {
					if (band->inherit_mv && ref_mb) {
						/* motion vector inheritance */
						if (mv_scale) {
							mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
							mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
						} else {
							mb->mv_x = ref_mb->mv_x;
							mb->mv_y = ref_mb->mv_y;
						}
					} else {
						/* decode motion vector deltas */
						mv_delta = get_vlc2(&_ctx->gb, _ctx->mb_vlc.tab->table,
											IVI_VLC_BITS, 1);
						mv_y += IVI_TOSIGNED(mv_delta);
						mv_delta = get_vlc2(&_ctx->gb, _ctx->mb_vlc.tab->table,
											IVI_VLC_BITS, 1);
						mv_x += IVI_TOSIGNED(mv_delta);
						mb->mv_x = mv_x;
						mb->mv_y = mv_y;
					}
				}
			}

			s = band->is_halfpel;
			if (mb->type)
				if (x + (mb->mv_x   >> s) + (y + (mb->mv_y   >> s))*band->pitch < 0 ||
					x + ((mb->mv_x + s) >> s) + band->mb_size - 1
					+ (y + band->mb_size - 1 + ((mb->mv_y + s) >> s))*band->pitch > band->bufsize - 1) {
					error("Indeo5decoder: motion vector %d %d outside reference", x * s + mb->mv_x, y * s + mb->mv_y);
					return -1;
				}

			mb++;
			if (ref_mb)
				ref_mb++;
			mb_offset += band->mb_size;
		}

		offs += row_offset;
	}

	_ctx->gb->align();

	return 0;
}


/**
 *  Switch buffers.
 *
 *  @param[in,out] ctx  ptr to the decoder context
 */
void Indeo5Decoder::switch_buffers() {
	switch (_ctx->prev_frame_type) {
	case FRAMETYPE_INTRA:
	case FRAMETYPE_INTER:
		_ctx->buf_switch ^= 1;
		_ctx->dst_buf = _ctx->buf_switch;
		_ctx->ref_buf = _ctx->buf_switch ^ 1;
		break;
	case FRAMETYPE_INTER_SCAL:
		if (!_ctx->inter_scal) {
			_ctx->ref2_buf   = 2;
			_ctx->inter_scal = 1;
		}
		SWAP(_ctx->dst_buf, _ctx->ref2_buf);
		_ctx->ref_buf = _ctx->ref2_buf;
		break;
	case FRAMETYPE_INTER_NOREF:
		break;
	}

	switch (_ctx->frame_type) {
	case FRAMETYPE_INTRA:
		_ctx->buf_switch = 0;
		/* FALLTHROUGH */
	case FRAMETYPE_INTER:
		_ctx->inter_scal = 0;
		_ctx->dst_buf = _ctx->buf_switch;
		_ctx->ref_buf = _ctx->buf_switch ^ 1;
		break;
	case FRAMETYPE_INTER_SCAL:
	case FRAMETYPE_INTER_NOREF:
	case FRAMETYPE_NULL:
		break;
	}
}


int Indeo5Decoder::is_nonnull_frame() {
	return _ctx->frame_type != FRAMETYPE_NULL;
}


/**
 *  Initialize Indeo5 decoder.
 */
Indeo5Decoder::Indeo5Decoder(uint16 width, uint16 height) {
	_ctx = new IVI45DecContext;
	int             result;

	_surface = new Graphics::Surface;

	_surface->create(width, height, g_system->getScreenFormat());

	ff_ivi_init_static_vlc();

	/* copy rvmap tables in our context so we can apply changes to them */
	memcpy(_ctx->rvmap_tabs, ff_ivi_rvmap_tabs, sizeof(ff_ivi_rvmap_tabs));

	/* set the initial picture layout according to the basic profile:
	   there is only one band per plane (no scalability), only one tile (no local decoding)
	   and picture format = YVU9 */
	_ctx->pic_conf.pic_width     = width;
	_ctx->pic_conf.pic_height    = height;
	_ctx->pic_conf.chroma_width  = (width  + 3) >> 2;
	_ctx->pic_conf.chroma_height = (height + 3) >> 2;
	_ctx->pic_conf.tile_width    = width;
	_ctx->pic_conf.tile_height   = height;
	_ctx->pic_conf.luma_bands    = _ctx->pic_conf.chroma_bands = 1;

	result = ff_ivi_init_planes(_ctx->planes, &_ctx->pic_conf);
	if (result) {
		error("Indeo5decoder: Couldn't allocate color planes!");
		return;
	}

	_ctx->buf_switch = 0;
	_ctx->inter_scal = 0;
}

/**
 * @file
 * This file contains functions and data shared by both Indeo4 and
 * Indeo5 decoders.
 */

/**
 * These are 2x8 predefined Huffman codebooks for coding macroblock/block
 * signals. They are specified using "huffman descriptors" in order to
 * avoid huge static tables. The decoding tables will be generated at
 * startup from these descriptors.
 */
/** static macroblock huffman tables */
static const IVIHuffDesc ivi_mb_huff_desc[8] = {
	{8,  {0, 4, 5, 4, 4, 4, 6, 6}},
	{12, {0, 2, 2, 3, 3, 3, 3, 5, 3, 2, 2, 2}},
	{12, {0, 2, 3, 4, 3, 3, 3, 3, 4, 3, 2, 2}},
	{12, {0, 3, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2}},
	{13, {0, 4, 4, 3, 3, 3, 3, 2, 3, 3, 2, 1, 1}},
	{9,  {0, 4, 4, 4, 4, 3, 3, 3, 2}},
	{10, {0, 4, 4, 4, 4, 3, 3, 2, 2, 2}},
	{12, {0, 4, 4, 4, 3, 3, 2, 3, 2, 2, 2, 2}}
};

/** static block huffman tables */
static const IVIHuffDesc ivi_blk_huff_desc[8] = {
	{10, {1, 2, 3, 4, 4, 7, 5, 5, 4, 1}},
	{11, {2, 3, 4, 4, 4, 7, 5, 4, 3, 3, 2}},
	{12, {2, 4, 5, 5, 5, 5, 6, 4, 4, 3, 1, 1}},
	{13, {3, 3, 4, 4, 5, 6, 6, 4, 4, 3, 2, 1, 1}},
	{11, {3, 4, 4, 5, 5, 5, 6, 5, 4, 2, 2}},
	{13, {3, 4, 5, 5, 5, 5, 6, 4, 3, 3, 2, 1, 1}},
	{13, {3, 4, 5, 5, 5, 6, 5, 4, 3, 3, 2, 1, 1}},
	{9,  {3, 4, 4, 5, 5, 5, 6, 5, 5}}
};

static VLC ivi_mb_vlc_tabs [8]; ///< static macroblock Huffman tables
static VLC ivi_blk_vlc_tabs[8]; ///< static block Huffman tables

typedef void (*ivi_mc_func) (int16 *buf, const int16 *ref_buf,
							 uint32 pitch, int mc_type);

static int ivi_mc(IVIBandDesc *band, ivi_mc_func mc,
				  int offs, int mv_x, int mv_y, int mc_type) {
	int ref_offs = offs + mv_y * band->pitch + mv_x;
	int buf_size = band->pitch * band->aheight;
	int min_size = band->pitch * (band->blk_size - 1) + band->blk_size;
	int ref_size = (mc_type > 1) * band->pitch + (mc_type & 1);

	assert(offs >= 0 && ref_offs >= 0 && band->ref_buf);
	assert(buf_size - min_size >= offs);
	assert(buf_size - min_size - ref_size >= ref_offs);

	mc(band->buf + offs, band->ref_buf + ref_offs, band->pitch, mc_type);

	return 0;
}

const uint8_t ff_reverse[256] = {
0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF,
};

/**
 *  Reverse "nbits" bits of the value "val" and return the result
 *  in the least significant bits.
 */
static uint16 inv_bits(uint16 val, int nbits) {
	uint16 res;

	if (nbits <= 8) {
		res = ff_reverse[val] >> (8 - nbits);
	} else
		res = ((ff_reverse[val & 0xFF] << 8) +
			   (ff_reverse[val >> 8])) >> (16 - nbits);

	return res;
}

/*
 *  Generate a huffman codebook from the given descriptor
 *  and convert it into the FFmpeg VLC table.
 *
 *  @param[in]   cb    pointer to codebook descriptor
 *  @param[out]  vlc   where to place the generated VLC table
 *  @param[in]   flag  flag: 1 - for static or 0 for dynamic tables
 *  @return     result code: 0 - OK, -1 = error (invalid codebook descriptor)
 */
static int ivi_create_huff_from_desc(const IVIHuffDesc *cb, VLC *vlc, int flag) {
	int         pos, i, j, codes_per_row, prefix, not_last_row;
	uint16    codewords[256]; /* FIXME: move this temporal storage out? */
	uint8     bits[256];

	pos = 0; /* current position = 0 */

	for (i = 0; i < cb->num_rows; i++) {
		codes_per_row = 1 << cb->xbits[i];
		not_last_row  = (i != cb->num_rows - 1);
		prefix        = ((1 << i) - 1) << (cb->xbits[i] + not_last_row);

		for (j = 0; j < codes_per_row; j++) {
			if (pos >= 256) /* Some Indeo5 codebooks can have more than 256 */
				break;      /* elements, but only 256 codes are allowed! */

			bits[pos] = i + cb->xbits[i] + not_last_row;
			if (bits[pos] > IVI_VLC_BITS)
				return -1; /* invalid descriptor */

			codewords[pos] = inv_bits((prefix | j), bits[pos]);
			if (!bits[pos])
				bits[pos] = 1;

			pos++;
		}//for j
	}//for i

	/* number of codewords = pos */
	return init_vlc(vlc, IVI_VLC_BITS, pos, bits, 1, 1, codewords, 2, 2,
					(flag ? INIT_VLC_USE_NEW_STATIC : 0) | INIT_VLC_LE);
}

void ff_ivi_init_static_vlc(void) {
	int i;
	static VLC_TYPE table_data[8192 * 16][2];
	static int initialized_vlcs = 0;

	if (initialized_vlcs)
		return;
	for (i = 0; i < 8; i++) {
		ivi_mb_vlc_tabs[i].table = table_data + i * 2 * 8192;
		ivi_mb_vlc_tabs[i].table_allocated = 8192;
		ivi_create_huff_from_desc(&ivi_mb_huff_desc[i],
								  &ivi_mb_vlc_tabs[i], 1);
		ivi_blk_vlc_tabs[i].table = table_data + (i * 2 + 1) * 8192;
		ivi_blk_vlc_tabs[i].table_allocated = 8192;
		ivi_create_huff_from_desc(&ivi_blk_huff_desc[i],
								  &ivi_blk_vlc_tabs[i], 1);
	}
	initialized_vlcs = 1;
}

/*
 *  Copy huffman codebook descriptors.
 *
 *  @param[out]  dst  ptr to the destination descriptor
 *  @param[in]   src  ptr to the source descriptor
 */
static void ivi_huff_desc_copy(IVIHuffDesc *dst, const IVIHuffDesc *src) {
	dst->num_rows = src->num_rows;
	memcpy(dst->xbits, src->xbits, src->num_rows);
}

/*
 *  Compare two huffman codebook descriptors.
 *
 *  @param[in]  desc1  ptr to the 1st descriptor to compare
 *  @param[in]  desc2  ptr to the 2nd descriptor to compare
 *  @return         comparison result: 0 - equal, 1 - not equal
 */
static int ivi_huff_desc_cmp(const IVIHuffDesc *desc1,
							 const IVIHuffDesc *desc2) {
	return desc1->num_rows != desc2->num_rows ||
		memcmp(desc1->xbits, desc2->xbits, desc1->num_rows);
}

int ff_ivi_dec_huff_desc(Common::BitStream *gb, int desc_coded, int which_tab,
						 IVIHuffTab *huff_tab) {
	int i, result;
	IVIHuffDesc new_huff;

	if (!desc_coded) {
		/* select default table */
		huff_tab->tab = (which_tab) ? &ivi_blk_vlc_tabs[7]
			: &ivi_mb_vlc_tabs [7];
		return 0;
	}

	huff_tab->tab_sel = gb->getBits(3);
	if (huff_tab->tab_sel == 7) {
		/* custom huffman table (explicitly encoded) */
		new_huff.num_rows = gb->getBits(4);
		if (!new_huff.num_rows) {
			error("Indeo5decoder: Empty custom Huffman table!");
			return -1;
		}

		for (i = 0; i < new_huff.num_rows; i++)
			new_huff.xbits[i] = gb->getBits(4);

		/* Have we got the same custom table? Rebuild if not. */
		if (ivi_huff_desc_cmp(&new_huff, &huff_tab->cust_desc) || !huff_tab->cust_tab.table) {
			ivi_huff_desc_copy(&huff_tab->cust_desc, &new_huff);

			if (huff_tab->cust_tab.table)
				ff_free_vlc(&huff_tab->cust_tab);
			result = ivi_create_huff_from_desc(&huff_tab->cust_desc,
											   &huff_tab->cust_tab, 0);
			if (result) {
				// reset faulty description
				huff_tab->cust_desc.num_rows = 0;
				error("Indeo5decoder: Error while initializing custom vlc table!");
				return result;
			}
		}
		huff_tab->tab = &huff_tab->cust_tab;
	} else {
		/* select one of predefined tables */
		huff_tab->tab = (which_tab) ? &ivi_blk_vlc_tabs[huff_tab->tab_sel]
			: &ivi_mb_vlc_tabs [huff_tab->tab_sel];
	}

	return 0;
}

/*
 *  Free planes, bands and macroblocks buffers.
 *
 *  @param[in]  planes  pointer to the array of the plane descriptors
 */
static void ivi_free_buffers(IVIPlaneDesc *planes) {
	int p, b, t;

	for (p = 0; p < 3; p++) {
		if (planes[p].bands)
			for (b = 0; b < planes[p].num_bands; b++) {
				free(&planes[p].bands[b].bufs[0]);
				free(&planes[p].bands[b].bufs[1]);
				free(&planes[p].bands[b].bufs[2]);

				if (planes[p].bands[b].blk_vlc.cust_tab.table)
					ff_free_vlc(&planes[p].bands[b].blk_vlc.cust_tab);
				for (t = 0; t < planes[p].bands[b].num_tiles; t++)
					free(&planes[p].bands[b].tiles[t].mbs);
				free(&planes[p].bands[b].tiles);
			}
		free(&planes[p].bands);
		planes[p].num_bands = 0;
	}
}

int ff_ivi_init_planes(IVIPlaneDesc *planes, const IVIPicConfig *cfg) {
	int p, b;
	uint32 b_width, b_height, align_fac, width_aligned,
		height_aligned, buf_size;
	IVIBandDesc *band;

	ivi_free_buffers(planes);

	if (cfg->pic_width < 1 || cfg->pic_height < 1 ||
		cfg->luma_bands < 1 || cfg->chroma_bands < 1)
		return -1;

	/* fill in the descriptor of the luminance plane */
	planes[0].width     = cfg->pic_width;
	planes[0].height    = cfg->pic_height;
	planes[0].num_bands = cfg->luma_bands;

	/* fill in the descriptors of the chrominance planes */
	planes[1].width     = planes[2].width     = (cfg->pic_width  + 3) >> 2;
	planes[1].height    = planes[2].height    = (cfg->pic_height + 3) >> 2;
	planes[1].num_bands = planes[2].num_bands = cfg->chroma_bands;

	for (p = 0; p < 3; p++) {
		planes[p].bands = (IVIBandDesc *)calloc(planes[p].num_bands, sizeof(IVIBandDesc));
		if (!planes[p].bands)
			return -3;

		/* select band dimensions: if there is only one band then it
		 *  has the full size, if there are several bands each of them
		 *  has only half size */
		b_width  = planes[p].num_bands == 1 ? planes[p].width
			: (planes[p].width  + 1) >> 1;
		b_height = planes[p].num_bands == 1 ? planes[p].height
			: (planes[p].height + 1) >> 1;

		/* luma   band buffers will be aligned on 16x16 (max macroblock size) */
		/* chroma band buffers will be aligned on   8x8 (max macroblock size) */
		align_fac       = p ? 8 : 16;
		width_aligned   = FFALIGN(b_width , align_fac);
		height_aligned  = FFALIGN(b_height, align_fac);
		buf_size        = width_aligned * height_aligned * sizeof(int16);

		for (b = 0; b < planes[p].num_bands; b++) {
			band = &planes[p].bands[b]; /* select appropriate plane/band */
			band->plane    = p;
			band->band_num = b;
			band->width    = b_width;
			band->height   = b_height;
			band->pitch    = width_aligned;
			band->aheight  = height_aligned;
			band->bufs[0]  = (int16 *)calloc(buf_size, 1);
			band->bufs[1]  = (int16 *)calloc(buf_size, 1);
			band->bufsize  = buf_size/2;
			if (!band->bufs[0] || !band->bufs[1])
				return -3;

			/* allocate the 3rd band buffer for scalability mode */
			if (cfg->luma_bands > 1) {
				band->bufs[2] = (int16 *)calloc(buf_size, 1);
				if (!band->bufs[2])
					return -3;
			}
			/* reset custom vlc */
			planes[p].bands[0].blk_vlc.cust_desc.num_rows = 0;
		}
	}

	return 0;
}

static int ivi_init_tiles(IVIBandDesc *band, IVITile *ref_tile,
						  int p, int b, int t_height, int t_width) {
	int x, y;
	IVITile *tile = band->tiles;

	for (y = 0; y < band->height; y += t_height) {
		for (x = 0; x < band->width; x += t_width) {
			tile->xpos     = x;
			tile->ypos     = y;
			tile->mb_size  = band->mb_size;
			tile->width    = MIN(band->width - x,  t_width);
			tile->height   = MIN(band->height - y, t_height);
			tile->is_empty = tile->data_size = 0;
			/* calculate number of macroblocks */
			tile->num_MBs  = IVI_MBs_PER_TILE(tile->width, tile->height,
											  band->mb_size);

			free(&tile->mbs);
			tile->mbs = (IVIMbInfo *)calloc(tile->num_MBs, sizeof(IVIMbInfo));
			if (!tile->mbs)
				return -3;

			tile->ref_mbs = 0;
			if (p || b) {
				if (tile->num_MBs != ref_tile->num_MBs) {
					error("Indeo5decoder: ref_tile mismatch");
					return -1;
				}
				tile->ref_mbs = ref_tile->mbs;
				ref_tile++;
			}
			tile++;
		}
	}

	return 0;
}

int ff_ivi_init_tiles(IVIPlaneDesc *planes,
							  int tile_width, int tile_height) {
	int p, b, x_tiles, y_tiles, t_width, t_height, ret;
	IVIBandDesc *band;

	for (p = 0; p < 3; p++) {
		t_width  = !p ? tile_width  : (tile_width  + 3) >> 2;
		t_height = !p ? tile_height : (tile_height + 3) >> 2;

		if (!p && planes[0].num_bands == 4) {
			t_width  >>= 1;
			t_height >>= 1;
		}
		if(t_width<=0 || t_height<=0)
			return -4;

		for (b = 0; b < planes[p].num_bands; b++) {
			band = &planes[p].bands[b];
			x_tiles = IVI_NUM_TILES(band->width, t_width);
			y_tiles = IVI_NUM_TILES(band->height, t_height);
			band->num_tiles = x_tiles * y_tiles;

			free(&band->tiles);
			band->tiles = (IVITile *)calloc(band->num_tiles, sizeof(IVITile));
			if (!band->tiles)
				return -3;

			/* use the first luma band as reference for motion vectors
			 * and quant */
			ret = ivi_init_tiles(band, planes[0].bands[0].tiles,
								 p, b, t_height, t_width);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

/*
 *  Decode size of the tile data.
 *  The size is stored as a variable-length field having the following format:
 *  if (tile_data_size < 255) than this field is only one byte long
 *  if (tile_data_size >= 255) than this field four is byte long: 0xFF X1 X2 X3
 *  where X1-X3 is size of the tile data
 *
 *  @param[in,out]  gb  the GetBit context
 *  @return     size of the tile data in bytes
 */
static int ivi_dec_tile_data_size(Common::BitStream *gb) {
	int    len;

	len = 0;
	if (gb->getBit()) {
		len = gb->getBits(8);
		if (len == 255)
			len = gb->getBits(24);
	}

	/* align the bitstream reader on the byte boundary */
	gb->align();

	return len;
}

static int ivi_dc_transform(IVIBandDesc *band, int *prev_dc, int buf_offs,
							int blk_size) {
	int buf_size = band->pitch * band->aheight - buf_offs;
	int min_size = (blk_size - 1) * band->pitch + blk_size;

	if (min_size > buf_size)
		return -1;

	band->dc_transform(prev_dc, band->buf + buf_offs,
					   band->pitch, blk_size);

	return 0;
}

static int ivi_decode_coded_blocks(Common::BitStream *gb, IVIBandDesc *band,
								   ivi_mc_func mc, int mv_x, int mv_y,
								   int *prev_dc, int is_intra, int mc_type,
								   uint32 quant, int offs) {
	const uint16 *base_tab  = is_intra ? band->intra_base : band->inter_base;
	RVMapDesc *rvmap = band->rv_map;
	uint8 col_flags[8];
	int32 trvec[64];
	uint32 sym = 0, lo, hi, q;
	int pos, run, val;
	int blk_size   = band->blk_size;
	int num_coeffs = blk_size * blk_size;
	int col_mask   = blk_size - 1;
	int scan_pos   = -1;
	int min_size   = band->pitch * (band->transform_size - 1) +
		band->transform_size;
	int buf_size   = band->pitch * band->aheight - offs;

	if (min_size > buf_size)
		return -1;

	if (!band->scan) {
		error("Indeo5decoder: Scan pattern is not set");
		return -1;
	}

	/* zero transform vector */
	memset(trvec, 0, num_coeffs * sizeof(trvec[0]));
	/* zero column flags */
	memset(col_flags, 0, sizeof(col_flags));
	while (scan_pos <= num_coeffs) {
		sym = get_vlc2(gb, band->blk_vlc.tab->table,
					   IVI_VLC_BITS, 1);
		if (sym == rvmap->eob_sym)
			break; /* End of block */

		/* Escape - run/val explicitly coded using 3 vlc codes */
		if (sym == rvmap->esc_sym) {
			run = get_vlc2(gb, band->blk_vlc.tab->table, IVI_VLC_BITS, 1) + 1;
			lo  = get_vlc2(gb, band->blk_vlc.tab->table, IVI_VLC_BITS, 1);
			hi  = get_vlc2(gb, band->blk_vlc.tab->table, IVI_VLC_BITS, 1);
			/* merge them and convert into signed val */
			val = IVI_TOSIGNED((hi << 6) | lo);
		} else {
			if (sym >= 256U) {
				error("Indeo5decoder: Invalid sym encountered: %u", sym);
				return -1;
			}
			run = rvmap->runtab[sym];
			val = rvmap->valtab[sym];
		}

		/* de-zigzag and dequantize */
		scan_pos += run;
		if (scan_pos >= num_coeffs || scan_pos < 0)
			break;
		pos = band->scan[scan_pos];

		if (!val)
			warning("Indeo5decoder: Val = 0 encountered");

		q = (base_tab[pos] * quant) >> 9;
		if (q > 1)
			val = val * q + FFSIGN(val) * (((q ^ 1) - 1) >> 1);
		trvec[pos] = val;
		/* track columns containing non-zero coeffs */
		col_flags[pos & col_mask] |= !!val;
	}

	if ((scan_pos < 0 || scan_pos >= num_coeffs) && sym != rvmap->eob_sym)
		return -1; /* corrupt block data */

	/* undoing DC coeff prediction for intra-blocks */
	if (is_intra && band->is_2d_trans) {
		*prev_dc     += trvec[0];
		trvec[0]      = *prev_dc;
		col_flags[0] |= !!*prev_dc;
	}

	if(band->transform_size > band->blk_size){
		error("Indeo5decoder: Too large transform");
		return -1;
	}

	/* apply inverse transform */
	band->inv_transform(trvec, band->buf + offs,
						band->pitch, col_flags);

	/* apply motion compensation */
	if (!is_intra)
		return ivi_mc(band, mc, offs, mv_x, mv_y, mc_type);

	return 0;
}
/*
 *  Decode block data:
 *  extract huffman-coded transform coefficients from the bitstream,
 *  dequantize them, apply inverse transform and motion compensation
 *  in order to reconstruct the picture.
 *
 *  @param[in,out]  gb    the GetBit context
 *  @param[in]      band  pointer to the band descriptor
 *  @param[in]      tile  pointer to the tile descriptor
 *  @return     result code: 0 - OK, -1 = error (corrupted blocks data)
 */
static int ivi_decode_blocks(Common::BitStream *gb, IVIBandDesc *band,
							 IVITile *tile) {
	int mbn, blk, num_blocks, blk_size, ret, is_intra, mc_type = 0;
	int mv_x = 0, mv_y = 0;
	int32 prev_dc;
	uint32 cbp, quant, buf_offs;
	IVIMbInfo *mb;
	ivi_mc_func mc_with_delta_func, mc_no_delta_func;
	const uint8 *scale_tab;

	/* init intra prediction for the DC coefficient */
	prev_dc    = 0;
	blk_size   = band->blk_size;
	/* number of blocks per mb */
	num_blocks = (band->mb_size != blk_size) ? 4 : 1;
	if (blk_size == 8) {
		mc_with_delta_func = ff_ivi_mc_8x8_delta;
		mc_no_delta_func   = ff_ivi_mc_8x8_no_delta;
	} else {
		mc_with_delta_func = ff_ivi_mc_4x4_delta;
		mc_no_delta_func   = ff_ivi_mc_4x4_no_delta;
	}

	for (mbn = 0, mb = tile->mbs; mbn < tile->num_MBs; mb++, mbn++) {
		is_intra = !mb->type;
		cbp      = mb->cbp;
		buf_offs = mb->buf_offs;

		quant = band->glob_quant + mb->q_delta;
		if (0 /*avctx->codec_id == AV_CODEC_ID_INDEO4 */)
			quant = CLIP((int32)quant, 0, 31);
		else
			quant = CLIP((int32)quant, 0, 23);

		scale_tab = is_intra ? band->intra_scale : band->inter_scale;
		if (scale_tab)
			quant = scale_tab[quant];

		if (!is_intra) {
			mv_x = mb->mv_x;
			mv_y = mb->mv_y;
			if (band->is_halfpel) {
				mc_type = ((mv_y & 1) << 1) | (mv_x & 1);
				mv_x >>= 1;
				mv_y >>= 1; /* convert halfpel vectors into fullpel ones */
			}
			if (mb->type) {
				int dmv_x, dmv_y, cx, cy;

				dmv_x = mb->mv_x >> band->is_halfpel;
				dmv_y = mb->mv_y >> band->is_halfpel;
				cx    = mb->mv_x &  band->is_halfpel;
				cy    = mb->mv_y &  band->is_halfpel;

				if (mb->xpos + dmv_x < 0 ||
					mb->xpos + dmv_x + band->mb_size + cx > band->pitch ||
					mb->ypos + dmv_y < 0 ||
					mb->ypos + dmv_y + band->mb_size + cy > band->aheight) {
					return -1;
				}
			}
		}

		for (blk = 0; blk < num_blocks; blk++) {
			/* adjust block position in the buffer according to its number */
			if (blk & 1) {
				buf_offs += blk_size;
			} else if (blk == 2) {
				buf_offs -= blk_size;
				buf_offs += blk_size * band->pitch;
			}

			if (cbp & 1) { /* block coded ? */
				ret = ivi_decode_coded_blocks(gb, band, mc_with_delta_func,
											  mv_x, mv_y, &prev_dc, is_intra,
											  mc_type, quant, buf_offs);
				if (ret < 0)
					return ret;
			} else {
				/* block not coded */
				/* for intra blocks apply the dc slant transform */
				/* for inter - perform the motion compensation without delta */
				if (is_intra) {
					ret = ivi_dc_transform(band, &prev_dc, buf_offs, blk_size);
					if (ret < 0)
						return ret;
				} else {
					ret = ivi_mc(band, mc_no_delta_func, buf_offs,
								 mv_x, mv_y, mc_type);
					if (ret < 0)
						return ret;
				}
			}

			cbp >>= 1;
		}// for blk
	}// for mbn

	gb->align();

	return 0;
}

/**
 *  Handle empty tiles by performing data copying and motion
 *  compensation respectively.
 *
 *  @param[in]  band      pointer to the band descriptor
 *  @param[in]  tile      pointer to the tile descriptor
 *  @param[in]  mv_scale  scaling factor for motion vectors
 */
static int ivi_process_empty_tile(IVIBandDesc *band,
								  IVITile *tile, int32 mv_scale) {
	int             x, y, need_mc, mbn, blk, num_blocks, mv_x, mv_y, mc_type;
	int             offs, mb_offset, row_offset, ret;
	IVIMbInfo       *mb, *ref_mb;
	const int16   *src;
	int16         *dst;
	ivi_mc_func     mc_no_delta_func;

	if (tile->num_MBs != IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size)) {
		error("Indeo5decoder: Allocated tile size %d mismatches "
			  "parameters %d in ivi_process_empty_tile()",
			  tile->num_MBs, IVI_MBs_PER_TILE(tile->width, tile->height, band->mb_size));
		return -1;
	}

	offs       = tile->ypos * band->pitch + tile->xpos;
	mb         = tile->mbs;
	ref_mb     = tile->ref_mbs;
	row_offset = band->mb_size * band->pitch;
	need_mc    = 0; /* reset the mc tracking flag */

	for (y = tile->ypos; y < (tile->ypos + tile->height); y += band->mb_size) {
		mb_offset = offs;

		for (x = tile->xpos; x < (tile->xpos + tile->width); x += band->mb_size) {
			mb->xpos     = x;
			mb->ypos     = y;
			mb->buf_offs = mb_offset;

			mb->type = 1; /* set the macroblocks type = INTER */
			mb->cbp  = 0; /* all blocks are empty */

			if (!band->qdelta_present && !band->plane && !band->band_num) {
				mb->q_delta = band->glob_quant;
				mb->mv_x    = 0;
				mb->mv_y    = 0;
			}

			if (band->inherit_qdelta && ref_mb)
				mb->q_delta = ref_mb->q_delta;

			if (band->inherit_mv && ref_mb) {
				/* motion vector inheritance */
				if (mv_scale) {
					mb->mv_x = ivi_scale_mv(ref_mb->mv_x, mv_scale);
					mb->mv_y = ivi_scale_mv(ref_mb->mv_y, mv_scale);
				} else {
					mb->mv_x = ref_mb->mv_x;
					mb->mv_y = ref_mb->mv_y;
				}
				need_mc |= mb->mv_x || mb->mv_y; /* tracking non-zero motion vectors */
				{
					int dmv_x, dmv_y, cx, cy;

					dmv_x = mb->mv_x >> band->is_halfpel;
					dmv_y = mb->mv_y >> band->is_halfpel;
					cx    = mb->mv_x &  band->is_halfpel;
					cy    = mb->mv_y &  band->is_halfpel;

					if (   mb->xpos + dmv_x < 0
						   || mb->xpos + dmv_x + band->mb_size + cx > band->pitch
						   || mb->ypos + dmv_y < 0
						   || mb->ypos + dmv_y + band->mb_size + cy > band->aheight) {
						error("Indeo5decoder: MV out of bounds");
						return -1;
					}
				}
			}

			mb++;
			if (ref_mb)
				ref_mb++;
			mb_offset += band->mb_size;
		} // for x
		offs += row_offset;
	} // for y

	if (band->inherit_mv && need_mc) { /* apply motion compensation if there is at least one non-zero motion vector */
		num_blocks = (band->mb_size != band->blk_size) ? 4 : 1; /* number of blocks per mb */
		mc_no_delta_func = (band->blk_size == 8) ? ff_ivi_mc_8x8_no_delta
			: ff_ivi_mc_4x4_no_delta;

		for (mbn = 0, mb = tile->mbs; mbn < tile->num_MBs; mb++, mbn++) {
			mv_x = mb->mv_x;
			mv_y = mb->mv_y;
			if (!band->is_halfpel) {
				mc_type = 0; /* we have only fullpel vectors */
			} else {
				mc_type = ((mv_y & 1) << 1) | (mv_x & 1);
				mv_x >>= 1;
				mv_y >>= 1; /* convert halfpel vectors into fullpel ones */
			}

			for (blk = 0; blk < num_blocks; blk++) {
				/* adjust block position in the buffer according with its number */
				offs = mb->buf_offs + band->blk_size * ((blk & 1) + !!(blk & 2) * band->pitch);
				ret = ivi_mc(band, mc_no_delta_func, offs,
							 mv_x, mv_y, mc_type);
				if (ret < 0)
					return ret;
			}
		}
	} else {
		/* copy data from the reference tile into the current one */
		src = band->ref_buf + tile->ypos * band->pitch + tile->xpos;
		dst = band->buf     + tile->ypos * band->pitch + tile->xpos;
		for (y = 0; y < tile->height; y++) {
			memcpy(dst, src, tile->width*sizeof(band->buf[0]));
			src += band->pitch;
			dst += band->pitch;
		}
	}

	return 0;
}

/**
 *  Decode an Indeo 4 or 5 band.
 *
 *  @param[in,out]  ctx    ptr to the decoder context
 *  @param[in,out]  band   ptr to the band descriptor
 *  @return         result code: 0 = OK, -1 = error
 */
int Indeo5Decoder::decode_band(IVIBandDesc *band) {
	int         result, i, t, idx1, idx2, pos;
	IVITile     *tile;

	band->buf     = band->bufs[_ctx->dst_buf];
	if (!band->buf) {
		error("Indeo5decoder: Band buffer points to no data");
		return -1;
	}
	band->ref_buf = band->bufs[_ctx->ref_buf];

	result = decode_band_hdr(band);
	if (result) {
		error("Indeo5decoder: Error while decoding band header: %d", result);
		return result;
	}

	if (band->is_empty) {
		error("Indeo5decoder: Empty band encountered!");
		return -1;
	}

	band->rv_map = &_ctx->rvmap_tabs[band->rvmap_sel];

	/* apply corrections to the selected rvmap table if present */
	for (i = 0; i < band->num_corr; i++) {
		idx1 = band->corr[i * 2];
		idx2 = band->corr[i * 2 + 1];
		SWAP(band->rv_map->runtab[idx1], band->rv_map->runtab[idx2]);
		SWAP(band->rv_map->valtab[idx1], band->rv_map->valtab[idx2]);
		if (idx1 == band->rv_map->eob_sym || idx2 == band->rv_map->eob_sym)
			band->rv_map->eob_sym ^= idx1 ^ idx2;
		if (idx1 == band->rv_map->esc_sym || idx2 == band->rv_map->esc_sym)
			band->rv_map->esc_sym ^= idx1 ^ idx2;
	}

	pos = _ctx->gb->pos();

	for (t = 0; t < band->num_tiles; t++) {
		tile = &band->tiles[t];

		if (tile->mb_size != band->mb_size) {
			error("Indeo5decoder: MB sizes mismatch: %d vs. %d",
				  band->mb_size, tile->mb_size);
			return -1;
		}
		tile->is_empty = _ctx->gb->getBit();
		if (tile->is_empty) {
			result = ivi_process_empty_tile(band, tile,
											(_ctx->planes[0].bands[0].mb_size >> 3) - (band->mb_size >> 3));
			if (result < 0)
				break;
			warning("Indeo5decoder: Empty tile encountered!");
		} else {
			tile->data_size = ivi_dec_tile_data_size(_ctx->gb);
			if (!tile->data_size) {
				error("Indeo5decoder: Tile data size is zero!");
				result = -1;
				break;
			}

			result = decode_mb_info(band, tile);
			if (result < 0)
				break;

			result = ivi_decode_blocks(_ctx->gb, band, tile);
			if (result < 0) {
				error("Indeo5decoder: Corrupted tile data encountered!");
				break;
			}

			if (((_ctx->gb->pos() - pos) >> 3) != tile->data_size) {
				error("Indeo5decoder: Tile data_size mismatch!");
				result = -1;
				break;
			}

			pos += tile->data_size << 3; // skip to next tile
		}
	}

	/* restore the selected rvmap table by applying its corrections in
	 * reverse order */
	for (i = band->num_corr-1; i >= 0; i--) {
		idx1 = band->corr[i*2];
		idx2 = band->corr[i*2+1];
		SWAP(band->rv_map->runtab[idx1], band->rv_map->runtab[idx2]);
		SWAP(band->rv_map->valtab[idx1], band->rv_map->valtab[idx2]);
		if (idx1 == band->rv_map->eob_sym || idx2 == band->rv_map->eob_sym)
			band->rv_map->eob_sym ^= idx1 ^ idx2;
		if (idx1 == band->rv_map->esc_sym || idx2 == band->rv_map->esc_sym)
			band->rv_map->esc_sym ^= idx1 ^ idx2;
	}

	_ctx->gb->align();

	return result;
}

const Graphics::Surface *Indeo5Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	int             result, p, b;

	_ctx->gb = new Common::BitStream8BEMSB frameData(stream);

	result = decode_pic_hdr();
	if (result) {
		error("Indeo5decoder: Error while decoding picture header: %d", result);
		return 0;
	}
	if (_ctx->gop_invalid) {
		error("Indeo5decoder: Invalid GOP");
		return 0;
	}

	if (_ctx->gop_flags & IVI5_IS_PROTECTED) {
		error("Indeo5decoder: Password-protected clip");
		return 0;
	}

	if (!_ctx->planes[0].bands) {
		error("Indeo5decoder: Color planes not initialized yet");
		return 0;
	}

	switch_buffers();

	//{ START_TIMER;

	if (is_nonnull_frame()) {
		_ctx->buf_invalid[_ctx->dst_buf] = 1;
		for (p = 0; p < 3; p++) {
			for (b = 0; b < _ctx->planes[p].num_bands; b++) {
				result = decode_band(&_ctx->planes[p].bands[b]);
				if (result < 0) {
					error("Indeo5decoder: Error while decoding band: %d, plane: %d", b, p);
					return 0;
				}
			}
		}
		_ctx->buf_invalid[_ctx->dst_buf] = 0;
	} else {
		if (_ctx->is_scalable) {
			error("Indeo5decoder: invalid data. Scalable");
			return 0;
		}

		for (p = 0; p < 3; p++) {
			if (!_ctx->planes[p].bands[0].buf) {
			error("Indeo5decoder: invalid data. Plane #%d", p);
				return 0;
			}
		}
	}
	if (_ctx->buf_invalid[_ctx->dst_buf]) {
		error("Indeo5decoder: invalid data. Buf invalid");
		return 0;
	}

	//STOP_TIMER("decode_planes"); }

	if (!is_nonnull_frame())
		return _surface;

	if (_surface->w != _ctx->planes[0].width || _surface->h != _ctx->planes[0].height) {
		_surface->free();
		_surface->create(_ctx->planes[0].width, _ctx->planes[0].height, g_system->getScreenFormat());
	}

	if (_ctx->is_scalable) {
		error("Scalable");
		//if (0 /*avctx->codec_id == AV_CODEC_ID_INDEO4 */)
		//	ff_ivi_recompose_haar(&_ctx->planes[0], frame->data[0], frame->linesize[0]);
		//else
		//	ff_ivi_recompose53   (&_ctx->planes[0], frame->data[0], frame->linesize[0]);
	} else {
		//ivi_output_plane(&_ctx->planes[0], frame->data[0], frame->linesize[0]);
	}

	delete _ctx->gb;

	const byte *srcY = _ctx->planes[0];
	const byte *srcU = _ctx->planes[1];
	const byte *srcV = _ctx->planes[2];

	int chrW = _ctx->pic_conf.chroma_width;
	int chrH = _ctx->pic_conf.chroma_height;

	// Create buffers for U/V with an extra row/column copied from the second-to-last
	// row/column.
	byte *tempU = new byte[(chrW + 1) * (chrH + 1)];
	byte *tempV = new byte[(chrW + 1) * (chrH + 1)];

	for (uint i = 0; i < chrH; i++) {
		memcpy(tempU + (chrW + 1) * i, srcU + chrW * i, chrW);
		memcpy(tempV + (chrW + 1) * i, srcV + chrW * i, chrW);
		tempU[(chrW + 1) * i + chrW] = srcU[chrW * (i + 1) - 1];
		tempV[(chrW + 1) * i + chrW] = srcV[chrW * (i + 1) - 1];
	}

	memcpy(tempU + (chrW + 1) * chrH, tempU + (chrW + 1) * (chrH - 1),
			chrW + 1);
	memcpy(tempV + (chrW + 1) * chrH, tempV + (chrW + 1) * (chrH - 1),
			chrW + 1);

	// Blit the frame onto the surface
	uint32 scaleWidth  = _surface->w / _ctx->pic_conf.pic_width;
	uint32 scaleHeight = _surface->h / _ctx->pic_conf.pic_height;

	if (scaleWidth == 1 && scaleHeight == 1) {
		// Shortcut: Don't need to scale so we can decode straight to the surface
		YUVToRGBMan.convert410(_surface, Graphics::YUVToRGBManager::kScaleITU, srcY, tempU, tempV,
				_ctx->pic_conf.pic_width, _ctx->pic_conf.pic_height, _ctx->pic_conf.pic_width, chrW + 1);
	} else {
		// Need to upscale, so decode to a temp surface first
		Graphics::Surface tempSurface;
		tempSurface.create(_ctx->pic_conf.pic_width, _ctx->pic_conf.pic_height, _surface->format);

		YUVToRGBMan.convert410(&tempSurface, Graphics::YUVToRGBManager::kScaleITU, srcY, tempU, tempV,
				_ctx->pic_conf.pic_width, _ctx->pic_conf.pic_height, _ctx->pic_conf.pic_width, chrW + 1);

		// Upscale
		for (int y = 0; y < _surface->h; y++) {
			for (int x = 0; x < _surface->w; x++) {
				if (_surface->format.bytesPerPixel == 1)
					*((byte *)_surface->getBasePtr(x, y)) = *((byte *)tempSurface.getBasePtr(x / scaleWidth, y / scaleHeight));
				else if (_surface->format.bytesPerPixel == 2)
					*((uint16 *)_surface->getBasePtr(x, y)) = *((uint16 *)tempSurface.getBasePtr(x / scaleWidth, y / scaleHeight));
				else if (_surface->format.bytesPerPixel == 4)
					*((uint32 *)_surface->getBasePtr(x, y)) = *((uint32 *)tempSurface.getBasePtr(x / scaleWidth, y / scaleHeight));
 			}
		}

		tempSurface.free();
	}

	delete[] tempU;
	delete[] tempV;

	return _surface;
}

/**
 *  Close Indeo5 decoder and clean up its context.
 */
Indeo5Decoder::~Indeo5Decoder() {
	ivi_free_buffers(&_ctx->planes[0]);

	if (_ctx->mb_vlc.cust_tab.table)
		ff_free_vlc(&_ctx->mb_vlc.cust_tab);

	_surface->free();

	delete _ctx;
}


/**
 *  Scan patterns shared between indeo4 and indeo5
 */
const uint8 ff_ivi_vertical_scan_8x8[64] = {
	0,  8, 16, 24, 32, 40, 48, 56,
	1,  9, 17, 25, 33, 41, 49, 57,
	2, 10, 18, 26, 34, 42, 50, 58,
	3, 11, 19, 27, 35, 43, 51, 59,
	4, 12, 20, 28, 36, 44, 52, 60,
	5, 13, 21, 29, 37, 45, 53, 61,
	6, 14, 22, 30, 38, 46, 54, 62,
	7, 15, 23, 31, 39, 47, 55, 63
};

const uint8 ff_ivi_horizontal_scan_8x8[64] = {
	0,  1,  2,  3,  4,  5,  6,  7,
	8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55,
	56, 57, 58, 59, 60, 61, 62, 63
};

const uint8 ff_ivi_direct_scan_4x4[16] = {
	0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};


/**
 *  Run-value (RLE) tables.
 */
const RVMapDesc ff_ivi_rvmap_tabs[9] = {
	{   /* MapTab0 */
		5, /* eob_sym */
		2, /* esc_sym */
		/* run table */
		{1,  1,  0,  1,  1,  0,  1,  1,  2,  2,  1,  1,  1,  1,  3,  3,
		 1,  1,  2,  2,  1,  1,  4,  4,  1,  1,  1,  1,  2,  2,  5,  5,
		 1,  1,  3,  3,  1,  1,  6,  6,  1,  2,  1,  2,  7,  7,  1,  1,
		 8,  8,  1,  1,  4,  2,  1,  4,  2,  1,  3,  3,  1,  1,  1,  9,
		 9,  1,  2,  1,  2,  1,  5,  5,  1,  1, 10, 10,  1,  1,  3,  3,
		 2,  2,  1,  1, 11, 11,  6,  4,  4,  1,  6,  1,  2,  1,  2, 12,
		 8,  1, 12,  7,  8,  7,  1, 16,  1, 16,  1,  3,  3, 13,  1, 13,
		 2,  2,  1, 15,  1,  5, 14, 15,  1,  5, 14,  1, 17,  8, 17,  8,
		 1,  4,  4,  2,  2,  1, 25, 25, 24, 24,  1,  3,  1,  3,  1,  8,
		 6,  7,  6,  1, 18,  8, 18,  1,  7, 23,  2,  2, 23,  1,  1, 21,
		 22,  9,  9, 22, 19,  1, 21,  5, 19,  5,  1, 33, 20, 33, 20,  8,
		 4,  4,  1, 32,  2,  2,  8,  3, 32, 26,  3,  1,  7,  7, 26,  6,
		 1,  6,  1,  1, 16,  1, 10,  1, 10,  2, 16, 29, 28,  2, 29, 28,
		 1, 27,  5,  8,  5, 27,  1,  8,  3,  7,  3, 31, 41, 31,  1, 41,
		 6,  1,  6,  7,  4,  4,  1,  1,  2,  1,  2, 11, 34, 30, 11,  1,
		 30, 15, 15, 34, 36, 40, 36, 40, 35, 35, 37, 37, 39, 39, 38, 38},

		/* value table */
		{ 1,  -1,   0,   2,  -2,   0,   3,  -3,   1,  -1,   4,  -4,   5,  -5,   1,  -1,
		  6,  -6,   2,  -2,   7,  -7,   1,  -1,   8,  -8,   9,  -9,   3,  -3,   1,  -1,
		  10, -10,   2,  -2,  11, -11,   1,  -1,  12,   4, -12,  -4,   1,  -1,  13, -13,
		  1,  -1,  14, -14,   2,   5,  15,  -2,  -5, -15,  -3,   3,  16, -16,  17,   1,
		  -1, -17,   6,  18,  -6, -18,   2,  -2,  19, -19,   1,  -1,  20, -20,   4,  -4,
		  7,  -7,  21, -21,   1,  -1,   2,   3,  -3,  22,  -2, -22,   8,  23,  -8,   1,
		  2, -23,  -1,   2,  -2,  -2,  24,   1, -24,  -1,  25,   5,  -5,   1, -25,  -1,
		  9,  -9,  26,   1, -26,   3,   1,  -1,  27,  -3,  -1, -27,   1,   3,  -1,  -3,
		  28,  -4,   4,  10, -10, -28,   1,  -1,   1,  -1,  29,   6, -29,  -6,  30,  -4,
		  3,   3,  -3, -30,   1,   4,  -1,  31,  -3,   1,  11, -11,  -1, -31,  32,  -1,
		  -1,   2,  -2,   1,   1, -32,   1,   4,  -1,  -4,  33,  -1,   1,   1,  -1,   5,
		  5,  -5, -33,  -1, -12,  12,  -5,  -7,   1,   1,   7,  34,   4,  -4,  -1,   4,
		  -34,  -4,  35,  36,  -2, -35,  -2, -36,   2,  13,   2,  -1,   1, -13,   1,  -1,
		  37,   1,  -5,   6,   5,  -1,  38,  -6,  -8,   5,   8,  -1,   1,   1, -37,  -1,
		  5,  39,  -5,  -5,   6,  -6, -38, -39, -14,  40,  14,   2,   1,   1,  -2, -40,
		  -1,  -2,   2,  -1,  -1,  -1,   1,   1,   1,  -1,   1,  -1,   1,  -1,   1,  -1}
	},{
		/* MapTab1 */
		0,  /* eob_sym */
		38, /* esc_sym */
			/* run table */
		{0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  8,  6,  8,  7,
		 7,  9,  9, 10, 10, 11, 11,  1, 12,  1, 12, 13, 13, 16, 14, 16,
		 14, 15, 15, 17, 17, 18,  0, 18, 19, 20, 21, 19, 22, 21, 20, 22,
		 25, 24,  2, 25, 24, 23, 23,  2, 26, 28, 26, 28, 29, 27, 29, 27,
		 33, 33,  1, 32,  1,  3, 32, 30, 36,  3, 36, 30, 31, 31, 35, 34,
		 37, 41, 34, 35, 37,  4, 41,  4, 49,  8,  8, 49, 40, 38,  5, 38,
		 40, 39,  5, 39, 42, 43, 42,  7, 57,  6, 43, 44,  6, 50,  7, 44,
		 57, 48, 50, 48, 45, 45, 46, 47, 51, 46, 47, 58,  1, 51, 58,  1,
		 52, 59, 53,  9, 52, 55, 55, 59, 53, 56, 54, 56, 54,  9, 64, 64,
		 60, 63, 60, 63, 61, 62, 61, 62,  2, 10,  2, 10, 11,  1, 11, 13,
		 12,  1, 12, 13, 16, 16,  8,  8, 14,  3,  3, 15, 14, 15,  4,  4,
		 1, 17, 17,  5,  1,  7,  7,  5,  6,  1,  2,  2,  6, 22,  1, 25,
		 21, 22,  8, 24,  1, 21, 25, 24,  8, 18, 18, 23,  9, 20, 23, 33,
		 29, 33, 20,  1, 19,  1, 29, 36,  9, 36, 19, 41, 28, 57, 32,  3,
		 28,  3,  1, 27, 49, 49,  1, 32, 26, 26,  2,  4,  4,  7, 57, 41,
		 2,  7, 10,  5, 37, 16, 10, 27,  8,  8, 13, 16, 37, 13,  1,  5},

		/* value table */
		{0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   1,   1,  -1,  -1,   1,
		 -1,   1,  -1,   1,  -1,   1,  -1,   2,   1,  -2,  -1,   1,  -1,   1,   1,  -1,
		 -1,   1,  -1,   1,  -1,   1,   0,  -1,   1,   1,   1,  -1,   1,  -1,  -1,  -1,
		 1,   1,   2,  -1,  -1,   1,  -1,  -2,   1,   1,  -1,  -1,   1,   1,  -1,  -1,
		 1,  -1,   3,   1,  -3,   2,  -1,   1,   1,  -2,  -1,  -1,  -1,   1,   1,   1,
		 1,   1,  -1,  -1,  -1,   2,  -1,  -2,   1,   2,  -2,  -1,   1,   1,   2,  -1,
		 -1,   1,  -2,  -1,   1,   1,  -1,   2,   1,   2,  -1,   1,  -2,  -1,  -2,  -1,
		 -1,   1,   1,  -1,   1,  -1,   1,   1,   1,  -1,  -1,   1,   4,  -1,  -1,  -4,
		 1,   1,   1,   2,  -1,  -1,   1,  -1,  -1,   1,  -1,  -1,   1,  -2,   1,  -1,
		 1,   1,  -1,  -1,   1,   1,  -1,  -1,   3,   2,  -3,  -2,   2,   5,  -2,   2,
		 2,  -5,  -2,  -2,  -2,   2,  -3,   3,   2,   3,  -3,   2,  -2,  -2,   3,  -3,
		 6,   2,  -2,   3,  -6,   3,  -3,  -3,   3,   7,  -4,   4,  -3,   2,  -7,   2,
		 2,  -2,  -4,   2,   8,  -2,  -2,  -2,   4,   2,  -2,   2,   3,   2,  -2,  -2,
		 2,   2,  -2,  -8,  -2,   9,  -2,   2,  -3,  -2,   2,  -2,   2,   2,   2,   4,
		 -2,  -4,  10,   2,   2,  -2,  -9,  -2,   2,  -2,   5,   4,  -4,   4,  -2,   2,
		 -5,  -4,  -3,   4,   2,  -3,   3,  -2,  -5,   5,   3,   3,  -2,  -3, -10,  -4}
	},{
		/* MapTab2 */
		2,  /* eob_sym */
		11, /* esc_sym */
			/* run table */
		{1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  0,  1,  1,  5,  5,
		 2,  2,  6,  6,  7,  7,  1,  8,  1,  8,  3,  3,  9,  9,  1,  2,
		 2,  1,  4, 10,  4, 10, 11, 11,  1,  5, 12, 12,  1,  5, 13, 13,
		 3,  3,  6,  6,  2,  2, 14, 14, 16, 16, 15,  7, 15,  8,  8,  7,
		 1,  1, 17, 17,  4,  4,  1,  1, 18, 18,  2,  2,  5,  5, 25,  3,
		 9,  3, 25,  9, 19, 24, 19, 24,  1, 21, 20,  1, 21, 22, 20, 22,
		 23, 23,  8,  6, 33,  6,  8, 33,  7,  7, 26, 26,  1, 32,  1, 32,
		 28,  4, 28, 10, 29, 27, 27, 10, 41,  4, 29,  2,  2, 41, 36, 31,
		 49, 31, 34, 30, 34, 36, 30, 35,  1, 49, 11,  5, 35, 11,  1,  3,
		 3,  5, 37, 37,  8, 40,  8, 40, 12, 12, 42, 42,  1, 38, 16, 57,
		 1,  6, 16, 39, 38,  6,  7,  7, 13, 13, 39, 43,  2, 43, 57,  2,
		 50,  9, 44,  9, 50,  4, 15, 48, 44,  4,  1, 15, 48, 14, 14,  1,
		 45, 45,  8,  3,  5,  8, 51, 47,  3, 46, 46, 47,  5, 51,  1, 17,
		 17, 58,  1, 58,  2, 52, 52,  2, 53,  7, 59,  6,  6, 56, 53, 55,
		 7, 55,  1, 54, 59, 56, 54, 10,  1, 10,  4, 60,  1, 60,  8,  4,
		 8, 64, 64, 61,  1, 63,  3, 63, 62, 61,  5, 11,  5,  3, 11, 62},

		/* value table */
		{ 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   0,   3,  -3,   1,  -1,
		  2,  -2,   1,  -1,   1,  -1,   4,   1,  -4,  -1,   2,  -2,   1,  -1,   5,   3,
		  -3,  -5,   2,   1,  -2,  -1,   1,  -1,   6,   2,   1,  -1,  -6,  -2,   1,  -1,
		  3,  -3,   2,  -2,   4,  -4,   1,  -1,   1,  -1,   1,   2,  -1,   2,  -2,  -2,
		  7,  -7,   1,  -1,   3,  -3,   8,  -8,   1,  -1,   5,  -5,   3,  -3,   1,   4,
		  2,  -4,  -1,  -2,   1,   1,  -1,  -1,   9,   1,   1,  -9,  -1,   1,  -1,  -1,
		  1,  -1,   3,  -3,   1,   3,  -3,  -1,   3,  -3,   1,  -1,  10,   1, -10,  -1,
		  1,   4,  -1,   2,   1,  -1,   1,  -2,   1,  -4,  -1,   6,  -6,  -1,   1,   1,
		  1,  -1,   1,   1,  -1,  -1,  -1,   1,  11,  -1,  -2,   4,  -1,   2, -11,   5,
		  -5,  -4,  -1,   1,   4,   1,  -4,  -1,  -2,   2,   1,  -1,  12,   1,  -2,   1,
		  -12,   4,   2,   1,  -1,  -4,   4,  -4,   2,  -2,  -1,   1,   7,  -1,  -1,  -7,
		  -1,  -3,   1,   3,   1,   5,   2,   1,  -1,  -5,  13,  -2,  -1,   2,  -2, -13,
		  1,  -1,   5,   6,   5,  -5,   1,   1,  -6,   1,  -1,  -1,  -5,  -1,  14,   2,
		  -2,   1, -14,  -1,   8,   1,  -1,  -8,   1,   5,   1,   5,  -5,   1,  -1,   1,
		  -5,  -1,  15,   1,  -1,  -1,  -1,   3, -15,  -3,   6,   1,  16,  -1,   6,  -6,
		  -6,   1,  -1,   1, -16,   1,   7,  -1,   1,  -1,  -6,  -3,   6,  -7,   3,  -1}
	},{
		/* MapTab3 */
		0,  /* eob_sym */
		35, /* esc_sym */
			/* run table */
		{0,  1,  1,  2,  2,  3,  3,  4,  4,  1,  1,  5,  5,  6,  6,  7,
		 7,  8,  8,  9,  9,  2,  2, 10, 10,  1,  1, 11, 11, 12, 12,  3,
		 3, 13, 13,  0, 14, 14, 16, 15, 16, 15,  4,  4, 17,  1, 17,  1,
		 5,  5, 18, 18,  2,  2,  6,  6,  8, 19,  7,  8,  7, 19, 20, 20,
		 21, 21, 22, 24, 22, 24, 23, 23,  1,  1, 25, 25,  3,  3, 26, 26,
		 9,  9, 27, 27, 28, 28, 33, 29,  4, 33, 29,  1,  4,  1, 32, 32,
		 2,  2, 31, 10, 30, 10, 30, 31, 34, 34,  5,  5, 36, 36, 35, 41,
		 35, 11, 41, 11, 37,  1,  8,  8, 37,  6,  1,  6, 40,  7,  7, 40,
		 12, 38, 12, 39, 39, 38, 49, 13, 49, 13,  3, 42,  3, 42, 16, 16,
		 43, 43, 14, 14,  1,  1, 44, 15, 44, 15,  2,  2, 57, 48, 50, 48,
		 57, 50,  4, 45, 45,  4, 46, 47, 47, 46,  1, 51,  1, 17, 17, 51,
		 8,  9,  9,  5, 58,  8, 58,  5, 52, 52, 55, 56, 53, 56, 55, 59,
		 59, 53, 54,  1,  6, 54,  7,  7,  6,  1,  2,  3,  2,  3, 64, 60,
		 60, 10, 10, 64, 61, 62, 61, 63,  1, 63, 62,  1, 18, 24, 18,  4,
		 25,  4,  8, 21, 21,  1, 24, 22, 25, 22,  8, 11, 19, 11, 23,  1,
		 20, 23, 19, 20,  5, 12,  5,  1, 16,  2, 12, 13,  2, 13,  1, 16},

		/* value table */
		{ 0,   1,  -1,   1,  -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,
		  -1,   1,  -1,   1,  -1,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   1,  -1,   2,
		  -2,   1,  -1,   0,   1,  -1,   1,   1,  -1,  -1,   2,  -2,   1,   4,  -1,  -4,
		  2,  -2,   1,  -1,  -3,   3,   2,  -2,   2,   1,   2,  -2,  -2,  -1,   1,  -1,
		  1,  -1,   1,   1,  -1,  -1,   1,  -1,   5,  -5,   1,  -1,   3,  -3,   1,  -1,
		  2,  -2,   1,  -1,   1,  -1,   1,   1,   3,  -1,  -1,   6,  -3,  -6,  -1,   1,
		  4,  -4,   1,   2,   1,  -2,  -1,  -1,   1,  -1,   3,  -3,   1,  -1,   1,   1,
		  -1,   2,  -1,  -2,   1,   7,  -3,   3,  -1,   3,  -7,  -3,   1,  -3,   3,  -1,
		  2,   1,  -2,   1,  -1,  -1,   1,   2,  -1,  -2,  -4,  -1,   4,   1,   2,  -2,
		  1,  -1,  -2,   2,   8,  -8,  -1,   2,   1,  -2,  -5,   5,   1,  -1,  -1,   1,
		  -1,   1,   4,  -1,   1,  -4,  -1,  -1,   1,   1,   9,   1,  -9,   2,  -2,  -1,
		  -4,   3,  -3,  -4,  -1,   4,   1,   4,   1,  -1,   1,  -1,   1,   1,  -1,   1,
		  -1,  -1,  -1,  10,   4,   1,   4,  -4,  -4, -10,   6,   5,  -6,  -5,   1,  -1,
		  1,   3,  -3,  -1,   1,  -1,  -1,  -1,  11,   1,   1, -11,  -2,  -2,   2,   5,
		  -2,  -5,  -5,   2,  -2,  12,   2,  -2,   2,   2,   5,  -3,  -2,   3,  -2, -12,
		  -2,   2,   2,   2,  -5,   3,   5,  13,  -3,   7,  -3,  -3,  -7,   3, -13,   3}
	},{
		/* MapTab4 */
		0,  /* eob_sym */
		34, /* esc_sym */
			/* run table */
		{0,  1,  1,  1,  2,  2,  1,  3,  3,  1,  1,  1,  4,  4,  1,  5,
		 2,  1,  5,  2,  1,  1,  6,  6,  1,  1,  1,  1,  1,  7,  3,  1,
		 2,  3,  0,  1,  2,  7,  1,  1,  1,  8,  1,  1,  8,  1,  1,  1,
		 9,  1,  9,  1,  2,  1,  1,  2,  1,  1, 10,  4,  1, 10,  1,  4,
		 1,  1,  1,  1,  1,  3,  1,  1,  1,  3,  2,  1,  5,  1,  1,  1,
		 2,  5,  1, 11,  1, 11,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		 2,  1,  6,  1,  6,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1, 12,
		 3,  1, 12,  1,  1,  1,  2,  1,  1,  3,  1,  1,  1,  1,  1,  1,
		 4,  1,  1,  1,  2,  1,  1,  4,  1,  1,  1,  1,  1,  1,  2,  1,
		 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  2,  1,  1,  5,
		 1,  1,  1,  1,  1,  7,  1,  7,  1,  1,  2,  3,  1,  1,  1,  1,
		 5,  1,  1,  1,  1,  1,  1,  2, 13,  1,  1,  1,  1,  1,  1,  1,
		 1,  1,  1,  1,  1,  1,  1,  1, 13,  2,  1,  1,  4,  1,  1,  1,
		 3,  1,  6,  1,  1,  1, 14,  1,  1,  1,  1,  1, 14,  6,  1,  1,
		 1,  1, 15,  2,  4,  1,  2,  3, 15,  1,  1,  1,  8,  1,  1,  8,
		 1,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1},

		/* value table */
		{ 0,   1,  -1,   2,   1,  -1,  -2,   1,  -1,   3,  -3,   4,   1,  -1,  -4,   1,
		  2,   5,  -1,  -2,  -5,   6,   1,  -1,  -6,   7,  -7,   8,  -8,   1,   2,   9,
		  3,  -2,   0,  -9,  -3,  -1,  10, -10,  11,   1, -11,  12,  -1, -12,  13, -13,
		  1,  14,  -1, -14,   4,  15, -15,  -4,  16, -16,   1,   2,  17,  -1, -17,  -2,
		  18, -18,  19, -19,  20,   3, -20,  21, -21,  -3,   5,  22,   2, -22, -23,  23,
		  -5,  -2,  24,   1, -24,  -1,  25, -25,  26, -26, -27,  27,  28,  29, -28, -29,
		  6,  30,   2, -31,  -2, -30,  31,  -6, -32,  32,  33, -33,  34, -35, -34,   1,
		  4, -36,  -1,  35,  37,  36,   7, -37,  38,  -4, -38,  39,  41,  40, -40, -39,
		  3,  42, -43, -41,  -7, -42,  43,  -3,  44, -44,  45, -45,  46,  47,   8, -47,
		  -48, -46,  50, -50,  48,  49,  51, -49,  52, -52,   5, -51,  -8, -53,  53,   3,
		  -56,  56,  55,  54, -54,   2,  60,  -2, -55,  58,   9,  -5,  59,  57, -57, -63,
		  -3, -58, -60, -61,  61, -59, -62,  -9,   1,  64,  62,  69, -64,  63,  65, -67,
		  -68,  66, -65,  68, -66, -69,  67, -70,  -1,  10,  71, -71,   4,  73,  72,  70,
		  6, -76,  -3,  74, -78, -74,   1,  78,  80, -72, -75,  76,  -1,   3, -73,  79,
		  75,  77,   1,  11,  -4, -79, -10,  -6,  -1, -77, -83, -80,   2,  81, -84,  -2,
		  83, -81,  82, -82,  84, -87, -86,  85, -11, -85,  86, -89,  87, -88,  88,  89}
	},{
		/* MapTab5 */
		2,  /* eob_sym */
		33, /* esc_sym */
			/* run table */
		{1,  1,  0,  2,  1,  2,  1,  3,  3,  1,  1,  4,  4,  2,  2,  1,
		 1,  5,  5,  6,  1,  6,  1,  7,  7,  3,  3,  2,  8,  2,  8,  1,
		 1,  0,  9,  9,  1,  1, 10,  4, 10,  4, 11, 11,  2,  1,  2,  1,
		 12, 12,  3,  3,  1,  1, 13,  5,  5, 13, 14,  1,  1, 14,  2,  2,
		 6,  6, 15,  1,  1, 15, 16,  4,  7, 16,  4,  7,  1,  1,  3,  3,
		 8,  8,  2,  2,  1,  1, 17, 17,  1,  1, 18, 18,  5,  5,  2,  2,
		 1,  1,  9, 19,  9, 19, 20,  3,  3, 20,  1, 10, 21,  1, 10,  4,
		 4, 21, 22,  6,  6, 22,  1,  1, 23, 24,  2,  2, 23, 24, 11,  1,
		 1, 11,  7, 25,  7,  1,  1, 25,  8,  8,  3, 26,  3,  1, 12,  2,
		 2, 26,  1, 12,  5,  5, 27,  4,  1,  4,  1, 27, 28,  1, 28, 13,
		 1, 13,  2, 29,  2,  1, 32,  6,  1, 30, 14, 29, 14,  6,  3, 31,
		 3,  1, 30,  1, 32, 31, 33,  9, 33,  1,  1,  7,  9,  7,  2,  2,
		 1,  1,  4, 36, 34,  4,  5, 10, 10,  5, 34,  1,  1, 35,  8,  8,
		 36,  3, 35,  1, 15,  3,  2,  1, 16, 15, 16,  2, 37,  1, 37,  1,
		 1,  1,  6,  6, 38,  1, 38, 11,  1, 39, 39, 40, 11,  2, 41,  4,
		 40,  1,  2,  4,  1,  1,  1, 41,  3,  1,  3,  1,  5,  7,  5,  7},

		/* value table */
		{ 1,  -1,   0,   1,   2,  -1,  -2,   1,  -1,   3,  -3,   1,  -1,   2,  -2,   4,
		  -4,   1,  -1,   1,   5,  -1,  -5,   1,  -1,   2,  -2,   3,   1,  -3,  -1,   6,
		  -6,   0,   1,  -1,   7,  -7,   1,   2,  -1,  -2,   1,  -1,   4,   8,  -4,  -8,
		  1,  -1,   3,  -3,   9,  -9,   1,   2,  -2,  -1,   1,  10, -10,  -1,   5,  -5,
		  2,  -2,   1,  11, -11,  -1,   1,   3,   2,  -1,  -3,  -2,  12, -12,   4,  -4,
		  2,  -2,  -6,   6,  13, -13,   1,  -1,  14, -14,   1,  -1,   3,  -3,   7,  -7,
		  15, -15,   2,   1,  -2,  -1,   1,   5,  -5,  -1, -16,   2,   1,  16,  -2,   4,
		  -4,  -1,   1,   3,  -3,  -1,  17, -17,   1,   1,  -8,   8,  -1,  -1,   2,  18,
		  -18,  -2,   3,   1,  -3,  19, -19,  -1,   3,  -3,   6,   1,  -6,  20,   2,   9,
		  -9,  -1, -20,  -2,   4,  -4,   1,  -5,  21,   5, -21,  -1,   1, -22,  -1,   2,
		  22,  -2,  10,   1, -10,  23,   1,   4, -23,   1,   2,  -1,  -2,  -4,  -7,   1,
		  7, -24,  -1,  24,  -1,  -1,   1,   3,  -1, -25,  25,   4,  -3,  -4,  11, -11,
		  26, -26,   6,   1,   1,  -6,  -5,  -3,   3,   5,  -1, -27,  27,   1,   4,  -4,
		  -1,  -8,  -1,  28,   2,   8, -12, -28,  -2,  -2,   2,  12,  -1,  29,   1, -29,
		  30, -30,   5,  -5,   1, -31,  -1,   3,  31,  -1,   1,   1,  -3, -13,   1,  -7,
		  -1, -32,  13,   7,  32,  33, -33,  -1,  -9, -34,   9,  34,  -6,   5,   6,  -5}
	},{
		/* MapTab6 */
		2,  /* eob_sym */
		13, /* esc_sym */
			/* run table */
		{1,  1,  0,  1,  1,  2,  2,  1,  1,  3,  3,  1,  1,  0,  2,  2,
		 4,  1,  4,  1,  1,  1,  5,  5,  1,  1,  6,  6,  2,  2,  1,  1,
		 3,  3,  7,  7,  1,  1,  8,  8,  1,  1,  2,  2,  1,  9,  1,  9,
		 4,  4, 10,  1,  1, 10,  1,  1, 11, 11,  3,  3,  1,  2,  1,  2,
		 1,  1, 12, 12,  5,  5,  1,  1, 13,  1,  1, 13,  2,  2,  1,  1,
		 6,  6,  1,  1,  4, 14,  4, 14,  3,  1,  3,  1,  1,  1, 15,  7,
		 15,  2,  2,  7,  1,  1,  1,  8,  1,  8, 16, 16,  1,  1,  1,  1,
		 2,  1,  1,  2,  1,  1,  3,  5,  5,  3,  4,  1,  1,  4,  1,  1,
		 17, 17,  9,  1,  1,  9,  2,  2,  1,  1, 10, 10,  1,  6,  1,  1,
		 6, 18,  1,  1, 18,  1,  1,  1,  2,  2,  3,  1,  3,  1,  1,  1,
		 4,  1, 19,  1, 19,  7,  1,  1, 20,  1,  4, 20,  1,  7, 11,  2,
		 1, 11, 21,  2,  8,  5,  1,  8,  1,  5, 21,  1,  1,  1, 22,  1,
		 1, 22,  1,  1,  3,  3,  1, 23,  2, 12, 24,  1,  1,  2,  1,  1,
		 12, 23,  1,  1, 24,  1,  1,  1,  4,  1,  1,  1,  2,  1,  6,  6,
		 4,  2,  1,  1,  1,  1,  1,  1,  1, 14, 13,  3,  1, 25,  9, 25,
		 14,  1,  9,  3, 13,  1,  1,  1,  1,  1, 10,  1,  1,  2, 10,  2},

		/* value table */
		{-20,  -1,   0,   2,  -2,   1,  -1,   3,  -3,   1,  -1,   4,  -4,   0,   2,  -2,
		 1,   5,  -1,  -5,   6,  -6,   1,  -1,   7,  -7,   1,  -1,   3,  -3,   8,  -8,
		 2,  -2,   1,  -1,   9,  -9,   1,  -1,  10, -10,   4,  -4,  11,   1, -11,  -1,
		 2,  -2,   1,  12, -12,  -1,  13, -13,   1,  -1,   3,  -3,  14,   5, -14,  -5,
		 -15,  15,  -1,   1,   2,  -2,  16, -16,   1,  17, -17,  -1,   6,  -6,  18, -18,
		 2,  -2, -19,  19,  -3,   1,   3,  -1,   4,  20,  -4,   1, -21,  21,   1,   2,
		 -1,  -7,   7,  -2,  22, -22,  23,   2, -23,  -2,   1,  -1, -24,  24, -25,  25,
		 -8, -26,  26,   8, -27,  27,   5,   3,  -3,  -5,  -4,  28, -28,   4,  29, -29,
		 1,  -1,  -2, -30,  30,   2,   9,  -9, -31,  31,   2,  -2, -32,   3,  32, -33,
		 -3,   1,  33, -34,  -1,  34, -35,  35, -10,  10,  -6,  36,   6, -36,  37, -37,
		 -5,  38,   1, -38,  -1,   3,  39, -39,  -1,  40,   5,   1, -40,  -3,   2, -11,
		 -41,  -2,   1,  11,  -3,  -4,  41,   3,  42,   4,  -1, -43, -42,  43,   1, -44,
		 45,  -1,  44, -45,  -7,   7, -46,   1, -12,   2,   1, -47,  46,  12,  47,  48,
		 -2,  -1, -48,  49,  -1, -50, -49,  50,  -6, -51,  51,  52, -13,  53,  -4,   4,
		 6,  13, -53, -52, -54,  55,  54, -55, -56,  -2,   2,  -8,  56,   1,  -3,  -1,
		 2,  58,   3,   8,  -2,  57, -58, -60, -59, -57,  -3,  60,  59, -14,   3,  14}
	},{
		/* MapTab7 */
		2,  /* eob_sym */
		38, /* esc_sym */
			/* run table */
		{1,  1,  0,  2,  2,  1,  1,  3,  3,  4,  4,  5,  5,  1,  1,  6,
		 6,  2,  2,  7,  7,  8,  8,  1,  1,  3,  3,  9,  9, 10, 10,  1,
		 1,  2,  2,  4,  4, 11,  0, 11, 12, 12, 13, 13,  1,  1,  5,  5,
		 14, 14, 15, 16, 15, 16,  3,  3,  1,  6,  1,  6,  2,  2,  7,  7,
		 8,  8, 17, 17,  1,  1,  4,  4, 18, 18,  2,  2,  1, 19,  1, 20,
		 19, 20, 21, 21,  3,  3, 22, 22,  5,  5, 24,  1,  1, 23,  9, 23,
		 24,  9,  2,  2, 10,  1,  1, 10,  6,  6, 25,  4,  4, 25,  7,  7,
		 26,  8,  1,  8,  3,  1, 26,  3, 11, 11, 27, 27,  2, 28,  1,  2,
		 28,  1, 12, 12,  5,  5, 29, 13, 13, 29, 32,  1,  1, 33, 31, 30,
		 32,  4, 30, 33,  4, 31,  3, 14,  1,  1,  3, 34, 34,  2,  2, 14,
		 6,  6, 35, 36, 35, 36,  1, 15,  1, 16, 16, 15,  7,  9,  7,  9,
		 37,  8,  8, 37,  1,  1, 39,  2, 38, 39,  2, 40,  5, 38, 40,  5,
		 3,  3,  4,  4, 10, 10,  1,  1,  1,  1, 41,  2, 41,  2,  6,  6,
		 1,  1, 11, 42, 11, 43,  3, 42,  3, 17,  4, 43,  1, 17,  7,  1,
		 8, 44,  4,  7, 44,  5,  8,  2,  5,  1,  2, 48, 45,  1, 12, 45,
		 12, 48, 13, 13,  1,  9,  9, 46,  1, 46, 47, 47, 49, 18, 18, 49},

		/* value table */
		{ 1,  -1,   0,   1,  -1,   2,  -2,   1,  -1,   1,  -1,   1,  -1,   3,  -3,   1,
		  -1,  -2,   2,   1,  -1,   1,  -1,   4,  -4,  -2,   2,   1,  -1,   1,  -1,   5,
		  -5,  -3,   3,   2,  -2,   1,   0,  -1,   1,  -1,   1,  -1,   6,  -6,   2,  -2,
		  1,  -1,   1,   1,  -1,  -1,  -3,   3,   7,   2,  -7,  -2,  -4,   4,   2,  -2,
		  2,  -2,   1,  -1,   8,  -8,   3,  -3,   1,  -1,  -5,   5,   9,   1,  -9,   1,
		  -1,  -1,   1,  -1,  -4,   4,   1,  -1,   3,  -3,   1, -10,  10,   1,   2,  -1,
		  -1,  -2,   6,  -6,   2,  11, -11,  -2,   3,  -3,   1,  -4,   4,  -1,   3,  -3,
		  1,   3,  12,  -3,  -5, -12,  -1,   5,   2,  -2,   1,  -1,  -7,   1,  13,   7,
		  -1, -13,   2,  -2,   4,  -4,   1,   2,  -2,  -1,   1,  14, -14,   1,   1,   1,
		  -1,  -5,  -1,  -1,   5,  -1,  -6,   2, -15,  15,   6,   1,  -1,  -8,   8,  -2,
		  -4,   4,   1,   1,  -1,  -1,  16,   2, -16,  -2,   2,  -2,   4,   3,  -4,  -3,
		  -1,  -4,   4,   1, -17,  17,  -1,  -9,   1,   1,   9,   1,  -5,  -1,  -1,   5,
		  -7,   7,   6,  -6,   3,  -3,  18, -18,  19, -19,   1, -10,  -1,  10,  -5,   5,
		  20, -20,  -3,   1,   3,   1,   8,  -1,  -8,   2,   7,  -1, -21,  -2,   5,  21,
		  5,  -1,  -7,  -5,   1,  -6,  -5, -11,   6,  22,  11,   1,   1, -22,  -3,  -1,
		  3,  -1,   3,  -3, -23,   4,  -4,   1,  23,  -1,   1,  -1,   1,  -2,   2,  -1}
	},{
		/* MapTab8 */
		4,  /* eob_sym */
		11, /* esc_sym */
			/* run table */
		{1,  1,  1,  1,  0,  2,  2,  1,  1,  3,  3,  0,  1,  1,  2,  2,
		 4,  4,  1,  1,  5,  5,  1,  1,  2,  2,  3,  3,  6,  6,  1,  1,
		 7,  7,  8,  1,  8,  2,  2,  1,  4,  4,  1,  3,  1,  3,  9,  9,
		 2,  2,  1,  5,  1,  5, 10, 10,  1,  1, 11, 11,  3,  6,  3,  4,
		 4,  6,  2,  2,  1, 12,  1, 12,  7, 13,  7, 13,  1,  1,  8,  8,
		 2,  2, 14, 14, 16, 15, 16,  5,  5,  1,  3, 15,  1,  3,  4,  4,
		 1,  1, 17, 17,  2,  2,  6,  6,  1, 18,  1, 18, 22, 21, 22, 21,
		 25, 24, 25, 19,  9, 20,  9, 23, 19, 24, 20,  3, 23,  7,  3,  1,
		 1,  7, 28, 26, 29,  5, 28, 26,  5,  8, 29,  4,  8, 27,  2,  2,
		 4, 27,  1,  1, 10, 36, 10, 33, 33, 36, 30,  1, 32, 32,  1, 30,
		 6, 31, 31, 35,  3,  6, 11, 11,  3,  2, 35,  2, 34,  1, 34,  1,
		 37, 37, 12,  7, 12,  5, 41,  5,  4,  7,  1,  8, 13,  4,  1, 41,
		 13, 38,  8, 38,  9,  1, 40, 40,  9,  1, 39,  2,  2, 49, 39, 42,
		 3,  3, 14, 16, 49, 14, 16, 42, 43, 43,  6,  6, 15,  1,  1, 15,
		 44, 44,  1,  1, 50, 48,  4,  5,  4,  7,  5,  2, 10, 10, 48,  7,
		 50, 45,  2,  1, 45,  8,  8,  1, 46, 46,  3, 47, 47,  3,  1,  1},

		/* value table */
		{ 1,  -1,   2,  -2,   0,   1,  -1,   3,  -3,   1,  -1,   0,   4,  -4,   2,  -2,
		  1,  -1,   5,  -5,   1,  -1,   6,  -6,   3,  -3,   2,  -2,   1,  -1,   7,  -7,
		  1,  -1,   1,   8,  -1,   4,  -4,  -8,   2,  -2,   9,   3,  -9,  -3,   1,  -1,
		  5,  -5,  10,   2, -10,  -2,   1,  -1,  11, -11,   1,  -1,  -4,   2,   4,   3,
		  -3,  -2,   6,  -6,  12,   1, -12,  -1,   2,   1,  -2,  -1,  13, -13,   2,  -2,
		  7,  -7,   1,  -1,   1,   1,  -1,   3,  -3,  14,   5,  -1, -14,  -5,   4,  -4,
		  15, -15,   1,  -1,   8,  -8,  -3,   3,  16,   1, -16,  -1,   1,   1,  -1,  -1,
		  1,   1,  -1,   1,   2,   1,  -2,   1,  -1,  -1,  -1,   6,  -1,   3,  -6,  17,
		  -17,  -3,   1,   1,   1,   4,  -1,  -1,  -4,   3,  -1,   5,  -3,  -1,  -9,   9,
		  -5,   1,  18, -18,   2,   1,  -2,   1,  -1,  -1,   1,  19,  -1,   1, -19,  -1,
		  4,   1,  -1,   1,   7,  -4,  -2,   2,  -7,  10,  -1, -10,   1,  20,  -1, -20,
		  1,  -1,   2,   4,  -2,   5,   1,  -5,   6,  -4,  21,   4,   2,  -6, -21,  -1,
		  -2,   1,  -4,  -1,  -3,  22,  -1,   1,   3, -22,  -1,  11, -11,   1,   1,   1,
		  8,  -8,   2,   2,  -1,  -2,  -2,  -1,   1,  -1,  -5,   5,   2,  23, -23,  -2,
		  1,  -1,  24, -24,  -1,  -1,   7,   6,  -7,   5,  -6,  12,  -3,   3,   1,  -5,
		  1,   1, -12,  25,  -1,  -5,   5, -25,  -1,   1,   9,   1,  -1,  -9,  26, -26}
	}
};

/**
 * @file
 * DSP functions (inverse transforms, motion compensation, wavelet recompostions)
 * for Indeo Video Interactive codecs.
 */

void ff_ivi_recompose53(const IVIPlaneDesc *plane, uint8 *dst,
						const int dst_pitch) {
	int             x, y, indx;
	int32         p0, p1, p2, p3, tmp0, tmp1, tmp2;
	int32         b0_1, b0_2, b1_1, b1_2, b1_3, b2_1, b2_2, b2_3, b2_4, b2_5, b2_6;
	int32         b3_1, b3_2, b3_3, b3_4, b3_5, b3_6, b3_7, b3_8, b3_9;
	int32         pitch, back_pitch;
	const short     *b0_ptr, *b1_ptr, *b2_ptr, *b3_ptr;
	const int       num_bands = 4;

	/* all bands should have the same pitch */
	pitch = plane->bands[0].pitch;

	/* pixels at the position "y-1" will be set to pixels at the "y" for the 1st iteration */
	back_pitch = 0;

	/* get pointers to the wavelet bands */
	b0_ptr = plane->bands[0].buf;
	b1_ptr = plane->bands[1].buf;
	b2_ptr = plane->bands[2].buf;
	b3_ptr = plane->bands[3].buf;

	for (y = 0; y < plane->height; y += 2) {

		if (y+2 >= plane->height)
			pitch= 0;
		/* load storage variables with values */
		if (num_bands > 0) {
			b0_1 = b0_ptr[0];
			b0_2 = b0_ptr[pitch];
		}

		if (num_bands > 1) {
			b1_1 = b1_ptr[back_pitch];
			b1_2 = b1_ptr[0];
			b1_3 = b1_1 - b1_2*6 + b1_ptr[pitch];
		}

		if (num_bands > 2) {
			b2_2 = b2_ptr[0];     // b2[x,  y  ]
			b2_3 = b2_2;          // b2[x+1,y  ] = b2[x,y]
			b2_5 = b2_ptr[pitch]; // b2[x  ,y+1]
			b2_6 = b2_5;          // b2[x+1,y+1] = b2[x,y+1]
		}

		if (num_bands > 3) {
			b3_2 = b3_ptr[back_pitch]; // b3[x  ,y-1]
			b3_3 = b3_2;               // b3[x+1,y-1] = b3[x  ,y-1]
			b3_5 = b3_ptr[0];          // b3[x  ,y  ]
			b3_6 = b3_5;               // b3[x+1,y  ] = b3[x  ,y  ]
			b3_8 = b3_2 - b3_5*6 + b3_ptr[pitch];
			b3_9 = b3_8;
		}

		for (x = 0, indx = 0; x < plane->width; x+=2, indx++) {
			if (x+2 >= plane->width) {
				b0_ptr --;
				b1_ptr --;
				b2_ptr --;
				b3_ptr --;
			}

			/* some values calculated in the previous iterations can */
			/* be reused in the next ones, so do appropriate copying */
			b2_1 = b2_2; // b2[x-1,y  ] = b2[x,  y  ]
			b2_2 = b2_3; // b2[x  ,y  ] = b2[x+1,y  ]
			b2_4 = b2_5; // b2[x-1,y+1] = b2[x  ,y+1]
			b2_5 = b2_6; // b2[x  ,y+1] = b2[x+1,y+1]
			b3_1 = b3_2; // b3[x-1,y-1] = b3[x  ,y-1]
			b3_2 = b3_3; // b3[x  ,y-1] = b3[x+1,y-1]
			b3_4 = b3_5; // b3[x-1,y  ] = b3[x  ,y  ]
			b3_5 = b3_6; // b3[x  ,y  ] = b3[x+1,y  ]
			b3_7 = b3_8; // vert_HPF(x-1)
			b3_8 = b3_9; // vert_HPF(x  )

			p0 = p1 = p2 = p3 = 0;

			/* process the LL-band by applying LPF both vertically and horizontally */
			if (num_bands > 0) {
				tmp0 = b0_1;
				tmp2 = b0_2;
				b0_1 = b0_ptr[indx+1];
				b0_2 = b0_ptr[pitch+indx+1];
				tmp1 = tmp0 + b0_1;

				p0 =  tmp0 << 4;
				p1 =  tmp1 << 3;
				p2 = (tmp0 + tmp2) << 3;
				p3 = (tmp1 + tmp2 + b0_2) << 2;
			}

			/* process the HL-band by applying HPF vertically and LPF horizontally */
			if (num_bands > 1) {
				tmp0 = b1_2;
				tmp1 = b1_1;
				b1_2 = b1_ptr[indx+1];
				b1_1 = b1_ptr[back_pitch+indx+1];

				tmp2 = tmp1 - tmp0*6 + b1_3;
				b1_3 = b1_1 - b1_2*6 + b1_ptr[pitch+indx+1];

				p0 += (tmp0 + tmp1) << 3;
				p1 += (tmp0 + tmp1 + b1_1 + b1_2) << 2;
				p2 +=  tmp2 << 2;
				p3 += (tmp2 + b1_3) << 1;
			}

			/* process the LH-band by applying LPF vertically and HPF horizontally */
			if (num_bands > 2) {
				b2_3 = b2_ptr[indx+1];
				b2_6 = b2_ptr[pitch+indx+1];

				tmp0 = b2_1 + b2_2;
				tmp1 = b2_1 - b2_2*6 + b2_3;

				p0 += tmp0 << 3;
				p1 += tmp1 << 2;
				p2 += (tmp0 + b2_4 + b2_5) << 2;
				p3 += (tmp1 + b2_4 - b2_5*6 + b2_6) << 1;
			}

			/* process the HH-band by applying HPF both vertically and horizontally */
			if (num_bands > 3) {
				b3_6 = b3_ptr[indx+1];            // b3[x+1,y  ]
				b3_3 = b3_ptr[back_pitch+indx+1]; // b3[x+1,y-1]

				tmp0 = b3_1 + b3_4;
				tmp1 = b3_2 + b3_5;
				tmp2 = b3_3 + b3_6;

				b3_9 = b3_3 - b3_6*6 + b3_ptr[pitch+indx+1];

				p0 += (tmp0 + tmp1) << 2;
				p1 += (tmp0 - tmp1*6 + tmp2) << 1;
				p2 += (b3_7 + b3_8) << 1;
				p3 +=  b3_7 - b3_8*6 + b3_9;
			}

			/* output four pixels */
			dst[x]             = CLIP((p0 >> 6) + 128, 0, 255);
			dst[x+1]           = CLIP((p1 >> 6) + 128, 0, 255);
			dst[dst_pitch+x]   = CLIP((p2 >> 6) + 128, 0, 255);
			dst[dst_pitch+x+1] = CLIP((p3 >> 6) + 128, 0, 255);
		}// for x

		dst += dst_pitch << 1;

		back_pitch = -pitch;

		b0_ptr += pitch + 1;
		b1_ptr += pitch + 1;
		b2_ptr += pitch + 1;
		b3_ptr += pitch + 1;
	}
}

void ff_ivi_recompose_haar(const IVIPlaneDesc *plane, uint8 *dst,
						   const int dst_pitch) {
	int             x, y, indx, b0, b1, b2, b3, p0, p1, p2, p3;
	const short     *b0_ptr, *b1_ptr, *b2_ptr, *b3_ptr;
	int32         pitch;

	/* all bands should have the same pitch */
	pitch = plane->bands[0].pitch;

	/* get pointers to the wavelet bands */
	b0_ptr = plane->bands[0].buf;
	b1_ptr = plane->bands[1].buf;
	b2_ptr = plane->bands[2].buf;
	b3_ptr = plane->bands[3].buf;

	for (y = 0; y < plane->height; y += 2) {
		for (x = 0, indx = 0; x < plane->width; x += 2, indx++) {
			/* load coefficients */
			b0 = b0_ptr[indx]; //should be: b0 = (num_bands > 0) ? b0_ptr[indx] : 0;
			b1 = b1_ptr[indx]; //should be: b1 = (num_bands > 1) ? b1_ptr[indx] : 0;
			b2 = b2_ptr[indx]; //should be: b2 = (num_bands > 2) ? b2_ptr[indx] : 0;
			b3 = b3_ptr[indx]; //should be: b3 = (num_bands > 3) ? b3_ptr[indx] : 0;

			/* haar wavelet recomposition */
			p0 = (b0 + b1 + b2 + b3 + 2) >> 2;
			p1 = (b0 + b1 - b2 - b3 + 2) >> 2;
			p2 = (b0 - b1 + b2 - b3 + 2) >> 2;
			p3 = (b0 - b1 - b2 + b3 + 2) >> 2;

			/* bias, convert and output four pixels */
			dst[x]                 = CLIP(p0 + 128, 0, 255);
			dst[x + 1]             = CLIP(p1 + 128, 0, 255);
			dst[dst_pitch + x]     = CLIP(p2 + 128, 0, 255);
			dst[dst_pitch + x + 1] = CLIP(p3 + 128, 0, 255);
		}// for x

		dst += dst_pitch << 1;

		b0_ptr += pitch;
		b1_ptr += pitch;
		b2_ptr += pitch;
		b3_ptr += pitch;
	}// for y
}

/** butterfly operation for the inverse Haar transform */
#define IVI_HAAR_BFLY(s1, s2, o1, o2, t)		\
	t  = ((s1) - (s2)) >> 1;					\
	o1 = ((s1) + (s2)) >> 1;					\
	o2 = (t);									\

/** inverse 8-point Haar transform */
#define INV_HAAR8(s1, s5, s3, s7, s2, s4, s6, s8,						\
				  d1, d2, d3, d4, d5, d6, d7, d8,						\
				  t0, t1, t2, t3, t4, t5, t6, t7, t8) {					\
		t1 = (s1) << 1; t5 = (s5) << 1;									\
		IVI_HAAR_BFLY(t1, t5, t1, t5, t0); IVI_HAAR_BFLY(t1, s3, t1, t3, t0); \
		IVI_HAAR_BFLY(t5, s7, t5, t7, t0); IVI_HAAR_BFLY(t1, s2, t1, t2, t0); \
		IVI_HAAR_BFLY(t3, s4, t3, t4, t0); IVI_HAAR_BFLY(t5, s6, t5, t6, t0); \
		IVI_HAAR_BFLY(t7, s8, t7, t8, t0);								\
		d1 = COMPENSATE(t1);											\
		d2 = COMPENSATE(t2);											\
		d3 = COMPENSATE(t3);											\
		d4 = COMPENSATE(t4);											\
		d5 = COMPENSATE(t5);											\
		d6 = COMPENSATE(t6);											\
		d7 = COMPENSATE(t7);											\
		d8 = COMPENSATE(t8); }

/** inverse 4-point Haar transform */
#define INV_HAAR4(s1, s3, s5, s7, d1, d2, d3, d4, t0, t1, t2, t3, t4) {	\
		IVI_HAAR_BFLY(s1, s3, t0, t1, t4);								\
		IVI_HAAR_BFLY(t0, s5, t2, t3, t4);								\
		d1 = COMPENSATE(t2);											\
		d2 = COMPENSATE(t3);											\
		IVI_HAAR_BFLY(t1, s7, t2, t3, t4);								\
		d3 = COMPENSATE(t2);											\
		d4 = COMPENSATE(t3); }

void ff_ivi_inverse_haar_8x8(const int32 *in, int16 *out, uint32 pitch,
							 const uint8 *flags) {
	int     i, shift, sp1, sp2, sp3, sp4;
	const int32 *src;
	int32 *dst;
	int     tmp[64];
	int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

	/* apply the InvHaar8 to all columns */
#define COMPENSATE(x) (x)
	src = in;
	dst = tmp;
	for (i = 0; i < 8; i++) {
		if (flags[i]) {
			/* pre-scaling */
			shift = !(i & 4);
			sp1 = src[ 0] << shift;
			sp2 = src[ 8] << shift;
			sp3 = src[16] << shift;
			sp4 = src[24] << shift;
			INV_HAAR8(    sp1,     sp2,     sp3,     sp4,
						  src[32], src[40], src[48], src[56],
						  dst[ 0], dst[ 8], dst[16], dst[24],
						  dst[32], dst[40], dst[48], dst[56],
						  t0, t1, t2, t3, t4, t5, t6, t7, t8);
		} else
			dst[ 0] = dst[ 8] = dst[16] = dst[24] =
				dst[32] = dst[40] = dst[48] = dst[56] = 0;

		src++;
		dst++;
	}
#undef  COMPENSATE

	/* apply the InvHaar8 to all rows */
#define COMPENSATE(x) (x)
	src = tmp;
	for (i = 0; i < 8; i++) {
		if (   !src[0] && !src[1] && !src[2] && !src[3]
			   && !src[4] && !src[5] && !src[6] && !src[7]) {
			memset(out, 0, 8 * sizeof(out[0]));
		} else {
			INV_HAAR8(src[0], src[1], src[2], src[3],
					  src[4], src[5], src[6], src[7],
					  out[0], out[1], out[2], out[3],
					  out[4], out[5], out[6], out[7],
					  t0, t1, t2, t3, t4, t5, t6, t7, t8);
		}
		src += 8;
		out += pitch;
	}
#undef  COMPENSATE
}

void ff_ivi_row_haar8(const int32 *in, int16 *out, uint32 pitch,
					  const uint8 *flags) {
	int     i;
	int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

	/* apply the InvHaar8 to all rows */
#define COMPENSATE(x) (x)
	for (i = 0; i < 8; i++) {
		if (   !in[0] && !in[1] && !in[2] && !in[3]
			   && !in[4] && !in[5] && !in[6] && !in[7]) {
			memset(out, 0, 8 * sizeof(out[0]));
		} else {
			INV_HAAR8(in[0],  in[1],  in[2],  in[3],
					  in[4],  in[5],  in[6],  in[7],
					  out[0], out[1], out[2], out[3],
					  out[4], out[5], out[6], out[7],
					  t0, t1, t2, t3, t4, t5, t6, t7, t8);
		}
		in  += 8;
		out += pitch;
	}
#undef  COMPENSATE
}

void ff_ivi_col_haar8(const int32 *in, int16 *out, uint32 pitch,
					  const uint8 *flags) {
	int     i;
	int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

	/* apply the InvHaar8 to all columns */
#define COMPENSATE(x) (x)
	for (i = 0; i < 8; i++) {
		if (flags[i]) {
			INV_HAAR8(in[ 0], in[ 8], in[16], in[24],
					  in[32], in[40], in[48], in[56],
					  out[0 * pitch], out[1 * pitch],
					  out[2 * pitch], out[3 * pitch],
					  out[4 * pitch], out[5 * pitch],
					  out[6 * pitch], out[7 * pitch],
					  t0, t1, t2, t3, t4, t5, t6, t7, t8);
		} else
			out[0 * pitch] = out[1 * pitch] =
				out[2 * pitch] = out[3 * pitch] =
				out[4 * pitch] = out[5 * pitch] =
				out[6 * pitch] = out[7 * pitch] = 0;

		in++;
		out++;
	}
#undef  COMPENSATE
}

void ff_ivi_inverse_haar_4x4(const int32 *in, int16 *out, uint32 pitch,
							 const uint8 *flags) {
	int     i, shift, sp1, sp2;
	const int32 *src;
	int32 *dst;
	int     tmp[16];
	int     t0, t1, t2, t3, t4;

	/* apply the InvHaar4 to all columns */
#define COMPENSATE(x) (x)
	src = in;
	dst = tmp;
	for (i = 0; i < 4; i++) {
		if (flags[i]) {
			/* pre-scaling */
			shift = !(i & 2);
			sp1 = src[0] << shift;
			sp2 = src[4] << shift;
			INV_HAAR4(   sp1,    sp2, src[8], src[12],
						 dst[0], dst[4], dst[8], dst[12],
						 t0, t1, t2, t3, t4);
		} else
			dst[0] = dst[4] = dst[8] = dst[12] = 0;

		src++;
		dst++;
	}
#undef  COMPENSATE

	/* apply the InvHaar8 to all rows */
#define COMPENSATE(x) (x)
	src = tmp;
	for (i = 0; i < 4; i++) {
		if (!src[0] && !src[1] && !src[2] && !src[3]) {
			memset(out, 0, 4 * sizeof(out[0]));
		} else {
			INV_HAAR4(src[0], src[1], src[2], src[3],
					  out[0], out[1], out[2], out[3],
					  t0, t1, t2, t3, t4);
		}
		src += 4;
		out += pitch;
	}
#undef  COMPENSATE
}

void ff_ivi_row_haar4(const int32 *in, int16 *out, uint32 pitch,
					  const uint8 *flags) {
	int     i;
	int     t0, t1, t2, t3, t4;

	/* apply the InvHaar4 to all rows */
#define COMPENSATE(x) (x)
	for (i = 0; i < 4; i++) {
		if (!in[0] && !in[1] && !in[2] && !in[3]) {
			memset(out, 0, 4 * sizeof(out[0]));
		} else {
			INV_HAAR4(in[0], in[1], in[2], in[3],
					  out[0], out[1], out[2], out[3],
					  t0, t1, t2, t3, t4);
		}
		in  += 4;
		out += pitch;
	}
#undef  COMPENSATE
}

void ff_ivi_col_haar4(const int32 *in, int16 *out, uint32 pitch,
					  const uint8 *flags) {
	int     i;
	int     t0, t1, t2, t3, t4;

	/* apply the InvHaar8 to all columns */
#define COMPENSATE(x) (x)
	for (i = 0; i < 4; i++) {
		if (flags[i]) {
			INV_HAAR4(in[0], in[4], in[8], in[12],
					  out[0 * pitch], out[1 * pitch],
					  out[2 * pitch], out[3 * pitch],
					  t0, t1, t2, t3, t4);
		} else
			out[0 * pitch] = out[1 * pitch] =
				out[2 * pitch] = out[3 * pitch] = 0;

		in++;
		out++;
	}
#undef  COMPENSATE
}

void ff_ivi_dc_haar_2d(const int32 *in, int16 *out, uint32 pitch,
					   int blk_size) {
	int     x, y;
	int16 dc_coeff;

	dc_coeff = (*in + 0) >> 3;

	for (y = 0; y < blk_size; out += pitch, y++) {
		for (x = 0; x < blk_size; x++)
			out[x] = dc_coeff;
	}
}

/** butterfly operation for the inverse slant transform */
#define IVI_SLANT_BFLY(s1, s2, o1, o2, t)		\
	t  = (s1) - (s2);							\
	o1 = (s1) + (s2);							\
	o2 = (t);									\

/** This is a reflection a,b = 1/2, 5/4 for the inverse slant transform */
#define IVI_IREFLECT(s1, s2, o1, o2, t)			\
	t  = (((s1) + (s2)*2 + 2) >> 2) + (s1);		\
	o2 = (((s1)*2 - (s2) + 2) >> 2) - (s2);		\
	o1 = (t);									\

/** This is a reflection a,b = 1/2, 7/8 for the inverse slant transform */
#define IVI_SLANT_PART4(s1, s2, o1, o2, t)		\
	t  = (s2) + (((s1)*4  - (s2) + 4) >> 3);	\
	o2 = (s1) + ((-(s1) - (s2)*4 + 4) >> 3);	\
	o1 = (t);									\

/** inverse slant8 transform */
#define IVI_INV_SLANT8(s1, s4, s8, s5, s2, s6, s3, s7,					\
					   d1, d2, d3, d4, d5, d6, d7, d8,					\
					   t0, t1, t2, t3, t4, t5, t6, t7, t8) {			\
		IVI_SLANT_PART4(s4, s5, t4, t5, t0);							\
																		\
		IVI_SLANT_BFLY(s1, t5, t1, t5, t0); IVI_SLANT_BFLY(s2, s6, t2, t6, t0);	\
		IVI_SLANT_BFLY(s7, s3, t7, t3, t0); IVI_SLANT_BFLY(t4, s8, t4, t8, t0);	\
																		\
		IVI_SLANT_BFLY(t1, t2, t1, t2, t0); IVI_IREFLECT  (t4, t3, t4, t3, t0);	\
		IVI_SLANT_BFLY(t5, t6, t5, t6, t0); IVI_IREFLECT  (t8, t7, t8, t7, t0);	\
		IVI_SLANT_BFLY(t1, t4, t1, t4, t0); IVI_SLANT_BFLY(t2, t3, t2, t3, t0);	\
		IVI_SLANT_BFLY(t5, t8, t5, t8, t0); IVI_SLANT_BFLY(t6, t7, t6, t7, t0);	\
		d1 = COMPENSATE(t1);											\
		d2 = COMPENSATE(t2);											\
		d3 = COMPENSATE(t3);											\
		d4 = COMPENSATE(t4);											\
		d5 = COMPENSATE(t5);											\
		d6 = COMPENSATE(t6);											\
		d7 = COMPENSATE(t7);											\
		d8 = COMPENSATE(t8);}

/** inverse slant4 transform */
#define IVI_INV_SLANT4(s1, s4, s2, s3, d1, d2, d3, d4, t0, t1, t2, t3, t4) { \
				IVI_SLANT_BFLY(s1, s2, t1, t2, t0); IVI_IREFLECT  (s4, s3, t4, t3, t0);	\
				\
				IVI_SLANT_BFLY(t1, t4, t1, t4, t0); IVI_SLANT_BFLY(t2, t3, t2, t3, t0);	\
				d1 = COMPENSATE(t1);									\
				d2 = COMPENSATE(t2);									\
				d3 = COMPENSATE(t3);									\
				d4 = COMPENSATE(t4);}

void ff_ivi_inverse_slant_8x8(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags) {
	int     i;
	const int32 *src;
	int32 *dst;
	int     tmp[64];
	int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

#define COMPENSATE(x) (x)
	src = in;
	dst = tmp;
	for (i = 0; i < 8; i++) {
		if (flags[i]) {
			IVI_INV_SLANT8(src[0], src[8], src[16], src[24], src[32], src[40], src[48], src[56],
						   dst[0], dst[8], dst[16], dst[24], dst[32], dst[40], dst[48], dst[56],
						   t0, t1, t2, t3, t4, t5, t6, t7, t8);
		} else
			dst[0] = dst[8] = dst[16] = dst[24] = dst[32] = dst[40] = dst[48] = dst[56] = 0;

		src++;
		dst++;
	}
#undef COMPENSATE

#define COMPENSATE(x) (((x) + 1)>>1)
	src = tmp;
	for (i = 0; i < 8; i++) {
		if (!src[0] && !src[1] && !src[2] && !src[3] && !src[4] && !src[5] && !src[6] && !src[7]) {
			memset(out, 0, 8*sizeof(out[0]));
		} else {
			IVI_INV_SLANT8(src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7],
						   out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7],
						   t0, t1, t2, t3, t4, t5, t6, t7, t8);
		}
		src += 8;
		out += pitch;
	}
#undef COMPENSATE
}

void ff_ivi_inverse_slant_4x4(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags) {
	int     i;
	const int32 *src;
	int32 *dst;
	int     tmp[16];
	int     t0, t1, t2, t3, t4;

#define COMPENSATE(x) (x)
	src = in;
	dst = tmp;
	for (i = 0; i < 4; i++) {
		if (flags[i]) {
			IVI_INV_SLANT4(src[0], src[4], src[8], src[12],
						   dst[0], dst[4], dst[8], dst[12],
						   t0, t1, t2, t3, t4);
		} else
			dst[0] = dst[4] = dst[8] = dst[12] = 0;

		src++;
		dst++;
	}
#undef COMPENSATE

#define COMPENSATE(x) (((x) + 1)>>1)
	src = tmp;
	for (i = 0; i < 4; i++) {
		if (!src[0] && !src[1] && !src[2] && !src[3]) {
			out[0] = out[1] = out[2] = out[3] = 0;
		} else {
			IVI_INV_SLANT4(src[0], src[1], src[2], src[3],
						   out[0], out[1], out[2], out[3],
						   t0, t1, t2, t3, t4);
		}
		src += 4;
		out += pitch;
	}
#undef COMPENSATE
}

void ff_ivi_dc_slant_2d(const int32 *in, int16 *out, uint32 pitch, int blk_size) {
	int     x, y;
	int16 dc_coeff;

	dc_coeff = (*in + 1) >> 1;

	for (y = 0; y < blk_size; out += pitch, y++) {
		for (x = 0; x < blk_size; x++)
			out[x] = dc_coeff;
	}
}

void ff_ivi_row_slant8(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags) {
	int     i;
	int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

#define COMPENSATE(x) (((x) + 1)>>1)
	for (i = 0; i < 8; i++) {
		if (!in[0] && !in[1] && !in[2] && !in[3] && !in[4] && !in[5] && !in[6] && !in[7]) {
			memset(out, 0, 8*sizeof(out[0]));
		} else {
			IVI_INV_SLANT8( in[0],  in[1],  in[2],  in[3],  in[4],  in[5],  in[6],  in[7],
							out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7],
							t0, t1, t2, t3, t4, t5, t6, t7, t8);
		}
		in += 8;
		out += pitch;
	}
#undef COMPENSATE
}

void ff_ivi_dc_row_slant(const int32 *in, int16 *out, uint32 pitch, int blk_size) {
	int     x, y;
	int16 dc_coeff;

	dc_coeff = (*in + 1) >> 1;

	for (x = 0; x < blk_size; x++)
		out[x] = dc_coeff;

	out += pitch;

	for (y = 1; y < blk_size; out += pitch, y++) {
		for (x = 0; x < blk_size; x++)
			out[x] = 0;
	}
}

void ff_ivi_col_slant8(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags) {
	int     i, row2, row4, row8;
	int     t0, t1, t2, t3, t4, t5, t6, t7, t8;

	row2 = pitch << 1;
	row4 = pitch << 2;
	row8 = pitch << 3;

#define COMPENSATE(x) (((x) + 1)>>1)
	for (i = 0; i < 8; i++) {
		if (flags[i]) {
			IVI_INV_SLANT8(in[0], in[8], in[16], in[24], in[32], in[40], in[48], in[56],
						   out[0], out[pitch], out[row2], out[row2 + pitch], out[row4],
						   out[row4 + pitch],  out[row4 + row2], out[row8 - pitch],
						   t0, t1, t2, t3, t4, t5, t6, t7, t8);
		} else {
			out[0] = out[pitch] = out[row2] = out[row2 + pitch] = out[row4] =
				out[row4 + pitch] =  out[row4 + row2] = out[row8 - pitch] = 0;
		}

		in++;
		out++;
	}
#undef COMPENSATE
}

void ff_ivi_dc_col_slant(const int32 *in, int16 *out, uint32 pitch, int blk_size) {
	int     x, y;
	int16 dc_coeff;

	dc_coeff = (*in + 1) >> 1;

	for (y = 0; y < blk_size; out += pitch, y++) {
		out[0] = dc_coeff;
		for (x = 1; x < blk_size; x++)
			out[x] = 0;
	}
}

void ff_ivi_row_slant4(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags) {
	int     i;
	int     t0, t1, t2, t3, t4;

#define COMPENSATE(x) (((x) + 1)>>1)
	for (i = 0; i < 4; i++) {
		if (!in[0] && !in[1] && !in[2] && !in[3]) {
			memset(out, 0, 4*sizeof(out[0]));
		} else {
			IVI_INV_SLANT4( in[0],  in[1],  in[2],  in[3],
							out[0], out[1], out[2], out[3],
							t0, t1, t2, t3, t4);
		}
		in  += 4;
		out += pitch;
	}
#undef COMPENSATE
}

void ff_ivi_col_slant4(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags) {
	int     i, row2;
	int     t0, t1, t2, t3, t4;

	row2 = pitch << 1;

#define COMPENSATE(x) (((x) + 1)>>1)
	for (i = 0; i < 4; i++) {
		if (flags[i]) {
			IVI_INV_SLANT4(in[0], in[4], in[8], in[12],
						   out[0], out[pitch], out[row2], out[row2 + pitch],
						   t0, t1, t2, t3, t4);
		} else {
			out[0] = out[pitch] = out[row2] = out[row2 + pitch] = 0;
		}

		in++;
		out++;
	}
#undef COMPENSATE
}

void ff_ivi_put_pixels_8x8(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags) {
	int     x, y;

	for (y = 0; y < 8; out += pitch, in += 8, y++)
		for (x = 0; x < 8; x++)
			out[x] = in[x];
}

void ff_ivi_put_dc_pixel_8x8(const int32 *in, int16 *out, uint32 pitch, int blk_size) {
	int     y;

	out[0] = in[0];
	memset(out + 1, 0, 7*sizeof(out[0]));
	out += pitch;

	for (y = 1; y < 8; out += pitch, y++)
		memset(out, 0, 8*sizeof(out[0]));
}

#define IVI_MC_TEMPLATE(size, suffix, OP)								\
	void ff_ivi_mc_ ## size ##x## size ## suffix (int16 *buf, const int16 *ref_buf, \
												  uint32 pitch, int mc_type) \
	{																	\
		int     i, j;													\
		const int16 *wptr;												\
																		\
		switch (mc_type) {												\
		case 0: /* fullpel (no interpolation) */						\
			for (i = 0; i < size; i++, buf += pitch, ref_buf += pitch) { \
				for (j = 0; j < size; j++) {							\
					OP(buf[j], ref_buf[j]);								\
				}														\
			}															\
			break;														\
		case 1: /* horizontal halfpel interpolation */					\
			for (i = 0; i < size; i++, buf += pitch, ref_buf += pitch)	\
				for (j = 0; j < size; j++)								\
					OP(buf[j], (ref_buf[j] + ref_buf[j+1]) >> 1);		\
			break;														\
		case 2: /* vertical halfpel interpolation */					\
			wptr = ref_buf + pitch;										\
			for (i = 0; i < size; i++, buf += pitch, wptr += pitch, ref_buf += pitch) \
				for (j = 0; j < size; j++)								\
					OP(buf[j], (ref_buf[j] + wptr[j]) >> 1);			\
			break;														\
		case 3: /* vertical and horizontal halfpel interpolation */		\
			wptr = ref_buf + pitch;										\
			for (i = 0; i < size; i++, buf += pitch, wptr += pitch, ref_buf += pitch) \
				for (j = 0; j < size; j++)								\
					OP(buf[j], (ref_buf[j] + ref_buf[j+1] + wptr[j] + wptr[j+1]) >> 2); \
			break;														\
		}																\
	}																	\

#define OP_PUT(a, b)  (a) = (b)
#define OP_ADD(a, b)  (a) += (b)

IVI_MC_TEMPLATE(8, _no_delta, OP_PUT)
IVI_MC_TEMPLATE(8, _delta,    OP_ADD)
IVI_MC_TEMPLATE(4, _no_delta, OP_PUT)
IVI_MC_TEMPLATE(4, _delta,    OP_ADD)

} // End of namespace Image

