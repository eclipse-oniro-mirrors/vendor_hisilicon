# PWM接口介绍
| API名称                                                      | 说明              |
| ------------------------------------------------------------ | ----------------- |
| unsigned int IoTPwmInit(WifiIotPwmPort port);                   | PWM模块初始化     |
| unsigned int IoTPwmStart(WifiIotPwmPort port, unsigned short duty, unsigned short freq); | 开始输出PWM信号   |
| unsigned int IoTPwmStop(WifiIotPwmPort port);                   | 停止输出PWM信号   |
| unsigned int IoTPwmDeinit(WifiIotPwmPort port);                 | 解除PWM模块初始化 |
| unsigned int PwmSetClock(WifiIotPwmClkSource clkSource);     | 设置PWM模块时钟源 |
-   将GPIO复用为PWM模式。Hi3861上一共有6路PWM。由于PWM通道太多就不一一列举了，PWM复用功能在SDK的hi_io.h中，详情用户可查阅该文件。使用交通灯板上的蜂鸣器，本案例复用GPIO5为PWM模式，使用前PWM初始化，包括通道选择，频率选择。
    ```
    IoTGpioInit(IOT_PWM_BEEP);
    IoSetFunc(IOT_PWM_BEEP, 5); /* 设置IO5的功能 */
    IoTGpioSetDir(IOT_PWM_BEEP, IOT_GPIO_DIR_OUT);
    IoTPwmInit(IOT_PWM_PORT_PWM0);  
    ```

-   PWM启动，包括通道选择，duty设置，分频系数设置。
    ```
    IoTPwmStart(IOT_PWM_PORT_PWM0, PWM_LOW_DUTY, PWM_FULL_DUTY); 
    ```

-   上图中的IOT_PWM_PORT_PWM0为复用通道，PWM_LOW_DUTY为占空比，范围（1-99），PWM_FULL_DUTY为频率，范围为（大于2442HZ），计算公式如下。

![输入图片说明](https://gitee.com/asd1122/tupian/raw/master/%E5%9B%BE%E7%89%87/%E5%9B%BE%E7%89%8779.png)

-   上面步骤完成后还无法使用PWM，需要修改.device/soc/hisilicon/hi3861v100/sdk_liteos/build/config/usr_config.mk文件。在这个配置文件中打开I2C驱动宏。搜索字段CONFIG_PWM_SUPPORT ，并打开PWM。配置如下：
    ```
    # CONFIG_PWM_SUPPORT is not set
    CONFIG_PWM_SUPPORT=y
    ```

-   创建一个任务线程，单独处理PWM控制蜂鸣器任务，串口通信的具体任务实现。
    ```
    static void PWMDemo(void)
    {
        osThreadAttr_t attr;
        attr.name = "PWMDemoTask";
        attr.attr_bits = 0U;
        attr.cb_mem = NULL;
        attr.cb_size = 0U;
        attr.stack_mem = NULL;
        attr.stack_size = 4096; /* 任务大小4096 */
        attr.priority = osPriorityNormal;
        if (osThreadNew(PWMDemoTask, NULL, &attr) == NULL) {
            printf("[PWMDemo] Failed to create PWMDemoTask!\n");
        }
    }
    SYS_RUN(PWMDemo);
    ```
