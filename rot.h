#ifndef ROT_H
#define ROT_H

#define TRANS_STOPPER_PIN  8
#define YAW_STOPPER_PIN    9
#define PITCH_STOPPER_PIN 10

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


/* Установка скорости в шагах в секунду */
void
rot_step_speed(struct _motor *m, uint16_t steps)
{
    
}


/* Прерывания по достижению границы */
void 
trans_at_end() { ROT_RESET(trans); }

void 
yaw_at_end()   { ROT_RESET(yaw);   }

void 
pitch_at_end() { ROT_RESET(pitch); }



int8_t 
rot_init()
{ 
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
    trans.len = 4096;
    yaw.len   = 4096;
    pitch.len = 4096;

    pinMode(trans.pin_s, OUTPUT);
    pinMode(trans.pin_d, OUTPUT);
    pinMode(yaw.pin_s,   OUTPUT);
    pinMode(yaw.pin_d,   OUTPUT);
    pinMode(pitch.pin_s, OUTPUT);
    pinMode(pitch.pin_d, OUTPUT);

    pinMode(TRANS_STOPPER_PIN, INPUT_PULLUP);
    pinMode(YAW_STOPPER_PIN,   INPUT_PULLUP);
    pinMode(PITCH_STOPPER_PIN, INPUT_PULLUP);

    attachInterrupt(TRANS_STOPPER_PIN, trans_at_end, FALLING);
    attachInterrupt(  YAW_STOPPER_PIN,   yaw_at_end, FALLING);
    attachInterrupt(PITCH_STOPPER_PIN, pitch_at_end, FALLING);

    /* позицию определяем как край и движемся в нуль */
    trans.pos = -1;
    yaw.pos   = -1; 
    pitch.pos = -1;
    ROT_SPEED(trans, 100);
    ROT_SPEED(yaw,   100);
    ROT_SPEED(pitch, 100);
    /* target по умолчанию инициализирован нулём */
    trans.moving = true;  
    yaw.moving   = true;
    pitch.moving = true;
    while (yaw.moving || trans.moving || pitch.moving) {
        time = micros();
        rot_tick(yaw_p, time);
        rot_tick(trans_p, time);
        rot_tick(pitch_p, time);
        delay(5);
    }  /* после достижения края - reset в прерывании */ 

    return 0;
}


#endif
