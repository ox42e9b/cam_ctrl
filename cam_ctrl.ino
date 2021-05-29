#include <Arduino.h>

#include "dtm.h"
#include "rot.h"

uint32_t time = 0;
uint32_t db_time = 0;


void 
setup() {
    init_dtm();
    Serial.println("info: DTM initialized");
    
    rot_init(); 
    Serial.println("info: ROT initialized"); 
}

void 
loop() {
    time = micros();

    rot_tick(trans_p, time);
    rot_tick(yaw_p,   time);
    rot_tick(pitch_p, time); 
     
    process_dtm();
}
