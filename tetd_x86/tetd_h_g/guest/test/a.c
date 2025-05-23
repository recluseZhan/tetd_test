#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEVNAME "/dev/readgpa_dev"
#include <sys/types.h>

#define HPA 0x22d3b0000
void main(){
    int fd;
    unsigned long a[4] = {HPA, 0x0, 0x1234, 0};
    unsigned long b[4] = {HPA, 0x0, 0x1234, 1};
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    //write(fd,&a[0],sizeof(a[0]));
    read(fd,a,sizeof(a));
    close(fd);
    //getchar();

    //fd = open(DEVNAME,O_RDWR);
    //printf("fd:%d\n",fd);
    //read(fd,b,sizeof(b));
    //close(fd); 
     
    return 0;
}
