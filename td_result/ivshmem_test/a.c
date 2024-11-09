#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define IVSHMEM_DEVICE "/sys/bus/pci/devices/0000:00:07.0/resource2"  // 使用 Region 2 的 BAR 文件路径
#define MEMORY_SIZE (1 * 1024 * 1024)  // 1MB，Region 2 的大小

// 写入 ivshmem
void write_ivshmem(uint64_t offset, uint32_t value, void *mapped_mem) {
    if (offset >= MEMORY_SIZE) {
        fprintf(stderr, "Offset out of bounds!\n");
        return;
    }
    *((uint32_t *)(mapped_mem + offset)) = value;
}

// 读取 ivshmem
uint32_t read_ivshmem(uint64_t offset, void *mapped_mem) {
    if (offset >= MEMORY_SIZE) {
        fprintf(stderr, "Offset out of bounds!\n");
        return -1;
    }
    return *((uint32_t *)(mapped_mem + offset));
}

int main() {
    int fd;
    void *mapped_mem;
    uint32_t value;
    uint64_t offset;

    // 打开 ivshmem 设备文件
    fd = open(IVSHMEM_DEVICE, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Failed to open ivshmem device");
        return 1;
    }

    // 将 ivshmem 设备映射到用户空间 (Region 2)
    mapped_mem = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("Failed to mmap ivshmem device");
        close(fd);
        return 1;
    }

    // 写入共享内存
    offset = 0;  // 选择你想要的偏移量
    value = 0x12345678;  // 写入的值
    printf("Writing value 0x%x to offset %llu\n", value, offset);
    write_ivshmem(offset, value, mapped_mem);

    // 读取共享内存
    uint32_t read_value = read_ivshmem(offset, mapped_mem);
    printf("Read value 0x%x from offset %llu\n", read_value, offset);

    // 释放映射
    munmap(mapped_mem, MEMORY_SIZE);
    close(fd);

    return 0;
}

