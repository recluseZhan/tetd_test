#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    unsigned long base_ipa = 0x20000000;
    size_t size = 0x1000;

    if (argc > 1) {
        char *endptr;
        base_ipa = strtoul(argv[1], &endptr, 0);
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid input for base_ipa: %s\n", argv[1]);
            return 1;
        }
    }

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("Error opening /dev/mem");
        return 1;
    }

    void *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, base_ipa);
    if (mapped == MAP_FAILED) {
        perror("Error mapping memory");
        close(mem_fd);
        return 1;
    }

    printf("Mapped physical address %lx to virtual address %p\n", base_ipa, mapped);

    char *data = (char *)mapped;
    printf("Value at mapped address: %u\n", *data);
    *data = 0x10;
    printf("Value at mapped address: %u\n", *data);

    munmap(mapped, size);
    close(mem_fd);

    return 0;
}

