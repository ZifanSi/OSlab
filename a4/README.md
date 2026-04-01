# A4 Practice Project

This folder contains a basic indexed-allocation file system simulator in C.

## Files
- `fs_indexed.h`: constants, structures, and function prototypes
- `fs_indexed.c`: file system implementation
- `main.c`: sample driver program
- `Makefile`: build and run targets
- `output-2.txt`: sample output from the handout

## Build
```sh
gcc -Wall -Wextra -std=c11 main.c fs_indexed.c -o a4_practice.exe
```

If you have `make` installed:

```sh
make
```

## Run
```sh
./a4_practice.exe
```

Or:

```sh
make run
```

## Current Behavior
- Initializes 64 blocks of 1024 bytes each
- Allocates one index block plus the required number of data blocks per file
- Tracks free blocks using a linked-list queue
- Recycles FIB IDs using a circular queue
- Lists files in FIB ID order
- Returns deleted file blocks to the tail of the free list
