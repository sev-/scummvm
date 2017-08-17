#include <cstdlib>
#include <iostream>

using namespace std;

typedef unsigned char byte;
typedef unsigned short uint16;

/* EGA Color Table */
/*
byte egaColorTable[256 * 3] = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0xaa,
    0x00, 0xaa, 0x00,
    0x00, 0xaa, 0xaa,
    0xaa, 0x00, 0x00,
    0xaa, 0x00, 0xaa,
    0xaa, 0x55, 0x00,
    0xaa, 0xaa, 0xaa,
    0x55, 0x55, 0x55,
    0x55, 0x55, 0xff,
    0x55, 0xff, 0x55,
    0x55, 0xff, 0xff,
    0xff, 0x55, 0x55,
    0xff, 0x55, 0xff,
    0xff, 0xff, 0x55,
    0xff, 0xff, 0xff,
};
*/

byte egaColorTable[256 * 3] = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0xaa,
    0x00, 0xaa, 0x00,
    0x00, 0xaa, 0xaa,
    0xaa, 0x00, 0x00,
    0xaa, 0x00, 0xaa,
    0xaa, 0x55, 0x00,
    0xaa, 0xaa, 0xaa,
    0x00, 0x00, 0x00,
    0x55, 0x55, 0xff,
    0x55, 0xff, 0x55,
    0x55, 0xff, 0xff,
    0xff, 0x55, 0x55,
    0xff, 0x55, 0xff,
    0xff, 0xff, 0x55,
    0xff, 0xff, 0xff,
};

void writeTga(const char *filename, byte *pixels, byte *palette, int width, int height) {
    byte identsize = 0;
    byte colourmaptype = 1;
    byte imagetype = 1;
    short colourmapstart = 0;
    short colourmaplength = 256;
    byte colourmapbits = 24;
    short xstart = 0;
    short ystart = 0;
    byte bits = 8;
    byte descriptor = 0x20;
    FILE *tga = fopen(filename, "wb");
    fwrite(&identsize, 1, 1, tga);
    fwrite(&colourmaptype, 1, 1, tga);
    fwrite(&imagetype, 1, 1, tga);
    fwrite(&colourmapstart, 2, 1, tga);
    fwrite(&colourmaplength, 2, 1, tga);
    fwrite(&colourmapbits, 1, 1, tga);
    fwrite(&xstart, 2, 1, tga);
    fwrite(&ystart, 2, 1, tga);
    fwrite(&width, 2, 1, tga);
    fwrite(&height, 2, 1, tga);
    fwrite(&bits, 1, 1, tga);
    fwrite(&descriptor, 1, 1, tga);
    fwrite(palette, 768, 1, tga);
    fwrite(pixels, width * height, 1, tga);
    fclose(tga);
}

int main(int argc, char *argv[]) {

    for (int i = 0; i < 16; i++) {
        byte t = egaColorTable[i * 3 + 0];
        egaColorTable[i * 3 + 0] = egaColorTable[i * 3 + 2];
        egaColorTable[i * 3 + 2] = t;
    }

    FILE *ega = fopen("Q:\\OldGames\\Kult\\chamber\\PERSO.EGA", "rb");
    int sprIndex = 0;
    
    fseek(ega, 4, SEEK_SET);

    while (1) {
        char fn[256];
        byte *buffer, *outBuffer, *outBufferPos;
        uint16 size;
        byte width, height;
        fread(&size, 2, 1, ega);
        if (feof(ega))
            break;
        fread(&width, 1, 1, ega);
        fread(&height, 1, 1, ega);
        printf("width = %d; height = %d\n", width, height);
        size -= 4; // sub bytes for size and w/h
        sprintf(fn, "Q:\\OldGames\\Kult\\Tools\\PERSO\\%04d.tga", sprIndex++);
        buffer = new byte[size];
        outBufferPos = outBuffer = new byte[size * 2];
        fread(buffer, size, 1, ega);
        for (int i = 0; i < size; i++) {
            *outBufferPos++ = (buffer[i] >> 4) & 0x0F;
            *outBufferPos++ = buffer[i] & 0x0F;
        }
        writeTga(fn, outBuffer, egaColorTable, width * 4, height);
        delete[] outBuffer;
        delete[] buffer;
    }
    
    fclose(ega);

    system("PAUSE");
    return EXIT_SUCCESS;
}
