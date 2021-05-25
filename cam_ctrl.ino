#include <Arduino.h>

#include "rot.h"
#include "dtm.h"

uint32_t time = 0;
uint32_t db_time = 0;


void 
debug_print()
{ 
    Serial.println("\n[Debug]");
    Serial.print("trans.pos = ");
    Serial.println(trans.pos);
    Serial.print("trans.tgt = ");
    Serial.println(trans.tgt);
    Serial.print("trans.speed = ");
    Serial.println(trans.speed);
    Serial.print("trans.moving = ");
    Serial.println(trans.moving);
    Serial.println();
}


void 
setup() {
    init_dtm();
    Serial.print("info: DTM initialized, sizeof(frame) = ");
    Serial.println(sizeof(frame));

    /*Serial.println("info: Initializing rotation system.");
    rot_init(); 
    Serial.println("info: Motors are in initial state.");
    
    ROT_SPEED(trans, 100);
    ROT_SPEED(yaw,   100);
    ROT_SPEED(pitch, 100);

    trans.tgt = 1024;
    trans.moving = true;*/
}

void 
loop() {
    /*time = micros();

    rot_tick(trans_p, time);
    rot_tick(yaw_p,   time);
    rot_tick(pitch_p, time); 

    if (time - db_time >= 1000000)
        debug_print();
    */
    process_dtm();
}
