#ifndef _PAK_
#define _PAK_

#include "common/scummsys.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/file.h"
#include "comet/pak.h"
#include "comet/unpack.h"

struct pakInfoStruct // warning: allignement unsafe
{
  uint32 discSize;
  uint32 uncompressedSize;
  uint8 compressionFlag;
  uint8 info5;
  uint16 offset;
};

uint8 *loadFromPak(const char *name, int index);
int loadPakToPtr(const char *name, int index, uint8 *ptr);
int getPakSize(const char *name, int index);
unsigned int PAK_getNumFiles(const char *name);

#endif
