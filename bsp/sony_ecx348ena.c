#include "nrf_drv_spi.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "sony_ecx348ena.h"

#define SPI_INSTANCE  2 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

#define MAX_TRANSFER_SIZE 255
static uint8_t m_tx_buf[MAX_TRANSFER_SIZE] = {0}; /**< TX buffer. */
static uint8_t m_rx_buf[MAX_TRANSFER_SIZE] = {0}; /**< RX buffer. */

void ecx348ena_io_init(void)
{
    nrf_gpio_cfg_output(OLEDA_RESET_PIN);
    nrf_gpio_pin_write(OLEDA_RESET_PIN, 0);
    nrf_gpio_cfg_output(OLEDB_RESET_PIN);
    nrf_gpio_pin_write(OLEDB_RESET_PIN, 0);
    
    nrf_gpio_cfg_output(OLED_PWR_PIN);
    nrf_gpio_pin_write(OLED_PWR_PIN, 0);
}

void ecx348ena_power_on_and_reset(void)
{
    nrf_gpio_pin_write(OLED_PWR_PIN, 1);
    nrf_delay_ms(100);
    nrf_gpio_pin_write(OLEDA_RESET_PIN, 1);
    nrf_delay_ms(100);
    nrf_gpio_pin_write(OLEDB_RESET_PIN, 1);
}

void ecx348ena_spi_init(oled_id_t id)
{
    uint8_t oled_ss_pin = 0;
    
    if (id == OLED_ID_A)
    {
        oled_ss_pin = OLEDA_SPI_CS_PIN;
    }
    else if (id == OLED_ID_B)
    {
        oled_ss_pin = OLEDB_SPI_CS_PIN;
    }
    else
    {
        NRF_LOG_ERROR("Invalid OLED ID.");
    }
    
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = oled_ss_pin;
    spi_config.miso_pin = OLED_SPI_MISO_PIN;
    spi_config.mosi_pin = OLED_SPI_MOSI_PIN;
    spi_config.sck_pin  = OLED_SPI_CLK_PIN;
    spi_config.frequency = NRF_DRV_SPI_FREQ_125K;
    spi_config.mode = NRF_DRV_SPI_MODE_3;
    spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL));
}

void ecx348ena_spi_uninit(void)
{
    nrf_drv_spi_uninit(&spi);
}

void ecx348ena_write_regs(ecx348ena_reg_wt_t *reg_wt)
{
    if (reg_wt->len <= 254)
    {
        m_tx_buf[0] = reg_wt->addr;
        memcpy(m_tx_buf + 1, reg_wt->data, reg_wt->len);
    }
    else
    {
        NRF_LOG_ERROR("Invalid data length.");
        return; 
    }
    
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, reg_wt->len + 1, NULL, 0));
}

void ecx348ena_enable_reg_read(void)
{
    /* Set Read mode on */
    uint8_t rd_enable = 0x01;
    ecx348ena_reg_wt_t reg_read_mode = {0x80, 1, &rd_enable};
    ecx348ena_write_regs(&reg_read_mode);
}

/* Just for debug */
void ecx348ena_read_regs(uint8_t addr, uint32_t len)
{
    /* 1st step: set (start) register to read */
    ecx348ena_reg_wt_t reg_read_addr = {0x81, 1, &addr};
    ecx348ena_write_regs(&reg_read_addr);
    
    uint8_t dummy = 0x81;
    /* 2nd step: read register(s) */
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, &dummy, 1, m_rx_buf, 1 + len));
    
    for (int i = 0; i < len; i++)
    {
        NRF_LOG_INFO("ecx348ena_read_regs [0x%02x] = 0x%02x", addr + i, m_rx_buf[i + 1]);
    }
    
    NRF_LOG_FLUSH();
}

/* 60p 4Lx2 registers configuration. Addr [0x01] to [0xBF] */

/* 50Hz 0x20, 0x30, 0x10, ! 50Hz */
/* 60Hz 0x10, 0x0C, 0xEC, ! 60Hz */
static uint8_t regs_val_wt[] = 
{
    0x00, 0x8C, 0x58, 0x03, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x44, 0x00, 0x0F, 0x04, 0x00, 0x0F, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xE0, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x80,
    0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00,
    0x00, 0x21, 0x01, 0x20, 0x30, 0x10, 0x40, 0x29, 0x61, 0x02,
    0x33, 0x01, 0x18, 0x01, 0x22, 0x34, 0x33, 0x04, 0x0D, 0x18,
    0x22, 0x35, 0x34, 0x04, 0x0D, 0x18, 0x00, 0x05, 0x78, 0x10, 
    0x78, 0x21, 0x78, 0x00, 0x13, 0x12, 0x11, 0x03, 0x00, 0xED, 
    0x05, 0x00, 0x17, 0xF9, 0x00, 0x09, 0xE8, 0x00, 0xEF, 0xF5, 
    0x10, 0x19, 0x11, 0x06, 0x16, 0x11, 0x0E, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
    0x03, 0x84, 0x12, 0x04, 0x01, 0xF4, 0x04, 0x65, 0xDA, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0B, 0x00, 
    0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x16, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01,
    0x23, 0x45, 0x67, 0x89, 0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x08
    
    /* Demo board: 120p, 5Lx2? */
//    0x00, 0x8C, 0xD9, 0x05, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x10, 0x44, 0x00, 0x0F, 0x04, 0x00, 0x0F, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//    0x00, 0x00, 0xE0, 0x80, 0x80, 0x80, 0x40, 0x40, 0x40, 0x80,
//    0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0x00,
//    0x00, 0x21, 0x01, 0x10, 0x0C, 0xEC, 0x40, 0x29, 0x61, 0x02,
//    0x33, 0x01, 0x18, 0x01, 0x22, 0x34, 0x33, 0x04, 0x0D, 0x18, 
//    0x22, 0x35, 0x34, 0x04, 0x0D, 0x18, 0x00, 0x05, 0x78, 0x19, 
//    0x78, 0x21, 0x78, 0x00, 0x13, 0x12, 0x1F, 0x06, 0x01, 0xD5, 
//    0x0B, 0x10, 0x2A, 0xED, 0x10, 0x0E, 0xC9, 0x11, 0xD8, 0xE5, 
//    0x10, 0x32, 0x21, 0x0B, 0x2C, 0x11, 0x1C, 0x00, 0x00, 0x00, 
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
//    0x03, 0x84, 0x12, 0x04, 0x01, 0xF4, 0x02, 0x65, 0xDA, 0x00,
//    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0B, 0x00, 
//    0x00, 0x00, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//    0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x16, 0x00, 0x00, 
//    0x08, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 
//    0x23, 0x45, 0x67, 0x89, 0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 
//    0x08
};

void ecx335sn_write_reg(uint8_t reg_addr, uint8_t val)
{
    uint8_t buf[2];
    buf[0] = reg_addr;
    buf[1] = val;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, buf, 2, NULL, 0));
}

void ecx335sn_enable_reg_read(void)
{
    uint8_t buf[2] = {0x80, 0x01};

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, buf, 2, NULL, 0));
}

uint8_t ecx335sn_read_reg(uint8_t reg_addr)
{
    uint8_t buf[2];
    buf[0] = 0x81;
    buf[1] = reg_addr;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, buf, 2, NULL, 0));

    uint8_t val[2] = {1, 1};
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, buf, 1, val, 2));

    return val[1];
}
/* Divide 400 (100 - 500) into 75 levels, user can set brightness 
 * between level 25 to level 100. level 25 = 1000 cd/m^2, level
 * 100 = 5000 cd/m^2
 */
static int s_brightness_level = 100;

/* Power saving transition */
void ecx348ena_sleep(void)
{
    ecx348ena_reg_wt_t regs_wt = {0};
    uint8_t ps_mode;
    regs_wt.len = 1;
    regs_wt.data = &ps_mode;
    
    /* OLED A */
    ecx348ena_spi_uninit();
    ecx348ena_spi_init(OLED_ID_A);
    
    regs_wt.addr = 0x00;
    ps_mode = 0x00;
    ecx348ena_write_regs(&regs_wt);
    
    nrf_delay_ms(10);
    
    /* OLED B */
    ecx348ena_spi_uninit();
    ecx348ena_spi_init(OLED_ID_B);
    
    regs_wt.addr = 0x00;
    ps_mode = 0x00;
    ecx348ena_write_regs(&regs_wt);
}

/* Power saving release */
void ecx348ena_wake_up(void)
{
    ecx348ena_reg_wt_t regs_wt = {0};
    uint8_t ps_mode;
    regs_wt.len = 1;
    regs_wt.data = &ps_mode;
    
    /* OLED A */
    ecx348ena_spi_uninit();
    ecx348ena_spi_init(OLED_ID_A);
    
    regs_wt.addr = 0x00;
    ps_mode = 0x01;
    ecx348ena_write_regs(&regs_wt);
    
    nrf_delay_us(500);
    
    ps_mode = 0x07;
    ecx348ena_write_regs(&regs_wt);
    
    nrf_delay_ms(10);
    
    /* OLED B */
    ecx348ena_spi_uninit();
    ecx348ena_spi_init(OLED_ID_B);
    
    regs_wt.addr = 0x00;
    ps_mode = 0x01;
    ecx348ena_write_regs(&regs_wt);
    
    nrf_delay_us(500);
    
    ps_mode = 0x07;
    ecx348ena_write_regs(&regs_wt);
}

void ecx348ena_brightness_set(int level)
{
    if (level < 25 || level > 100)
    {
        /* Error value */
    }
    else
    {
        s_brightness_level = level;
        uint16_t brt_value = 100 + (level - 25) * ((500 - 100) / (100 - 25));
        
        ecx348ena_reg_wt_t regs_wt = {0};
        uint8_t data;
        regs_wt.len = 1;
        regs_wt.data = &data;
        
        /* OLED A */
        ecx348ena_spi_uninit();
        ecx348ena_spi_init(OLED_ID_A);

        regs_wt.addr = 0x12;
        data = brt_value / 0x100;
        ecx348ena_write_regs(&regs_wt);
        
        regs_wt.addr = 0x13;
        data = brt_value % 0x100;
        ecx348ena_write_regs(&regs_wt);
        
        nrf_delay_ms(10);
        
        /* OLED B */
        ecx348ena_spi_uninit();
        ecx348ena_spi_init(OLED_ID_B);
        
        regs_wt.addr = 0x12;
        data = brt_value / 0x100;
        ecx348ena_write_regs(&regs_wt);
        
        regs_wt.addr = 0x13;
        data = brt_value % 0x100;
        ecx348ena_write_regs(&regs_wt);
        
        nrf_delay_ms(10);
    }
}

void ecx348ena_regs_init(oled_id_t id)
{
    ecx348ena_spi_uninit();
    ecx348ena_spi_init(id);
    nrf_delay_ms(20);

    NRF_LOG_INFO("Data length = %d\n", sizeof(regs_val_wt) / sizeof(uint8_t));
    ecx348ena_reg_wt_t regs_wt = {0};
    /* PS0 */
    /* Registers setting */
    regs_wt.addr = 0x01;
    regs_wt.len = sizeof(regs_val_wt) / sizeof(uint8_t);
    regs_wt.data = regs_val_wt;
    ecx348ena_write_regs(&regs_wt);
    
    ecx348ena_enable_reg_read();
    ecx348ena_read_regs(0x01, sizeof(regs_val_wt) / sizeof(uint8_t));
    
    /* Capture from Demo board */
    nrf_delay_ms(100);
    
    uint8_t data;
    /* Direct duty setting (can change luminance by setting this one) */
    regs_wt.addr = 0x8B;
    regs_wt.len = 1;
    data = 0xDA; // 0xE3?
    regs_wt.data = &data;
    ecx348ena_write_regs(&regs_wt);
    nrf_delay_ms(500);
    
    regs_wt.addr = 0x04;
    data = 0x03;
    ecx348ena_write_regs(&regs_wt);
    nrf_delay_ms(8);
    
    /* Unused configurations fold... */
    
    /* Luminance setting: max = 0x01, 0xF4 (5000cd/m2); min = 0x00, 0x64 (1000cd/m2) */
//    if (id == OLED_ID_B)
//    {
        regs_wt.addr = 0x12;
        data = 0x01;
        ecx348ena_write_regs(&regs_wt);
        
        regs_wt.addr = 0x13;
        data = 0xF4;
        ecx348ena_write_regs(&regs_wt);
//    }
//    else if (id == OLED_ID_A)
//    {
//        regs_wt.addr = 0x12;
//        data = 0x00;
//        ecx348ena_write_regs(&regs_wt);
//        
//        regs_wt.addr = 0x13;
//        data = 0x64;
//        ecx348ena_write_regs(&regs_wt);
//    }
    
    nrf_delay_ms(10);
    
    /* Make sure LVDS signals has input before PS0 release */
    /* PS1 (PS0 release) */
    uint8_t ps_mode = 0x01;
    regs_wt.addr = 0x00;
    regs_wt.len = 1;
    regs_wt.data = &ps_mode;
    ecx348ena_write_regs(&regs_wt);
//    ecx348ena_write_regs(&regs_wt);
    
    nrf_delay_ms(1);
    
    /* PS2 (PS1 release) */
    ps_mode = 0x03;
    ecx348ena_write_regs(&regs_wt);
//    ecx348ena_write_regs(&regs_wt);
    
    nrf_delay_ms(1);
    
    /* Change to normal state (PS2 release) */
    ps_mode = 0x07;
    ecx348ena_write_regs(&regs_wt);
//    ecx348ena_write_regs(&regs_wt);
    
    nrf_delay_ms(8);

//    regs_wt.addr = 0x05;
//    regs_wt.len = 1;
//    data = 0x00;
//    regs_wt.data = &data;
//    ecx348ena_write_regs(&regs_wt);

//    regs_wt.addr = 0x06;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
    
    ecx348ena_enable_reg_read();
    ecx348ena_read_regs(0x40, 3);
    NRF_LOG_FLUSH();
}

//    regs_wt.addr = 0x05;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x06;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x07;
//    data = 0x10;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x08;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x09;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
    
//    regs_wt.addr = 0x0A;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
    
//    regs_wt.addr = 0x0B;
//    data = 0x01;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
    
//    regs_wt.addr = 0x0C;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
    
//    regs_wt.addr = 0x0D;
//    data = 0x10;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x0E;
//    data = 0x44;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
    
//    regs_wt.addr = 0x0F;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x10;
//    data = 0x0F;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x11;
//    data = 0x04;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x12;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x13;
//    data = 0x0F;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x14;
//    data = 0x00;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
    
//    regs_wt.addr = 0x15;
//    data = 0x11;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x16;
//    data = 0x19;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x17;
//    data = 0x32;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x18;
//    data = 0x39;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x19;
//    data = 0x37;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x1A;
//    data = 0x15;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x1B;
//    data = 0x11;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x1C;
//    data = 0x0F;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x1D;
//    data = 0x0D;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x1E;
//    data = 0x2E;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x1F;
//    data = 0x3C;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x20;
//    data = 0x3A;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x21;
//    data = 0x35;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x22;
//    data = 0x1E;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x23;
//    data = 0x0E;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x24;
//    data = 0x18;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x25;
//    data = 0x31;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x26;
//    data = 0x3E;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x27;
//    data = 0x3D;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x28;
//    data = 0x36;
//    ecx348ena_write_regs(&regs_wt);
//    nrf_delay_ms(8);
//    
//    regs_wt.addr = 0x29;
//    data = 0x1E;
//    ecx348ena_write_regs(&regs_wt);
//        
//    nrf_delay_ms(80);
