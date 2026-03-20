# Lab 3

This folder contains both required C programs from `Lab 3-2.pdf`.

## Files

- `lab3a.c`: Part I (virtual-to-physical address translation using the given page table).
- `lab3b.c`: Part II (memory-map `numbers.bin`, copy integers with `memcpy`, print sum).
- `labaddr.txt`: Input addresses for Part I.
- `numbers.bin`: Input binary file for Part II.

## Build

`lab3a.c` is standard C and built successfully in this folder.
`lab3b.c` uses POSIX `mmap()` and should be compiled on Linux/macOS/WSL.

Run from inside the `lab3` folder:

```bash
gcc -Wall -Wextra -std=c11 lab3a.c -o lab3a
gcc -Wall -Wextra -std=c11 lab3b.c -o lab3b
```

## Run

```bash
./lab3a
./lab3b
```

Optional explicit input paths:

```bash
./lab3a labaddr.txt
./lab3b numbers.bin
```

## Expected Output Highlights

- `lab3a` should print 20 translated addresses (same format as the handout sample).
- `lab3b` should print:

```text
Sum of numbers = 92
```

## Verified

- `lab3a` output was verified and matches the sample in the handout line-for-line.
- `numbers.bin` contents sum to `92`, which matches the expected `lab3b` output.
