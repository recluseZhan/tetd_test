#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>

int main() {
    unsigned long phys_addr = 0x7ffff000;  // 假设这是一个有效的物理地址
    size_t size = 4096;  // 映射1页内存

    // 打开 /dev/mem（需要 root 权限）
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("Error opening /dev/mem");
        return 1;
    }

    // 使用 mmap 映射物理内存到用户空间
    void *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr);
    if (mapped == MAP_FAILED) {
        perror("Error mapping memory");
        close(mem_fd);
        return 1;
    }

    printf("Mapped physical address %lx to virtual address %p\n", phys_addr, mapped);

    // 访问映射后的内存
    unsigned long *data = (unsigned long *)mapped;
    printf("Value at mapped address: %lx\n", *data);
    *data = 0x10;
    printf("Value at mapped address: %lx\n", *data);

    // 清理
    munmap(mapped, size);
    close(mem_fd);

    return 0;
}

