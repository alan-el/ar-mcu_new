#include "nrfx_twis.h"
#include "nrf_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "i2c_comm_slave.h"

#define TWIS_INSTANCE_ID 1
#define I2C_COMM_SLAVE_ADDR 0x27
#define I2C_COMM_SLAVE_SCL  13
#define I2C_COMM_SLAVE_SDA  14

#define WRITE_BUF_MAX_LEN   32
#define EVENT_READ_QUEUE_MAX_LEN  5

static const nrfx_twis_t m_twis = NRFX_TWIS_INSTANCE(TWIS_INSTANCE_ID);

static bool m_error_flag = false;

uint8_t read_buf[EVENT_READ_QUEUE_MAX_LEN * I2C_COMM_MSG_LEN];
uint8_t write_data[WRITE_BUF_MAX_LEN];

/* 主机读取事件队列 TODO 待测试*/
uint8_t event_read_queue[EVENT_READ_QUEUE_MAX_LEN];
uint8_t *erq_head = event_read_queue;
uint8_t *erq_rear = event_read_queue;
bool is_queue_empty = true;

/* 进队 */
int32_t i2c_comm_event_read_queue_in(uint8_t event)
{
    if(!is_queue_empty && erq_head == erq_rear)
    {
        NRF_LOG_INFO("i2c_comm_event_read_queue_in: queue is full!");
        return -1;
    }
    else
    {
        *erq_rear = event;
        erq_rear == (event_read_queue + EVENT_READ_QUEUE_MAX_LEN - 1) ? \
        erq_rear = event_read_queue : \
        erq_rear++;

        is_queue_empty = false;
    }
    return 0;
}

/* 出队 */
int32_t i2c_comm_event_read_queue_out(uint8_t *pevent)
{
    if(is_queue_empty)
    {
        NRF_LOG_INFO("i2c_comm_event_read_queue_in: queue is empty!");
        return -1;
    }
    else
    {
        *pevent = *erq_head;
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
    int event_num = 0;
    uint8_t event;
    
    while(0 == i2c_comm_event_read_queue_out(&event))
    {
        i2c_comm_msg_t msg = 
        {
            .start_code = I2C_COMM_MSG_START_CODE,
            .cmd_type = event
        };

        memcpy(pread_buf, (uint8_t *)&msg, I2C_COMM_MSG_LEN);
        
        pread_buf += I2C_COMM_MSG_LEN;
        event_num++;
    }
    
    ret = nrfx_twis_tx_prepare(&m_twis, read_buf, event_num * I2C_COMM_MSG_LEN);
    APP_ERROR_CHECK(ret);
}

void i2c_comm_slave_read_end(uint32_t count)
{
    NRF_LOG_INFO("i2c_comm_slave: send %d byte(s)", count);
}

void i2c_comm_slave_write_begin(void)
{
    ret_code_t ret;
    ret = nrfx_twis_rx_prepare(&m_twis, write_data, sizeof(write_data));
    APP_ERROR_CHECK(ret);
}

void i2c_comm_slave_write_end(uint32_t count)
{
    NRF_LOG_INFO("i2c_comm_slave: receive %d byte(s):", count);
    
    for(int i = 0; i < count; i++)
        NRF_LOG_INFO("i2c_comm_slave: 0x%02x", write_data[i]);
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

