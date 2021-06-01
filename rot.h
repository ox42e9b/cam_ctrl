#ifndef ROT_H
#define ROT_H

#include <limits.h>
#include <math.h>

#define VALID_SET_ELEMENT(x, a, b) ((x > a) ? ((x < b) ? x : b) : a)

#define TRANS_STOPPER_PIN  9
#define YAW_STOPPER_PIN    11
#define PITCH_STOPPER_PIN  10

#define ROT_STEP_DELAY 10  /* не меньше 4 (мкс) */


struct _stepper {
    uint8_t pin_s, pin_d;  /* номера step, dir пинов к драйверу     */
    int8_t dir;            /* +-1 : добавляется к pos за 1 тик      */
    uint8_t moving;        /* если false, то тик не срабатывает     */ 
    uint32_t prev, delta;  /* время пред-го тика, период тика (mcs) */
    int32_t speed;         /* шагов в секунду                       */
    int32_t tgt, pos;      /* в шагах                               */
    int32_t min, max, max_speed;  /* ограничение вращения           */ 
} trans = {0}, 
  yaw   = {0}, 
  pitch = {0};


void rot_set_target(struct _stepper* m, int32_t target);
void rot_set_speed(struct _stepper* m, int32_t speed);
inline void rot_tick(struct _stepper* m, uint32_t mcs);
int8_t rot_init();


struct _stepper *trans_p = &trans;
struct _stepper *yaw_p   = &yaw;
struct _stepper *pitch_p = &pitch;


void
rot_set_target(struct _stepper *m, int32_t target)
{
    m->tgt = VALID_SET_ELEMENT(target, m->min, m->max);

    if (m->tgt < m->pos) {
        digitalWrite(m->pin_d, 0);
        m->dir = -1;
    } else if (m->tgt > m->pos) {
        digitalWrite(m->pin_d, 1);
        m->dir = 1;
    } 
}

void 
rot_set_speed(struct _stepper *m, int32_t speed)
{
    m->speed = VALID_SET_ELEMENT(speed, 0, m->max_speed);

    if (0 == speed)
        m->moving = false; 
    else
        m->delta = 1000000 / speed;
}


inline void
rot_tick(struct _stepper *m, uint32_t mcs) {
    if (m->moving && abs(mcs - m->prev) >= m->delta) {        
        m->prev = mcs;
        if (m->pos == m->tgt)                            
            m->moving = false;    
        else {              
            m->pos += m->dir;                              
            /* шаг step-dir драйвера */   
            digitalWrite(m->pin_s, 1);
            delayMicroseconds(ROT_STEP_DELAY);
            digitalWrite(m->pin_s, 0);
        }
    }
}


void
rot_reset()
{
    uint32_t time;

    /* позицию определяем как край и движемся в нуль */
    trans.pos = LONG_MAX;
    yaw.pos   = LONG_MAX; 
    pitch.pos = LONG_MAX;

    rot_set_speed(trans_p, 1500);
    rot_set_speed(yaw_p,   800);
    rot_set_speed(pitch_p, 800);

    rot_set_target(trans_p, 0);
    rot_set_target(yaw_p,   0);
    rot_set_target(pitch_p, 0);

    trans.moving = true;
    yaw.moving   = true;
    pitch.moving = true;

    /* после достижения края запоминаем позицию */
    bool end_t = 0, end_y = 0, end_p = 0;

    Serial.println("info: motors are reaching initial state");
    while (yaw.moving || trans.moving || pitch.moving) {
        time = micros(); 
        rot_tick(trans_p, time); 
        rot_tick(yaw_p,   time); 
        rot_tick(pitch_p, time); 

        if (!end_t && trans.moving && (0 == digitalRead(TRANS_STOPPER_PIN))) {
            trans.pos = 0;
            Serial.println("info: trans reached the end");
            end_t = true;
        } 
        if (!end_y && yaw.moving && (0 == digitalRead(YAW_STOPPER_PIN))) { 
            yaw.pos = 0;
            Serial.println("info: yaw reached the end");
            end_y = true;
        }
        if (!end_p && pitch.moving && (0 == digitalRead(PITCH_STOPPER_PIN))) {
            pitch.pos = 0;
            Serial.println("info: pitch reached the end");
            end_p = true;
        }
    }
}


int8_t 
rot_init()
{ 
    trans.pin_s = 2; 
    trans.pin_d = 3; 
    yaw.pin_s   = 4; 
    yaw.pin_d   = 5;
    pitch.pin_s = 6; 
    pitch.pin_d = 7;

    trans.max = 46000;
    trans.min = 0;
    trans.max_speed = 23000; 
    yaw.max   = LONG_MAX; 
    yaw.min   = LONG_MIN;
    yaw.max_speed   = 3280; 
    pitch.max = 3280;
    pitch.min = 0;
    pitch.max_speed = 2460;

    pinMode(trans.pin_s, OUTPUT);
    pinMode(trans.pin_d, OUTPUT);
    pinMode(yaw.pin_s,   OUTPUT);
    pinMode(yaw.pin_d,   OUTPUT);
    pinMode(pitch.pin_s, OUTPUT);
    pinMode(pitch.pin_d, OUTPUT);

    pinMode(TRANS_STOPPER_PIN, INPUT_PULLUP);
    pinMode(YAW_STOPPER_PIN,   INPUT);  /* датчик холла */
    pinMode(PITCH_STOPPER_PIN, INPUT_PULLUP);

    //rot_reset(); 
    
    return 0;
}


#endif
