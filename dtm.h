#ifndef DTM_H
#define DTM_H

#include "rot.h"


enum { GENERAL = 'g', TRANS = 't', YAW = 'y', PITCH = 'p' };

enum {
    TARGET='t', REL_TARGET='T', SPEED='s', REL_SPEED='S', 
    PAUSE='p',  UNPAUSE='u', DEBUG_PRINT='d', 
};


struct {
    uint8_t type;
    uint8_t action;
    char value[14];
} frame;


inline int8_t
process_dtm()
{
    static uint32_t value;
    static struct _motor *m;

    if (Serial.available() >= 2) {
        Serial.readBytesUntil('\n', (uint8_t*)&frame, sizeof(frame));

        m = NULL;
        if (frame.type == TRANS)
            m = trans_p;
        else if (frame.type == YAW)
            m = yaw_p;
        else if (frame.type == PITCH)
            m = pitch_p;

        if (NULL != m) {
            value = atoi(frame.value);
            switch (frame.action) {
            case TARGET:
                Serial.print("Target recieved: ");
                Serial.println(value);
                rot_set_target(m, value);
                break;
            case REL_TARGET:
                rot_set_target(m, m->pos + value);
                break;
            case SPEED:
                rot_set_speed(m, value);
                break;
            case REL_SPEED:
                rot_set_speed(m, m->speed + value);
            case PAUSE:
                m->moving = false; 
                break;
            case UNPAUSE:
                m->moving = true;
                break;
            case DEBUG_PRINT: 
                Serial.print((m == trans_p) ? "\n[Trans" : ((m == yaw_p) ? "\n[yaw" : "\n[pitch"));
                Serial.println(" motor]");
                Serial.print("position: ");
                Serial.println(m->pos);
                Serial.print("target: ");
                Serial.println(m->tgt);
                Serial.print("speed: ");
                Serial.println(m->speed);
                Serial.print("moving: ");
                Serial.println(m->moving);
                Serial.println();
                break;
            } 
        }  /* очищаем обработанный кадр */
        memset(&frame, 0, sizeof(frame));
    }

    return 0;
}


int8_t
init_dtm()
{
    Serial.begin(9600);

    return 0;
}

#endif
