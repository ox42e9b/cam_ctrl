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

#define SIGN(x) ((x > 0) ? 1 : -1)


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
} __attribute__((packed)) frame = {0};

struct _stepper {
    int8_t type;
    int32_t min, max;
    uint16_t valve;
    double scale;
}       /*              левая граница     правая граница     клапан     мак. скорость v          */
trans = {.type = TRANS, .min = -23000,    .max = 23000,    .valve = 512, .scale =  8192 / 32767.0}, 
yaw   = {.type = YAW,   .min = -3 * 4720, .max = 3 * 4720, .valve = 64,  .scale =  1024 / 32767.0}, 
pitch = {.type = PITCH, .min = -1640,     .max = 1640,     .valve = 64,  .scale =  1024 / 32767.0};


struct _stepper *trans_p = &trans;
struct _stepper *yaw_p   = &yaw;
struct _stepper *pitch_p = &pitch;
int sock;


ssize_t
send_frame()
{
    return write(sock, &frame, sizeof(frame));
}

void
rot_set_target(struct _stepper* m, int32_t value)
{
    frame.type = m->type;
    frame.action = TARGET;
    frame.value = value;
    send_frame();
}

void
rot_pause(struct _stepper* m)
{
    frame.type = m->type;
    frame.action = PAUSE;
    send_frame();
}

void 
rot_unpause(struct _stepper* m)
{
    frame.type = m->type;
    frame.action = UNPAUSE;
    send_frame();
}

void 
rot_set_speed(struct _stepper *m, int32_t value)
{
    frame.type = m->type;
    frame.action = SPEED;
    frame.value = value;
    send_frame();
}

void 
rot_set_rel_speed(struct _stepper *m, int32_t value)
{
    frame.type = m->type;
    frame.action = REL_SPEED;
    frame.value = value;
    send_frame();
}

void
rot_centrate()
{   
    /* 1 : 4 максимальной скорости */
    rot_set_speed(trans_p, trans.scale * 32767 / 4);
    rot_set_speed(  yaw_p,   yaw.scale * 32767 / 4);
    rot_set_speed(pitch_p, pitch.scale * 32767 / 4);
    rot_set_target(trans_p, 0); 
    rot_set_target(  yaw_p, 0);
    rot_set_target(pitch_p, 0);
    rot_unpause(trans_p);
    rot_unpause(  yaw_p);
    rot_unpause(pitch_p);
}


int 
main(int argc, char* argv[])
{
    struct sockaddr_rc addr = {0};
    char str_addr[18] = "98:D3:71:F9:84:96";
    int status;
    int joy_fd, *axis = NULL, num_of_axis = 0, num_of_buttons = 0, x;
    int32_t temp;
    char *button = NULL, name_of_joystick[80];
    struct js_event js_ev;
    struct _stepper *m;  

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
    /*sock = open("/dev/null", O_WRONLY);
    if (-1 == sock) {
        perror("open /dev/null");
        return -1;
    } // - для теста без bluetooth */

    joy_fd = open(JOYDEV, O_RDONLY); 
   
    if (-1 == joy_fd) {
        perror("open(\""JOYDEV"\")");
        return -1;
    }

    ioctl(joy_fd, JSIOCGAXES, &num_of_axis);
    ioctl(joy_fd, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);

      axis = calloc( num_of_axis,    sizeof(int)  );
    button = calloc( num_of_buttons, sizeof(char) );

    printf("[%s]\n", name_of_joystick);
   
    while (1) {
	read(joy_fd, &js_ev, sizeof(struct js_event));

        switch (js_ev.type & ~JS_EVENT_INIT) {
	case JS_EVENT_AXIS:
	    switch (js_ev.number) {
                case TRANS_AXIS: m = trans_p; break;
                case YAW_AXIS:   m = yaw_p;   break;
                case PITCH_AXIS: m = pitch_p; break;
            }
            js_ev.value *= m->scale;
            /* остановить, если необходимо */
            if (0 == js_ev.value && 0 != axis[js_ev.number]) {
                rot_pause(m);
                axis[js_ev.number] = js_ev.value;
                puts("\033[34mstop\033[0m\n");
            }
            /* изменить направление если необходимо */
            else if ((SIGN(js_ev.value) != SIGN(axis[js_ev.number])) 
                     || (0 != js_ev.value && 0 == axis[js_ev.number])) {
                temp = (js_ev.value < 0) ? m->min : m->max;
                rot_set_target(m, temp);  
                if (0 != js_ev.value && 0 == axis[js_ev.number])
                    rot_unpause(m); 
                axis[js_ev.number] = js_ev.value;
                printf("\033[32mnew target: %d\033[0m\n", temp);
            } 
            temp = SIGN(js_ev.value) * (js_ev.value - axis[js_ev.number]);
            if (abs(temp) > m->valve) {  
                rot_set_rel_speed(m, temp);
                axis[js_ev.number] = js_ev.value;
                printf("\033[35mrel speed: \033[33m%d\033[0m\n", temp);
            }
	    break;
	case JS_EVENT_BUTTON:
            if (1 == js_ev.number && js_ev.value == 1) { 
                frame.type = GENERAL;
                frame.action = RESET;
                send_frame();  
                puts("\033[36mreset\033[0m\n");
            }
            else if (9 == js_ev.number && js_ev.value == 1) {
                rot_centrate();
                puts("\033[36mcentrate\033[0m\n");
            }
            button[js_ev.number] = js_ev.value;
	    break;
	}
    }
    close(joy_fd);
    close(sock); 
    return 0;
}
