#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define INT_SIZE 4
#define INT_COUNT 10
#define MEMORY_SIZE (INT_COUNT * INT_SIZE)

int intArray[INT_COUNT];
signed char *mmapfptr;

int main(int argc, char *argv[]) {
    const char *input_path = (argc > 1) ? argv[1] : "numbers.bin";
    int mmapfile_fd = open(input_path, O_RDONLY);
    int i;
    long long sum = 0;

    if (mmapfile_fd < 0) {
        perror("open");
        return 1;
    }

    mmapfptr = mmap(NULL, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, mmapfile_fd, 0);
    if (mmapfptr == MAP_FAILED) {
        perror("mmap");
        close(mmapfile_fd);
        return 1;
    }

    for (i = 0; i < INT_COUNT; ++i) {
        memcpy(intArray + i, mmapfptr + (INT_SIZE * i), INT_SIZE);
    }

    if (munmap(mmapfptr, MEMORY_SIZE) != 0) {
        perror("munmap");
        close(mmapfile_fd);
        return 1;
    }

    close(mmapfile_fd);

    for (i = 0; i < INT_COUNT; ++i) {
        sum += intArray[i];
    }

    printf("Sum of numbers = %lld\n", sum);
    return 0;
}
