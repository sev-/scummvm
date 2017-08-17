#include <cstdlib>
#include <iostream>

using namespace std;

typedef unsigned char byte;

int main(int argc, char *argv[]) {

    FILE *font = fopen("Q:\\OldGames\\Kult\\Res\\CARPC.BIN", "rb");
    
    for (int ch = 0; ch < 64; ch++) {
        printf("%03d - %c\n", ch, ch + 32);
        for (int i = 0; i < 6; i++) {
            byte b;
            fread(&b, 1, 1, font);
            for (int bit = 7; bit >= 0; bit--) {
                printf((b & (1 << bit)) ? "1" : "0");
            }
            printf("\n");
        }
    }
    
    fclose(font);

    system("PAUSE");
    return EXIT_SUCCESS;
}
