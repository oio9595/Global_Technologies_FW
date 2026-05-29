
#include "main.h"
#include "framework.h"

#define LONG_PRESS_TIME (1000U) /* 1초 이상 누르면 Long Press */
#define DEBOUNCE_TIME   (25U)   /* 25ms 디바운스 */

#define EVENT_QUEUE_SIZE    (8U)

typedef struct
{
    uint8_t btn_id;
    BtnEvent_t type;
}ButtonEvent_t;

typedef struct
{
    GPIO_TypeDef* port;
    uint16_t pin;
    uint32_t start_tick;
    uint8_t last_state;     /* 0: Pressed, 1: Released (Pull-up 기준) */
    bool is_long_sent;      /* Long Press 중복 발생 방지 플래그 */
}Button_t;

typedef struct
{
    ButtonEvent_t buffer[EVENT_QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
}EventQueue_t;

static struct{
    MGRSTATUS	status;

    Button_t btns[BTN_MAX];

    THREAD_ID det_thr;
}__priv_det;

static EventQueue_t event_q;

static void event_queue_init(void)
{
    event_q.head = 0U;
    event_q.tail = 0U;
}

static bool event_enqueue(uint8_t id, BtnEvent_t type)
{
    uint8_t next_head = (event_q.head + 1U) % EVENT_QUEUE_SIZE;

    /* 큐가 가득 찼는지 확인 */
    if(next_head == event_q.tail)
    {
        return false;
    }

    event_q.buffer[event_q.head].btn_id = id;
    event_q.buffer[event_q.head].type = type;
    event_q.head = next_head;

    return true;
}

/* 큐에서 이벤트 꺼내기 (Main Loop에서 호출) */
bool event_dequeue(ButtonEvent_t *event)
{
    /* 큐가 비어있는지 확인 */
    if(event_q.head == event_q.tail)
    {
        return false;
    }

    *event = event_q.buffer[event_q.tail];
    event_q.tail = (event_q.tail + 1U) % EVENT_QUEUE_SIZE;

    return true;
}

static void On_Button_Event(uint8_t id, BtnEvent_t type)
{
    event_enqueue(id, type);

    MGR_APP()->noti((uint32_t)id, (void*)&type);
#if 0
    switch (type)
    {
    case EVT_PRESSED:       /* 버튼 눌린 순간 */
        break;
    case EVT_RELEASED:      /* 버튼 뗀 순간 */
        break;
    case EVT_SHORT_CLICK:   /* 짧게 눌렀다 뗌 (모드 변경 등) */
        break;
    case EVT_LONG_PRESS:    /* 길게 누름 (전원 Off, 설정 초기화 등) */
        break;
    default:
        break;
    }
#endif
}

static bool _detect_thread(struct thread_data* td)
{
    for(uint8_t i = 0; i < BTN_MAX; ++i)
    {
        Button_t *b = &__priv_det.btns[i];
        uint8_t current_state = (b->port->IDR & b->pin) ? SET : RESET;
        uint32_t duration = get_system_tick() - b->start_tick;

        /* 1. 버튼을 눌렀을 때 (Falling Edge) */
        if((current_state == RESET) && (b->last_state == SET))
        {
            if(duration > DEBOUNCE_TIME)
            {
                b->start_tick = get_system_tick();
                b->last_state = RESET;
                b->is_long_sent = false;
                On_Button_Event(i, BTN_EVT_PRESSED); /* 눌림 즉시 실행 */
            }
        }
        /* 2. 버튼을 누르고 있는 중 (Long Press 체크) */
        else if((current_state == RESET) && (b->last_state == RESET))
        {
            if((!b->is_long_sent) && (duration > LONG_PRESS_TIME))
            {
                b->is_long_sent = true;
                On_Button_Event(i, BTN_EVT_LONG_PRESS);
            }
        }
        /* 3. 버튼을 뗐을 때 (Rising Edge) */
        else if((current_state == SET) && (b->last_state == RESET))
        {
            if(duration > DEBOUNCE_TIME)
            {
                b->last_state = SET;
                b->start_tick = get_system_tick();

                On_Button_Event(i, BTN_EVT_RELEASED); /* 뗌 즉시 실행 */

                /* Long Press가 아니었다면 Short Click으로 판정 */
                if(!b->is_long_sent)
                {
                    On_Button_Event(i, BTN_EVT_SHORT_CLICK);
                }
            }
        }
    }

    return true;
}

static void _power(bool on)
{
    if(true == on)
    {
        __priv_det.status = STATUS_NORMAL;

        event_queue_init();

        for(uint8_t i = 0; i < BTN_MAX; ++i)
        {
            Button_t *b = &__priv_det.btns[i];

            b->port = B1_GPIO_Port;
            b->pin = B1_Pin;
            b->start_tick = 0U;
            b->last_state = (b->port->IDR & b->pin) ? SET : RESET;
            b->is_long_sent = false;
        }

        __priv_det.det_thr = fw_begin_thread_ex(_detect_thread, 5U);    /* 5ms */
    }
    else
    {
        if(__priv_det.det_thr != INVALID_THREAD_ID)
        {
            fw_thread_stop(__priv_det.det_thr);
            __priv_det.det_thr = INVALID_THREAD_ID;
        }

        __priv_det.status = STATUS_UNPOWER;
    }
}

static void _enable(bool en)
{

}

static MGRSTATUS _status(void)
{
    return __priv_det.status;
}

static uint32_t _cmd(uint32_t cmd, void* val)
{
    return MGRET_OK;
}

static uint32_t _write(uint32_t addr, void* val, uint32_t len)
{
    return MGRET_OK;
}

static uint32_t _read(uint32_t addr, void* val, uint32_t len)
{
    return 0;
}

static uint32_t _noti(uint32_t type, void* val)
{
    return MGRET_OK;
}

struct manager __det_mgr=
{
    _power,
    _enable,
    _status,
    _cmd,
    _write,
    _read,
    _noti
};

