#include <cstdlib>
#include <iostream>

using namespace std;

typedef unsigned char byte;
typedef unsigned short uint16;
typedef unsigned long uint32;

int main(int argc, char *argv[]) {

    FILE *souri = fopen("Q:\\OldGames\\Kult\\Res\\SOURI.EGA", "rb");

    for (int i = 0; i < 9; i++) {

    for (int y = 0; y < 16; y++) {
        uint32 v;
        fread(&v, 4, 1, souri);
        //printf("v = %08X\n", v);
        for (int bit = 31; bit >= 0; bit--) {
            printf((v & (1 << bit)) ? "1" : "0");
        }
        printf("\n");
    }
        printf("\n");

    }

    fclose(souri);

    system("PAUSE");
    return EXIT_SUCCESS;
}
