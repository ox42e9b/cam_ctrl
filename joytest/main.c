#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/joystick.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

/* check this with jstest */
#define JOYDEV "/dev/input/js0"
#define TRANS_AXIS 0
#define   YAW_AXIS 3
#define PITCH_AXIS 4


enum { GENERAL, TRANS, YAW, PITCH};

enum { RESET };

enum {
    TARGET, REL_TARGET, SPEED, REL_SPEED, 
    PAUSE, UNPAUSE, DEBUG_PRINT, 
};

struct _frame {
    unsigned type   : 2;
    unsigned action : 6;
    int32_t value;
} frame;

struct _stepper {
    long dv;  /* фрагментация изменения скорости */
    long min, max;
    long max_speed;
    float scale;  /* (js.val1 - js.val0) * scale - приращение скорости */
} trans = {.dv = 3200,  .min = 0,        .max = 46000,    .scale = 23000 / 32767.0}, 
  yaw   = {.dv = 228,   .min = LONG_MIN, .max = LONG_MAX, .scale =  2460 / 32767.0}, 
  pitch = {.dv = 228,   .min = 0,        .max = 3280,     .scale =  2460 / 32767.0};


struct _stepper *trans_p = &trans;
struct _stepper *yaw_p   = &yaw;
struct _stepper *pitch_p = &pitch;


int 
main(int argc, char* argv[])
{
    struct sockaddr_rc addr = {0};
    char str_addr[18] = "98:D3:71:F9:84:96";
    int sock, status;
    int joy_fd, *axis = NULL, num_of_axis = 0, num_of_buttons = 0, x;
    uint32_t prev_time;
    char *button = NULL, name_of_joystick[80];
    struct js_event js_ev;
    struct _stepper *m;
    bool reset_sent = false;

    if (2 == argc)
        strncpy(str_addr, argv[1], 18);
    
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    
    status = str2ba(str_addr, &addr.rc_bdaddr);
    if (0 != status) 
        perror("str2ba");

    status = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    if (status < 0) { 
        perror("bluetooth"); 
        return -1;
    }
    
    joy_fd = open(JOYDEV, O_RDONLY); 
   
    if (-1 == joy_fd) {
        perror("open(\""JOYDEV"\")");
        return -1;
    }

    ioctl(joy_fd, JSIOCGAXES, &num_of_axis);
    ioctl(joy_fd, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);

    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );

    printf("[%s]\n", name_of_joystick);
   
    prev_time = 0;
    while (1) {
	read(joy_fd, &js_ev, sizeof(struct js_event));
        if (js_ev.time - prev_time < 2)
            continue;
        prev_time = js_ev.time;

        switch (js_ev.type & ~JS_EVENT_INIT) {
	case JS_EVENT_AXIS:
            if (!reset_sent)
                continue;
	    switch (js_ev.number) {
                case TRANS_AXIS:
                    m = trans_p;
                    frame.type = TRANS;
                    break;
                case YAW_AXIS:
                    m = yaw_p;
                    frame.type = YAW;
                    break;
                case PITCH_AXIS:
                    m = pitch_p;
                    frame.type = PITCH;
                    break;
            }

            /* остановить, если необходимо */
            if (js_ev.value == 0 && axis[js_ev.number] != 0) {
                frame.action = SPEED;
                frame.value = 0;
                write(sock, &frame, sizeof(struct _frame));
                axis[js_ev.number] = 0;
                puts("\033[34mstop\033[0m");
            }
            /* изменить направление если необходимо */
            else if (((js_ev.value & (1 << 31)) != (axis[js_ev.number] & (1 << 31))) 
                     || (0 != js_ev.value && 0 == axis[js_ev.number])) {
                frame.action = TARGET;
                frame.value = (js_ev.value < 0) ? m->min : m->max;
                write(sock, &frame, sizeof(struct _frame)); 
                axis[js_ev.number] = js_ev.value;
                puts("\033[32mnew target\033[0m");
            }
            frame.action = REL_SPEED;
            frame.value = round((js_ev.value - axis[js_ev.number]) * m->scale); 
            if (abs(frame.value) > m->dv) { 
                write(sock, &frame, sizeof(struct _frame));
                axis[js_ev.number] = js_ev.value;
                printf("\033[35mrel speed: \033[33m%d\033[0m\n", frame.value);
            }
	    break;
	case JS_EVENT_BUTTON:
            if (1 == js_ev.number && js_ev.value == 1) { 
                frame.type = GENERAL;
                frame.action = RESET;
                write(sock, &frame, sizeof(struct _frame));
                printf("\033[36mreset\033[0m\n");
                reset_sent = true;
            }
            button[js_ev.number] = js_ev.value;
	    break;
	}
    }
    close(joy_fd);
    close(sock); 
    return 0;
}
