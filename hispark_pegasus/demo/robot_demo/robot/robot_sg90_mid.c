/*
 * Copyright (C) 2022 HiHope Open Source Organization .
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"

#define GPIO2 2
#define COUNT 10
#define TASK_STAK_SIZE    (1024*10)
void set_angle(unsigned int duty)
{
    unsigned int time = 20000;
    IoTGpioSetDir(GPIO2, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(GPIO2, IOT_GPIO_VALUE1);
    hi_udelay(duty);
    IoTGpioSetOutputVal(GPIO2, IOT_GPIO_VALUE0);
    hi_udelay(time - duty);
}

/* Turn 45 degrees to the left of the steering gear
1、依据角度与脉冲的关系，设置高电平时间为1000微秒
2、不断地发送信号，控制舵机向左旋转45度
*/
void engine_turn_left_45(void)
{
    unsigned int angle = 1000;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}

/* Turn 90 degrees to the left of the steering gear
1、依据角度与脉冲的关系，设置高电平时间为500微秒
2、不断地发送信号，控制舵机向左旋转90度
*/
void engine_turn_left_90(void)
{
    unsigned int angle = 500;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}

/* Turn 45 degrees to the right of the steering gear
1、依据角度与脉冲的关系，设置高电平时间为2000微秒
2、不断地发送信号，控制舵机向右旋转45度
*/
void engine_turn_right_45(void)
{
    unsigned int angle = 2000;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}

/* Turn 90 degrees to the right of the steering gear
1、依据角度与脉冲的关系，设置高电平时间为2500微秒
2、不断地发送信号，控制舵机向右旋转90度
*/
void engine_turn_right_90(void)
{
    unsigned int angle = 2500;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}

/* The steering gear is centered
1、依据角度与脉冲的关系，设置高电平时间为1500微秒
2、不断地发送信号，控制舵机居中
*/
void regress_middle(void)
{
    unsigned int angle = 1500;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}

/* 任务实现 */
void RobotTask(void* parame)
{
    (void)parame;
    printf("The steering gear is centered\r\n");
    regress_middle();
}


static void RobotDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "RobotTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TASK_STAK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew(RobotTask, NULL, &attr) == NULL) {
        printf("[RobotDemo] Falied to create RobotTask!\n");
    }
}

APP_FEATURE_INIT(RobotDemo);
