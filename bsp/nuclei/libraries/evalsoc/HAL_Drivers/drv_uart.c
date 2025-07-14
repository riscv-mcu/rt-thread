/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-22     hqfang       First version
 * 2025-07-03     hqfang       Modify for evalsoc
 */

#include <drv_uart.h>

#ifdef RT_USING_SERIAL

#if !defined(BSP_USING_UART0)
    #error "Please define at least one BSP_USING_UARTx"
    /* this driver can be enabled at menuconfig -> 
    Hardware Drivers Config -> On-chip Peripheral Drivers -> Enable UART */
#endif

enum
{
#ifdef BSP_USING_UART0
    UART0_INDEX,
#endif
};

static void evalsoc_uart_isr(struct rt_serial_device *serial);
void eclic_uart0_handler(void);

#ifdef UART0_IRQn
#define SOC_UART0_IRQn UART0_IRQn
#else
#define SOC_UART0_IRQn SOC_INT51_IRQn
#endif

static struct evalsoc_uart_config uart_config[] =
{
#ifdef BSP_USING_UART0
    {
        "uart0",
        UART0,
        SOC_UART0_IRQn,
    },
#endif
};

static struct evalsoc_uart uart_obj[sizeof(uart_config) / sizeof(uart_config[0])] = {0};

static rt_err_t evalsoc_configure(struct rt_serial_device *serial,
                               struct serial_configure *cfg)
{
    struct evalsoc_uart *uart_obj;
    struct evalsoc_uart_config *uart_cfg;
    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart_obj = (struct evalsoc_uart *) serial->parent.user_data;
    uart_cfg = uart_obj->config;
    RT_ASSERT(uart_cfg != RT_NULL);

    uart_init(uart_cfg->uart, cfg->baud_rate);

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart_config_stopbit(uart_cfg->uart, UART_STOP_BIT_1);
        break;
    case STOP_BITS_2:
        uart_config_stopbit(uart_cfg->uart, UART_STOP_BIT_2);
        break;
    default:
        uart_config_stopbit(uart_cfg->uart, UART_STOP_BIT_1);
        break;
    }

    return RT_EOK;
}

static rt_err_t evalsoc_control(struct rt_serial_device *serial, int cmd,
                             void *arg)
{
    struct evalsoc_uart *uart_obj;
    struct evalsoc_uart_config *uart_cfg;

    RT_ASSERT(serial != RT_NULL);
    uart_obj = (struct evalsoc_uart *) serial->parent.user_data;
    uart_cfg = uart_obj->config;
    RT_ASSERT(uart_cfg != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        ECLIC_DisableIRQ(uart_cfg->irqn);
        uart_disable_rxint(uart_cfg->uart);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        ECLIC_EnableIRQ(uart_cfg->irqn);
        ECLIC_SetShvIRQ(uart_cfg->irqn, ECLIC_NON_VECTOR_INTERRUPT);
        ECLIC_SetLevelIRQ(uart_cfg->irqn, 1);
        uart_enable_rxint(uart_cfg->uart);
        break;
    }

    return RT_EOK;
}

static int evalsoc_putc(struct rt_serial_device *serial, char ch)
{
    struct evalsoc_uart *uart_obj;
    struct evalsoc_uart_config *uart_cfg;

    RT_ASSERT(serial != RT_NULL);
    uart_obj = (struct evalsoc_uart *) serial->parent.user_data;
    uart_cfg = uart_obj->config;
    RT_ASSERT(uart_cfg != RT_NULL);

    uart_write(uart_cfg->uart, ch);

    return 1;
}

static int evalsoc_getc(struct rt_serial_device *serial)
{
    int ch;
    uint32_t rxfifo;
    struct evalsoc_uart *uart_obj;
    struct evalsoc_uart_config *uart_cfg;

    RT_ASSERT(serial != RT_NULL);
    uart_obj = (struct evalsoc_uart *) serial->parent.user_data;
    uart_cfg = uart_obj->config;
    RT_ASSERT(uart_cfg != RT_NULL);

    ch = -1;
    rxfifo = uart_cfg->uart->RXFIFO;
    if ((rxfifo & UART_RXFIFO_EMPTY) != UART_RXFIFO_EMPTY) {
        ch = (int)(uint8_t)(rxfifo);
    }
    return ch;
}

static const struct rt_uart_ops evalsoc_uart_ops = { evalsoc_configure, evalsoc_control,
           evalsoc_putc, evalsoc_getc,
           RT_NULL
};

static void evalsoc_uart_isr(struct rt_serial_device *serial)
{
    struct evalsoc_uart *uart_obj;
    struct evalsoc_uart_config *uart_cfg;

    RT_ASSERT(serial != RT_NULL);
    uart_obj = (struct evalsoc_uart *) serial->parent.user_data;
    uart_cfg = uart_obj->config;
    RT_ASSERT(uart_cfg != RT_NULL);

    if (uart_cfg->uart->IP & UART_IP_RXIP_MASK) {
        uart_clear_status(uart_cfg->uart, UART_IP_RXIP_MASK);
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
    }
}

#ifdef BSP_USING_UART0

void eclic_uart0_handler(void)
{
    rt_interrupt_enter();

    evalsoc_uart_isr(&uart_obj[UART0_INDEX].serial);

    rt_interrupt_leave();
}

#endif


/* For Nuclei evalsoc Uart, when CPU freq is lower than 8M
   The uart read will only work on baudrate <= 57600.
   Nowadays, we usually distribute FPGA bitsteam with CPU Freq 16MHz */
#define DRV_UART_BAUDRATE       BAUD_RATE_115200

int rt_hw_uart_init(void)
{
    rt_size_t obj_num;
    int index;

    obj_num = sizeof(uart_obj) / sizeof(struct evalsoc_uart);
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = DRV_UART_BAUDRATE;
    rt_err_t result = 0;

    for (index = 0; index < obj_num; index++)
    {
        /* init UART object */
        uart_obj[index].config = &uart_config[index];
        uart_obj[index].serial.ops = &evalsoc_uart_ops;
        uart_obj[index].serial.config = config;

        /* register UART device */
        result = rt_hw_serial_register(&uart_obj[index].serial,
                                       uart_obj[index].config->name,
                                       RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                                       &uart_obj[index]);
        RT_ASSERT(result == RT_EOK);
    }
#ifdef BSP_USING_UART0
    ECLIC_SetVector(SOC_UART0_IRQn, (rv_csr_t)eclic_uart0_handler);
#endif

    return result;
}


#endif /* RT_USING_SERIAL */

/******************** end of file *******************/
