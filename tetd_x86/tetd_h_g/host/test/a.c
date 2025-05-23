#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEVNAME "/dev/readgpa_dev"
#include <sys/types.h>

void main(){
    int fd;
    unsigned long a[1] = {0x123456789000};
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd,&a[0],sizeof(a[0]));
    read(fd,a,sizeof(a));
    close(fd);  
    return 0;
}
