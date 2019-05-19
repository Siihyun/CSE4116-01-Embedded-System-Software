#include <unistd.h>
#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE_NAME "/dev/dev_driver"
struct mystruct{
    int interval;
    int count;
    int option;
};

int main(int argc, char* argv[]){
    struct mystruct my_st;
    int fp;
    if(argc!=4){
        printf("input error!\n");
        return -1;
    }
    my_st.interval = atoi(argv[1]);
    my_st.count = atoi(argv[2]);
    my_st.option = atoi(argv[3]);

    if(my_st.interval < 0 || my_st.interval > 100 || my_st.count < 1 || my_st.count > 100 
            || my_st.option < 1 || my_st.option > 8000){
        printf("input error!\n");
        return -1;
    }

    unsigned int res = syscall(376,&my_st);
    fp = open(DEVICE_NAME, O_WRONLY);
    if(fp < 0){
        printf("Device open error!\n");
        return -1;
    }
    ioctl(fp,0,(char *)&res);
    close(fp);
    return 0;
}

