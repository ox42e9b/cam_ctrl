#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define NEW_TARGET(a, b) ((a > 0 && b < 0) || (a < 0 && b > 0) || (a == 0 && b != 0) || (a != 0 && b == 0))

/* check this with jstest */
#define JOYDEV "/dev/input/js0"
#define TRANS_AXIS 0
#define   YAW_AXIS 3
#define PITCH_AXIS 4


struct _stepper {
    int min, max;
    int fragmentation;  /* чувствительность [0; 32767] (увеличение нагружает dtm) */ 
}


int
send_pos(char pos) {

}


int 
main(int argc, char* argv[])
{
    struct sockaddr_rc addr = {0};
    char str_addr[18] = "88:F8:72:35:58:6F";
    int sock, status;
    int joy_fd, *axis = NULL, num_of_axis = 0, num_of_buttons = 0, x;
    char *button = NULL, name_of_joystick[80];
    struct js_event js_ev;

    /*if (2 == argc)
        strncpy(str_addr, argv[1], 18);
    
    sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    
    status = str2ba(str_addr, &addr.rc_bdaddr);
    if (0 != status) 
        perror("str2ba");

    status = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    if (0 == status)
        status = write(sock, "hello!", 6);

    if (status < 0) 
        perror("error"); 

    close(sock);*/
    
    joy_fd = open(JOYDEV, O_RDONLY); 
   
    if (-1 == joy_fd)
        perror("open(JOYDEV)");

    ioctl(joy_fd, JSIOCGAXES, &num_of_axis);
    ioctl(joy_fd, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);

    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );

    printf("[%s]\n", name_of_joystick);
    
    while (1) {
	read(joy_fd, &js_ev, sizeof(struct js_event));
		
        switch (js_ev.type & ~JS_EVENT_INIT) {
	case JS_EVENT_AXIS:
	    switch (js_ev.number) {
                case TRANS_AXIS:
                    if (NEW_TARGET(js_ev.value, axis[js_ev.number])) {
                        if (js_ev.value == )    
                    }
                    break;
                case YAW_AXIS:
                    puts("yaw");
                    break;
                case PITCH_AXIS:
                    puts("pitch");
                    break;
            }
            axis[js_ev.number] = js_ev.value;
	    break;
	case JS_EVENT_BUTTON:
	    button[js_ev.number] = js_ev.value;
	    break;
	}
    }
    close(joy_fd);
    close(sock); 
    return 0;
}
