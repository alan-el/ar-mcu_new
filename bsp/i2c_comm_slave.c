#include "nrfx_twis.h"
#include "nrf_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "i2c_comm_slave.h"
#include "seeya0_49.h"
#include "lt7911.h"
#include "sony_ecx348ena.h"

#define TWIS_INSTANCE_ID 1
#define I2C_COMM_SLAVE_ADDR 0x27
#define I2C_COMM_SLAVE_SCL  13
#define I2C_COMM_SLAVE_SDA  14

#define EVENT_READ_QUEUE_MAX_LEN  5

static const nrfx_twis_t m_twis = NRFX_TWIS_INSTANCE(TWIS_INSTANCE_ID);

static bool m_error_flag = false;

uint8_t read_buf[EVENT_READ_QUEUE_MAX_LEN * I2C_COMM_MSG_LEN];
i2c_comm_msg_t write_data;

i2c_comm_msg_t event_read_queue[EVENT_READ_QUEUE_MAX_LEN];
i2c_comm_msg_t *erq_head = event_read_queue;
i2c_comm_msg_t *erq_rear = event_read_queue;
bool is_queue_empty = true;

/* 进队 */
int32_t i2c_comm_event_read_queue_in(i2c_comm_msg_t msg)
{
    if(!is_queue_empty && erq_head == erq_rear)
    {
        NRF_LOG_INFO("i2c_comm_event_read_queue_in: queue is full!");
        return -1;
    }
    else
    {
        *erq_rear = msg;
        erq_rear == (event_read_queue + EVENT_READ_QUEUE_MAX_LEN - 1) ? \
        erq_rear = event_read_queue : \
        erq_rear++;

        is_queue_empty = false;
    }
    return 0;
}

/* 出队 */
int32_t i2c_comm_event_read_queue_out(i2c_comm_msg_t *pmsg)
{
    if(is_queue_empty)
    {
        NRF_LOG_INFO("i2c_comm_event_read_queue_out: queue is empty!");
        return -1;
    }
    else
    {
        *pmsg = *erq_head;
        erq_head == (event_read_queue + EVENT_READ_QUEUE_MAX_LEN - 1) ? \
        erq_head = event_read_queue : \
        erq_head++;

        if(erq_head == erq_rear)
            is_queue_empty = true;
    }
    return 0;
}

/* 这里的 read/write 的主语是主机 (master) */
void i2c_comm_slave_read_begin(void)
{
    ret_code_t ret;
    uint8_t *pread_buf = read_buf;
    int msg_num = 0;
    i2c_comm_msg_t msg;
    
    while(0 == i2c_comm_event_read_queue_out(&msg))
    {
        memcpy(pread_buf, (uint8_t *)&msg, sizeof(i2c_comm_msg_t));
        pread_buf += sizeof(i2c_comm_msg_t);
        msg_num++;
    }
    
    ret = nrfx_twis_tx_prepare(&m_twis, read_buf, msg_num * sizeof(i2c_comm_msg_t));
    APP_ERROR_CHECK(ret);
}

void i2c_comm_slave_read_end(uint32_t count)
{
    NRF_LOG_INFO("i2c_comm_slave: send %d byte(s)", count);
}

void i2c_comm_slave_write_begin(void)
{
    ret_code_t ret;
    ret = nrfx_twis_rx_prepare(&m_twis, &write_data, sizeof(i2c_comm_msg_t));
    APP_ERROR_CHECK(ret);
}

void i2c_comm_slave_write_end(uint32_t count)
{
    if (count < sizeof(i2c_comm_msg_t))
    {
        NRF_LOG_INFO("Error: Recive i2c msg bytes num error: %d\n", count);
    }
    else if (write_data.start_code != I2C_COMM_MSG_START_CODE_W)
    {
        NRF_LOG_INFO("Error: Recive i2c msg start code error: %d\n", write_data.start_code);
    }
    else
    {
        switch (write_data.cmd_type)
        {
        case CMD_OLED_SLEEP:
#ifdef SONY_OLED
            ecx348ena_sleep();
#else
            jdf_oled_sleep(write_data.data[0]);
#endif
            break;
        case CMD_OLED_WAKE_UP:
#ifdef SONY_OLED
            ecx348ena_wake_up();
#else
            jdf_oled_wake_up(write_data.data[0]);
#endif
            break;
        case CMD_OLED_SET_BRIGHTNESS:
            /* data_len */
            if (write_data.data_len != 1)
            {
                NRF_LOG_INFO("Error: CMD_OLED_SET_BRIGHTNESS data_len != 1\n");
            }
            /* data[0] */
#ifdef SONY_OLED
            ecx348ena_brightness_set(write_data.data[0]);
#else
            jdf_oled_brightness_set(write_data.data[0]);
#endif
            break;
        case CMD_OLED_POWEROFF_LT7911:
            lt7911_poweroff();
            NRF_LOG_INFO("LT7911 poweroff!\n");
            break;
        case CMD_OLED_REG_CFG:
        {
            uint8_t start_addr, index, which;
            uint16_t addr, reg_value;
            
            if (write_data.data_len < 2)
            {
                NRF_LOG_INFO("Error: CMD_OLED_REG_CFG data_len < 2\n");
            }
            else
            {
                which = write_data.data[0];
                start_addr = write_data.data[1];
//                NRF_LOG_INFO("which = %d\n", which);
//                NRF_LOG_INFO("start_addr = %d\n", start_addr);
                NRF_LOG_INFO("write_data.data_len = %d\n", write_data.data_len);
                
                if (which & 0x01)
                {
                    seeya_set_i2c_slave_addr(OLEDA_I2C_SLAVE_ADDR);
                    
                    if (write_data.data_len == 2)
                    {
                        addr = (uint16_t)start_addr << 8;
                        seeya_write_reg(addr, 0x0000);
                    }
                    else
                    {
                        
                        for (index = 2; index < write_data.data_len; index++)
                        {
                            addr = ((uint16_t)start_addr << 8) + (index - 2);
                            reg_value = write_data.data[index];
                            seeya_write_reg(addr, reg_value);

                        }
                    }
                }
                
                if (which & 0x02)
                {
                    seeya_set_i2c_slave_addr(OLEDB_I2C_SLAVE_ADDR);
                    
                    if (write_data.data_len == 2)
                    {
                        addr = (uint16_t)start_addr << 8;
                        seeya_write_reg(addr, 0x0000);
                    }
                    else
                    {
                        for (index = 2; index < write_data.data_len; index++)
                        {
                            addr = ((uint16_t)start_addr << 8) + (index - 2);
                            reg_value = write_data.data[index];
                            seeya_write_reg(addr, reg_value);
                        }
                    }
                }
            }
        }
            break;
        default:
            break;
        }
        
        /* TODO: echo data to master for debug, need disable in release version */
//        write_data.start_code = I2C_COMM_MSG_START_CODE_R;
//        i2c_comm_event_read_queue_in(write_data);
        
        NRF_LOG_INFO("Recive i2c msg : %d\n", write_data.cmd_type);
    }
}

/* TODO */
void i2c_comm_slave_trans_error_handle(void)
{
    NRF_LOG_INFO("i2c_comm_slave: error occured");
}
static void twis_event_handler(nrfx_twis_evt_t const * const p_event)
{
    switch (p_event->type)
    {
    case NRFX_TWIS_EVT_READ_REQ:
        if (p_event->data.buf_req)
        {
            i2c_comm_slave_read_begin();
        }
        break;
    case NRFX_TWIS_EVT_READ_DONE:
        i2c_comm_slave_read_end(p_event->data.tx_amount);
        break;
    case NRFX_TWIS_EVT_WRITE_REQ:
        if (p_event->data.buf_req)
        {
            i2c_comm_slave_write_begin();
        }
        break;
    case NRFX_TWIS_EVT_WRITE_DONE:
        i2c_comm_slave_write_end(p_event->data.rx_amount);
        break;

    case NRFX_TWIS_EVT_READ_ERROR:
    case NRFX_TWIS_EVT_WRITE_ERROR:
    case NRFX_TWIS_EVT_GENERAL_ERROR:
        m_error_flag = true;
        i2c_comm_slave_trans_error_handle();
        break;
    default:
        break;
    }
}

void i2c_comm_slave_init(void)
{
    ret_code_t ret;
    const nrfx_twis_config_t config =
    {
        .addr               = {I2C_COMM_SLAVE_ADDR, 0},
        .scl                = I2C_COMM_SLAVE_SCL,
        .scl_pull           = NRF_GPIO_PIN_PULLUP,
        .sda                = I2C_COMM_SLAVE_SDA,
        .sda_pull           = NRF_GPIO_PIN_PULLUP,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };

    do
    {
        ret = nrfx_twis_init(&m_twis, &config, twis_event_handler);
        if (NRF_SUCCESS != ret)
        {
            break;
        }

        nrfx_twis_enable(&m_twis);
    }while (0);

    APP_ERROR_CHECK(ret);
}

