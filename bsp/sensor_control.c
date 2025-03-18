#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "sensor_control.h"

void sensor_control_io_init(void)
{
    nrf_gpio_cfg_output(IMX214_PWREN_PIN);
    nrf_gpio_pin_write(IMX214_PWREN_PIN, 0);
    nrf_gpio_cfg_output(IMX586_PWREN_PIN);
    nrf_gpio_pin_write(IMX586_PWREN_PIN, 0);
    nrf_gpio_cfg_output(SENSOR_MIPI_MUX_PIN);
    nrf_gpio_pin_write(SENSOR_MIPI_MUX_PIN, 0);
}

void imx214_power_enable(void)
{
    imx586_power_disable();
    nrf_gpio_pin_write(IMX214_PWREN_PIN, 0);
    nrf_delay_ms(10);
    nrf_gpio_pin_write(IMX214_PWREN_PIN, 1);
}

void imx214_power_disable(void)
{
    nrf_gpio_pin_write(IMX214_PWREN_PIN, 0);
}

void imx586_power_enable(void)
{
    imx214_power_disable();
    nrf_gpio_pin_write(IMX586_PWREN_PIN, 0);
    nrf_delay_ms(10);
    nrf_gpio_pin_write(IMX586_PWREN_PIN, 1);
}

void imx586_power_disable(void)
{
    nrf_gpio_pin_write(IMX586_PWREN_PIN, 0);
}

void sensor_control_mipi_mux_to_imx214(void)
{
    nrf_gpio_pin_write(SENSOR_MIPI_MUX_PIN, 1);
}

void sensor_control_mipi_mux_to_imx586(void)
{
    nrf_gpio_pin_write(SENSOR_MIPI_MUX_PIN, 0);
}
