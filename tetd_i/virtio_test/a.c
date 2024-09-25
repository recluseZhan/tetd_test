#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PMEM_FILE "/dev/pmem0"
#define BUFFER_SIZE 128

int main() {
    int fd;
    char buffer[BUFFER_SIZE];

    // 打开 PMEM 设备
    fd = open(PMEM_FILE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open PMEM device");
        return EXIT_FAILURE;
    }
    /*
    // 写入数据
    const char *message = "Hello from PMEM!";
    if (write(fd, message, strlen(message)) < 0) {
        perror("Failed to write to PMEM device");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Data written to PMEM: %s\n", message);
*/
    // 从 PMEM 读取数据
    lseek(fd, 0, SEEK_SET); // 将文件指针移动到开头
    if (read(fd, buffer, BUFFER_SIZE) < 0) {
        perror("Failed to read from PMEM device");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Data read from PMEM: %s\n", buffer);

    // 关闭文件描述符
    close(fd);
    return EXIT_SUCCESS;
}

