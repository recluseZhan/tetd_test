#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEVNAME "/dev/tdi_dev"
#include <sys/types.h>

void main(){
    int fd;
    unsigned long a[3] = {1,2,3};
    unsigned long gva = 0x0;
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    gva = read(fd,a,sizeof(a));
    close(fd);
    printf("target gva 0x%lx\n",gva);

    unsigned long b[2] = {0, 1023};
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd, b, sizeof(b));
    close(fd);
    
    return 0;
}
