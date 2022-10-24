#include "app_error.h"
#include "app_button.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

#include "button.h"


APP_TIMER_DEF(m_btn_long_push_tmr);

static const app_button_cfg_t m_buttons[BUTTONS_NUMBER] = 
{
    BUTTON_CFG(BUTTON1_IO_NUM),
    BUTTON_CFG(BUTTON2_IO_NUM)
};

static uint8_t m_btn_list[BUTTONS_NUMBER] = 
{   
    BUTTON1_IO_NUM,
    BUTTON2_IO_NUM
};

static button_event_cfg_t m_btn_event_list[BUTTONS_NUMBER] = 
{
    // button 1
    {
        BUTTON_EVENT_NOTHING,
        BUTTON_EVENT_NOTHING,
        BUTTON_EVENT_NOTHING,
        BUTTON_EVENT_NOTHING
    },
    // button 2
    {
        BUTTON_EVENT_NOTHING,
        BUTTON_EVENT_SCALE_UP,
        BUTTON_EVENT_SCALE_UP_CONTINUE_START,
        BUTTON_EVENT_SCALE_UP_CONTINUE_STOP
    }
};

static uint32_t button_pin_to_idx(uint8_t pin_no)
{
    uint32_t i;
    uint32_t ret = 0xFFFFFFFF;
    for (i = 0; i < BUTTONS_NUMBER; ++i)
    {
        if (m_btn_list[i] == pin_no)
        {
            ret = i;
            break;
        }
    }
    return ret;
}

static void button_long_push_detect(void *p_context)
{
    uint8_t pin_no = *((uint8_t *)p_context);

    button_evt_handler(pin_no, BUTTON_ACTION_LONG_PUSH);
}

static uint32_t button_long_push_timer_create(void)
{
    uint32_t ret_code;
    ret_code = app_timer_create(&m_btn_long_push_tmr, APP_TIMER_MODE_SINGLE_SHOT, button_long_push_detect);
    return ret_code;
}

static void button_command_process(button_event_t btn_evt)
{
    int32_t ret = 0;
    switch(btn_evt)
    {
        case BUTTON_EVENT_SCALE_UP:
            //ret = i2c_comm_event_read_queue_in(BUTTON_EVENT_SCALE_UP);
            break;

        case BUTTON_EVENT_SCALE_DOWN:
            break;

        case BUTTON_EVENT_SCALE_UP_CONTINUE_START:
            break;

        case BUTTON_EVENT_SCALE_UP_CONTINUE_STOP:
            break;

        case BUTTON_EVENT_SCALE_DOWN_CONTINUE_START:
            break;    
    
        case BUTTON_EVENT_SCALE_DOWN_CONTINUE_STOP:
            break;
            
        case BUTTON_EVENT_CHANGE_OLE_LUMINANCE:
            //ecx335sn_change_luminance();
            break;
        default:
            break;
    }
    
    if(ret < 0)
    {
        /* TODO queue full handle */
    }
}

void button_evt_handler(uint8_t pin_no, uint8_t button_action)
{
    button_event_t     event  = BUTTON_EVENT_NOTHING;
    uint32_t           button = 0;
    uint32_t           err_code;
    static uint8_t     current_long_push_pin_no;            /**< Pin number of a currently pushed button, that could become a long push if held long enough. */
    static bool is_cur_state_long_push[BUTTONS_NUMBER];     /**< Array of what the release event of each button was last time it was pushed, so that no release event is sent if the event was bound after the push of the button. */

    button = button_pin_to_idx(pin_no);

    if (button < BUTTONS_NUMBER)
    {
        switch (button_action)
        {
            case APP_BUTTON_PUSH:
                event = m_btn_event_list[button].push_event;
                if (m_btn_event_list[button].long_push_event != BUTTON_EVENT_NOTHING)
                {
                    err_code = app_timer_start(m_btn_long_push_tmr, APP_TIMER_TICKS(BUTTON_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no);
                    if (err_code == NRF_SUCCESS)
                    {
                        current_long_push_pin_no = pin_no;
                    }
                }
                is_cur_state_long_push[button] = false;
                NRF_LOG_INFO("Button %d push.", button);

                break;
            case APP_BUTTON_RELEASE:
                (void)app_timer_stop(m_btn_long_push_tmr);
                if (is_cur_state_long_push[button] == false)
                {
                    event = m_btn_event_list[button].release_event;
                    NRF_LOG_INFO("Button %d release.", button);
                }
                else
                {
                    event = m_btn_event_list[button].long_push_release_event;
                    NRF_LOG_INFO("Button %d long push release.", button);
                }
                
                break;
            case BUTTON_ACTION_LONG_PUSH:
                event = m_btn_event_list[button].long_push_event;
                is_cur_state_long_push[button] = true;
                NRF_LOG_INFO("Button %d long push.", button);
                break;
        }
    }

    if (event != BUTTON_EVENT_NOTHING)
    {
        button_command_process(event);
    }
}

void buttons_init(void)
{
    uint32_t ret_code;
    ret_code = app_button_init(m_buttons, BUTTONS_NUMBER, APP_TIMER_TICKS(50));
    APP_ERROR_CHECK(ret_code);
    
    ret_code = app_button_enable();
    APP_ERROR_CHECK(ret_code);
    
    ret_code = button_long_push_timer_create();
    APP_ERROR_CHECK(ret_code);
}

int count = 0;
void button_event(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{    
    count++;
    NRF_LOG_INFO("key pushed.");
}

void buttons_init2(void)
{
    uint32_t err_code;
    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }
    nrf_drv_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    config.pull = NRF_GPIO_PIN_PULLDOWN;
    err_code = nrf_drv_gpiote_in_init(BUTTON1_IO_NUM, &config, button_event);
    if (err_code != NRFX_SUCCESS)
    {
        while(1); //zyy
    }
    nrf_drv_gpiote_in_event_enable(BUTTON1_IO_NUM, true);
    
}

