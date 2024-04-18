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
    
}i2c_comm_msg_t;

typedef enum
{
    CMD_OLED_SLEEP = 0,
    CMD_OLED_WAKE_UP,
} cmd_type_t;

void i2c_comm_slave_init(void);
int32_t i2c_comm_event_read_queue_in(uint8_t event);

#endif /* I2C_COMM_SLAVE_H */

