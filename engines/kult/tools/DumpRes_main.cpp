#include <cstdlib>
#include <iostream>

using namespace std;

struct Entry {
    char *filename;
    int offset, size;
};

static const Entry entries[] = {
    {"KULTEGA.BIN", 0xF947, 27337},
    {"ARPLA.BIN", 0x2E8D, 8003},
    {"ALEAT.BIN", 0xD40, 256},
    {"ICONE.BIN", 0xE40, 2756},
    {"SOUCO.BIN", 0x17DA0, 425},
    {"CARPC.BIN", 0x505C, 384},
    {"SOURI.EGA", 0x4DD0, 576}, // CHECKME
    {"MURSM.BIN", 0x5010, 76},
    {"GAUSS.EGA", 0x200, 2880},
    {"LUTIN.BIN", 0x1B9F, 2800},
    {"ANIMA.BIN", 0x268F, 2046},
    {"ANICO.BIN", 0x1904, 667},
    {"ZONES.BIN", 0x9BF8, 9014},
    {NULL, 0, 0}
};


int main(int argc, char *argv[]) {

    FILE *exe = fopen("Q:\\OldGames\\Kult\\KULT2.PXI.000", "rb");

    for (const Entry *entry = &entries[0]; entry->filename; entry++) {
        printf("%s @ %08X (%d)...\n", entry->filename, entry->offset, entry->size);
        FILE *outFile = fopen(entry->filename, "wb");
        void *buffer = malloc(entry->size);
        fseek(exe, entry->offset, SEEK_SET);
        fread(buffer, entry->size, 1, exe);
        fwrite(buffer, entry->size, 1, outFile);
        free(buffer);
        fclose(outFile);
    }
    
    fclose(exe);

    system("PAUSE");
    return EXIT_SUCCESS;
}
