#include <cstdlib>
#include <iostream>
#include <SDL/sdl.h>
#undef main

using namespace std;

typedef unsigned char byte;
typedef unsigned short uint16;

/* EGA Color Table */
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

SDL_Surface *screen;

void drawFlipped(byte *source, byte *dest, int width, int height, int destPitch) {
    width *= 4;
    for (int yc = 0; yc < height; yc++) {
        byte *dstp = dest + width - 1;
        for (int xc = 0; xc < width; xc++) {
            if (*source != 0) {
                *dstp = *source;
            }
            source++;
            dstp--;
        }
        dest += destPitch;
    }
}

void drawNormal(byte *source, byte *dest, int width, int height, int destPitch) {
    for (int yc = 0; yc < height; yc++) {
        byte *dstp = dest;
        for (int xc = 0; xc < width; xc++) {
            for (int p = 0; p < 4; p++)
                if (source[p] != 0)
                    dstp[p] = source[p];
            source += 4;
            dstp += 4;
        }
        dest += destPitch;
    }
}

void unpackSprite(const char *filename, int index, byte *&outBuffer, int &outWidth, int &outHeight) {
    FILE *ega = fopen(filename, "rb");
    fseek(ega, 4, SEEK_SET);
    while (1) {
        uint16 size;
        fread(&size, 2, 1, ega);
        if (index == 0) {
            byte *buffer, *outBufferPos;
            byte width, height;
            fread(&width, 1, 1, ega);
            fread(&height, 1, 1, ega);
            size -= 4; // sub bytes for size and w/h
            buffer = new byte[size];
            outBufferPos = outBuffer = new byte[size * 2];
            memset(outBuffer, 0, size * 2);
            fread(buffer, size, 1, ega);
            for (int i = 0; i < size; i++) {
                *outBufferPos++ = (buffer[i] >> 4) & 0x0F;
                *outBufferPos++ = buffer[i] & 0x0F;
            }
            outWidth = width;
            outHeight = height;
            delete[] buffer;
            break;
        }
        fseek(ega, size - 2, SEEK_CUR);
        index--;
    }
    fclose(ega);
}

bool drawLutin(int index, byte *&outBuffer, int &outWidth, int &outHeight) {

    FILE *lutin = fopen("Q:\\OldGames\\Kult\\Res\\LUTIN.BIN", "rb");
    bool result = false;
    
    outBuffer = NULL;

    while (1) {
        byte size, w, h;
        byte buffer[256];
        fread(&size, 1, 1, lutin);
        if (feof(lutin) || index < 0 || size == 1)
            break;
        if (index == 0) {
            fread(&w, 1, 1, lutin);
            fread(&h, 1, 1, lutin);
            size -= 3;
            fread(buffer, size, 1, lutin);

            outBuffer = new byte[w * h * 4 + 100000];
            memset(outBuffer, 0, w * h * 4);

            for (int i = 0; i < size; i += 3) {
                int spriteIndex = buffer[i + 0];
                uint16 ofs = buffer[i + 1] | (buffer[i + 2] << 8);
                byte *spriteBuffer;
                int spriteWidth, spriteHeight;
                unpackSprite("Q:\\OldGames\\Kult\\chamber\\SPRIT.EGA",
                    spriteIndex, spriteBuffer, spriteWidth, spriteHeight);
                drawNormal(spriteBuffer, outBuffer + (ofs & 0x7FFF) * 4, spriteWidth, spriteHeight, w * 4);
                delete[] spriteBuffer;
            }
            
            outWidth = w;
            outHeight = h;
            result = true;

            break;
        }
        index--;
        fseek(lutin, size - 1, SEEK_CUR);
    }

    fclose(lutin);

    return result;
}

void doLutin() {
    int lutinIndex = 0;

    bool done = false;
    while (!done) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                done = true;
                break;
            }
        }

        printf("lutinIndex = %d\n", lutinIndex);

        SDL_FillRect(screen, 0, 0);
        byte *outBuffer;
        int outWidth, outHeight;
        if (drawLutin(lutinIndex, outBuffer, outWidth, outHeight)) {
            //printf("outWidth = %d; outHeight = %d\n", outWidth, outHeight);
            drawNormal(outBuffer, (byte*)screen->pixels, outWidth, outHeight, 320);
            lutinIndex++;
        } else {
            lutinIndex = 0;
            exit(1);
        }
        delete[] outBuffer;

        SDL_Flip(screen);
        SDL_Delay(1000);

        /*
        char fn[256];
        sprintf(fn, "lutin%03d.bmp", lutinIndex - 1);
        SDL_SaveBMP(screen, fn);
        */

    }

}

bool drawArpla(int index) {

    FILE *arpla = fopen("Q:\\OldGames\\Kult\\Res\\ARPLA.BIN", "rb");
    bool result = false;

    while (1) {
        byte size;
        byte buffer[256];
        fread(&size, 1, 1, arpla);
        if (feof(arpla) || index < 0 || size == 1)
            break;
        if (index == 0) {
            size--;
            fread(buffer, size, 1, arpla);
            for (int i = 0; i < size; i += 3) {
                int spriteIndex = buffer[i + 0];
                int x = buffer[i + 1], y = buffer[i + 2];
                bool flipX, flipY;
                printf("spriteIndex = %d; x = %d; y = %d\n", spriteIndex, x, y);

                byte *spriteBuffer;
                int spriteWidth, spriteHeight;
                
                if (spriteIndex == 83) {
                    printf("!!!!!!!!!!!!!!!!\n");
                }
                
                if (spriteIndex < 83)
                    unpackSprite("Q:\\OldGames\\Kult\\chamber\\PUZZL.EGA", spriteIndex, spriteBuffer, spriteWidth, spriteHeight);
                else
                    unpackSprite("Q:\\OldGames\\Kult\\chamber\\PUZZ1.EGA", spriteIndex - 83, spriteBuffer, spriteWidth, spriteHeight);

                flipX = x & 0x80;
                flipY = y & 0x80;

                //if (flipX) printf("- flip x\n");
                //if (flipY) printf("- flip y !!!!!!\n");

                x = (x & 0x7F) * 4;
                y = (y & 0x7F) * 2;

                //printf("y + spriteHeight = %d\n", y + spriteHeight);

                if (y + spriteHeight >= 200)
                    spriteHeight = 199 - y;
                    
                //printf("y + spriteHeight = %d\n", y + spriteHeight);
                byte *dest = (byte*)screen->pixels + x + y * 320;
                int destPitch = 320;
                
                if (flipY) {
                    dest += (spriteHeight - 1) * 320;
                    destPitch = -destPitch;
                }

                if (flipX)
                    drawFlipped(spriteBuffer, dest, spriteWidth, spriteHeight, destPitch);
                else
                    drawNormal(spriteBuffer, dest, spriteWidth, spriteHeight, destPitch);

                delete[] spriteBuffer;

            }

            result = true;
            break;
        }
        index--;
        fseek(arpla, size - 1, SEEK_CUR);
    }

    fclose(arpla);

    return result;
}

void doArpla() {

    int arplaIndex = 0;

    bool done = false;
    while (!done) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                done = true;
                break;
            }
        }

        SDL_FillRect(screen, 0, 0);
        if (!drawArpla(arplaIndex))
            break;

        arplaIndex++;

        SDL_Flip(screen);
        SDL_Delay(1000);

        /*
        char fn[256];
        sprintf(fn, "arpla%03d.bmp", arplaIndex - 1);
        SDL_SaveBMP(screen, fn);
        */

    }

}

int main(int argc, char *argv[]) {

    for (int i = 0; i < 16; i++) {
        byte t = egaColorTable[i * 3 + 0];
        egaColorTable[i * 3 + 0] = egaColorTable[i * 3 + 2];
        egaColorTable[i * 3 + 2] = t;
    }

    screen = SDL_SetVideoMode(320, 200, 8, SDL_HWPALETTE);
    
    SDL_Color colors[256];
    memset(colors, 0, sizeof(colors));
    for (int i = 0; i < 16; i++) {
        colors[i].b = egaColorTable[i * 3 + 0];
        colors[i].g = egaColorTable[i * 3 + 1];
        colors[i].r = egaColorTable[i * 3 + 2];
    }
    
    SDL_SetColors(screen, colors, 0, 256);

    //unpackSprite(10, outBuffer, outWidth, outHeight);
    //drawNormal(outBuffer, (byte*)screen->pixels, outWidth, outHeight, 320);

    //doLutin();
    doArpla();

    return EXIT_SUCCESS;
}
