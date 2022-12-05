#ifndef SEEYA_0_49_H
#define SEEYA_0_49_H

#define OLEDA_RESET_PIN  (2)
#define OLEDB_RESET_PIN  (3)
#define OLED_I2C_SDA_PIN (4)
#define OLED_I2C_SCL_PIN (5)
#define OLEDA_OCP_OUT_PIN    (6)
#define OLEDA_GPIO_PIN   (7)
#define OLEDB_OCP_OUT_PIN    (17)
#define OLEDB_GPIO_PIN   (16)
#define OLED_PWR_EN      (12)

#define OLEDA_I2C_SLAVE_ADDR (0x4C)
#define OLEDB_I2C_SLAVE_ADDR (0x4D)

void seeya_oled_power_on_sequence(void);


#endif
