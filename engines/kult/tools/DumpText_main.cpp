#include <cstdlib>
#include <iostream>

using namespace std;

typedef unsigned char byte;

int main(int argc, char *argv[]) {

    //FILE *bin = fopen("P:\\Games\\1\\chamber\\DESCE.BIN", "rb");
    //FILE *bin = fopen("P:\\Games\\1\\chamber\\VEPCE.BIN", "rb");
    //FILE *bin = fopen("P:\\Games\\1\\chamber\\DIALE.BIN", "rb");
    FILE *bin = fopen("P:\\Games\\1\\chamber\\MOTSE.BIN", "rb");
    
    freopen("MOTSE.log", "wt", stdout);
    
    //fseek(bin, 1, SEEK_SET);

    int num = 0;

    while (1) {
        byte len;
        fread(&len, 1, 1, bin);
        if (feof(bin))
            break;
        printf("%04d: ", num);
        while (--len) {
            byte ch;
            fread(&ch, 1, 1, bin);
            printf("%c", (ch & 0x3F) + 0x20);
            if ((ch & 0xC0) == 0xC0) printf("A");
            else if (ch & 0x80) printf("E");
            else if (ch & 0x40) printf(" ");
            //if ((ch & 0x3F) == 0) printf("<br>");
        }
        printf("\n");
        num++;
        //if (num++ >= 100) break;
    }
    
    fclose(bin);

    //system("PAUSE");
    return EXIT_SUCCESS;
}
