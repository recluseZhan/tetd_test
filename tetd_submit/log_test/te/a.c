#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEVNAME "/dev/tdi_dev"
#include <sys/types.h>

void main(){
    int fd;
    unsigned long a[3] = {1,2,3};
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    read(fd,a,sizeof(a));
    close(fd);

    return 0;
}
