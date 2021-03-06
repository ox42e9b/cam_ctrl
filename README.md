# Управление шаговыми двигателями

В этом репозитории содержится код, представляющий собой программный интерфейс для управления шаговыми двигателями.


## Использованные технологии

+ Язык программирования __Arduino__:
    - ``Serial``
    - ``delayMicrosecond()``
    - ``digitalWrite()``, ``digitalRead()``

+ Язык программирования __C99__ - на нем написаны:
    - Функции, структуры, предоставляющие интерфейс для управления двигателями.
    - Приложение __joytest__ для Linux, демонстрирующее взаимодействие с интерфейсом контроллера.

+ Библиотеки ``bluetooth``, ``linux/joystick`` - для связи с контроллером по bluetooth, взаимодействия с джойстиком соответственно. 

Использование __C99__ для создания абстрактного интерфейса позволит _(при необходимости)_ легко портировать код на другие платформы, не связанные с Arduino. 


## Интерфейс контроллера

+ ``cam_ctrl.ino`` - объединяет функционал ``dtm.h`` и ``rot.h``.
+ ``rot.h`` - предоставляет следующий интерфейс: 
```c
/* структуры, содержащие настройки двигателей */
struct _stepper trans, yaw, pitch;

/* функции управления двигателями */
void rot_set_target(struct _stepper* m, int32_t target);
void rot_set_speed(struct _stepper* m, int32_t speed);
inline void rot_tick(struct _stepper* m, uint32_t mcs);
void rot_reset();
int8_t rot_init();
```
+ ``dtm.h`` - определяет структуру, типы управляющих сообщений, а также функции для их обработки:
```c
enum { GENERAL, TRANS, YAW, PITCH};
enum { RESET };
enum {
    TARGET, REL_TARGET, SPEED, REL_SPEED, 
    PAUSE, UNPAUSE, DEBUG_PRINT
};

struct {
    unsigned type : 2;
    unsigned action : 6;
    int32_t value;
} frame;

inline int8_t process_dtm();
int8_t init_dtm();
```

## Возможности
Исходя из реализации ПО контроллера, управляющее приложение может делать следующее:
+ Задавать положение _(в шагах)_, к которому должны стремиться двигатели.
+ Изменять скорость _(в шагах/сек)_ передвижения каждого из двигателей.
+ Возвращать систему в начальное положение.
+ Повторно калибровать систему.
+ Запрашивать текущее состояние каждого из двигателей.
