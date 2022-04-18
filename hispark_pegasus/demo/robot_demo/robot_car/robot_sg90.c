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
void set_angle(unsigned int duty)
{
    unsigned int time = 20000;
    IoTGpioInit(GPIO2);
    IoTGpioSetDir(GPIO2, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(GPIO2, IOT_GPIO_VALUE1);
    hi_udelay(duty);
    IoTGpioSetOutputVal(GPIO2, IOT_GPIO_VALUE0);
    hi_udelay(time - duty);
}

/*  Steering gear turn left */
void engine_turn_left(void)
{
    unsigned int angle = 1000;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}

/* Steering gear turn right */
void engine_turn_right(void)
{
    unsigned int angle = 2000;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}

/* Steering gear return to middle */
void regress_middle(void)
{
    unsigned int angle = 1500;
    for (int i = 0; i < COUNT; i++) {
        set_angle(angle);
    }
}
