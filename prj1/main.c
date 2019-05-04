#include "20141544.h"
int main(){
    init_main();
    terminate = 0;
    int status;
    int input_pid = fork();
    if(input_pid==-1){
        printf("input fork error!\n");
        return -1;
    }
    else if(input_pid == 0) { // input process
        //TO DO : SEND MSG TO MAIN
        send_msg();
    }
    else{ // main
        int output_pid = fork();
        if(output_pid==-1){
            printf("output fork error!\n");
            return -1;
        }
        else if(output_pid ==0){ // output process
            //TO DO : RECEIVE MSG FROM MAIN
            init_output();
            rcv_msg();
        }
        else{ // main
            //TO DO : RECEIVE MSG FROM INPUT PROCESS AND SEND OUTPUT TO OUTPUT PROCESS
            listen();
            if(terminate){ // back button check flag
                sleep(1); 
                kill(input_pid,9); // process kill
                kill(output_pid,9); // process kill
                exit(0); 
            }
        }
    }
}
void listen(){ // input process
    key_t key_id = msgget((key_t)1234, IPC_CREAT|0666); // key generate
    pthread_t p_thread[3]; // thread create for key,switch,time
    int thr_id_for_key;
    int thr_id_for_switch;
    int thr_id_for_time;
    thr_id_for_key = pthread_create(&p_thread[0],NULL,receive_from_key,NULL); 
    if(thr_id_for_key < 0){
        perror("thread create error :");
        return;
    }

    thr_id_for_switch = pthread_create(&p_thread[1],NULL,receive_from_switch,NULL); 
    if(thr_id_for_switch < 0){
        perror("thread create error :");
        return;
    }
        
    thr_id_for_time = pthread_create(&p_thread[2],NULL,send_time,NULL); 
    if(thr_id_for_time < 0){
        perror("thread create error :");
        return;
    }

    pthread_join(p_thread[0], (void **)NULL); 
    pthread_join(p_thread[1], (void **)NULL);
    pthread_join(p_thread[2], (void **)NULL);

    return;
}
void *receive_from_key(){ // key¿ ¿¿ ¿¿
    key_queue rcv_key;
    rcv_key.msgtype = 1;
    long msgtype = 1;

    while(1)
    {
        if (msgrcv( key_id, (void *)&rcv_key, sizeof(key_queue)-sizeof(long), msgtype, 0) == -1)
        {
            perror("msgrcv from key error");
            return;

        }
        if(rcv_key.code == BACK){ // back key push
            clear_func(); // clear fnd, led, lcd , dot
            if(msgrcv( key_id, (void *)&rcv_key, sizeof(key_queue)-sizeof(long), msgtype, 0) == -1){
                perror("msgrcv from key error");
                return;
            }
            terminate = 1; // process termination flag
            break;
        }
        else if(rcv_key.code == MODE_PLUS && rcv_key.value == KEY_PRESS){ // mode plus
            mode = (mode+1)%5; 
            mode_initialize(mode); 
        }
        else if(rcv_key.code == MODE_MINUS && rcv_key.value == KEY_PRESS){ // mode minus
            mode = (mode-1)%5;
            if(mode==-1) mode = 4;
            mode_initialize(mode);
        }

    }
    pthread_exit(NULL);
    return;
}
void *receive_from_switch(){
    int i;
    int cnt;
    switch_queue rcv_switch;
    fnd send_fnd;
    rcv_switch.msgtype = 2;
    send_fnd.msgtype = 3;
    long msgtype = 2;
    while(1){
        if(msgrcv( key_id, (void *)&rcv_switch, sizeof(switch_queue) - sizeof(long), msgtype, MSG_NOERROR) == -1){
            perror("msgrcv from switch error");
            return;
        }
        cnt = 0;
        for(i = 0 ; i < MAX_BUTTON ; i++){ // check which button is pushed
            if(rcv_switch.push_sw_buff[i]){
                cnt++;
                check = i;
            }
        }
    
        if(cnt){ // check how many buttons are pressed
            fp[mode](check,rcv_switch.push_sw_buff); 
            pre = check;
        }
        else pre = -1;
        
        
        if(terminate) break; 
    }
    pthread_exit(NULL);
}
void *send_time(){
    fnd time;
    int i,cur_hour,cur_min;
    int flag = 0;
    time.msgtype = 3;
    while(1){
        if(terminate) pthread_exit(NULL);
        if(mode!=0) continue; // clock mode
        if(time_mode) continue; // operate only in time mode
        for(i = 0 ; i < 60000; i++){ // 1 min sleep
            usleep(1000);
            if(terminate){
                pthread_exit(NULL);
            }
			if (time_mode) continue;
        }
        cur_hour = last_clock[0] * 10 + last_clock[1];
        cur_min = last_clock[2] * 10 + last_clock[3];
        if(time_mode == 0){ // min is increased only in time mode
            if(++cur_min>=60){
                    cur_hour = (cur_hour+1)%12;
                    cur_min = 0;
                }
            }
		    /* time renew */
            time.data[0] = cur_hour/10;
            time.data[1] = cur_hour%10;
            time.data[2] = cur_min/10;
            time.data[3] = cur_min%10;
            for(i = 0 ; i < 4 ; i++)
                last_clock[i] = time.data[i]; // time renew
        if(time_mode == 0 && mode == 0) 
           msgsnd(key_id, (void *)&time, sizeof(fnd) - sizeof(long), IPC_NOWAIT);
    }
     
}
