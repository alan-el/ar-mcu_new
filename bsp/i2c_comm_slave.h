#ifndef I2C_COMM_SLAVE_H
#define I2C_COMM_SLAVE_H

#define I2C_COMM_MSG_START_CODE_R (0x12)
#define I2C_COMM_MSG_START_CODE_W (0x13)
#define I2C_COMM_MSG_LEN    (sizeof(i2c_comm_msg_t))

/* 单片机发给主机的数据包结构体 */
typedef struct 
{
    uint8_t start_code;     // 起始码
    uint8_t cmd_type;       // 命令类型对应
    uint8_t data_len;       // 数据长度
    uint8_t dummy;
    uint8_t data[64];      // 数据内容
}i2c_comm_msg_t;

typedef enum
{
    CMD_OLED_SLEEP = 0,
    CMD_OLED_WAKE_UP,
    CMD_OLED_SET_BRIGHTNESS,
    CMD_OLED_POWEROFF_LT7911,
    CMD_OLED_REG_CFG,
} cmd_type_t;

void i2c_comm_slave_init(void);
int32_t i2c_comm_event_read_queue_in(i2c_comm_msg_t msg);

#endif /* I2C_COMM_SLAVE_H */

