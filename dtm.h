#ifndef DTM_H
#define DTM_H

#include "rot.h"


enum {
    GENERAL = '0', TARGET = '0', STOP_MOVING   = '0',  
    TRANS   = '1', SPEED  = '1', RESUME_MOVING = '1',
    YAW     = '2', 
    PITCH   = '3', 
};

struct {
    uint8_t type;
    uint8_t action;
    uint16_t value;
} frame;


inline int8_t
process_dtm()
{
    if (Serial.available() >= 4) {
        Serial.readBytes((uint8_t*)&frame, sizeof(frame));

        switch (frame.type) {
        case GENERAL:
            switch (frame.action) {
            case STOP_MOVING:
                Serial.println("Stop moving recieved");
                break;
            case RESUME_MOVING:
                Serial.println("Resume moving received");
                break;
            }
            break;
        case TRANS:
            switch (frame.action) {
            case TARGET:
                Serial.print("Target recieved: ");
                Serial.println(frame.value);
                break;
            case SPEED:
                Serial.print("Speed recieved: ");
                Serial.println(frame.value);
                break;
            }
            break;
        case YAW:
            switch (frame.action) {
            case TARGET:
                Serial.print("Target recieved: ");
                Serial.println(frame.value);
                break;
            case SPEED:
                Serial.print("Speed recieved: ");
                Serial.println(frame.value);
                break;
            }
            break;
        case PITCH:
            switch (frame.action) {
            case TARGET:
                Serial.print("Target recieved: ");
                Serial.println(frame.value);
                break;
            case SPEED:
                Serial.print("Speed recieved: ");
                Serial.println(frame.value);
                break;
            }
            break;
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
