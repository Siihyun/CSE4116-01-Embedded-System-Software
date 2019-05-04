#ifndef __20141544_HEADER__
#define __20141544_HEADER__
#include <time.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>

int pre; // 전에 입력된 버튼을 저장하여 한 버튼을 꾹 누르고 있었을때 fnd count가 중복으로 올라가는것을 방지
int check; // 전에 입력된 버튼을 저장하여 한 버튼을 꾹 누르고 있었을때 fnd count가 중복으로 올라가는 것을 방지
int terminate; // back button이 눌렸을때 프로그램 종료를 위한 flag
unsigned char quit; // switch에서 버튼 입력 종료를 위한 flag

/* mode 1 */
int time_mode; // mode 1에서 시간 변경 모드인지 아닌지를 확인하기 위한 flag
unsigned char last_clock[4]; //현재 fnd 시간 저장
/* mode 1 end */

/* mode 2 */
unsigned char mode2_flag; 
int digit; // 현재 진법이 몇진법인지 저장
unsigned char last_num[4]; // 현재 fnd의 number저장
/* mode 2 end */

/* mode 3*/
int previous_input;// mode 3에서 전이랑 같은 input이 들어 왔을 시 문자를 바꿔주기 위한 변수
int previous_idx; // mode 3에서 전이랑 같은 input이 들어 왔을 시 문자를 바꿔주기 위한 변수
char text[9][3]; // 문자판을 저장하기 위한 배열 
char str[33]; // LCD 출력을 위한 string 
char number[10]; // 0~9까지 숫자를 담고있응 배열
int button_cnt; // button이 몇개 눌렸는지를 판단
int num_mode; // 숫자 모드인지 알파벳 모드인지 판단
int len; // string 길이 담고있는 변수
/*mode 3 end*/

/* mode 4 */
int x; // 현재 dot matrix의 x좌표
int y; // 현재 dot matrix의 y좌표
/* mode 4 end */

/* mode 5 */
int stage; // stage를 담고 있음
/* mode 5 end */
int life;
/* switch  macro */
#define KEY_RELEASE 0
#define KEY_PRESS 1
#define BUFF_SIZE 64
#define MAX_LCD_BUFF 9
#define MAX_BUTTON 9
#define BACK 158
#define MODE_PLUS 115
#define MODE_MINUS 114
/* switch key  macro */

/* open define macro  */
#define FND 0
#define LED 1
#define LCD 2
#define DOT 3
#define FND_DEVICE "/dev/fpga_fnd"
#define LED_DEVICE "/dev/mem"
#define LCD_DEVICE "/dev/fpga_text_lcd"
#define DOT_DEVICE "/dev/fpga_dot"
/* open  define macro  */

/* LED mmap macro */
#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16
unsigned long *fpga_addr;
unsigned char *led_addr;
/* LED mmap macro*/

/* msg queueing  key, switch, FND, LED, LCD, DOT */

typedef struct _key{
    long msgtype;
    int type;
    int value;
    int code;
    int seq;
}key_queue;

typedef struct _swtich{
    long msgtype;
    unsigned char push_sw_buff[MAX_BUTTON];
    int button_num;
}switch_queue;

typedef struct _FND{
    long msgtype;
    unsigned char data[4];
}fnd;

typedef struct _LED{
    long msgtype;
    unsigned char data;
    int status;
}led;

typedef struct _LCD{
    long msgtype;
    char string[33];
}lcd;

typedef struct _DOT{
    long msgtype;
    char matrix[10];
    char dotmatrix[10][7];
    int status;
    int x;
    int y;
}dot;

key_t key_id;
fnd send_fnd;
led send_led;
lcd send_lcd;
dot send_dot;
dot rcvdot;
int dev[5];

/* msg queueing */


/* msg  */
void *receive_from_key(); // key를 받는 함수
void *receive_from_switch(); // switch를 받는 함수
void *rcv_msg_for_led(); // led를 받는 함수
void *rcv_msg_for_fnd(); // fnd를 받는 함수
void *rcv_msg_for_lcd(); // lcd를 받는 함수
void *rcv_msg_for_dot(); // dot를 받는 함수
void *key_msg(); // key를 전달하는 함수
void *switch_msg(); // switch를 전달하는 함수
void *send_time(); // time을 보내는 함수
void send_msg();
void user_signal1(int sig);
void listen(); // main에서 input으로부터의 msg를 받는 함수
/* msg */


/* mode */
void(*fp[5])(int, unsigned char[]); // mode function pointer
int mode; // 
void mode1(int n,unsigned char[]);
void mode2(int n,unsigned char[]);
void mode3(int n,unsigned char[]);
void mode4(int n,unsigned char[]);
void mode5(int n,unsigned char[]);
/* mode별로 함수를 나누어 구현하였습니다  */

/* 초기화 함수들 입니다  */
void init_main();
void init_output();
void init_input();
void init_led();
void init_text();
void mode_initialize(int mode);
/* 초기화 함수들 입니다  */


/*시간 관련 함수들 입니다 */
struct tm* get_board_time();
void print_cur_time();
void print_last_time();
void set_last_clock();
/*시간 관련 함수들 입니다  */

void led_start(unsigned char data); // data로 넘겨받아 led를 출력합니다
void change_digit(); // 진수를 변환해줍니다
void calculate_matrix(int sec); // dot matrix를 출력하기위해 배열을 계산해줍니다
void clear_func(); // back button 눌러 종료시 화면 초기화를 위해 사용합니다

#endif
