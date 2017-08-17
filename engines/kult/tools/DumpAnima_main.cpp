#include <cstdlib>
#include <iostream>

using namespace std;

typedef unsigned char byte;

int main(int argc, char *argv[]) {

    FILE *anima = fopen("Q:\\OldGames\\Kult\\Res\\ANIMA.BIN", "rb");
    int count = 0;

    freopen("anima.log", "wt", stdout);
    
    while (1) {
        byte size;
        byte buffer[256];
        fread(&size, 1, 1, anima);
        if (feof(anima))
            break;

        printf("%d:\n", count);
        printf("size = %d\n", size);
        
        size--;
        fread(buffer, size, 1, anima);
        
        for (int i = 0; i < size; i++) {
            printf("%02X ", buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
        
        for (int i = 0; i < size; i++) {
            byte cmd = buffer[i];
            switch (cmd) {
            case 0xFE:
                printf("setPos(%d, %d)\n", buffer[i + 1], buffer[i + 2]);
                i += 2;
                break;
            case 0xFD:
                printf("playSound(%d)\n", buffer[i + 1]);
                i += 2;
                break;
            case 0xFC:
                printf("unused()\n");
                break;
            default:
                cmd = cmd & 0x07;
                switch (cmd) {
                case 0:
                    printf("drawLutin(%02X, %d)\n", buffer[i + 0] & 0xF8, buffer[i + 1]);
                    i += 1;
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    printf("cmd%02X(%02X, %02X)\n", cmd, buffer[i + 0], buffer[i + 1]);
                    i += 1 + buffer[i + 1] & 0x07;
                    break;
                default:
                    printf("## Invalid cmd %02X!\n", cmd);
                }
            }
        }

        printf("-----------------------------------------------------\n");
        
        //system("PAUSE");
        count++;
    }

    fclose(anima);

    printf("count = %d\n", count);

    //system("PAUSE");
    return EXIT_SUCCESS;
}
