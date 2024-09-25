#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#define PHYSICAL_ADDRESS 0x7f000000  // 你要读取的物理地址
#define SIZE 1  // 读取的字节数

int main() {
    int fd = open("/dev/mem", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    void *map = mmap(NULL, SIZE, PROT_READ, MAP_SHARED, fd, PHYSICAL_ADDRESS);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    uint8_t *ptr = (uint8_t *)map;
    for (int i = 0; i < SIZE; i++) {
        printf("%02x ", ptr[i]);
    }
    printf("\n");

    munmap(map, SIZE);
    close(fd);
    return 0;
}

