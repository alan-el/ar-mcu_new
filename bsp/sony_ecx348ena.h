#ifndef SONY_ECX348ENA_H
#define SONY_ECX348ENA_H

#include "stdint.h"

#define OLED_PWR_PIN (12)
#define OLEDA_RESET_PIN (2)
#define OLEDB_RESET_PIN (3)
#define OLEDA_SPI_CS_PIN (8)
#define OLEDB_SPI_CS_PIN (4)
#define OLED_SPI_CLK_PIN (7)
#define OLED_SPI_MOSI_PIN (6)
#define OLED_SPI_MISO_PIN (5)

typedef enum
{
    OLED_ID_A = 0,
    OLED_ID_B,
} oled_id_t;

typedef struct
{
    uint8_t addr;
    uint8_t len;
    uint8_t *data;
} ecx348ena_reg_wt_t;

void ecx348ena_regs_init(oled_id_t id);
void ecx348ena_io_init(void);
void ecx348ena_power_on_and_reset(void);
void ecx348ena_brightness_set(int level);
void ecx348ena_sleep(void);
void ecx348ena_wake_up(void);

#endif /* SONY_ECX348ENA_H */
