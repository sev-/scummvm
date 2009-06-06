/*
    TODO:
        - clip lines when drawing polygons
        - clip filled polygons?
        
        - move methods from CometEngine into respective classes
          where suitable
        
*/

#include <cstdlib>
#include <iostream>

#include "common/util.h"
#include "common/endian.h"
#include "common/rect.h"

#undef main

#include "graphics/primitives.h"

#include "comet/comet.h"

#include "comet/font.h"

#include "comet/pak.h"

using namespace std;

/*
void *g_engine = NULL;
void *g_system = NULL;

SDL_Surface *sdl_screen;
*/

namespace Comet {

Anim *a;


int random(int maxValue) {
    if (maxValue < 2)
        return 0;
    else
        return rand() % maxValue;
}

/*
TODO
void drawPixels(int x1, int y1, int x2, int y2) {

    if (x2 < x1 || y2 < y1)
        return;
        
    int width = x2 - x1;
    int height = y2 - y1;
    int counterMax = width / 2 + height;
    
    for (int i = 0; i < counterMax; i++) {

        int pixelX = x1 + random(width);
        int pixelY = y1 + random(height);
        
        plotProc(pixelX, pixelY, 1);
        
        if (random(2) != 0) {
            plotProc(pixelX, pixelY - 1, 1);
            plotProc(pixelX, pixelY + 1, 1);
            plotProc(pixelX - 1, pixelY, 1);
            plotProc(pixelX + 1, pixelY, 1);
        }

    }

}
*/

/* SceneObjects */



/* Menus */

Anim *_icone;

void drawGameMenu(int selectedItem) {
    const int x = 137;
    const int y = 65;
    const int itemHeight = 23;
	// FIXME: disabled to fix compilation
    /* draw menu */
    //_icone->runSeq1(10, 0, 0);
    /* draw selection rectangle */
    //_icone->runSeq1(11, x, y + itemHeight * selectedItem);
}


} // namespace Comet








int main(int argc, char *argv[])
{

	// FIXME
    //Comet::CometEngine *_vm = new Comet::CometEngine();

#if 0
    SDL_Init(SDL_INIT_VIDEO);
    sdl_screen = SDL_SetVideoMode(320, 200, 8, SDL_HWPALETTE);
    
    memset(_vm->getScreen(), 0, 64000);

    /* palette */
    byte palette[768];
    //FILE *palF = fopen("CTU.PAL", "rb");
    FILE *palF = fopen("Q:\\OldGames\\SotC\\RES\\CDINTRO.PAL.07", "rb");
    //FILE *palF = fopen("Q:\\OldGames\\SotC\\RES\\PALI0.PAL.08", "rb");
    fread(palette, 768, 1, palF);
    fclose(palF);
    SDL_Color colors[256];
    for (int i = 0; i < 256; i++) {
        colors[i].r = palette[i * 3 + 0];
        colors[i].g = palette[i * 3 + 1];
        colors[i].b = palette[i * 3 + 2];
    }
    SDL_SetColors(sdl_screen, colors, 0, 256);
#endif

#if 0

    freopen("debug.log", "wb", stdout);

    byte *bg = new byte[64000];
    memset(bg, 0, 64000);
    
    /*
    {
        FILE *i = fopen("Q:\\OldGames\\SotC\\D01\\I39.RAW.26", "rb");
        fread(bg, 64000, 1, i);
    }
    */

    Comet::Anim *a = new Comet::Anim(_vm);

    byte *buf;
    int size;
    
    buf = loadFromPak("Q:\\Games\\Shadow\\SHADOW\\A09", 0);
    
    /*
    //FILE *i = fopen("CRISE.VA2", "rb");
    //FILE *i = fopen("AMERFLAG.VA2", "rb");
    //FILE *i = fopen("ICONE.VA2", "rb");
    //FILE *i = fopen("MONEY00.VA2", "rb");
    //FILE *i = fopen("Q:\\OldGames\\SotC\\D01\\I39.VA2.27", "rb");
    FILE *i = fopen("Q:\\OldGames\\SotC\\RES\\MARCHE0.VA2.02", "rb");
    fseek(i, 0, SEEK_END);
    size = ftell(i);
    fseek(i, 0, SEEK_SET);
    buf = new byte[size];
    fread(buf, size, 1, i);
    fclose(i);
    */

    a->load(buf);

    byte *scr = _vm->getScreen();

    int seqCount = READ_LE_UINT32(a->getSection(0)) / 4;

    int seqIndex = 0, xp = 0, yp = 0;
    
    memcpy(_vm->getScreen(), bg, 64000);

    a->runSeq1(seqIndex, xp, yp);

    bool done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_MOUSEMOTION:
                    xp = e.motion.x;
                    yp = e.motion.y;
                    memcpy(_vm->getScreen(), bg, 64000);
                    a->runSeq1(seqIndex, xp, yp);
                    break;

                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                        case SDLK_LEFT:
                            if (seqIndex > 0) seqIndex--;
                            memcpy(_vm->getScreen(), bg, 64000);
                            a->runSeq1(seqIndex, xp, yp);
                            break;
                        case SDLK_RIGHT:
                            if (seqIndex + 1 < seqCount) seqIndex++;
                            memcpy(_vm->getScreen(), bg, 64000);
                            a->runSeq1(seqIndex, xp, yp);
                            break;
                    }
            }
        }
        
        char str[512];
        sprintf(str, "seq = %d", seqIndex); fflush(stdout);
        SDL_WM_SetCaption(str, NULL);
        
        memcpy(sdl_screen->pixels, _vm->getScreen(), 64000);
        
        SDL_Flip(sdl_screen);
        SDL_Delay(2);
        
    }

    /*
    FILE *o = fopen("D:\\Dev2\\SotC\\test.0", "wb");
    fwrite(screen->pixels, 64000, 1, o);
    fclose(o);
    */

    delete[] buf;

#endif

#if 0
    drawPixels(10, 10, 80, 35);

    bool done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
            }
        }

        //drawPixels(10, 10, 80, 35);

        SDL_Flip(screen);
        SDL_Delay(2);

    }
#endif

#if 0

    Comet::Font font;

    byte *buf;
    int size;
    FILE *i = fopen("font.pfn", "rb");

    fseek(i, 0, SEEK_END);
    size = ftell(i);
    fseek(i, 0, SEEK_SET);
    buf = new byte[size];
    fread(buf, size, 1, i);
    fclose(i);

    font.load(buf);
    font.setColor(220);
    //font.drawText(0, 0, (byte*)(screen->pixels), "Hallo Leute.");
    font.drawTextOutlined(0, 0, (byte*)(screen->pixels), "Hallo Leute.", 200, 220);

    bool done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
            }
        }

        SDL_Flip(screen);
        SDL_Delay(2);

    }
#endif

#if 0

    _icone = new Comet::Anim();

    animLoad("ICONE.VA2", _icone);
    
    a = _icone;
    
    drawGameMenu(3);

    bool done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
            }
        }

        SDL_Flip(screen);
        SDL_Delay(2);

    }
#endif

#if 0
    Comet::CometEngine engine;
    int dir = 0;

    byte *bg = new byte[64000];
    memset(bg, 0, 64000);

    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 320; x++) {
            bg[x + y * 320] = engine.calcDirection(160, 100, x, y);
        }

    memcpy(screen->pixels, bg, 64000);
    SDL_Flip(screen);

    bool done = false;
    while (!done) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_MOUSEMOTION:
                    dir = engine.calcDirection(160, 100, e.motion.x, e.motion.y);
                    break;

                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
            }
        }

        char str[512];
        sprintf(str, "dir = %d", dir); fflush(stdout);
        SDL_WM_SetCaption(str, NULL);

        SDL_Delay(2);

    }

    SDL_Quit();
#endif

//#if 0
    int scriptIndex = 2, ofs = 0;
    
    byte script[3000];
    FILE *scr = fopen("Q:\\Games\\Shadow\\SHADOW\\R09.CC4", "rb");
    fseek(scr, scriptIndex * 4, SEEK_SET);
    fread(&ofs, 4, 1, scr);
    fseek(scr, ofs, SEEK_SET);
    fread(script, 3000, 1, scr);
    fclose(scr);

	// FIXME
	warning("Script interpreter");
	/*
    Comet::ScriptInterpreter *_inter = new Comet::ScriptInterpreter(_vm);

    _inter->initialize(script);
    _inter->runScript(0);

    delete _inter;
	*/
    system("PAUSE");
//#endif
    
    return EXIT_SUCCESS;
}
