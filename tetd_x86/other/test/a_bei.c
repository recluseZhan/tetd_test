#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEVNAME "/dev/dump_dev"
#include <sys/types.h>

void main(){
    unsigned char a[20]="hello,world!thiswork";
    printf("%lx\n",a);
    int fd = 0;
    
    pid_t pid=getpid();
    unsigned long va = a;
    char pp[1]= {'0'};
    //fd = open(DEVNAME,O_RDWR);
    //printf("fd:%d\n",fd);
    //read(fd,pp,sizeof(pp));
    //close(fd);  
    
    fd = open(DEVNAME,O_RDWR);
    printf("fd:%d\n",fd);
    write(fd,pp,sizeof(pp));
    close(fd); 
    
    getchar();
    printf("%d\n",a);
    return 0;
}
