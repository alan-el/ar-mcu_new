#include <stdlib.h>
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "lt7911.h"

void lt7911_io_init(void)
{
    nrf_gpio_cfg(
        LT7911_RESET_PIN,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_S0S1,
        NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_write(LT7911_RESET_PIN, 1);
    
    nrf_gpio_cfg_input(LT7911_GPIO5_PIN, NRF_GPIO_PIN_NOPULL);
}

void lt7911_reset(void)
{
    nrf_gpio_pin_write(LT7911_RESET_PIN, 0);
    nrf_delay_ms(100);
    nrf_gpio_pin_write(LT7911_RESET_PIN, 1);
    nrf_delay_ms(100);
}

void lt7911_poweroff(void)
{
    nrf_gpio_pin_write(LT7911_RESET_PIN, 0);
    nrf_delay_ms(100);
}

uint32_t lt7911_gpio5_value(void)
{
    return nrf_gpio_pin_read(LT7911_GPIO5_PIN);
}
