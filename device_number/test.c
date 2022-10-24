#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
    int status = -1;
    int dev = open("/dev/my_device", O_RDONLY);
    if (dev == -1){
        printf("Canoot open\n");
    }else{
        status=0;
        printf("Opening Successful\n");
        close(dev);
    }
     return status;
}