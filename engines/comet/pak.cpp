
#include "comet/pak.h"

void readPakInfo(pakInfoStruct* pPakInfo, Common::File &file) {
	memset(pPakInfo, 0, sizeof(pakInfoStruct));
    file.read(&pPakInfo->discSize, 4);
    file.read(&pPakInfo->uncompressedSize, 4);
    file.read(&pPakInfo->compressionFlag, 1);
    file.read(&pPakInfo->info5, 1);
    file.read(&pPakInfo->offset, 2);
    pPakInfo->discSize = READ_LE_UINT32(&pPakInfo->discSize);
    pPakInfo->uncompressedSize = READ_LE_UINT32(&pPakInfo->uncompressedSize);
    pPakInfo->offset = READ_LE_UINT16(&pPakInfo->offset);
}

unsigned int PAK_getNumFiles(const char *name) {

    Common::File file;
    uint32 fileOffset;

    file.open(name);
    if (!file.isOpen())
        return 0;

    file.seek(4);
    file.read(&fileOffset, 4);
    file.close();
    
    return fileOffset / 4 - 2;
}

int loadPakToPtr(const char *name, int index, uint8 *ptr) {
    uint8 *tempData;
    tempData = loadFromPak(name, index);
    memcpy(ptr, tempData, getPakSize(name, index));
    free(tempData);
    return 1;
}

int getPakSize(const char *name, int index) {

    Common::File file;
    uint32 fileOffset;
    uint32 additionalDescriptorSize;
    pakInfoStruct pakInfo;
    uint32 size = 0;

    file.open(name);
  
    if (!file.isOpen())
        return 0;

    file.seek((index + 1) * 4);

    file.read(&fileOffset, 4);
    file.seek(fileOffset);

    file.read(&additionalDescriptorSize, 4);

    readPakInfo(&pakInfo, file);

    file.seek(pakInfo.offset, SEEK_CUR);

    if (pakInfo.compressionFlag == 0) {
        size = pakInfo.discSize;
    } else if (pakInfo.compressionFlag == 1 || pakInfo.compressionFlag == 4) {
        size = pakInfo.uncompressedSize;
    }

    file.close();

    return size;
}

uint8 *loadFromPak(const char *name, int index) {

    char nameBuffer[256];
    Common::File file;
    uint32 fileOffset;
    uint32 additionalDescriptorSize;
    pakInfoStruct pakInfo;
    uint8 *ptr = NULL;

	printf("name = %s; index = %d\n", name, index);

    file.open(name);
  
    if (!file.isOpen())
        return NULL;

	printf("PAK: index = %d\n", index);

    file.seek((index + 1) * 4);

    file.read(&fileOffset, 4);

	printf("PAK: offset = %08X\n", fileOffset);

	fflush(stdout);

    file.seek(fileOffset);

    file.read(&additionalDescriptorSize, 4);

    readPakInfo(&pakInfo, file);

    if (pakInfo.offset) {
        file.read(nameBuffer, pakInfo.offset);
        printf("name = %s\n", nameBuffer);
    } else {
        file.seek(pakInfo.offset, SEEK_CUR);
    }
    
	printf("pakInfo.compressionFlag = %d\n", pakInfo.compressionFlag);
	fflush(stdout);

    switch (pakInfo.compressionFlag) {
    case 0:
        {
            ptr = (uint8*)malloc(pakInfo.discSize);
            file.read(ptr, pakInfo.discSize);
            break;
        }
    case 1:
        {
			printf("pakInfo.uncompressedSize = %d; pakInfo.discSize = %d\n", pakInfo.uncompressedSize, pakInfo.discSize); fflush(stdout);

			printf("%08X\n", pakInfo.uncompressedSize); fflush(stdout);

            ptr = (uint8*)malloc(pakInfo.uncompressedSize);

            printf("ptr = %p\n", ptr); fflush(stdout);

            uint8 *compressedDataPtr = (uint8*)malloc(pakInfo.discSize);
            
            printf("compressedDataPtr = %p\n", compressedDataPtr); fflush(stdout);
            
            file.read(compressedDataPtr, pakInfo.discSize);
            memset(ptr, 0, pakInfo.uncompressedSize);
            PAK_explode(compressedDataPtr, ptr, pakInfo.discSize, pakInfo.uncompressedSize, pakInfo.info5);
            free(compressedDataPtr);
            break;
        }
/*
    case 4:
      {
        char * compressedDataPtr = (char *) malloc(pakInfo.discSize);
        fread(compressedDataPtr, pakInfo.discSize, 1, fileHandle);
        ptr = (char *) malloc(pakInfo.uncompressedSize);

        PAK_deflate(compressedDataPtr, ptr, pakInfo.discSize, pakInfo.uncompressedSize);

        free(compressedDataPtr);
        break;
      }
*/
    default:
        printf("PAK: Compression method %d not supported\n", pakInfo.compressionFlag);
        fflush(stdout);
    }
    
    /*
    FILE *o = fopen(nameBuffer, "wb");
    fwrite(ptr, pakInfo.uncompressedSize, 1, o);
    fclose(o);
    */
    
    file.close();

    return ptr;
}
