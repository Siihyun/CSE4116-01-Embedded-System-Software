#include "20141544.h"
void send_msg(){
    key_id = msgget((key_t)1234, IPC_CREAT|0666);
    pthread_t p_thread[2];
    int thr_id_for_key;
    int thr_id_for_switch;
    /* key와 switch input을 받는 thread 2개 생성 */
    thr_id_for_key = pthread_create(&p_thread[0],NULL,key_msg,NULL);
    if(thr_id_for_key < 0){
        perror("thread create error :");
        return;
    }
    
    thr_id_for_switch = pthread_create(&p_thread[1],NULL,switch_msg,NULL);
    if(thr_id_for_switch < 0){
        perror("thread create error :");
        return;
    }
    pthread_join(p_thread[0], (void **)NULL);
    pthread_join(p_thread[1], (void **)NULL);

    return;
}
void *key_msg(){ // ksg msg를 받아 main으로 전달
    key_queue send_key;
    struct input_event ev[BUFF_SIZE];
    int i, fd , rd , value , size = sizeof(struct input_event);
    char name[256] ="Unknown";
    char* device = "/dev/input/event0";
    if((fd = open (device, O_RDONLY)) == -1){
        printf("%s is not a valid device.n",device);
        return;
    }
    send_key.seq = 0;
    i = 0;

    while(1){
        send_key.msgtype = 1;
        send_key.seq = i;
        rd = read(fd, ev, size*BUFF_SIZE);
        value = ev[0].value;
        send_key.type = ev[0].type;
        send_key.value = ev[0].value;
        send_key.code = ev[0].code;

        if (msgsnd( key_id, (void *)&send_key, sizeof(key_queue)-sizeof(long), IPC_NOWAIT) == -1)
        {
            perror("msgsnd error : ");
            return;
        }
    }
    close(fd);
    if(terminate) pthread_exit(NULL);
}
void *switch_msg(){ // switch msg를 받아 main으로 전달
    quit = 0;
    switch_queue send_switch;
    send_switch.msgtype = 2;
    int i, dev, buff_size;
    
    dev = open("/dev/fpga_push_switch", O_RDWR);
    if(dev<0){
        printf("Device Open Error\n");
        close(dev);
        return;
    }
    
    (void)signal(SIGINT, user_signal1);
    buff_size = sizeof(send_switch.push_sw_buff);
    while(!quit){
        usleep(400000);
        read(dev,&send_switch.push_sw_buff, buff_size);
        if(msgsnd( key_id, (void *)&send_switch, sizeof(switch_queue)-sizeof(long),0==-1)){
                perror("swich msg send error"); return;
        }
    }
    close(dev);
    if(terminate) pthread_exit(NULL);
}
void user_signal1(int sig){
    quit = 1;
}
