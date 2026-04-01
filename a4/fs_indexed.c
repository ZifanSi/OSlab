#include "fs_indexed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FileSystem fs;

static BlockNode *makeBlockNode(int blockNumber) {
    BlockNode *node = malloc(sizeof(*node));

    if (node == NULL) {
        return NULL;
    }

    node->blockNumber = blockNumber;
    node->next = NULL;
    return node;
}

static void clearFileEntry(FileInformationBlock *fib) {
    fib->inUse = 0;
    fib->fibId = -1;
    fib->filename[0] = '\0';
    fib->sizeBytes = 0;
    fib->indexBlock = -1;
    fib->dataBlockCount = 0;

    for (int i = 0; i < FS_MAX_DATA_BLOCKS_PER_FILE; ++i) {
        fib->dataBlocks[i] = -1;
    }
}

static int blocksRequired(size_t sizeBytes) {
    if (sizeBytes == 0) {
        return 0;
    }

    return (int) ((sizeBytes + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE);
}

void destroyFS(void) {
    BlockNode *current = fs.vcb.freeHead;

    while (current != NULL) {
        BlockNode *next = current->next;
        free(current);
        current = next;
    }

    memset(&fs, 0, sizeof(fs));

    for (int i = 0; i < FS_MAX_FILES; ++i) {
        clearFileEntry(&fs.files[i]);
    }
}

void initFS(void) {
    destroyFS();

    fs.vcb.totalBlocks = FS_TOTAL_BLOCKS;
    fs.vcb.blockSize = FS_BLOCK_SIZE;
    fs.vcb.fibIdHead = 0;
    fs.vcb.fibIdTail = 0;
    fs.vcb.fibIdCount = FS_MAX_FILES;

    for (int i = 0; i < FS_MAX_FILES; ++i) {
        fs.vcb.availableFibIds[i] = i;
        clearFileEntry(&fs.files[i]);
    }

    for (int blockNumber = 0; blockNumber < FS_TOTAL_BLOCKS; ++blockNumber) {
        returnFreeBlock(blockNumber);
    }

    printf("Filesystem initialized with %d blocks of %d bytes each.\n",
           fs.vcb.totalBlocks,
           fs.vcb.blockSize);
}

int allocateFreeBlock(void) {
    BlockNode *head = fs.vcb.freeHead;
    int blockNumber;

    if (head == NULL) {
        return -1;
    }

    blockNumber = head->blockNumber;
    fs.vcb.freeHead = head->next;

    if (fs.vcb.freeHead == NULL) {
        fs.vcb.freeTail = NULL;
    }

    free(head);
    fs.vcb.freeBlockCount--;
    return blockNumber;
}

void returnFreeBlock(int blockNumber) {
    BlockNode *node = makeBlockNode(blockNumber);

    if (node == NULL) {
        fprintf(stderr, "Failed to return block %d to the free list.\n", blockNumber);
        return;
    }

    if (fs.vcb.freeTail != NULL) {
        fs.vcb.freeTail->next = node;
    } else {
        fs.vcb.freeHead = node;
    }

    fs.vcb.freeTail = node;
    fs.vcb.freeBlockCount++;
}

int getFileInformationBlockID(void) {
    int fibId;

    if (fs.vcb.fibIdCount == 0) {
        return -1;
    }

    fibId = fs.vcb.availableFibIds[fs.vcb.fibIdHead];
    fs.vcb.fibIdHead = (fs.vcb.fibIdHead + 1) % FS_MAX_FILES;
    fs.vcb.fibIdCount--;
    return fibId;
}

void returnFileInformationBlockID(int fibId) {
    if (fibId < 0 || fibId >= FS_MAX_FILES || fs.vcb.fibIdCount >= FS_MAX_FILES) {
        return;
    }

    fs.vcb.availableFibIds[fs.vcb.fibIdTail] = fibId;
    fs.vcb.fibIdTail = (fs.vcb.fibIdTail + 1) % FS_MAX_FILES;
    fs.vcb.fibIdCount++;
}

int findFileIndex(const char *filename) {
    if (filename == NULL) {
        return -1;
    }

    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (fs.files[i].inUse && strcmp(fs.files[i].filename, filename) == 0) {
            return i;
        }
    }

    return -1;
}

int createFile(const char *filename, size_t sizeBytes) {
    int fibId;
    int indexBlock;
    int dataBlockCount;
    int totalBlocksNeeded;
    FileInformationBlock *fib;

    if (filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Create failed: filename is required.\n");
        return -1;
    }

    if (strlen(filename) >= FS_MAX_FILENAME) {
        fprintf(stderr, "Create failed: filename '%s' is too long.\n", filename);
        return -1;
    }

    if (findFileIndex(filename) != -1) {
        fprintf(stderr, "Create failed: '%s' already exists.\n", filename);
        return -1;
    }

    dataBlockCount = blocksRequired(sizeBytes);

    if (dataBlockCount > FS_MAX_DATA_BLOCKS_PER_FILE) {
        fprintf(stderr, "Create failed: '%s' exceeds index block capacity.\n", filename);
        return -1;
    }

    totalBlocksNeeded = dataBlockCount + 1;

    if (fs.vcb.fibIdCount == 0) {
        fprintf(stderr, "Create failed: no file information blocks are available.\n");
        return -1;
    }

    if (totalBlocksNeeded > fs.vcb.freeBlockCount) {
        fprintf(stderr, "Create failed: not enough free blocks for '%s'.\n", filename);
        return -1;
    }

    fibId = getFileInformationBlockID();

    if (fibId == -1) {
        fprintf(stderr, "Create failed: could not allocate a FIB ID.\n");
        return -1;
    }

    indexBlock = allocateFreeBlock();

    if (indexBlock == -1) {
        returnFileInformationBlockID(fibId);
        fprintf(stderr, "Create failed: could not allocate an index block.\n");
        return -1;
    }

    fib = &fs.files[fibId];
    clearFileEntry(fib);
    fib->fibId = fibId;
    fib->indexBlock = indexBlock;
    fib->sizeBytes = sizeBytes;
    fib->dataBlockCount = dataBlockCount;
    strncpy(fib->filename, filename, FS_MAX_FILENAME - 1);
    fib->filename[FS_MAX_FILENAME - 1] = '\0';

    for (int i = 0; i < dataBlockCount; ++i) {
        int blockNumber = allocateFreeBlock();

        if (blockNumber == -1) {
            for (int j = 0; j < i; ++j) {
                returnFreeBlock(fib->dataBlocks[j]);
            }

            returnFreeBlock(indexBlock);
            returnFileInformationBlockID(fibId);
            clearFileEntry(fib);
            fprintf(stderr, "Create failed: block allocation rolled back for '%s'.\n", filename);
            return -1;
        }

        fib->dataBlocks[i] = blockNumber;
    }

    fib->inUse = 1;

    printf("File '%s' created with %d data blocks + 1 index block.\n",
           fib->filename,
           fib->dataBlockCount);
    return 0;
}

int deleteFile(const char *filename) {
    int fileIndex;
    int fibId;
    int dataBlockCount;
    int indexBlock;
    char deletedName[FS_MAX_FILENAME];

    if (filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Delete failed: filename is required.\n");
        return -1;
    }

    fileIndex = findFileIndex(filename);

    if (fileIndex == -1) {
        fprintf(stderr, "Delete failed: '%s' was not found.\n", filename);
        return -1;
    }

    fibId = fs.files[fileIndex].fibId;
    dataBlockCount = fs.files[fileIndex].dataBlockCount;
    indexBlock = fs.files[fileIndex].indexBlock;
    strncpy(deletedName, fs.files[fileIndex].filename, FS_MAX_FILENAME - 1);
    deletedName[FS_MAX_FILENAME - 1] = '\0';

    for (int i = 0; i < dataBlockCount; ++i) {
        returnFreeBlock(fs.files[fileIndex].dataBlocks[i]);
    }

    returnFreeBlock(indexBlock);
    returnFileInformationBlockID(fibId);
    clearFileEntry(&fs.files[fileIndex]);

    printf("File '%s' deleted.\n", deletedName);
    return 0;
}

void listFiles(void) {
    int fileCount = 0;

    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (fs.files[i].inUse) {
            fileCount++;
        }
    }

    printf("\nRoot Directory Listing (%d files):\n", fileCount);

    for (int i = 0; i < FS_MAX_FILES; ++i) {
        if (!fs.files[i].inUse) {
            continue;
        }

        printf("  %-10s | %6llu bytes | %2d data blocks | FIBID=%d\n",
               fs.files[i].filename,
               (unsigned long long) fs.files[i].sizeBytes,
               fs.files[i].dataBlockCount,
               fs.files[i].fibId);
    }

    printf("\n");
}

void printFreeBlocks(void) {
    BlockNode *current = fs.vcb.freeHead;

    printf("Free Blocks (%d): ", fs.vcb.freeBlockCount);

    while (current != NULL) {
        printf("[%d] -> ", current->blockNumber);
        current = current->next;
    }

    printf("NULL\n");
}
