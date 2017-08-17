#include <cstdlib>
#include <iostream>

using namespace std;

typedef unsigned char byte;
typedef unsigned short uint16;

byte readByte(FILE *fd) {
    byte result;
    fread(&result, 1, 1, fd);
    return result;
}

uint16 readUint16(FILE *fd) {
    uint16 result;
    fread(&result, 2, 1, fd);
    return result;
}

inline uint16 SWAP_BYTES_16(const uint16 a) {
	return (a >> 8) | (a << 8);
}

int main(int argc, char *argv[]) {

    freopen("zones.log", "wt", stdout);

    FILE *zones = fopen("Q:\\OldGames\\Kult\\Res\\ZONES.BIN", "rb");

    while (1) {
        int zoneSize = readByte(zones);
        
        if (feof(zones))
            break;
        
        byte zone_byte_19993 = readByte(zones);
        byte ARPLA_index = readByte(zones);
        byte MOTSE_zoneDefaultIndex = readByte(zones);
        byte zone_byte_199B5 = readByte(zones);
        byte zzoneRectsCount = readByte(zones);

        printf("zoneSize = %d\nzone_byte_19993 = %d\nARPLA_index = %d\nMOTSE_zoneDefaultIndex = %d\nzone_byte_199B5 = %d\nzzoneRectsCount = %d\n",
            zoneSize, zone_byte_19993, ARPLA_index, MOTSE_zoneDefaultIndex, zone_byte_199B5, zzoneRectsCount);

        zoneSize -= 6;

        for (int i = 0; i < zzoneRectsCount; i++) {
            uint16 flags = SWAP_BYTES_16(readUint16(zones));
            uint16 mask = 0x10;
            printf("zz %d:\n", i);
            while (mask > 0) {
                if (flags & mask) {
                    printf("- zz(0x%02X): %04X\n", mask, SWAP_BYTES_16(readUint16(zones)));
                    zoneSize -= 2;
                }
                mask >>= 1;
            }
            zoneSize -= 2;
        }

        while (zoneSize > 0) {
            byte rectX1 = readByte(zones);
            byte rectX2 = readByte(zones);
            byte rectY1 = readByte(zones);
            byte rectY2 = readByte(zones);
            byte flags = readByte(zones);
            byte MOTSE_index = readByte(zones);
            uint16 animCommand = SWAP_BYTES_16(readUint16(zones));
            printf("rectX1 = %3d, rectX2 = %3d, rectY1 = %3d, rectY2 = %3d\n - flags = %02X, MOTSE_index = %3d, animCommand = %04X\n",
                rectX1, rectX2, rectY1, rectY2, flags, MOTSE_index, animCommand);
            zoneSize -= 8;
        }

        printf("==========================================\n");

    }

    fclose(zones);

    return EXIT_SUCCESS;
}
