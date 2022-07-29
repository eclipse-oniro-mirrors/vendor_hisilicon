# I2C通信介绍<a name="ZH-CN_TOPIC_0000001130176841"></a>
| API名称                                                      | 说明                          |
| ------------------------------------------------------------ | ----------------------------- |
| IoTI2cInit(unsigned int id, unsigned int baudrate);         | 用指定的波特速率初始化I2C设备 |
| IoTI2cDeinit(unsigned int id);                               | 取消初始化I2C设备             |
| IoTI2cWrite(unsigned int id, unsigned short deviceAddr, const unsigned char *data, unsigned int dataLen); | 将数据写入I2C设备             |
| IoTI2cRead(unsigned int id, unsigned short deviceAddr, unsigned char *data, unsigned int dataLen); | 从I2C设备中读取数据           |
| IoTI2cSetBaudrate(unsigned int id, unsigned int baudrate);   | 设置I2C设备的波特率           |


-    例如在 Hi3861 上外接一个 OLED 屏，查阅资料知道，OLED 屏为 I2C 通讯方式，Hi3861 发送命令驱动 OLED 屏显示。
-    GPIO 初始化及引脚功能复用 为 I2C 模式。Hi3861 的 SOC 上一共有两路 I2C，分别是 I2C0 和 I2C1。Hi3861 上的 GPIO 引脚能复用为 I2C0 的有两组，分别为 GPIO9（I2C0_SCL）、 GPIO10（I2C0_SDA）和 GPIO13（I2C0_SDA）、GPIO14（I2C0_SCL）；GPIO 引脚能复用为 I2C1 的有 两组，分别为 GPIO0（I2C1_SDA）、GPIO1（I2C1_SCL）。本案例使用 GPIO13（I2C0_SDA）、 GPIO14（I2C0_SCL）这一组作为 OLED 屏和 Hi3861 进行 I2C 通信 。本案例复用GPIO13,GPIO14。
```
IoTGpioInit(13); 
IoSetFunc(13, 6); /* gpio13复用I2C0_SDA */
IoTGpioInit(14); /* 初始化gpio14 */
IoSetFunc(14, 6); /* gpio14复用I2C0_SCL */
```

-   I2C 初始化配置，包括通道选择：0，设置初始化波特率：OLED_I2C_BAUDRATE(400kbps)，Hi3861 最高波特率为400kbps。
    ```
    IoTI2cInit(0, OLED_I2C_BAUDRATE);
    ```

-   设置 OLED 屏的初始化，查阅 OLED 屏的 datasheet 可知 OLED 屏的初始化命令，Hi3861 通过 I2C 将 OLED 屏初始化命令发送给 OLED 屏。
    ```
    static const uint8_t initCmds[] = {
        0xAE, // --display off
        0x00, // ---set low column address
        0x10, // ---set high column address
        0x40, // --set start line address
        0xB0, // --set page address
        0x81, // contract control
        0xFF, // --128
        0xA1, // set segment remap
        0xA6, // --normal / reverse
        0xA8, // --set multiplex ratio(1 to 64)
        0x3F, // --1/32 duty
        0xC8, // Com scan direction
        0xD3, // -set display offset
        0x00,
        0xD5, // set osc division
        0x80,
        0xD8, // set area color mode off
        0x05,
        0xD9, // Set Pre-Charge Period
        0xF1,
        0xDA, // set com pin configuration
        0x12,
        0xDB, // set Vcomh
        0x30,
        0x8D, // set charge pump enable
        0x14,
        0xAF, // --turn on oled panel
    };
    ```

-   Hi3861 通过 I2C 发送显示信息给 OLED 屏，OLED 屏收到后显示在屏上。
    ```
    OledFillScreen(); /* 全屏填充 */
    OledShowString(20, 3, "Hello  World", 1); /* 屏幕第20列3行显示1行 */
    ```

-   上面步骤完成后还无法使用PWM，需要修改.device/soc/hisilicon/hi3861v100/sdk_liteos/build/config/usr_config.mk文件。在这个配置文件中打开PWM驱动宏。搜索字段CONFIG_I2C_SUPPORT ，并打开I2C。配置如下：
    ```
    # CONFIG_I2C_SUPPORT is not set
    CONFIG_I2C_SUPPORT=y
    ```

-   创建一个任务线程，单独处理OLED显示任务，串口通信的具体任务实现。
    ```
    static void OLEDDemo(void)
    {
        osThreadAttr_t attr;
        attr.name = "UartDemoTask";
        attr.attr_bits = 0U;
        attr.cb_mem = NULL;
        attr.cb_size = 0U;
        attr.stack_mem = NULL;
        attr.stack_size = 4096; /* 任务大小4096 */
        attr.priority = osPriorityNormal;
        if (osThreadNew(OLEDDemoTask, NULL, &attr) == NULL) {
            printf("[OledDemo] Failed to create OLEDDemoTask!\n");
        }
    }
    SYS_RUN(OLEDDemo);
    ```
