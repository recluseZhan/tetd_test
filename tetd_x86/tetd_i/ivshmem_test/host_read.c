#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define SHM_PATH "/dev/shm/ivshmem"
#define SHM_SIZE 20

int main() {
    // 打开共享内存文件
    int fd = open(SHM_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    // 将共享内存映射到进程地址空间
    void *shm_addr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    // 读取共享内存中的数据
    uint32_t value = *((uint32_t *)shm_addr);
    printf("Read value from shared memory: 0x%x\n", value);

    // 清理
    munmap(shm_addr, SHM_SIZE);
    close(fd);

    return 0;
}

