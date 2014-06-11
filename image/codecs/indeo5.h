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

#ifndef IMAGE_CODECS_INDEO5_H
#define IMAGE_CODECS_INDEO5_H

#include "image/codecs/codec.h"

namespace Common {
	class BitStream;
}

namespace Image {

/**
 *  Common scan patterns (defined in ivi_common.c)
 */
extern const uint8 ff_ivi_vertical_scan_8x8[64];
extern const uint8 ff_ivi_horizontal_scan_8x8[64];
extern const uint8 ff_ivi_direct_scan_4x4[16];

/**
 *  Indeo 4 frame types.
 */
enum {
	IVI4_FRAMETYPE_INTRA       = 0,
	IVI4_FRAMETYPE_INTRA1      = 1,  ///< intra frame with slightly different bitstream coding
	IVI4_FRAMETYPE_INTER       = 2,  ///< non-droppable P-frame
	IVI4_FRAMETYPE_BIDIR       = 3,  ///< bidirectional frame
	IVI4_FRAMETYPE_INTER_NOREF = 4,  ///< droppable P-frame
	IVI4_FRAMETYPE_NULL_FIRST  = 5,  ///< empty frame with no data
	IVI4_FRAMETYPE_NULL_LAST   = 6   ///< empty frame with no data
};

#define VLC_TYPE int16

struct VLC {
	int bits;
	VLC_TYPE (*table)[2]; ///< code, bits
	int table_size, table_allocated;
	void * volatile init_state;
};

#define IVI_VLC_BITS 13 ///< max number of bits of the ivi's huffman codes
#define IVI4_STREAM_ANALYSER    0
#define IVI5_IS_PROTECTED       0x20

/**
 *  huffman codebook descriptor
 */
 struct IVIHuffDesc {
 	int32     num_rows;
 	uint8     xbits[16];
 };

/**
 *  macroblock/block huffman table descriptor
 */
 struct IVIHuffTab {
	int32     tab_sel;    /// index of one of the predefined tables
	/// or "7" for custom one
	VLC         *tab;       /// pointer to the table associated with tab_sel

	/// the following are used only when tab_sel == 7
	IVIHuffDesc cust_desc;  /// custom Huffman codebook descriptor
	VLC         cust_tab;   /// vlc table for custom codebook
};

enum {
	IVI_MB_HUFF   = 0,      /// Huffman table is used for coding macroblocks
	IVI_BLK_HUFF  = 1       /// Huffman table is used for coding blocks
};


/**
 *  Declare inverse transform function types
 */
 typedef void (InvTransformPtr)(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags);
 typedef void (DCTransformPtr) (const int32 *in, int16 *out, uint32 pitch, int blk_size);


/**
 *  run-value (RLE) table descriptor
 */
 struct RVMapDesc {
	uint8     eob_sym; ///< end of block symbol
	uint8     esc_sym; ///< escape symbol
	uint8     runtab[256];
	int8      valtab[256];
};

extern const RVMapDesc ff_ivi_rvmap_tabs[9];


/**
 *  information for Indeo macroblock (16x16, 8x8 or 4x4)
 */
 struct IVIMbInfo {
 	int16     xpos;
 	int16     ypos;
	uint32    buf_offs; ///< address in the output buffer for this mb
	uint8     type;     ///< macroblock type: 0 - INTRA, 1 - INTER
	uint8     cbp;      ///< coded block pattern
	int8      q_delta;  ///< quant delta
	int8      mv_x;     ///< motion vector (x component)
	int8      mv_y;     ///< motion vector (y component)
};


/**
 *  information for Indeo tile
 */
 struct IVITile {
 	int         xpos;
 	int         ypos;
 	int         width;
 	int         height;
 	int         mb_size;
	int         is_empty;  ///< = 1 if this tile doesn't contain any data
	int         data_size; ///< size of the data in bytes
	int         num_MBs;   ///< number of macroblocks in this tile
	IVIMbInfo   *mbs;      ///< array of macroblock descriptors
	IVIMbInfo   *ref_mbs;  ///< ptr to the macroblock descriptors of the reference tile
};


/**
 *  information for Indeo wavelet band
 */
 struct IVIBandDesc {
	int             plane;          ///< plane number this band belongs to
	int             band_num;       ///< band number
	int             width;
	int             height;
	int             aheight;        ///< aligned band height
	const uint8   *data_ptr;      ///< ptr to the first byte of the band data
	int             data_size;      ///< size of the band data
	int16         *buf;           ///< pointer to the output buffer for this band
	int16         *ref_buf;       ///< pointer to the reference frame buffer (for motion compensation)
	int16         *bufs[3];       ///< array of pointers to the band buffers
	int             pitch;          ///< pitch associated with the buffers above
	int             is_empty;       ///< = 1 if this band doesn't contain any data
	int             mb_size;        ///< macroblock size
	int             blk_size;       ///< block size
	int             is_halfpel;     ///< precision of the motion compensation: 0 - fullpel, 1 - halfpel
	int             inherit_mv;     ///< tells if motion vector is inherited from reference macroblock
	int             inherit_qdelta; ///< tells if quantiser delta is inherited from reference macroblock
	int             qdelta_present; ///< tells if Qdelta signal is present in the bitstream (Indeo5 only)
	int             quant_mat;      ///< dequant matrix index
	int             glob_quant;     ///< quant base for this band
	const uint8   *scan;          ///< ptr to the scan pattern
	int             scan_size;      ///< size of the scantable

	IVIHuffTab      blk_vlc;        ///< vlc table for decoding block data

	int             num_corr;       ///< number of correction entries
	uint8         corr[61*2];     ///< rvmap correction pairs
	int             rvmap_sel;      ///< rvmap table selector
	RVMapDesc       *rv_map;        ///< ptr to the RLE table for this band
	int             num_tiles;      ///< number of tiles in this band
	IVITile         *tiles;         ///< array of tile descriptors
	InvTransformPtr *inv_transform;
	int             transform_size;
	DCTransformPtr  *dc_transform;
	int             is_2d_trans;    ///< 1 indicates that the two-dimensional inverse transform is used
	int32         checksum;       ///< for debug purposes
	int             checksum_present;
	int             bufsize;        ///< band buffer size in bytes
	const uint16  *intra_base;    ///< quantization matrix for intra blocks
	const uint16  *inter_base;    ///< quantization matrix for inter blocks
	const uint8   *intra_scale;   ///< quantization coefficient for intra blocks
	const uint8   *inter_scale;   ///< quantization coefficient for inter blocks
};


/**
 *  color plane (luma or chroma) information
 */
 struct IVIPlaneDesc {
 	uint16    width;
 	uint16    height;
	uint8     num_bands;  ///< number of bands this plane subdivided into
	IVIBandDesc *bands;     ///< array of band descriptors
};

struct IVIPicConfig {
	uint16    pic_width;
	uint16    pic_height;
	uint16    chroma_width;
	uint16    chroma_height;
	uint16    tile_width;
	uint16    tile_height;
	uint8     luma_bands;
	uint8     chroma_bands;
};

struct IVI45DecContext {
	Common::BitStream   *gb;
	RVMapDesc       rvmap_tabs[9];   ///< local corrected copy of the static rvmap tables

	uint32        frame_num;
	int           frame_type;
	int           prev_frame_type; ///< frame type of the previous frame
	uint32        data_size;       ///< size of the frame data in bytes from picture header
	int           is_scalable;
	int           transp_status;   ///< transparency mode status: 1 - enabled
	int           inter_scal;      ///< signals a sequence of scalable inter frames
	uint32        pic_hdr_size;    ///< picture header size in bytes
	uint8         frame_flags;
	uint16        checksum;        ///< frame checksum

	IVIPicConfig    pic_conf;
	IVIPlaneDesc    planes[3];       ///< color planes

	int             buf_switch;      ///< used to switch between three buffers
	int             dst_buf;         ///< buffer index for the currently decoded frame
	int             ref_buf;         ///< inter frame reference buffer index
	int             ref2_buf;        ///< temporal storage for switching buffers

	IVIHuffTab      mb_vlc;          ///< current macroblock table descriptor
	IVIHuffTab      blk_vlc;         ///< current block table descriptor

	uint8         rvmap_sel;
	uint8         in_imf;
	uint8         in_q;            ///< flag for explicitly stored quantiser delta
	uint8         pic_glob_quant;
	uint8         unknown1;

	uint16        gop_hdr_size;
	uint8         gop_flags;
	uint32        lock_word;

	int gop_invalid;
	int buf_invalid[3];
};

/**
 * Intel Indeo 5 decoder.
 *
 * Used by AVI.
 *
 * Used in video:
 *  - AVIDecoder
 */
class Indeo5Decoder : public Codec {

 public:
	Indeo5Decoder(uint16 width, uint16 height);
	~Indeo5Decoder();

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);
	Graphics::PixelFormat getPixelFormat() const;
 private:
	Graphics::Surface *_surface;

	Graphics::PixelFormat _pixelFormat;

	IVI45DecContext *_ctx;

	int decode_pic_hdr();
	int decode_band_hdr(IVIBandDesc *band);
	int decode_mb_info(IVIBandDesc *band, IVITile *tile);
	void switch_buffers();
	int is_nonnull_frame();
	int decode_band(IVIBandDesc *band);
	int decode_gop_header();
};

/** compare some properties of two pictures */
static inline int ivi_pic_config_cmp(IVIPicConfig *str1, IVIPicConfig *str2) {
    return str1->pic_width    != str2->pic_width    || str1->pic_height    != str2->pic_height    ||
		str1->chroma_width != str2->chroma_width || str1->chroma_height != str2->chroma_height ||
		str1->tile_width   != str2->tile_width   || str1->tile_height   != str2->tile_height   ||
		str1->luma_bands   != str2->luma_bands   || str1->chroma_bands  != str2->chroma_bands;
}


/** calculate number of tiles in a stride */
#define IVI_NUM_TILES(stride, tile_size) (((stride) + (tile_size) - 1) / (tile_size))

/** calculate number of macroblocks in a tile */
#define IVI_MBs_PER_TILE(tile_width, tile_height, mb_size)				\
    ((((tile_width) + (mb_size) - 1) / (mb_size)) * (((tile_height) + (mb_size) - 1) / (mb_size)))

/** convert unsigned values into signed ones (the sign is in the LSB) */
#define IVI_TOSIGNED(val) (-(((val) >> 1) ^ -((val) & 1)))

/** scale motion vector */
static inline int ivi_scale_mv(int mv, int mv_scale) {
    return (mv + (mv > 0) + (mv_scale - 1)) >> mv_scale;
}

/**
 * Initialize static codes used for macroblock and block decoding.
 */
void ff_ivi_init_static_vlc(void);

/**
 *  Decode a huffman codebook descriptor from the bitstream
 *  and select specified huffman table.
 *
 *  @param[in,out]  gb          the GetBit context
 *  @param[in]      desc_coded  flag signalling if table descriptor was coded
 *  @param[in]      which_tab   codebook purpose (IVI_MB_HUFF or IVI_BLK_HUFF)
 *  @param[out]     huff_tab    pointer to the descriptor of the selected table
 *  @return             zero on success, negative value otherwise
 */
int  ff_ivi_dec_huff_desc(Common::BitStream *gb, int desc_coded, int which_tab,
                          IVIHuffTab *huff_tab);

/**
 *  Initialize planes (prepares descriptors, allocates buffers etc).
 *
 *  @param[in,out]  planes  pointer to the array of the plane descriptors
 *  @param[in]      cfg     pointer to the ivi_pic_config structure describing picture layout
 *  @return             result code: 0 - OK
 */
int  ff_ivi_init_planes(IVIPlaneDesc *planes, const IVIPicConfig *cfg);

/**
 *  Initialize tile and macroblock descriptors.
 *
 *  @param[in,out]  planes       pointer to the array of the plane descriptors
 *  @param[in]      tile_width   tile width
 *  @param[in]      tile_height  tile height
 *  @return             result code: 0 - OK
 */
int  ff_ivi_init_tiles(IVIPlaneDesc *planes, int tile_width, int tile_height);

/**
 * @file
 * DSP functions (inverse transforms, motion compensations, wavelet recompostion)
 * for Indeo Video Interactive codecs.
 */

/**
 *  5/3 wavelet recomposition filter for Indeo5
 *
 *  @param[in]   plane        pointer to the descriptor of the plane being processed
 *  @param[out]  dst          pointer to the destination buffer
 *  @param[in]   dst_pitch    pitch of the destination buffer
 */
void ff_ivi_recompose53(const IVIPlaneDesc *plane, uint8 *dst,
                        const int dst_pitch);

/**
 *  Haar wavelet recomposition filter for Indeo 4
 *
 *  @param[in]  plane        pointer to the descriptor of the plane being processed
 *  @param[out] dst          pointer to the destination buffer
 *  @param[in]  dst_pitch    pitch of the destination buffer
 */
void ff_ivi_recompose_haar(const IVIPlaneDesc *plane, uint8 *dst,
                           const int dst_pitch);

/**
 *  two-dimensional inverse Haar 8x8 transform for Indeo 4
 *
 *  @param[in]  in        pointer to the vector of transform coefficients
 *  @param[out] out       pointer to the output buffer (frame)
 *  @param[in]  pitch     pitch to move to the next y line
 *  @param[in]  flags     pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_inverse_haar_8x8(const int32 *in, int16 *out, uint32 pitch,
                             const uint8 *flags);
void ff_ivi_inverse_haar_8x1(const int32 *in, int16 *out, uint32 pitch,
                             const uint8 *flags);
void ff_ivi_inverse_haar_1x8(const int32 *in, int16 *out, uint32 pitch,
                             const uint8 *flags);

/**
 *  one-dimensional inverse 8-point Haar transform on rows for Indeo 4
 *
 *  @param[in]  in        pointer to the vector of transform coefficients
 *  @param[out] out       pointer to the output buffer (frame)
 *  @param[in]  pitch     pitch to move to the next y line
 *  @param[in]  flags     pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_row_haar8(const int32 *in, int16 *out, uint32 pitch,
                      const uint8 *flags);

/**
 *  one-dimensional inverse 8-point Haar transform on columns for Indeo 4
 *
 *  @param[in]  in        pointer to the vector of transform coefficients
 *  @param[out] out       pointer to the output buffer (frame)
 *  @param[in]  pitch     pitch to move to the next y line
 *  @param[in]  flags     pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_col_haar8(const int32 *in, int16 *out, uint32 pitch,
                      const uint8 *flags);

/**
 *  two-dimensional inverse Haar 4x4 transform for Indeo 4
 *
 *  @param[in]  in        pointer to the vector of transform coefficients
 *  @param[out] out       pointer to the output buffer (frame)
 *  @param[in]  pitch     pitch to move to the next y line
 *  @param[in]  flags     pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_inverse_haar_4x4(const int32 *in, int16 *out, uint32 pitch,
                             const uint8 *flags);

/**
 *  one-dimensional inverse 4-point Haar transform on rows for Indeo 4
 *
 *  @param[in]  in        pointer to the vector of transform coefficients
 *  @param[out] out       pointer to the output buffer (frame)
 *  @param[in]  pitch     pitch to move to the next y line
 *  @param[in]  flags     pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_row_haar4(const int32 *in, int16 *out, uint32 pitch,
                      const uint8 *flags);

/**
 *  one-dimensional inverse 4-point Haar transform on columns for Indeo 4
 *
 *  @param[in]  in        pointer to the vector of transform coefficients
 *  @param[out] out       pointer to the output buffer (frame)
 *  @param[in]  pitch     pitch to move to the next y line
 *  @param[in]  flags     pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_col_haar4(const int32 *in, int16 *out, uint32 pitch,
                      const uint8 *flags);

/**
 *  DC-only two-dimensional inverse Haar transform for Indeo 4.
 *  Performing the inverse transform in this case is equivalent to
 *  spreading DC_coeff >> 3 over the whole block.
 *
 *  @param[in]  in          pointer to the dc coefficient
 *  @param[out] out         pointer to the output buffer (frame)
 *  @param[in]  pitch       pitch to move to the next y line
 *  @param[in]  blk_size    transform block size
 */
void ff_ivi_dc_haar_2d(const int32 *in, int16 *out, uint32 pitch,
                       int blk_size);

/**
 *  two-dimensional inverse slant 8x8 transform
 *
 *  @param[in]    in      pointer to the vector of transform coefficients
 *  @param[out]   out     pointer to the output buffer (frame)
 *  @param[in]    pitch   pitch to move to the next y line
 *  @param[in]    flags   pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_inverse_slant_8x8(const int32 *in, int16 *out, uint32 pitch,
                              const uint8 *flags);

/**
 *  two-dimensional inverse slant 4x4 transform
 *
 *  @param[in]    in      pointer to the vector of transform coefficients
 *  @param[out]   out     pointer to the output buffer (frame)
 *  @param[in]    pitch   pitch to move to the next y line
 *  @param[in]    flags   pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_inverse_slant_4x4(const int32 *in, int16 *out, uint32 pitch,
                              const uint8 *flags);

/**
 *  DC-only two-dimensional inverse slant transform.
 *  Performing the inverse slant transform in this case is equivalent to
 *  spreading (DC_coeff + 1)/2 over the whole block.
 *  It works much faster than performing the slant transform on a vector of zeroes.
 *
 *  @param[in]    in          pointer to the dc coefficient
 *  @param[out]   out         pointer to the output buffer (frame)
 *  @param[in]    pitch       pitch to move to the next y line
 *  @param[in]    blk_size    transform block size
 */
void ff_ivi_dc_slant_2d(const int32 *in, int16 *out, uint32 pitch, int blk_size);

/**
 *  inverse 1D row slant transform
 *
 *  @param[in]    in      pointer to the vector of transform coefficients
 *  @param[out]   out     pointer to the output buffer (frame)
 *  @param[in]    pitch   pitch to move to the next y line
 *  @param[in]    flags   pointer to the array of column flags (unused here)
 */
void ff_ivi_row_slant8(const int32 *in, int16 *out, uint32 pitch,
                       const uint8 *flags);

/**
 *  inverse 1D column slant transform
 *
 *  @param[in]    in      pointer to the vector of transform coefficients
 *  @param[out]   out     pointer to the output buffer (frame)
 *  @param[in]    pitch   pitch to move to the next y line
 *  @param[in]    flags   pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_col_slant8(const int32 *in, int16 *out, uint32 pitch,
                       const uint8 *flags);

/**
 *  inverse 1D row slant transform
 *
 *  @param[in]    in      pointer to the vector of transform coefficients
 *  @param[out]   out     pointer to the output buffer (frame)
 *  @param[in]    pitch   pitch to move to the next y line
 *  @param[in]    flags   pointer to the array of column flags (unused here)
 */
void ff_ivi_row_slant4(const int32 *in, int16 *out, uint32 pitch,
                       const uint8 *flags);

/**
 *  inverse 1D column slant transform
 *
 *  @param[in]    in      pointer to the vector of transform coefficients
 *  @param[out]   out     pointer to the output buffer (frame)
 *  @param[in]    pitch   pitch to move to the next y line
 *  @param[in]    flags   pointer to the array of column flags:
 *                        != 0 - non_empty column, 0 - empty one
 *                        (this array must be filled by caller)
 */
void ff_ivi_col_slant4(const int32 *in, int16 *out, uint32 pitch,
                       const uint8 *flags);

/**
 *  DC-only inverse row slant transform
 */
void ff_ivi_dc_row_slant(const int32 *in, int16 *out, uint32 pitch, int blk_size);

/**
 *  DC-only inverse column slant transform
 */
void ff_ivi_dc_col_slant(const int32 *in, int16 *out, uint32 pitch, int blk_size);

/**
 *  Copy the pixels into the frame buffer.
 */
void ff_ivi_put_pixels_8x8(const int32 *in, int16 *out, uint32 pitch, const uint8 *flags);

/**
 *  Copy the DC coefficient into the first pixel of the block and
 *  zero all others.
 */
void ff_ivi_put_dc_pixel_8x8(const int32 *in, int16 *out, uint32 pitch, int blk_size);

/**
 *  8x8 block motion compensation with adding delta
 *
 *  @param[in,out]   buf      pointer to the block in the current frame buffer containing delta
 *  @param[in]       ref_buf  pointer to the corresponding block in the reference frame
 *  @param[in]       pitch    pitch for moving to the next y line
 *  @param[in]       mc_type  interpolation type
 */
void ff_ivi_mc_8x8_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

/**
 *  4x4 block motion compensation with adding delta
 *
 *  @param[in,out]   buf      pointer to the block in the current frame buffer containing delta
 *  @param[in]       ref_buf  pointer to the corresponding block in the reference frame
 *  @param[in]       pitch    pitch for moving to the next y line
 *  @param[in]       mc_type  interpolation type
 */
void ff_ivi_mc_4x4_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

/**
 *  motion compensation without adding delta
 *
 *  @param[in,out]  buf      pointer to the block in the current frame receiving the result
 *  @param[in]      ref_buf  pointer to the corresponding block in the reference frame
 *  @param[in]      pitch    pitch for moving to the next y line
 *  @param[in]      mc_type  interpolation type
 */
void ff_ivi_mc_8x8_no_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

/**
 *  4x4 block motion compensation without adding delta
 *
 *  @param[in,out]  buf      pointer to the block in the current frame receiving the result
 *  @param[in]      ref_buf  pointer to the corresponding block in the reference frame
 *  @param[in]      pitch    pitch for moving to the next y line
 *  @param[in]      mc_type  interpolation type
 */
void ff_ivi_mc_4x4_no_delta(int16 *buf, const int16 *ref_buf, uint32 pitch, int mc_type);

} // End of namespace Image

#endif

