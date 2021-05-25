#ifndef ROT_H
#define ROT_H

#define TRANS_STOPPER_PIN  9
#define YAW_STOPPER_PIN    10
#define PITCH_STOPPER_PIN  11

#define ROT_TARGET(m, tgt) (m.tgt = ((tgt < m.len) ? ((tgt > 0) ? tgt : 0) : m.len))
#define ROT_STOP(m)        (m.speed = 0, m.moving = false, m.tgt = m.pos)
#define ROT_RESET(m)       (m.pos = 0, ROT_STOP(m), m.delta = 10000)
#define ROT_SPEED(m, s)    (m.speed = s * m.delta / 1000000)

#define ROT_BACK(m)  (digitalWrite(m.pin_d, 0))
#define ROT_FRONT(m) (digitalWrite(m.pin_d, 1))

#define ROT_STEP_DELAY 4  /* не меньше 4 (мкс) */


struct _motor {
    uint8_t pin_s, pin_d;
    volatile uint8_t moving; 
    volatile uint32_t prev, delta;  /* время пред-го тика, период тика (mcs) */
    volatile uint16_t speed;        /* шагов в тик */
    volatile uint32_t tgt, pos;
    uint32_t len; 
} trans = {0}, 
  yaw   = {0}, 
  pitch = {0};


struct _motor* trans_p = &trans;
struct _motor* yaw_p   = &yaw;
struct _motor* pitch_p = &pitch;


inline void
rot_steps(struct _motor *m, uint16_t steps)
{
    uint16_t i;

    for (i = 0; i < steps - 1; ++i) {
        digitalWrite(m->pin_s, 1);
        delayMicroseconds(ROT_STEP_DELAY);
        digitalWrite(m->pin_s, 0);
        delayMicroseconds(ROT_STEP_DELAY);
    }
    digitalWrite(m->pin_s, 1);
    delayMicroseconds(ROT_STEP_DELAY);
    digitalWrite(m->pin_s, 0);
}

inline void
rot_tick(struct _motor *m, uint32_t mcs) {
    if (m->moving && mcs - m->prev >= m->delta) {        
        m->prev += m->delta;             
        m->pos += m->speed;                             
        if (m->speed < 0 && m->pos < m->tgt) { 
            rot_steps(m, m->speed + m->tgt - m->pos);     
            m->pos = m->tgt;                            
            m->moving = false;                         
        } else if (m->speed > 0 && m->pos > m->tgt) {    
            rot_steps(m, m->speed - m->pos + m->tgt);      
            m->pos = m->tgt;                            
            m->moving = false;                         
        } else                                        
            rot_steps(m, m->speed);                              
    }
}


int8_t 
rot_init()
{
    bool end_t = 0, end_y = 0, end_p = 0;
    uint32_t time;

    trans.delta = 10000;
    yaw.delta = 10000;
    pitch.delta = 10000;

    trans.pin_s = 2; 
    trans.pin_d = 3; 
    yaw.pin_s   = 4; 
    yaw.pin_d   = 5;
    pitch.pin_s = 6; 
    pitch.pin_d = 7;
    trans.len = 6400;
    yaw.len   = 6400;
    pitch.len = 6400;

    pinMode(trans.pin_s, OUTPUT);
    pinMode(trans.pin_d, OUTPUT);
    pinMode(yaw.pin_s,   OUTPUT);
    pinMode(yaw.pin_d,   OUTPUT);
    pinMode(pitch.pin_s, OUTPUT);
    pinMode(pitch.pin_d, OUTPUT);

    pinMode(TRANS_STOPPER_PIN, INPUT_PULLUP);
    pinMode(YAW_STOPPER_PIN,   INPUT_PULLUP);
    pinMode(PITCH_STOPPER_PIN, INPUT_PULLUP);

    /* позицию определяем как край и движемся в нуль */
    trans.pos = -1;
    yaw.pos   = -1; 
    pitch.pos = -1;
    ROT_SPEED(trans, 800);
    ROT_SPEED(yaw,   800);
    ROT_SPEED(pitch, 800);
    /* target по умолчанию инициализирован нулём */
    trans.moving = true;  
    yaw.moving   = true;
    pitch.moving = true;
    /* после достижения края - ROT_RESET */
    /*Serial.println("info: reaching initial state");
    while (yaw.moving || trans.moving || pitch.moving) {
        time = micros(); 
        if (trans.moving) {
            rot_tick(trans_p, time);
            if (0 == digitalRead(TRANS_STOPPER_PIN)) {
                ROT_FRONT(trans);
                rot_steps(trans_p, 10);
                ROT_RESET(trans);
            }
        } 
        if (yaw.moving) {
            rot_tick(yaw_p, time);
            if (0 == digitalRead(YAW_STOPPER_PIN)) {
                ROT_FRONT(yaw);
                rot_steps(yaw_p, 10);
                ROT_RESET(yaw);
            }
        }
        if (pitch.moving) { 
            rot_tick(pitch_p, time);
            if (0 == digitalRead(PITCH_STOPPER_PIN)) {
                ROT_FRONT(pitch);
                rot_steps(pitch_p, 10);
                ROT_RESET(pitch);
            }
        }
        delay(10);
    }*/
    ROT_RESET(trans);
    ROT_RESET(yaw);
    ROT_RESET(pitch);

    return 0;
}


#endif
