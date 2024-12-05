#include <stdlib.h>
#include "nrf_gpio.h"
#include "nrfx_twi.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "seeya0_49.h"

/* TWI instance ID */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static const nrfx_twi_t m_twi = NRFX_TWI_INSTANCE(TWI_INSTANCE_ID);

void 
seeya_io_init(void)
{
    nrf_gpio_cfg_output(OLEDA_RESET_PIN);
    nrf_gpio_pin_write(OLEDA_RESET_PIN, 1);
    nrf_gpio_cfg_output(OLEDB_RESET_PIN);
    nrf_gpio_pin_write(OLEDB_RESET_PIN, 1);
    
    nrf_gpio_cfg_output(OLED_PWR_EN);
    nrf_gpio_pin_write(OLED_PWR_EN, 0);
    
    nrf_gpio_cfg_input(OLEDA_OCP_OUT_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(OLEDA_GPIO_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(OLEDB_OCP_OUT_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(OLEDB_GPIO_PIN, NRF_GPIO_PIN_NOPULL);
}

void 
seeya_oled_power_enable(void)
{
    nrf_gpio_pin_write(OLED_PWR_EN, 1);
}

void 
seeya_oled_power_disable(void)
{
    nrf_gpio_pin_write(OLED_PWR_EN, 0);
}

void 
seeya_oleda_reset(void)
{
    nrf_gpio_pin_write(OLEDA_RESET_PIN, 0);
    nrf_delay_ms(1000);
    nrf_gpio_pin_write(OLEDA_RESET_PIN, 1);
}

void 
seeya_oledb_reset(void)
{
    nrf_gpio_pin_write(OLEDB_RESET_PIN, 0);
    nrf_delay_ms(1000);
    nrf_gpio_pin_write(OLEDB_RESET_PIN, 1);
}

uint32_t 
seeya_oleda_ocp_out_value(void)
{
    return nrf_gpio_pin_read(OLEDA_OCP_OUT_PIN);
}

uint32_t 
seeya_oleda_gpio_value(void)
{
    return nrf_gpio_pin_read(OLEDA_GPIO_PIN);
}

uint32_t 
seeya_oledb_ocp_out_value(void)
{
    return nrf_gpio_pin_read(OLEDB_OCP_OUT_PIN);
}

uint32_t 
seeya_oledb_gpio_value(void)
{
    return nrf_gpio_pin_read(OLEDB_GPIO_PIN);
}

void 
seeya_i2c_init(void)
{
    ret_code_t err_code;
    const nrfx_twi_config_t config = 
    {
        .scl                = OLED_I2C_SCL_PIN,
        .sda                = OLED_I2C_SDA_PIN,
        .frequency          = NRF_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .hold_bus_uninit     = true
    };
    
    err_code = nrfx_twi_init(&m_twi, &config, NULL, NULL);
//    APP_ERROR_CHECK(err_code);
    
    nrfx_twi_enable(&m_twi);
}

static uint8_t i2c_slave_addr = 0;

void seeya_set_i2c_slave_addr(uint8_t addr)
{
    i2c_slave_addr = addr;
}

void 
seeya_write_reg(const uint16_t reg_addr,
                        const uint16_t data)
{
    ret_code_t err_code;
    
    uint8_t write_buf[4] = {(reg_addr & 0xFF00) >> 8, 
                            reg_addr & 0x00FF,
                            (data & 0xFF00) >> 8,
                            data & 0x00FF};
    
    err_code = nrfx_twi_tx(&m_twi, i2c_slave_addr, write_buf, 4, false);
//    APP_ERROR_CHECK(err_code);
    
}

void 
seeya_read_reg(const uint16_t reg_addr,
                        uint8_t *read_buf)
{
    ret_code_t err_code;
    uint8_t send_data[2] = {(reg_addr & 0xFF00) >> 8, reg_addr & 0x00FF};

    err_code = nrfx_twi_tx(&m_twi, i2c_slave_addr, send_data, 2, false);
//    APP_ERROR_CHECK(err_code);
    
    err_code = nrfx_twi_rx(&m_twi, i2c_slave_addr, read_buf, 2);
//    APP_ERROR_CHECK(err_code);
}

void 
seeya_oled_reg_init(bool is_high_level)
{
#ifdef USE_SEEYA_OLED
#ifndef USE_SEEYA_NEW_CONFIG
    // CMD1
    seeya_write_reg(0x5300, 0x0029);

    seeya_write_reg(0x5100, 0x00FF);
    seeya_write_reg(0x5101, 0x0001);

    seeya_write_reg(0x0300, 0x0000);

    seeya_write_reg(0x8000, 0x0000);
    seeya_write_reg(0x8001, 0x00E0);
    seeya_write_reg(0x8002, 0x00E0);
    seeya_write_reg(0x8003, 0x000E);
    seeya_write_reg(0x8004, 0x0000);
    seeya_write_reg(0x8005, 0x0031);

    seeya_write_reg(0x8100, 0x0003);
    seeya_write_reg(0x8101, 0x0004);
    seeya_write_reg(0x8102, 0x0000);
    seeya_write_reg(0x8103, 0x0029);
    seeya_write_reg(0x8104, 0x0000);
    seeya_write_reg(0x8105, 0x0004);
    seeya_write_reg(0x8106, 0x0000);

    seeya_write_reg(0x8200, 0x0003);
    seeya_write_reg(0x8201, 0x0004);
    seeya_write_reg(0x8202, 0x0000);
    seeya_write_reg(0x8203, 0x0029);
    seeya_write_reg(0x8204, 0x0000);
    seeya_write_reg(0x8205, 0x0004);
    seeya_write_reg(0x8206, 0x0001);

    seeya_write_reg(0x3500, 0x0000);

    seeya_write_reg(0x2600, 0x0020);

    seeya_write_reg(0xF000, 0x00AA);
    seeya_write_reg(0xF001, 0x0011);

    seeya_write_reg(0xC200, 0x0003);
    seeya_write_reg(0xC201, 0x00FF);
    seeya_write_reg(0xC202, 0x0003);
    seeya_write_reg(0xC203, 0x00FF);
    seeya_write_reg(0xC204, 0x0003);
    seeya_write_reg(0xC205, 0x00FF);
    seeya_write_reg(0xC206, 0x0003);
    seeya_write_reg(0xC207, 0x00FF);

    //CMD2,P0
    seeya_write_reg(0xF000, 0x00AA);
    seeya_write_reg(0xF001, 0x0012);

    seeya_write_reg(0xBF00, 0x0037);
    seeya_write_reg(0xBF01, 0x00A3);

    //CMD3,P0
    seeya_write_reg(0xFF00, 0x005A);
    seeya_write_reg(0xFF01, 0x0080);

    seeya_write_reg(0xF22F, 0x0001);

    //CMD3,P1
    seeya_write_reg(0xFF00, 0x005A);
    seeya_write_reg(0xFF01, 0x0081);

    seeya_write_reg(0xF205, 0x0020);

    seeya_write_reg(0xF916, 0x0001);
    seeya_write_reg(0xF917, 0x0061);
    seeya_write_reg(0xF918, 0x0065);
    seeya_write_reg(0xF919, 0x0069);
    seeya_write_reg(0xF91A, 0x006D);
    seeya_write_reg(0xF91B, 0x0071);
    seeya_write_reg(0xF91C, 0x0076);
    seeya_write_reg(0xF91D, 0x007A);
    seeya_write_reg(0xF91E, 0x007E);
    seeya_write_reg(0xF91F, 0x0082);
    seeya_write_reg(0xF920, 0x0086);
    seeya_write_reg(0xF921, 0x008B);
    seeya_write_reg(0xF922, 0x008F);
    seeya_write_reg(0xF923, 0x0093);
#else
    //CMD1
    seeya_write_reg(0x5300, 0x0029);

    seeya_write_reg(0x5100, 0x00FF);
    seeya_write_reg(0x5101, 0x0001);

    seeya_write_reg(0x0300, 0x0000);

    seeya_write_reg(0x8000, 0x0000);
    seeya_write_reg(0x8001, 0x0040);
    seeya_write_reg(0x8002, 0x00E0);
    seeya_write_reg(0x8003, 0x000E);
    seeya_write_reg(0x8004, 0x0000);
    seeya_write_reg(0x8005, 0x0031);


    seeya_write_reg(0x8100, 0x0003);
    seeya_write_reg(0x8101, 0x0004);
    seeya_write_reg(0x8102, 0x0000);
    seeya_write_reg(0x8103, 0x0010);
    seeya_write_reg(0x8104, 0x0000);
    seeya_write_reg(0x8105, 0x0010);
    seeya_write_reg(0x8106, 0x0000);

    seeya_write_reg(0x8200, 0x0003);
    seeya_write_reg(0x8201, 0x0004);
    seeya_write_reg(0x8202, 0x0000);
    seeya_write_reg(0x8203, 0x0010);
    seeya_write_reg(0x8204, 0x0000);
    seeya_write_reg(0x8205, 0x0010);
    seeya_write_reg(0x8206, 0x0001);

    seeya_write_reg(0x3500, 0x0000);
    seeya_write_reg(0x2600, 0x0020);


    seeya_write_reg(0xF000, 0x00AA);
    seeya_write_reg(0xF001, 0x0011);

    seeya_write_reg(0xC200, 0x0003);
    seeya_write_reg(0xC201, 0x00FF);
    seeya_write_reg(0xC202, 0x0003);
    seeya_write_reg(0xC203, 0x00FF);
    seeya_write_reg(0xC204, 0x0003);
    seeya_write_reg(0xC205, 0x00FF);
    seeya_write_reg(0xC206, 0x0003);
    seeya_write_reg(0xC207, 0x00FF);

    //CMD2,P0
    seeya_write_reg(0xF000, 0x00AA);
    seeya_write_reg(0xF001, 0x0012);

    seeya_write_reg(0xBF00, 0x0037);
    seeya_write_reg(0xBF01, 0x00A9);

    //CMD3 P0
    seeya_write_reg(0xFF00, 0x005A);
    seeya_write_reg(0xFF01, 0x0080);

    seeya_write_reg(0xF22F, 0x0001);

    //CMD3 P1
    seeya_write_reg(0xFF00, 0x005A);
    seeya_write_reg(0xFF01, 0x0081);
    seeya_write_reg(0xF205, 0x0022);
    seeya_write_reg(0xF20A, 0x0000);

    seeya_write_reg(0xF917, 0x005E);
    seeya_write_reg(0xF918, 0x0062);
    seeya_write_reg(0xF919, 0x0066);
    seeya_write_reg(0xF91A, 0x006A);
    seeya_write_reg(0xF91B, 0x006F);
    seeya_write_reg(0xF91C, 0x0073);
    seeya_write_reg(0xF91D, 0x0077);
    seeya_write_reg(0xF91E, 0x007B);
    seeya_write_reg(0xF91F, 0x007F);
    seeya_write_reg(0xF920, 0x0084);
    seeya_write_reg(0xF921, 0x0088);
    seeya_write_reg(0xF922, 0x008C);
    seeya_write_reg(0xF923, 0x0090);

#endif
    /* Pin[RL_SEL] = 0, I2C Slave address = 0x4C */
    if(!is_high_level)	
    {
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0011);

        seeya_write_reg(0xC000, 0x0000);
    }
    /* Pin[RL_SEL] = 1, I2C Slave address = 0x4D */
    else
    {
        /* Original */
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0013);

        seeya_write_reg(0xC100, 0x0094);
        seeya_write_reg(0xC101, 0x00A2);

        /* Rotate 180 degree */
//        seeya_write_reg(0x3600, 0x0003);
//        
//        seeya_write_reg(0xF000, 0x00AA);
//        seeya_write_reg(0xF000, 0x0013);
//        seeya_write_reg(0x6500, 0x0001);
//        seeya_write_reg(0xC100, 0x0022);
//        seeya_write_reg(0x6500, 0x0006);
//        seeya_write_reg(0xC400, 0x0031);
//        seeya_write_reg(0xC401, 0x0042);
//        seeya_write_reg(0xC402, 0x0056);
//        seeya_write_reg(0xC403, 0x0012);
//        seeya_write_reg(0xC404, 0x0053);
//        seeya_write_reg(0xC405, 0x0064);
//        
//        seeya_write_reg(0xF000, 0x00AA);
//        seeya_write_reg(0xF001, 0x0016);
//        seeya_write_reg(0x6500, 0x0006);
//        seeya_write_reg(0xB600, 0x0031);
//        seeya_write_reg(0xB601, 0x0042);
//        seeya_write_reg(0xB602, 0x0056);
//        seeya_write_reg(0xB603, 0x0012);
//        seeya_write_reg(0xB604, 0x0053);
//        seeya_write_reg(0xB605, 0x0064);
//        seeya_write_reg(0xB000, 0x0000);
//        seeya_write_reg(0xB001, 0x0044);
    }

    nrf_delay_ms(20);
    seeya_write_reg(0x1100, 0x0000);
    nrf_delay_ms(100);
    seeya_write_reg(0x2900, 0x0000);
    nrf_delay_ms(20);
#else   // Use JDF OLED
    seeya_write_reg(0x0300, 0x0000);
    seeya_write_reg(0x5300, 0x002D);
    
    seeya_write_reg(0x5100, 0x00FF);
    seeya_write_reg(0x5101, 0x0001);
    seeya_write_reg(0x8000, 0x0001);
    seeya_write_reg(0x8001, 0x00E0);
    seeya_write_reg(0x8002, 0x00E0);
    seeya_write_reg(0x8003, 0x00E0);
    seeya_write_reg(0x8004, 0x0000);
    seeya_write_reg(0x8005, 0x0031);
    seeya_write_reg(0x8100, 0x0003);
    seeya_write_reg(0x8101, 0x0004);
    seeya_write_reg(0x8102, 0x0000);
    seeya_write_reg(0x8103, 0x0029);
    seeya_write_reg(0x8104, 0x0000);
    seeya_write_reg(0x8105, 0x0004);
    seeya_write_reg(0x8106, 0x0000);
    seeya_write_reg(0x8200, 0x0003);
    seeya_write_reg(0x8201, 0x0004);
    seeya_write_reg(0x8202, 0x0000);
    seeya_write_reg(0x8203, 0x0010);
    seeya_write_reg(0x8204, 0x0000);
    seeya_write_reg(0x8205, 0x0010);
    seeya_write_reg(0x8206, 0x0001);
    seeya_write_reg(0x3500, 0x0000);
    seeya_write_reg(0x2600, 0x0020);
    
    seeya_write_reg(0xFF00, 0x005A);
    seeya_write_reg(0xFF01, 0x0080);
    seeya_write_reg(0x6500, 0x002F);
    seeya_write_reg(0xF200, 0x0001);
    
    seeya_write_reg(0xFF00, 0x005A);
    seeya_write_reg(0xFF01, 0x0081);
    seeya_write_reg(0x6500, 0x0016);
    
    seeya_write_reg(0xF900, 0x0000);
    seeya_write_reg(0xF901, 0x0052);
    seeya_write_reg(0xF902, 0x0059);
    seeya_write_reg(0xF903, 0x0060);
    seeya_write_reg(0xF904, 0x0067);
    seeya_write_reg(0xF905, 0x006B);
    seeya_write_reg(0xF906, 0x0070);
    seeya_write_reg(0xF907, 0x0075);
    seeya_write_reg(0xF908, 0x0079);
    seeya_write_reg(0xF909, 0x007E);
    seeya_write_reg(0xF90A, 0x0083);
    seeya_write_reg(0xF90B, 0x0087);
    seeya_write_reg(0xF90C, 0x008C);
    seeya_write_reg(0xF90D, 0x0091);
    seeya_write_reg(0xF90E, 0x0095);
    seeya_write_reg(0xF90F, 0x009A);
    seeya_write_reg(0xF910, 0x009F);
    seeya_write_reg(0xF911, 0x00A6);
    seeya_write_reg(0x6500, 0x0005);
    seeya_write_reg(0xF200, 0x0023);
    seeya_write_reg(0x6500, 0x000A);
    seeya_write_reg(0xF200, 0x0000);
    
    /* Pin[RL_SEL] = 0, I2C Slave address = 0x4C */
    if(!is_high_level)	
    {
        seeya_write_reg(0x3600, 0x0000);
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0013);
        seeya_write_reg(0x6500, 0x0001);
        seeya_write_reg(0xC101, 0x0022);
        seeya_write_reg(0xC400, 0x0031);
        seeya_write_reg(0xC401, 0x0042);
        seeya_write_reg(0xC402, 0x0056);
        seeya_write_reg(0xC403, 0x0012);
        seeya_write_reg(0xC404, 0x0053);
        seeya_write_reg(0xC405, 0x0064);
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0016);
        seeya_write_reg(0xB600, 0x0031);
        seeya_write_reg(0xB601, 0x0042);
        seeya_write_reg(0xB602, 0x0056);
        seeya_write_reg(0xB603, 0x0012);
        seeya_write_reg(0xB604, 0x0053);
        seeya_write_reg(0xB605, 0x0064);
        seeya_write_reg(0xB000, 0x0000);
        seeya_write_reg(0xB001, 0x0054);
    }
    /* Pin[RL_SEL] = 1, I2C Slave address = 0x4D */
    else
    {
        seeya_write_reg(0x3600, 0x0000);
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0013);
        seeya_write_reg(0x6500, 0x0001);
        seeya_write_reg(0xC101, 0x00A2);
        seeya_write_reg(0xC400, 0x0031);
        seeya_write_reg(0xC401, 0x0042);
        seeya_write_reg(0xC402, 0x0056);
        seeya_write_reg(0xC403, 0x0012);
        seeya_write_reg(0xC404, 0x0053);
        seeya_write_reg(0xC405, 0x0064);
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0016);
        seeya_write_reg(0xB600, 0x0012);
        seeya_write_reg(0xB601, 0x0053);
        seeya_write_reg(0xB602, 0x0064);
        seeya_write_reg(0xB603, 0x0031);
        seeya_write_reg(0xB604, 0x0042);
        seeya_write_reg(0xB605, 0x0056);
        seeya_write_reg(0xB000, 0x0000);
        seeya_write_reg(0xB001, 0x0055);
    }
    
    seeya_write_reg(0x1100, 0x0000);
    nrf_delay_ms(200);
    seeya_write_reg(0x2900, 0x0000);
#endif
}
/* Divide 1023 (0x3FF) into 100 levels, user can set brightness 
 * between level 25 to level 100. 
 */
static int s_brightness_level = 100;
/* Enter sleep */
void jdf_oled_sleep(uint8_t which)
{
    if (which & 0x01)
    {
        i2c_slave_addr = OLEDA_I2C_SLAVE_ADDR;
        /* Set brightness to 0 */
        jdf_oled_brightness_set(0);
        seeya_write_reg(0x2200, 0x0000); // Pixel off
        seeya_write_reg(0x2800, 0x0000); // Screen off
        nrf_delay_ms(20);
        seeya_write_reg(0x1000, 0x0000); // Enter sleep    
        nrf_delay_ms(10);
    }
    
    if (which & 0x02)
    {
        i2c_slave_addr = OLEDB_I2C_SLAVE_ADDR;
        jdf_oled_brightness_set(0);
        seeya_write_reg(0x2200, 0x0000);
        seeya_write_reg(0x2800, 0x0000);
        nrf_delay_ms(20);
        seeya_write_reg(0x1000, 0x0000);
    }
}

/* Wake up */
void jdf_oled_wake_up(uint8_t which)
{
    if (which & 0x01)
    {
        i2c_slave_addr = OLEDA_I2C_SLAVE_ADDR;
        seeya_write_reg(0x1100, 0x0000); // Exit sleep
        nrf_delay_ms(20);
        seeya_write_reg(0x2900, 0x0000); // Screen on
        seeya_write_reg(0x1300, 0x0000); // Pixel on
        jdf_oled_brightness_set(s_brightness_level);
        nrf_delay_ms(10);
    }
    
    if (which & 0x02)
    {
        i2c_slave_addr = OLEDB_I2C_SLAVE_ADDR;
        seeya_write_reg(0x1100, 0x0000);
        nrf_delay_ms(20);
        seeya_write_reg(0x2900, 0x0000);
        seeya_write_reg(0x1300, 0x0000);
        jdf_oled_brightness_set(s_brightness_level);
    }
}

void jdf_oled_brightness_set(int level)
{
    if (level < 25 || level > 100)
    {
        /* Error value */
    }
    else
    {
        s_brightness_level = level;
        
        uint16_t brightness_l = 0;
        uint16_t brightness_h = 0;
        
        int brt_value = ((double)level / 100) * 0x3FF;
        brightness_l = brt_value % 0x0100;
        brightness_h = brt_value / 0x0100;
        
        NRF_LOG_INFO("brightness_l = 0x%x, brightness_h = 0x%x", brightness_l, brightness_h);
        
        i2c_slave_addr = OLEDA_I2C_SLAVE_ADDR;
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0011);
        seeya_write_reg(0xC200, brightness_h);
        seeya_write_reg(0xC201, brightness_l);
        seeya_write_reg(0xC202, brightness_h);
        seeya_write_reg(0xC203, brightness_l);
        seeya_write_reg(0xC204, brightness_h);
        seeya_write_reg(0xC205, brightness_l);
        seeya_write_reg(0xC206, brightness_h);
        seeya_write_reg(0xC207, brightness_l);
        
        nrf_delay_ms(10);
        i2c_slave_addr = OLEDB_I2C_SLAVE_ADDR;
        seeya_write_reg(0xF000, 0x00AA);
        seeya_write_reg(0xF001, 0x0011);
        seeya_write_reg(0xC200, brightness_h);
        seeya_write_reg(0xC201, brightness_l);
        seeya_write_reg(0xC202, brightness_h);
        seeya_write_reg(0xC203, brightness_l);
        seeya_write_reg(0xC204, brightness_h);
        seeya_write_reg(0xC205, brightness_l);
        seeya_write_reg(0xC206, brightness_h);
        seeya_write_reg(0xC207, brightness_l);
    }
}

void jdf_oled_check_brightness(void)
{
    seeya_write_reg(0xF000, 0x00AA);
    seeya_write_reg(0xF001, 0x0011);
    
    uint8_t data[2];
    seeya_read_reg(0xC200, data);
    NRF_LOG_INFO("0xC200 = 0x%x", data[1]);
    seeya_read_reg(0xC201, data);
    NRF_LOG_INFO("0xC201 = 0x%x", data[1]);
    seeya_read_reg(0xC202, data);
    NRF_LOG_INFO("0xC202 = 0x%x", data[1]);
    seeya_read_reg(0xC203, data);
    NRF_LOG_INFO("0xC203 = 0x%x", data[1]);
    seeya_read_reg(0xC204, data);
    NRF_LOG_INFO("0xC204 = 0x%x", data[1]);
    seeya_read_reg(0xC205, data);
    NRF_LOG_INFO("0xC205 = 0x%x", data[1]);
    seeya_read_reg(0xC206, data);
    NRF_LOG_INFO("0xC206 = 0x%x", data[1]);
    seeya_read_reg(0xC207, data);
    NRF_LOG_INFO("0xC207 = 0x%x", data[1]);
    seeya_read_reg(0xC208, data);
    NRF_LOG_INFO("0xC208 = 0x%x", data[1]);
    nrf_delay_ms(200);
}

#define TWI_ADDRESSES   (127)
void 
detect_i2c_slave_devices(void)
{
    ret_code_t err_code;
    uint8_t address;
    uint8_t sample_data;
    bool detected_device = false;

    for (address = 1; address <= TWI_ADDRESSES; address++)
    {
        err_code = nrfx_twi_rx(&m_twi, address, &sample_data, sizeof(sample_data));
        if (err_code == NRF_SUCCESS)
        {
            detected_device = true;
            NRF_LOG_INFO("TWI device detected at address 0x%x.", address);
        }
        NRF_LOG_FLUSH();
    }

    if (!detected_device)
    {
        NRF_LOG_INFO("No device was found.");
        NRF_LOG_FLUSH();
    }
}

void 
seeya_oled_reg_read_write_test()
{
    uint8_t data[2];
    seeya_read_reg(0x2A01, data);
    NRF_LOG_INFO("Before write, XS[7:0]: 0x%02x", data[1]);
    seeya_write_reg(0x2A01, 0x0001);
    seeya_read_reg(0x2A01, data);
    NRF_LOG_INFO("After write, XS[7:0]: 0x%02x", data[1]);
    NRF_LOG_FLUSH();
}


void
seeya_oled_power_on_sequence(void)
{
    seeya_io_init();
    seeya_i2c_init();
    nrf_delay_ms(1000);
    
    seeya_oled_power_enable();
    nrf_delay_ms(1000);
    
//    seeya_oleda_reset();
//    nrf_delay_ms(1000);

//    seeya_oledb_reset();
//    nrf_delay_ms(1000);

//    seeya_oled_reg_read_write_test();
    i2c_slave_addr = OLEDA_I2C_SLAVE_ADDR;
    NRF_LOG_INFO("############## Configure address 0x4C: #############");
    seeya_oled_reg_init(false);
    nrf_delay_ms(10);
    i2c_slave_addr = OLEDB_I2C_SLAVE_ADDR;
    NRF_LOG_INFO("############## Configure address 0x4D: #############");
    seeya_oled_reg_init(true);
    
    NRF_LOG_FLUSH();
}

