#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define OFFSET_BITS 12U
#define PAGE_SIZE (1U << OFFSET_BITS)
#define OFFSET_MASK (PAGE_SIZE - 1U)
#define PAGES 8U

int main(int argc, char *argv[]) {
    const char *input_path = (argc > 1) ? argv[1] : "labaddr.txt";
    unsigned int page_table[PAGES] = {6U, 4U, 3U, 7U, 0U, 1U, 2U, 5U};
    char buff[32];
    FILE *fptr = fopen(input_path, "r");

    if (fptr == NULL) {
        perror("fopen");
        return 1;
    }

    while (fgets(buff, sizeof(buff), fptr) != NULL) {
        char *endptr = NULL;
        unsigned long logical_ul;
        unsigned int logical_address;
        unsigned int page_number;
        unsigned int offset;
        unsigned int frame_number;
        unsigned int physical_address;

        errno = 0;
        logical_ul = strtoul(buff, &endptr, 10);

        if (endptr == buff || errno == ERANGE) {
            continue;
        }

        logical_address = (unsigned int)logical_ul;
        page_number = logical_address >> OFFSET_BITS;
        if (page_number >= PAGES) {
            fprintf(stderr, "Skipping address out of page-table range: %u\n", logical_address);
            continue;
        }

        offset = logical_address & OFFSET_MASK;
        frame_number = page_table[page_number];
        physical_address = (frame_number << OFFSET_BITS) | offset;

        printf(
            "Virtual addr is %u: Page# = %u & Offset = %u. Physical addr = %u.\n",
            logical_address,
            page_number,
            offset,
            physical_address
        );
    }

    fclose(fptr);
    return 0;
}
