#include "20141544.h"
//#include "./fpga_dot_font.h"
void rcv_msg(){
    
    key_t key_id = msgget((key_t)1234, IPC_CREAT|0666);
    /*fnd , led, lcd, dot을 각각 관리하는 4개의 thread 생성 */
    pthread_t p_thread[4];
    int thr_id_for_fnd;
    int thr_id_for_led;
    int thr_id_for_lcd;
    int thr_id_for_dot;

    thr_id_for_fnd = pthread_create(&p_thread[0],NULL,rcv_msg_for_fnd,NULL);
    if(thr_id_for_fnd < 0){
        perror("thread create error :");
        return;
    }
    
    thr_id_for_led = pthread_create(&p_thread[1],NULL,rcv_msg_for_led,NULL);
    if(thr_id_for_led < 0){
        perror("thread create error :");
        return;
    }

    thr_id_for_lcd = pthread_create(&p_thread[2],NULL,rcv_msg_for_lcd,NULL);
    if(thr_id_for_lcd < 0){
        perror("thread create error :");
        return;
    }

    thr_id_for_dot = pthread_create(&p_thread[3],NULL,rcv_msg_for_dot,NULL);
    if(thr_id_for_dot < 0){
        perror("thread create error :");
        return;
    }

    pthread_join(p_thread[0], (void **)NULL);
    pthread_join(p_thread[1], (void **)NULL);
    pthread_join(p_thread[2], (void **)NULL);
    pthread_join(p_thread[3], (void **)NULL);
    return;
}
void *rcv_msg_for_fnd(){ //fnd를 받는 함수

    fnd rcvfnd;
    int i,msgtype;
    unsigned char retval;
    msgtype = 3;
    
    while(1){
        if (msgrcv( key_id, (void *)&rcvfnd, sizeof(fnd), msgtype, 0) ==-1){
            perror("msgrcv error");
            return;
        }
        retval = write(dev[FND],&rcvfnd.data,32); // 받아서 write
    }
    return;
}
void *rcv_msg_for_led(){ // led를 받는 함수. state == 0이면 받아온 data값에 따라 led 출력, state == 1 이면 2,3led 깜빡깜빡 거리는 기능
    led rcvled;
    int i,msgtype;
    int cnt = 0;
    unsigned char retval;
    msgtype = 4;
    struct msqid_ds msqstat;
    while(1){
        if (msgrcv( key_id, (void *)&rcvled, sizeof(led), msgtype, 0) ==-1){
            perror("msgrcv error");
            return;
        }
        if(rcvled.status == 0){ 
            led_start(rcvled.data); 
        }
        else if(rcvled.status == 1){
            while(1){
                msgctl(key_id,MSG_STAT,&msqstat); // 다음 메세지가 들어올 때 까지 깜빡거림
                if(msqstat.msg_qnum != 0)  break;
                led_start(32);
                led_start(16);
            }
        }
    }
    return;
}
void *rcv_msg_for_lcd(){ // rcv한 string을 lcd로 출력해주는 함수
    lcd rcvlcd;
    int i,msgtype;
    int cnt = 0;
    unsigned char retval;
    char str[32];
    memset(str,' ',32);
    msgtype = 5;
    while(1){
        if (msgrcv( key_id, (void *)&rcvlcd, sizeof(lcd), msgtype, 0) ==-1){
            perror("msgrcv error");
            return;
        }

        strncpy(str,rcvlcd.string,32);
        write(dev[LCD],str,32);
    }
    return;
}

void *rcv_msg_for_dot(){ // rcv한 matrix를 dot으로 출력해주는 함수.

    struct msqid_ds msqstat;
    int i,msgtype,x,y;
    int cnt = 0;
    unsigned char retval;
    msgtype = 6;
    while(1){
        if (msgrcv( key_id, (void *)&rcvdot, sizeof(dot), msgtype, 0) ==-1){
            perror("msgrcv error");
            return;
        }
        x = rcvdot.x;
        y = rcvdot.y;
        if (rcvdot.status == 1 && rcvdot.dotmatrix[x][y]==0){
            while(1){
                msgctl(key_id,MSG_STAT,&msqstat);
                if(msqstat.msg_qnum != 0){
                    rcvdot.dotmatrix[x][y] = 0;
                    break;
                }
                if(rcvdot.dotmatrix[x][y]) rcvdot.dotmatrix[x][y] = 0;
                else rcvdot.dotmatrix[x][y] = 1;
                calculate_matrix(1);
                write(dev[DOT],rcvdot.matrix,10);
            }
            
        }
        if(rcvdot.status!=3)
            calculate_matrix(0);
        write(dev[DOT],rcvdot.matrix,10);
    }
    return;
}
void calculate_matrix(int sec){ // char[10][7] 배열을 만들어 0과 1로 켜짐과 꺼짐을 구분하여, 그것을 계산하여 dot matrix를 완성 시켜 주는 함수
    int i,j,mul,num;
    for(i = 0 ; i < 10 ; i++){
        num = 0;
        mul = 64;
        for(j = 0 ; j < 7 ; j++){
            if(rcvdot.dotmatrix[i][j]){
                num += mul;
            }
            mul/=2;
        }
        rcvdot.matrix[i] = num;
    }
    if(sec) sleep(sec);
}
void init_led(){ // led 초기화 함수
	int i;
    fpga_addr = 0;
	led_addr = 0;
	fpga_addr = (unsigned long *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, dev[LED], FPGA_BASE_ADDRESS);
	if (fpga_addr == MAP_FAILED) //mapping fail check
	{
		printf("mmap error!\n");
		close(dev[LED]);
		exit(1);
	}
	led_addr=(unsigned char*)((void*)fpga_addr+LED_ADDR);
}
void led_start(unsigned char data){ // led 출력 함수
	*led_addr=data;
	sleep(1);
	return;

}
