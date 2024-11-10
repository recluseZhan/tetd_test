#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  // shm_open, O_CREAT, O_RDWR
#include <sys/mman.h>  // mmap, PROT_WRITE, MAP_SHARED
#include <unistd.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 1024  // 定义共享内存大小

int main() {
    // 创建并打开共享内存对象
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // 设置共享内存大小
    ftruncate(shm_fd, SHM_SIZE);

    // 映射共享内存
    char *shm_ptr = (char*) mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // 写入数据
    strncpy(shm_ptr, "hello", SHM_SIZE);
    printf("Data written to memory: %s\n", shm_ptr);

    // 解除映射
    munmap(shm_ptr, SHM_SIZE);

    // 关闭共享内存对象
    close(shm_fd);
    getchar();
    return 0;
}

