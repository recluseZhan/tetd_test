#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEVNAME "/dev/tdi_dev"
#include <sys/types.h>

void main(){
    int fd;
    unsigned long a[1] = {0};
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd,a,sizeof(a));
    close(fd);

    return 0;
}
