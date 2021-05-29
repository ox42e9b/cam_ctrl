#ifndef ROT_H
#define ROT_H

#define TRANS_STOPPER_PIN  9
#define YAW_STOPPER_PIN    11
#define PITCH_STOPPER_PIN  10

#define ROT_STEP_DELAY 4  /* не меньше 4 (мкс) */

#define MAX_SPEED 12800


struct _motor {
    uint8_t pin_s, pin_d;
    int8_t dir;
    uint8_t moving; 
    uint32_t prev, delta;  /* время пред-го тика, период тика (mcs) */
    uint32_t speed;        /* шагов в секунду */
    uint32_t tgt, pos;
    uint32_t len; 
} trans = {0}, 
  yaw   = {0}, 
  pitch = {0};


void rot_set_target(struct _motor* m, int32_t target);
void rot_set_speed(struct _motor* m, int32_t speed);
inline void rot_step(struct _motor* m);
inline void rot_tick(struct _motor* m, uint32_t mcs);
int8_t rot_init();


struct _motor* trans_p = &trans;
struct _motor* yaw_p   = &yaw;
struct _motor* pitch_p = &pitch;


void
rot_set_target(struct _motor *m, int32_t target)
{
    m->tgt = (target >= 0) ? ((target < m->len) ? target : m->len) : 0;

    if (m->tgt < m->pos) {
        digitalWrite(m->pin_d, 0);
        m->dir = -1;
    } else if (m->tgt > m->pos) {
        digitalWrite(m->pin_d, 1);
        m->dir = 1;
    } 
}

void  /* шаги в секунду */
rot_set_speed(struct _motor *m, int32_t speed)
{
    m->speed = (speed > 0) ? ((speed < MAX_SPEED) ? speed : MAX_SPEED) : 0;
    if (0 == speed)
        m->moving = false; 
    else
        m->delta = 1000000 / speed;
}


inline void
rot_step(struct _motor *m)
{
    digitalWrite(m->pin_s, 1);
    delayMicroseconds(ROT_STEP_DELAY);
    digitalWrite(m->pin_s, 0);
}

inline void
rot_tick(struct _motor *m, uint32_t mcs) {
    if (m->moving && mcs - m->prev >= m->delta) {        
        m->prev = mcs;
        if (m->pos == m->tgt)                            
            m->moving = false;    
        else {              
            m->pos += m->dir;                                                
            rot_step(m);                              
        }
    }
}


int8_t 
rot_init()
{
    uint32_t time;

    trans.delta = 9984;
    yaw.delta = 9984;
    pitch.delta = 9984;

    trans.pin_s = 2; 
    trans.pin_d = 3; 
    yaw.pin_s   = 4; 
    yaw.pin_d   = 5;
    pitch.pin_s = 6; 
    pitch.pin_d = 7;
    trans.len = 46000;
    yaw.len   = 3280;
    pitch.len = 3280;

    pinMode(trans.pin_s, OUTPUT);
    pinMode(trans.pin_d, OUTPUT);
    pinMode(yaw.pin_s,   OUTPUT);
    pinMode(yaw.pin_d,   OUTPUT);
    pinMode(pitch.pin_s, OUTPUT);
    pinMode(pitch.pin_d, OUTPUT);

    pinMode(TRANS_STOPPER_PIN, INPUT_PULLUP);
    pinMode(YAW_STOPPER_PIN,   INPUT);  /* датчик холла */
    pinMode(PITCH_STOPPER_PIN, INPUT_PULLUP);

    /* позицию определяем как край и движемся в нуль */
    trans.pos = -1;  /* (переполнение подразумевается) */
    yaw.pos   = -1; 
    pitch.pos = -1;

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

    delay(7000);

    Serial.println("info: reaching initial state");
    while (yaw.moving || trans.moving || pitch.moving) {
        time = micros(); 
        rot_tick(trans_p, time); 
        rot_tick(yaw_p,   time); 
        rot_tick(pitch_p, time); 

        if (!end_t && trans.moving && (0 == digitalRead(TRANS_STOPPER_PIN))) {
            trans.pos = 0; 
            Serial.println("info: trans reached");
            end_t = true;
        } 
        if (!end_y && yaw.moving && (0 == digitalRead(YAW_STOPPER_PIN))) { 
            yaw.pos = 0;
            Serial.println("info: yaw reached");
            end_y = true;
        }
        if (!end_p && pitch.moving && (0 == digitalRead(PITCH_STOPPER_PIN))) {
            pitch.pos = 0;
            Serial.println("info: pitch reached");
            end_p = true;
        }
    }
    
    return 0;
}


#endif
