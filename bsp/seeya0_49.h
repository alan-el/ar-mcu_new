#ifndef SEEYA_0_49_H
#define SEEYA_0_49_H

#define OLEDA_RESET_PIN     (2)
#define OLEDB_RESET_PIN     (3)
#define OLED_I2C_SDA_PIN    (4)
#define OLED_I2C_SCL_PIN    (5)

#define OLEDA_OCP_OUT_PIN   (9)
#define OLEDA_GPIO_PIN      (8)
#define OLEDB_OCP_OUT_PIN   (7)
#define OLEDB_GPIO_PIN      (6)

#define OLEDA_PWR_EN        (12)
#define OLEDB_PWR_EN        (18)

#define OLEDA_I2C_SWITCH    (16)
#define OLEDB_I2C_SWITCH    (15)

#define POWERCHIP_I2C_SLAVE_ADDR    (0x3E)

#define OLEDA_I2C_SLAVE_ADDR (0x4C) // Right eye
#define OLEDB_I2C_SLAVE_ADDR (0x4D)

void seeya_set_i2c_slave_addr(uint8_t addr);
void seeya_oled_power_on_sequence(void);
void jdf_oled_sleep(uint8_t which);
void jdf_oled_wake_up(uint8_t which);
void jdf_oled_brightness_set(int value);
void seeya_write_reg(const uint16_t reg_addr, const uint16_t data);
#endif
