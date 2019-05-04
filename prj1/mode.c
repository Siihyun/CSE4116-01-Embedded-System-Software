#include "20141544.h"
#include "./fpga_dot_font.h"
int temp;
void init_main(){
    int i,j;
    key_id = msgget((key_t)1234, IPC_CREAT|0666); // msg queue를 위한 key id 설정

	/* 각 device별로 msg type 설정*/
    send_fnd.msgtype = 3;
    send_led.msgtype = 4;
    send_lcd.msgtype = 5;
    send_dot.msgtype = 6;
    send_dot.status = 1;
	/* 각 device별로 msg type 설정*/

	/* input 중복 check를 위한 변수 */
    previous_input = -1; 
    previous_idx = -1;
	/* input 중복 check를 위한 변수 */

    button_cnt = 0;
    x = 0;
    y = 0;
    set_last_clock();

    for(i = 0 ; i < 10 ; i++) // dot matrix 초기화
        memset(send_dot.dotmatrix[i],0,sizeof(send_dot.dotmatrix[i]));
    memset(send_lcd.string,' ',33);
    memset(str,' ',33);
    time_mode = 0;
    num_mode = 0;
    mode = 0;
    check = -1;
    pre = -1;
    stage = 0;
    fp[0] = mode1;
    fp[1] = mode2;
    fp[2] = mode3;
    fp[3] = mode4;
    fp[4] = mode5;
    mode2_flag = 64;
    for(i = 0 ; i < 4 ; i++)
        last_num[i] = 0;
    init_text();
    return;
}
void init_text(){
    number[0] = '0'; number[1] = '1'; number[2] = '2';
    number[3] = '3'; number[4] = '4'; number[5] = '5';
    number[6] = '6'; number[7] = '7'; number[8] = '8';
    number[9] = '9';
    text[0][0] = '.'; text[0][1] = 'Q'; text[0][2] = 'Z';
    text[1][0] = 'A'; text[1][1] = 'B'; text[1][2] = 'C';
    text[2][0] = 'D'; text[2][1] = 'E'; text[2][2] = 'F';
    text[3][0] = 'G'; text[3][1] = 'H'; text[3][2] = 'I';
    text[4][0] = 'J'; text[4][1] = 'K'; text[4][2] = 'L';
    text[5][0] = 'M'; text[5][1] = 'N'; text[5][2] = 'O';
    text[6][0] = 'P'; text[6][1] = 'R'; text[6][2] = 'S';
    text[7][0] = 'T'; text[7][1] = 'U'; text[7][2] = 'V';
    text[8][0] = 'W'; text[8][1] = 'X'; text[8][2] = 'Y'; 
}
void open_device(){ //device open function
    dev[FND] = open(FND_DEVICE, O_RDWR);
    dev[LED] = open(LED_DEVICE, O_RDWR | O_SYNC);
    dev[LCD] = open(LCD_DEVICE, O_WRONLY);
    dev[DOT] = open(DOT_DEVICE, O_WRONLY);

    if(dev[FND]<0){ // fnd open
        printf("DEVICE OPEN ERROR : %s\n",FND_DEVICE);
        return;
    }
    if(dev[LED]<0){ // led open
        printf("DEVICE OPEN ERROR : %s\n",LED_DEVICE);
        return;
    }
    if(dev[LCD]<0){ // lcd open
        printf("DEVICE OPEN ERROR : %s\n",LCD_DEVICE);
        return;
    }
    if(dev[DOT]<0){ // dot open
        printf("DEVICE OPEN ERROR : %s\n",LCD_DEVICE);
        return;
    }
}
void init_output(){ // initiate output process
    open_device(); // open fnd,led,lcd,dot
    init_led(); // initiate led
    print_cur_time(); // print cur time at the board
    /* initiate setting */
    send_led.status = 0;
    send_led.data = 128;
    send_dot.status = 3;
    memset(send_dot.matrix,0,10);
    memset(send_lcd.string,' ',33);
    msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT);
    msgsnd(key_id, (void *)&send_dot, sizeof(dot)-sizeof(long), IPC_NOWAIT);
    msgsnd(key_id, (void *)&send_lcd, sizeof(lcd)-sizeof(long), IPC_NOWAIT);
    /* initiate setting */
}
struct tm* get_board_time(){ // get current board time to timer
    time_t timer;
    timer = time(NULL);
    return localtime(&timer);
}
void print_cur_time(){ // print current time on the board
   // msgctl(key_id, IPC_RMID, NULL);
    int i;    
    struct tm* cur_time = get_board_time(); // get current time at cur_time
    unsigned char hour = cur_time->tm_hour; 
    unsigned char min = cur_time->tm_min;
    send_fnd.data[0] = hour/10; // save cur time at send_fnd data
    send_fnd.data[1] = hour%10;
    send_fnd.data[2] = min/10;
    send_fnd.data[3] = min%10;
    for(i = 0 ; i < 4 ; i++) // save cur time at last_clock to print it on the board
        last_clock[i] = send_fnd.data[i];

    if(msgsnd(key_id, (void *)&send_fnd, sizeof(fnd) - sizeof(long), IPC_NOWAIT)==-1){ // if there is msg send error
        perror("msgsnd for output error"); return;
    }
}
void print_last_time(){ // this is a function that print last_time on the fnd. it is used to print time after change.
    int i;
    for(i = 0 ; i < 4 ; i++)
        send_fnd.data[i] = last_clock[i];
    if(msgsnd(key_id, (void *)&send_fnd, sizeof(fnd) - sizeof(long), IPC_NOWAIT)==-1){
        perror("msgsnd for output error"); return;
    }
}
void mode_initialize(int mode){ // this is a function for intialize depends on the mode that has been changed.
    int i;
    button_cnt = 0;
    
    if(mode==0){ // print current time on the board.
        print_cur_time();
        send_led.status = 0;
        send_led.data = 128;
        send_dot.status = 3;
        memset(send_dot.matrix,0,10);
        memset(send_lcd.string,' ',33);
        msgsnd(key_id, (void *)&send_lcd, sizeof(lcd)-sizeof(long), IPC_NOWAIT);
        msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT);
        msgsnd(key_id, (void *)&send_dot, sizeof(dot)-sizeof(long), IPC_NOWAIT); 
    }
    else if(mode==1){ // set fnd : 0000 , led : 01000000, dot : 0
        send_led.status = 0;
        send_led.data = 64;
        send_dot.status = 3;
        memset(send_dot.matrix,0,10);
        memset(send_lcd.string,' ',33);
        for(i = 0 ; i < 4 ; i++)
            send_fnd.data[i] = 0;
        msgsnd(key_id, (void *)&send_lcd, sizeof(lcd)-sizeof(long), IPC_NOWAIT);
        msgsnd(key_id, (void *)&send_fnd, sizeof(fnd)-sizeof(long), IPC_NOWAIT); // fnd 0000
        msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT); // led 2번
        msgsnd(key_id, (void *)&send_dot, sizeof(dot)-sizeof(long), IPC_NOWAIT); // dot 0
    }
    else if(mode==2){ // set fnd : 0 , led : 00000000, dot : 1
        for(i = 0 ; i < 4 ; i++)
            send_fnd.data[i] = 0;
        send_led.status = 0;
        send_led.data =0;
        send_dot.status = 3;
        num_mode = 0;
        strcpy(send_dot.matrix,fpga_number[0]);
        memset(send_lcd.string,' ',33);
        memset(str,' ',33);
        len = 0;
        msgsnd(key_id, (void *)&send_lcd, sizeof(lcd)-sizeof(long), IPC_NOWAIT);
        msgsnd(key_id, (void *)&send_dot, sizeof(dot)-sizeof(long), IPC_NOWAIT); // dot 1나오게 함
        msgsnd(key_id, (void *)&send_fnd, sizeof(fnd)-sizeof(long), IPC_NOWAIT); // fnd 초기화
        msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT); // led 초기화
    }
    else if(mode==3){ // set fnd , led, dot to initial state
        memset(send_dot.matrix,0,10);
        for(i = 0 ; i < 10 ; i++)
            memset(send_dot.dotmatrix[i],0,sizeof(send_dot.dotmatrix[i]));
        for(i = 0 ; i < 4 ; i++)
            send_fnd.data[i] = 0;
        send_led.status = 0;
        send_led.data =0;
        send_dot.status = 1;
        send_dot.x = 0; send_dot.y = 0;
        x = 0; y = 0;
        memset(send_lcd.string,' ',33);
        memset(send_dot.matrix,0,10);
        msgsnd(key_id, (void *)&send_lcd, sizeof(lcd)-sizeof(long), IPC_NOWAIT);
        msgsnd(key_id, (void *)&send_fnd, sizeof(fnd)-sizeof(long), IPC_NOWAIT); // fnd 초기화
        msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT); // led 초기화
        msgsnd(key_id, (void *)&send_dot, sizeof(dot)-sizeof(long), IPC_NOWAIT); // dot 초기화
    }
    else{ // set fnd, led, dot to initial state
        for(i = 0 ; i < 4 ; i++)
            send_fnd.data[i] = 0;
        send_led.status = 0;
        send_led.data = 1;
        send_dot.status = 1;
        send_dot.x = 0; send_dot.y = 0;
        x = 0; y = 0;
        memset(send_lcd.string,' ',33);
        memset(send_dot.matrix,0,10);
        msgsnd(key_id, (void *)&send_lcd, sizeof(lcd)-sizeof(long), IPC_NOWAIT); // lcd 초기화
        msgsnd(key_id, (void *)&send_fnd, sizeof(fnd)-sizeof(long), IPC_NOWAIT); // fnd 초기화
        msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT); // led 초기화
        stage = 0;
        for(i = 0 ; i < 10 ; i++)
            memcpy(send_dot.dotmatrix[i],map[stage][i],7);
        x = 9 ; y = 5;
        send_dot.x = x;
        send_dot.y = y;
        msgsnd(key_id, (void *)&send_dot, sizeof(dot)-sizeof(long), IPC_NOWAIT); // dot 초기화
    }

}
void set_last_clock(){ // set last clock when the program is on. it is used to initialize last clock
    struct tm* cur_time = get_board_time();
    unsigned char hour = cur_time->tm_hour;
    unsigned char min = cur_time->tm_min;
    last_clock[0] = hour/10;
    last_clock[1] = hour%10;
    last_clock[2] = min/10;
    last_clock[3] = min%10;
}
void mode1(int n,unsigned char data[]){ // mode 1 : clock mode
    if(pre == n) return;
    n++;
    int i;
    unsigned char hour,min;

    if(n==1){ // 첫번째 버튼이 눌리면 시간 수정 모드에서 기본  모드로 전환 , 혹은 기본 모드에서 시간 수정 모드로 전환
        if(time_mode==0){ 
            time_mode = 1;
            send_led.status = 1;
            msgsnd(key_id, (void *)&send_led, sizeof(led) - sizeof(long), IPC_NOWAIT);
            print_last_time();
            return;
        }
        else{
            time_mode = 0;
            send_led.status = 0;
            send_led.data = 128;
            for(i = 0 ; i < 4 ; i++)
                send_fnd.data[i] = last_clock[i];
            msgsnd(key_id, (void *)&send_led, sizeof(led) - sizeof(long), IPC_NOWAIT);
            msgsnd(key_id, (void *)&send_fnd, sizeof(fnd) - sizeof(long), IPC_NOWAIT);
            return;
        }
    }   
    else if(n==2){ // 2면 현재 시간 출력
        if(time_mode)
            print_cur_time();
        else
            print_last_time();
        return;
    }
    else{
        if(time_mode==0){ // 시간 수정 모드가 아니면
            print_last_time(); // 현재 시간 출력
            return;
        }
        /*for time renew */
        hour = last_clock[0] * 10 + last_clock[1];
        min = last_clock[2] * 10 + last_clock[3];
        if(n==3) hour = (hour+1) % 12;
        else if(n==4){
            if(++min>=60){
                hour = (hour+1) % 12;
                min = 0;
            }
        }
        send_fnd.data[0] = hour/10;
        send_fnd.data[1] = hour%10;
        send_fnd.data[2] = min/10;
        send_fnd.data[3] = min%10;
        for(i = 0 ; i < 4 ; i++)
            last_clock[i] = send_fnd.data[i];
        /* time renew end */
    }
    if(msgsnd(key_id, (void *)&send_fnd, sizeof(fnd) - sizeof(long), IPC_NOWAIT)==-1){
        perror("msgsnd for output error"); return;
    }
    return;
}
void clear_func(){

    int i;
    send_led.status = 0;
    send_led.data = 0;
    send_dot.status = 3;
    memset(send_dot.matrix,0,10);
    memset(send_lcd.string,' ',33);
    for(i = 0 ; i < 4 ; i++)
        send_fnd.data[i] = 0;

    msgsnd(key_id, (void *)&send_lcd, sizeof(lcd)-sizeof(long), IPC_NOWAIT); // lcd 초기화
    msgsnd(key_id, (void *)&send_fnd, sizeof(fnd)-sizeof(long), IPC_NOWAIT); // fnd 초기화
    msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT); // led 초기화
    msgsnd(key_id, (void *)&send_dot, sizeof(dot)-sizeof(long), IPC_NOWAIT); // dot 초기화
}
void change_digit(){ // 진법을 바꾸어 주는 함수
    int i;
    int mul = 1;
    int num = 0, next_digit;
    char arr[100];
    memset(arr,0,sizeof(arr));
    for(i = 3 ; i > 0 ; i--){ // 10진수로 변환
        num += last_num[i] * mul;
        mul*=digit;
    }
    if(digit==10) next_digit = 8; // 다음 진법 확인
    else if(digit==8) next_digit = 4;
    else if(digit==4) next_digit = 2;
    else next_digit = 10;
    i = 3;

    while(i){ // 새로운 진법으로 변환을 해줌
        last_num[i] = num % next_digit;
        num /= next_digit;
        i--;
    }

    return;
}
void mode2(int n,unsigned char data[]){
    if(n==pre) return;
    int i;
    n++;
    if(n==1){
        change_digit();
        mode2_flag = mode2_flag/2;
        if(mode2_flag==8) mode2_flag = 128;
        send_led.status = 0;
        send_led.data = mode2_flag;
        msgsnd(key_id, (void *)&send_led, sizeof(led) - sizeof(long), IPC_NOWAIT);
    }

    if(mode2_flag == 64) digit = 10;
    else if(mode2_flag == 32) digit = 8;
    else if(mode2_flag == 16) digit = 4;
    else digit = 2;

    
    if(n==2){ // 100의자리 증가
        if(++last_num[1] >= digit)
            last_num[1] = 0;
    }

    else if(n==3){ // 10의자리 증가
        if(++last_num[2]>=digit){
            last_num[2]%=digit;
            if(++last_num[1]>=digit)
                last_num[1] = 0;
        }
    }

    else if(n==4){ // 1의자리 증가
        if(++last_num[3]>=digit){
            last_num[3] %= digit;
            if(++last_num[2]>=digit){
                last_num[2] %= digit;
                if(++last_num[1]>=digit) 
                    last_num[1] = 0;
            }
        }
    }
    for(i = 1 ; i < 4 ; i++)
        send_fnd.data[i] = last_num[i];
    msgsnd(key_id, (void *)&send_fnd, sizeof(fnd) - sizeof(long), IPC_NOWAIT);
    return;
}
void mode3(int n,unsigned char data[]){ 
    if(n==pre) return;
    button_cnt++;
    int count = button_cnt;
    int i,cnt=0,button_num=0;
    for(i = 3 ; i >= 0 ; i--){
        send_fnd.data[i] = count%10;
        count/=10;
    }
    msgsnd(key_id, (void *)&send_fnd, sizeof(fnd) - sizeof(long), IPC_NOWAIT);

    for(i = 0 ; i < MAX_BUTTON ; i++){
        if(data[i]){
            cnt++;
            button_num = i;
        }
    }

    if(len == 32){ // 길이가 꽉 찼을때
        if(button_num != previous_input){ // 전의 입력과 다르면
            len--;
            for(i = 1 ; i < 32 ; i++) // 한칸씩 밀어 줍니다.
                str[i-1] = str[i];
        }

    }


    
    if(cnt==1){ // 문자 하나만 입력
        if(num_mode){
            str[len++] = number[button_num+1];
            previous_input = -1;
        }
        else{
            if(button_num == previous_input){ // 같은 입력
                previous_idx = (previous_idx+1)%3;
                str[len-1] = text[button_num][previous_idx]; 
            }   
            else{ // 전이랑 다른 입력
                str[len++] = text[button_num][0];
                previous_input = button_num;
                previous_idx = 0;
            }
        }
    }
    else if(cnt == 2){
        if(data[1] && data[2]){ // 문자열 초기화
            memset(str,' ',33);
            len = 0;
        }
        
        else if(data[4] && data[5] && n == 5){ // 한글/숫자 전환
            if(num_mode==1){
                num_mode = 0;
                strcpy(send_dot.matrix,fpga_number[0]);
                send_dot.status = 3;
                msgsnd(key_id, (void *)&send_dot, sizeof(dot) - sizeof(long), IPC_NOWAIT);
            }
            else{
                num_mode = 1;
                strcpy(send_dot.matrix,fpga_number[1]);
                send_dot.status = 3;
                msgsnd(key_id, (void *)&send_dot, sizeof(dot) - sizeof(long), IPC_NOWAIT);
            }
        }
        else if(data[7] && data[8] && n == 8){ // 공백 입력
            str[len++] = ' ';
            previous_idx = 0;
            previous_input = -1;
        }
    }
    
    memcpy(send_lcd.string,str,sizeof(str));
    msgsnd(key_id, (void *)&send_lcd, sizeof(lcd) - sizeof(long), IPC_NOWAIT);

    return;
}
void mode4(int n,unsigned char data[]){
    if(n==pre) return;    
    button_cnt++;
    int count = button_cnt;
    int i,j,num,mul;
    for(i = 3 ; i >= 0 ; i--){
        send_fnd.data[i] = count%10;
        count/=10;
    }
    msgsnd(key_id, (void *)&send_fnd, sizeof(fnd) - sizeof(long), IPC_NOWAIT);
    n++;
    if(n==2){ // 버튼에 따라 상 하 좌 우 움직임
        if(x>0) x--;
    }
    else if(n==4){
        if(y>0) y--;
    }
    else if(n==6){
        if(y<6) y++;
    }
    else if(n==8){
        if(x<9) x++;
    }
    else if(n==1){ // 초기화
        for(i = 0 ; i < 10; i++)    //reset
            memset(send_dot.dotmatrix[i],0,7);
        x = 0;
        y = 0;
        send_dot.status = 1;
    }
    else if(n==3){ // 커서 초기화
        if(send_dot.status) send_dot.status = 0;
        else send_dot.status = 1;
    }
    else if(n==7){ // 초기화
        for(i = 0 ; i < 10 ; i++) //clear
            memset(send_dot.dotmatrix[i],0,7);
    }
    else if(n==9){ // 반전
        for(i = 0 ; i < 10 ; i++){
            for(j = 0 ; j < 7 ; j++){
                if(send_dot.dotmatrix[i][j]) send_dot.dotmatrix[i][j] = 0;
                else send_dot.dotmatrix[i][j] = 1;
            }
        }
    }
    else if(n==5){ // 선택/취소
        if(send_dot.dotmatrix[x][y]) send_dot.dotmatrix[x][y] = 0;
        else send_dot.dotmatrix[x][y] = 1; //select
    }

    send_dot.x = x;
    send_dot.y = y;
    msgsnd(key_id, (void *)&send_dot, sizeof(dot) - sizeof(long), IPC_NOWAIT);
    
    return;
}
void mode5(int n,unsigned char data[]){
    if(n==pre) return;   
    n++;
    int tmp = 1;
    int i;

    if(n==7){ // stage change
        stage++;
        stage = stage%5;
        int tmp = 1;
        for(i = 0 ; i <= stage; i++) // stage에 따른 led 상태 변환
            tmp*=2;
        send_led.status = 0;
        send_led.data = tmp-1;
        msgsnd(key_id, (void *)&send_led, sizeof(led)-sizeof(long), IPC_NOWAIT); //led setting
        for(i = 0 ; i < 10 ; i++)
            memcpy(send_dot.dotmatrix[i],map[stage][i],7);
        x = 9; y = 5;
        send_dot.x = 9;
        send_dot.y = 5;
        msgsnd(key_id, (void *)&send_dot, sizeof(dot) - sizeof(long), IPC_NOWAIT);
        return;
    }
    int savex = x; // 전 좌표 저장
    int savey = y;
    if(n==2){ // 상,하,좌,우 움직임
        if(x>0) x--;
    }
    else if(n==4){
        if(y>0) y--;
    }
    else if(n==6){
        if(y<6) y++;
    }
    else if(n==8){
        if(x<9) x++;
    }
    if(send_dot.dotmatrix[x][y] == 1){ // die;
        x = 9; y = 5;
        send_dot.x = x; send_dot.y = y;
    }
    if(x == 0){
        stage = (stage+1)%5;
        for(i = 0 ; i < 10 ; i++)
            memcpy(send_dot.dotmatrix[i],map[stage][i],7);
        x = 9; y = 5;
    }
    send_dot.x = x;
    send_dot.y = y;
    msgsnd(key_id, (void *)&send_dot, sizeof(dot) - sizeof(long), IPC_NOWAIT);
    return;
}

