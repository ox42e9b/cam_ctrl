#ifndef DTM_H
#define DTM_H

#include "rot.h"


enum { GENERAL, TRANS, YAW, PITCH};

enum {
    TARGET, REL_TARGET, SPEED, REL_SPEED, 
    PAUSE, UNPAUSE, DEBUG_PRINT, 
};

struct {
    unsigned type   : 2;
    unsigned action : 6;
    int32_t value;
} frame;


inline int8_t
process_dtm()
{
    static struct _stepper *m;

    if (Serial.available() >= 2) {
        Serial.readBytes((uint8_t*)&frame, sizeof(frame));

        m = NULL;
        if (frame.type == TRANS)
            m = trans_p;
        else if (frame.type == YAW)
            m = yaw_p;
        else if (frame.type == PITCH)
            m = pitch_p;

        if (NULL != m) {
            switch (frame.action) {
            case TARGET:
                rot_set_target(m, frame.value);
                break;
            case REL_TARGET:
                rot_set_target(m, m->pos + frame.value);
                break;
            case SPEED:
                rot_set_speed(m, frame.value);
                break;
            case REL_SPEED:
                rot_set_speed(m, m->speed + frame.value);
                break;
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
    Serial.begin(38400);

    return 0;
}

#endif
