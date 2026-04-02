#ifndef FS_INDEXED_H
#define FS_INDEXED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 1024
#define TOTAL_BLOCKS 64
#define MAX_FILES 10
#define MAX_FILENAME_LEN 64
#define MAX_DATA_BLOCKS_PER_FILE (BLOCK_SIZE / (int)sizeof(int))

/*
 * Each logical disk block is 1 KB.
 */
typedef struct Block {
    unsigned char data[BLOCK_SIZE];
    int blockNumber;
} Block;

/*
 * Linked-list node used to simulate the free block list.
 */
typedef struct FreeBlockNode {
    int blockNumber;
    struct FreeBlockNode *next;
} FreeBlockNode;

/*
 * File Information Block (FIB)
 * - fibID: unique file information block id
 * - fileName: file name
 * - fileSize: file size in bytes
 * - blockCount: number of DATA blocks used by the file
 * - indexBlock: pointer to the block that stores the data-block numbers
 */
typedef struct FIB {
    int fibID;
    char fileName[MAX_FILENAME_LEN];
    int fileSize;
    int blockCount;
    Block *indexBlock;
} FIB;

/*
 * Volume Control Block / file system structure.
 */
typedef struct VolumeControlBlock {
    FreeBlockNode *freeHead;
    FreeBlockNode *freeTail;
    Block disk[TOTAL_BLOCKS];
    int fibInUse[MAX_FILES];
    FIB files[MAX_FILES];
    int fileCount;
} VolumeControlBlock;

/*
 * Global simulated file system.
 * Kept static so the header can be included in one .c file directly.
 */
static VolumeControlBlock fs;

/* ========================= Helper functions ========================= */

static int countFreeBlocks(void) {
    int count = 0;
    FreeBlockNode *curr = fs.freeHead;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

static int blocksNeededForSize(int size) {
    if (size <= 0) {
        return 0;
    }
    return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

static int findFileIndex(const char *filename) {
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (fs.fibInUse[i] && strcmp(fs.files[i].fileName, filename) == 0) {
            return i;
        }
    }
    return -1;
}

static int getFreeFIBSlot(void) {
    int i;
    for (i = 0; i < MAX_FILES; i++) {
        if (!fs.fibInUse[i]) {
            return i;
        }
    }
    return -1;
}

static void writeIndexEntry(Block *indexBlock, int entryIndex, int blockNumber) {
    int *entries = (int *)indexBlock->data;
    entries[entryIndex] = blockNumber;
}

static int readIndexEntry(const Block *indexBlock, int entryIndex) {
    const int *entries = (const int *)indexBlock->data;
    return entries[entryIndex];
}

/* ========================= Required functions ========================= */

/*
 * initFS() – initializes the file system.
 */
static void initFS(void) {
    int i;

    fs.freeHead = NULL;
    fs.freeTail = NULL;
    fs.fileCount = 0;

    for (i = 0; i < TOTAL_BLOCKS; i++) {
        FreeBlockNode *node = (FreeBlockNode *)malloc(sizeof(FreeBlockNode));
        if (node == NULL) {
            fprintf(stderr, "Memory allocation failed while initializing free block list.\n");
            exit(EXIT_FAILURE);
        }

        memset(fs.disk[i].data, 0, BLOCK_SIZE);
        fs.disk[i].blockNumber = i;

        node->blockNumber = i;
        node->next = NULL;

        if (fs.freeTail == NULL) {
            fs.freeHead = node;
            fs.freeTail = node;
        } else {
            fs.freeTail->next = node;
            fs.freeTail = node;
        }
    }

    for (i = 0; i < MAX_FILES; i++) {
        fs.fibInUse[i] = 0;
        fs.files[i].fibID = -1;
        fs.files[i].fileName[0] = '\0';
        fs.files[i].fileSize = 0;
        fs.files[i].blockCount = 0;
        fs.files[i].indexBlock = NULL;
    }
}

/*
 * allocateFreeBlock() – removes a free block from the head of the free block list and returns it.
 */
static Block *allocateFreeBlock(void) {
    FreeBlockNode *node;
    int blockNumber;

    if (fs.freeHead == NULL) {
        return NULL;
    }

    node = fs.freeHead;
    blockNumber = node->blockNumber;
    fs.freeHead = fs.freeHead->next;

    if (fs.freeHead == NULL) {
        fs.freeTail = NULL;
    }

    free(node);
    memset(fs.disk[blockNumber].data, 0, BLOCK_SIZE);
    return &fs.disk[blockNumber];
}

/*
 * returnFreeBlock(Block) – adds a free block back to the tail of the free block list.
 */
static void returnFreeBlock(Block *block) {
    FreeBlockNode *node;

    if (block == NULL) {
        return;
    }

    memset(block->data, 0, BLOCK_SIZE);

    node = (FreeBlockNode *)malloc(sizeof(FreeBlockNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed while returning a free block.\n");
        exit(EXIT_FAILURE);
    }

    node->blockNumber = block->blockNumber;
    node->next = NULL;

    if (fs.freeTail == NULL) {
        fs.freeHead = node;
        fs.freeTail = node;
    } else {
        fs.freeTail->next = node;
        fs.freeTail = node;
    }
}

/*
 * printFreeBlocks() – displays all free block numbers and the total count of free block.
 */
static void printFreeBlocks(void) {
    FreeBlockNode *curr = fs.freeHead;
    int count = 0;

    printf("Free blocks: ");
    while (curr != NULL) {
        printf("%d ", curr->blockNumber);
        count++;
        curr = curr->next;
    }
    printf("\nTotal free blocks: %d\n", count);
}

/*
 * createFile(filename, size) – creates a file in file system.
 * Returns 1 on success, 0 on failure.
 */
static int createFile(const char *filename, int size) {
    int fibSlot;
    int dataBlocksNeeded;
    int i;
    Block *indexBlock;

    if (filename == NULL || filename[0] == '\0') {
        printf("Error: invalid file name.\n");
        return 0;
    }

    if ((int)strlen(filename) >= MAX_FILENAME_LEN) {
        printf("Error: file name too long.\n");
        return 0;
    }

    if (size < 0) {
        printf("Error: file size cannot be negative.\n");
        return 0;
    }

    if (findFileIndex(filename) != -1) {
        printf("Error: file '%s' already exists.\n", filename);
        return 0;
    }

    fibSlot = getFreeFIBSlot();
    if (fibSlot == -1) {
        printf("Error: maximum file limit reached.\n");
        return 0;
    }

    dataBlocksNeeded = blocksNeededForSize(size);
    if (dataBlocksNeeded > MAX_DATA_BLOCKS_PER_FILE) {
        printf("Error: file too large for one index block.\n");
        return 0;
    }

    /* Need 1 index block + data blocks. */
    if (countFreeBlocks() < dataBlocksNeeded + 1) {
        printf("Error: not enough free blocks to create '%s'.\n", filename);
        return 0;
    }

    indexBlock = allocateFreeBlock();
    if (indexBlock == NULL) {
        printf("Error: failed to allocate index block.\n");
        return 0;
    }

    for (i = 0; i < dataBlocksNeeded; i++) {
        Block *dataBlock = allocateFreeBlock();
        if (dataBlock == NULL) {
            int j;
            for (j = 0; j < i; j++) {
                int oldBlockNum = readIndexEntry(indexBlock, j);
                returnFreeBlock(&fs.disk[oldBlockNum]);
            }
            returnFreeBlock(indexBlock);
            printf("Error: failed to allocate all data blocks.\n");
            return 0;
        }

        writeIndexEntry(indexBlock, i, dataBlock->blockNumber);
    }

    fs.fibInUse[fibSlot] = 1;
    fs.files[fibSlot].fibID = fibSlot;
    strncpy(fs.files[fibSlot].fileName, filename, MAX_FILENAME_LEN - 1);
    fs.files[fibSlot].fileName[MAX_FILENAME_LEN - 1] = '\0';
    fs.files[fibSlot].fileSize = size;
    fs.files[fibSlot].blockCount = dataBlocksNeeded;
    fs.files[fibSlot].indexBlock = indexBlock;
    fs.fileCount++;

    printf("File '%s' created successfully.\n", filename);
    printf("  FIB ID: %d\n", fs.files[fibSlot].fibID);
    printf("  Index block: %d\n", indexBlock->blockNumber);
    printf("  Data blocks: ");
    for (i = 0; i < dataBlocksNeeded; i++) {
        printf("%d ", readIndexEntry(indexBlock, i));
    }
    printf("\n");

    return 1;
}

/*
 * deleteFile(filename) – deletes a file with the given file name.
 * Returns 1 on success, 0 on failure.
 */
static int deleteFile(const char *filename) {
    int fileIndex;
    int i;
    FIB *file;

    fileIndex = findFileIndex(filename);
    if (fileIndex == -1) {
        printf("Error: file '%s' not found.\n", filename);
        return 0;
    }

    file = &fs.files[fileIndex];

    for (i = 0; i < file->blockCount; i++) {
        int dataBlockNumber = readIndexEntry(file->indexBlock, i);
        returnFreeBlock(&fs.disk[dataBlockNumber]);
    }

    returnFreeBlock(file->indexBlock);

    fs.fibInUse[fileIndex] = 0;
    fs.files[fileIndex].fibID = -1;
    fs.files[fileIndex].fileName[0] = '\0';
    fs.files[fileIndex].fileSize = 0;
    fs.files[fileIndex].blockCount = 0;
    fs.files[fileIndex].indexBlock = NULL;
    fs.fileCount--;

    printf("File '%s' deleted successfully.\n", filename);
    return 1;
}

/*
 * listFiles() – lists all the files in the flat file system.
 */
static void listFiles(void) {
    int i;
    int found = 0;

    printf("\nFiles in RootDirectory:\n");
    printf("-------------------------------------------------------------\n");

    for (i = 0; i < MAX_FILES; i++) {
        int j;
        if (!fs.fibInUse[i]) {
            continue;
        }

        found = 1;
        printf("FIB ID: %d\n", fs.files[i].fibID);
        printf("File name: %s\n", fs.files[i].fileName);
        printf("File size: %d bytes\n", fs.files[i].fileSize);
        printf("Data block count: %d\n", fs.files[i].blockCount);
        printf("Index block: %d\n", fs.files[i].indexBlock->blockNumber);
        printf("Data blocks: ");
        for (j = 0; j < fs.files[i].blockCount; j++) {
            printf("%d ", readIndexEntry(fs.files[i].indexBlock, j));
        }
        printf("\n-------------------------------------------------------------\n");
    }

    if (!found) {
        printf("No files in the file system.\n");
        printf("-------------------------------------------------------------\n");
    }
}

#endif
