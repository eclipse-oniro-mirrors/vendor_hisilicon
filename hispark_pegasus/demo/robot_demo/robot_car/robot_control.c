#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"
#include "hi_adc.h"
#include "iot_errno.h"
#include "robot_hcsr04.h"
#include "robot_l9110s.h"
#include "robot_sg90.h"
#include "trace_model.h"
#include "robot_control.h"

#define     GPIO5 5
#define     FUNC_GPIO 0
#define     IOT_IO_PULL_UP 1
#define     VLT_MIN                     (100)
#define     OLED_FALG_ON                ((unsigned char)0x01)
#define     OLED_FALG_OFF               ((unsigned char)0x00)
unsigned short  g_gpio5_adc_buf[ADC_TEST_LENGTH] = {0 };
unsigned int    g_gpio5_tick = 0;
unsigned int    g_car_control_demo_task_id = 0;
unsigned char   g_car_status = CAR_STOP_STATUS;

void switch_init(void)
{
    IoTGpioInit(GPIO5);
    hi_io_set_func(GPIO5, FUNC_GPIO);
    IoTGpioSetDir(GPIO5, IOT_GPIO_DIR_IN);
    hi_io_set_pull(GPIO5, IOT_IO_PULL_UP);
}

unsigned char GetCarStatus(void)
{
    return g_car_status;
}

// 按键中断响应函数
void gpio5_isr_func_mode(void)
{
    printf("gpio5_isr_func_mode start\n");
    unsigned int tick_interval = 0;
    unsigned int current_gpio5_tick = 0;

    current_gpio5_tick = hi_get_tick();
    tick_interval = current_gpio5_tick - g_gpio5_tick;
    
    if (tick_interval < KEY_INTERRUPT_PROTECT_TIME) {
        return NULL;
    }
    g_gpio5_tick = current_gpio5_tick;

    if (g_car_status == CAR_STOP_STATUS) {
        g_car_status = CAR_TRACE_STATUS;                 // 寻迹
        printf("trace\n");
    } else if (g_car_status == CAR_TRACE_STATUS) {
        g_car_status = CAR_OBSTACLE_AVOIDANCE_STATUS;    // 超声波
        printf("ultrasonic\n");
    } else if (g_car_status == CAR_OBSTACLE_AVOIDANCE_STATUS) {
        g_car_status = CAR_STOP_STATUS;                  // 停止
        printf("stop\n");
    }
}

unsigned char get_gpio5_voltage(void *param)
{
    int i;
    unsigned short data;
    unsigned int ret;
    unsigned short vlt;
    float voltage;
    float vlt_max = 0;
    float vlt_min = VLT_MIN;
    float a = 1.8;
    float c = 4096.0;
    int b = 4;
    float vlt_1 = 0.6;
    float vlt_2 = 1.0;

    hi_unref_param(param);
    memset_s(g_gpio5_adc_buf, sizeof(g_gpio5_adc_buf), 0x0, sizeof(g_gpio5_adc_buf));
    for (i = 0; i < ADC_TEST_LENGTH; i++) {
        ret = hi_adc_read(HI_ADC_CHANNEL_2, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0xF0);
		// ADC_Channal_2  自动识别模式  CNcomment:4次平均算法模式 CNend
        if (ret != IOT_SUCCESS) {
            printf("ADC Read Fail\n");
            return  NULL;
        }
        g_gpio5_adc_buf[i] = data;
    }

    for (i = 0; i < ADC_TEST_LENGTH; i++) {
        vlt = g_gpio5_adc_buf[i];
        voltage = (float)vlt * a * b / c;
		/* vlt * 1.8* 4 / 4096.0为将码字转换为电压 */
        vlt_max = (voltage > vlt_max) ? voltage : vlt_max;
        vlt_min = (voltage < vlt_min) ? voltage : vlt_min;
    }
    printf("vlt_max is %f\r\n", vlt_max);
    if (vlt_max > vlt_1 && vlt_max < vlt_2) {
        gpio5_isr_func_mode();
    }
}

// 按键中断
void interrupt_monitor(void)
{
    unsigned int  ret = 0;
    /* gpio5 switch2 mode */
    g_gpio5_tick = hi_get_tick();
    ret = IoTGpioRegisterIsrFunc(GPIO5, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, get_gpio5_voltage, NULL);
    if (ret == IOT_SUCCESS) {
        printf(" register gpio5\r\n");
    }
}

/* Judge steering gear */
static unsigned int engine_go_where(void)
{
    float left_distance = 0;
    float right_distance = 0;
    unsigned int time = 100;
    /* 舵机往左转动测量左边障碍物的距离 */

    engine_turn_left();
    hi_sleep(time);
    left_distance = GetDistance();
    hi_sleep(time);

    /* 归中 */
    regress_middle();
    hi_sleep(time);

    /* 舵机往右转动测量右边障碍物的距离 */
    engine_turn_right();
    hi_sleep(time);
    right_distance = GetDistance();
    hi_sleep(time);

    /* 归中 */
    regress_middle();
    if (left_distance > right_distance) {
        return CAR_TURN_LEFT;
    } else {
        return CAR_TURN_RIGHT;
    }
}

/* 根据障碍物的距离来判断小车的行走方向
1、距离大于等于20cm继续前进
2、距离小于20cm，先停止再后退0.5s, 再继续进行测距, 再进行判断
*/
/* Judge the direction of the car */
static void car_where_to_go(float distance)
{
    unsigned int time = 500;
    if (distance < DISTANCE_BETWEEN_CAR_AND_OBSTACLE) {
        car_stop();
        hi_sleep(time);
        car_backward();
        hi_sleep(time);
        car_stop();
        unsigned int ret = engine_go_where();
        printf("ret is %ud\r\n", ret);
        if (ret == CAR_TURN_LEFT) {
            car_left();
            hi_sleep(time);
        } else if (ret == CAR_TURN_RIGHT) {
            car_right();
            hi_sleep(time);
        }
        car_stop();
    } else {
        car_forward();
        }
}

/* car mode control func */
static void car_mode_control_func(void)
{
    float m_distance = 0.0;
    regress_middle();
    unsigned int time = 20;
    while (1) {
        if (g_car_status != CAR_OBSTACLE_AVOIDANCE_STATUS) {
            printf("car_mode_control_func 1 module changed\n");
            regress_middle();
            break;
        }

        /* 获取前方物体的距离 */
        m_distance = GetDistance();
        car_where_to_go(m_distance);
        hi_sleep(time);
    }
}

void *RobotCarTestTask(void* param)
{
    printf("switch\r\n");
    switch_init();
    interrupt_monitor();
    unsigned int time = 20;

    while (1) {
        switch (g_car_status) {
            case CAR_STOP_STATUS:
                car_stop();
                break;
            case CAR_OBSTACLE_AVOIDANCE_STATUS:
                car_mode_control_func();
                break;
            case CAR_TRACE_STATUS:
                trace_module();
                break;
            default:
                break;
        }
        IoTWatchDogDisable();
        osDelay(time);
    }
}

void RobotCarDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "RobotCarTestTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CAR_CONTROL_DEMO_TASK_STAK_SIZE;
    attr.priority = CAR_CONTROL_DEMO_TASK_PRIORITY;

    if (osThreadNew(RobotCarTestTask, NULL, &attr) == NULL) {
        printf("[Ssd1306TestDemo] Falied to create RobotCarTestTask!\n");
    }
}
APP_FEATURE_INIT(RobotCarDemo);
