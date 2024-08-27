#ifndef LT7911_H
#define LT7911_H

#define LT7911_GPIO5_PIN    (11)
#define LT7911_RESET_PIN    (15)

void lt7911_io_init(void);
void lt7911_reset(void);
void lt7911_poweroff(void);

#endif

