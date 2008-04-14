#ifndef _UNPACK_DAMIEN_H_
#define _UNPACK_DAMIEN_H_

#include "common/util.h"
#include "common/endian.h"

int PAK_deflate(uint8 *srcBuffer, uint8 *dstBuffer, uint32 compressedSize, uint32 uncompressedSize);
int PAK_explode(uint8 *srcBuffer, uint8 *dstBuffer, uint32 compressedSize, uint32 uncompressedSize, uint16 flags);

#endif
