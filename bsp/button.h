#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>


#define BUTTONS_NUMBER  (2)

#define BUTTON1_IO_NUM  (9)
#define BUTTON2_IO_NUM  (10)

#define BUTTON_LONG_PUSH_TIMEOUT_MS (1000)
#define BUTTON_ACTIVE_STATE     APP_BUTTON_ACTIVE_HIGH

#define BUTTON_ACTION_LONG_PUSH (2)

#define BUTTON_CFG(_pin_no) \
{ \
    .pin_no = _pin_no, \
    .active_state = BUTTON_ACTIVE_STATE, \
    .pull_cfg = NRF_GPIO_PIN_PULLDOWN, \
    .button_handler = button_evt_handler, \
}

typedef enum
{
    BUTTON_EVENT_SCALE_UP = 1,
    BUTTON_EVENT_SCALE_DOWN,
    BUTTON_EVENT_SCALE_UP_CONTINUE_START,
    BUTTON_EVENT_SCALE_UP_CONTINUE_STOP,
    BUTTON_EVENT_SCALE_DOWN_CONTINUE_START,
    BUTTON_EVENT_SCALE_DOWN_CONTINUE_STOP,
    
    BUTTON_EVENT_CHANGE_OLE_LUMINANCE = 0xFE,
    BUTTON_EVENT_NOTHING = 0xFF
}button_event_t;

typedef struct
{
    button_event_t push_event;              /**< The event to fire on regular button press. */
    button_event_t release_event;           /**< The event to fire on button release. */
    button_event_t long_push_event;         /**< The event to fire on long button press. */
    button_event_t long_push_release_event; /**< The event to fire on long button press release. */
} button_event_cfg_t;

void button_evt_handler(uint8_t pin_no, uint8_t button_action);
void buttons_init(void);
void buttons_init2(void);
#endif

