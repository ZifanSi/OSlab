#include "fs_indexed.h"

int main(void) {
    initFS();

    printf("After init:\n");
    printFreeBlocks();

    printf("\nCreating files...\n");
    createFile("a.txt", 1500);
    createFile("b.bin", 4096);
    createFile("empty.dat", 0);

    listFiles();
    printFreeBlocks();

    printf("\nDeleting a.txt ...\n");
    deleteFile("a.txt");

    listFiles();
    printFreeBlocks();

    return 0;
}
