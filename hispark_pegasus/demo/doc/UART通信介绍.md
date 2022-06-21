# UART通信介绍

| API                                                          | 接口说明                 |
| ------------------------------------------------------------ | ------------------------ |
| unsigned int IoTUartInit(unsigned int id, const IotUartAttribute *param); | 初始化，配置一个UART设备 |
| int IoTUartRead(unsigned int id, unsigned char *data, unsigned int dataLen); | 从UART设备中读取数据     |
| int IoTUartWrite(unsigned int id, const unsigned char *data, unsigned int dataLen); | 将数据写入UART设备       |

-   例如在 Hi3861 上外接一个 GPS 模块，GPS 模块跟 Hi3861 之间使用 UART 通讯方式，GPS 模块上电 后会通过串口将经纬度及卫星个数等数据发送给 Hi3861。首先需要进行 GPIO 引脚初始化和引脚复 用、Hi3861 上的 UART 通道选择（Hi3861 一共有 3 路串口，其中 UART0 作为 log 调试口，剩余的 UART1 和 UART2 供用户使用），在本章的最后附上 Hi3861 引脚复用关系表。
-   在 app_io_init.c 中将 GPIO 引脚复用为串口。下面以实际的 GPS 模块和 Hi3861 通信，使用 UART1 通道进行通信，首先在 app_io_init.c 中将 GPIO 引脚复用为 UART1 的 TX 和 RX。通过查阅[Hi3861V100／Hi3861LV100／Hi3881V100 WiFi芯片 用户指南](https://gitee.com/openharmony/device_soc_hisilicon/blob/master/hi3861v100/doc/Hi3861V100%EF%BC%8FHi3861LV100%EF%BC%8FHi3881V100%20WiFi%E8%8A%AF%E7%89%87%20%E7%94%A8%E6%88%B7%E6%8C%87%E5%8D%97.pdf)，可以 复用为 UART1 的 GPIO 引脚有两组，分别是 GPIO0（UART1 的 TX）、GPIO1（UART1 的 RX），另一 组为 GPIO5（UART1 的 RX）、GPIO6（UART1 的 TX）。需要注意的是 GPIO6 引脚会影响 Hi3861 的启 动和烧录，所以一般情况下不使用 GPIO5 和 GPIO6 作为 UART1 使用，而是选择 GPIO0 和 GPIO1 作为 UART1 使用，所以本案例将使用 GPIO0 和 GPIO1 作为 UART1 使用。
```
#ifdef ROBOT_BOARD
    IoSetFunc(HI_IO_NAME_GPIO_5, IOT_IO_FUNC_GPIO_5_UART1_RXD);
    IoSetFunc(HI_IO_NAME_GPIO_6, IOT_IO_FUNC_GPIO_6_UART1_TXD);
    /* IOT_BOARD */
#elif defined (EXPANSION_BOARD)
    IoSetFunc(HI_IO_NAME_GPIO_0, IOT_IO_FUNC_GPIO_0_UART1_TXD);
    IoSetFunc(HI_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_UART1_RXD);
#endif
```

-   UART 初始化配置及处理流程 在上述步骤中已经将 GPIO 引脚复用关系配置好，接下来是 UART1 的初始化配置，使 UART1 功能正常 启动使用。步骤如下： 1. UART1 通讯前的基本配置，包括通讯波特率、数据位、停止位、奇偶校验位等一一进行配置 查阅 GPS 模块资料得知，GPS 模块 UART 通讯的波特率配置为 9600Hz，数据位为 8 位，停止位为 1，奇偶校验位为 0。所以 Hi3861 上的 UART1 设置要和 GPS 模块上的设置一致。 
    ```
    hi_uart_attribute uart_attr = {
        .baud_rate = 9600,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 0,
    }
    ```

-   Hi3861 UART 初始化，通道选择，将结构体配置信息配置好。
    ```
    ret = IoTUartInit(DEMO_UART_NUM, &uart_attr)
    ```

-   创建一个任务线程，单独处理串口收发任务，串口通信的具体任务实现。
    ```
    static void UartDemo(void)
    {
        osThreadAttr_t attr;
        attr.name = "UartDemoTask";
        attr.attr_bits = 0U;
        attr.cb_mem = NULL;
        attr.cb_size = 0U;
        attr.stack_mem = NULL;
        attr.stack_size = 4096; /* 任务大小4096 */
        attr.priority = osPriorityNormal;
        if (osThreadNew(UartDemoTask, NULL, &attr) == NULL) {
            printf("[UartDemo] Failed to create UartDemoTask!\n");
        }
    }
    SYS_RUN(UartDemo);
    ```
