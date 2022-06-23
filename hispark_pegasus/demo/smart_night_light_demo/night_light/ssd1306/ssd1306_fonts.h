#ifndef HISPARK_PEGASUS_DEMO_SMART_NIGHT_LIGHT_DEMO_NIGHT_LIGHT_SSD1306_SSD1306_FONTS_H
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
#define HISPARK_PEGASUS_DEMO_SMART_NIGHT_LIGHT_DEMO_NIGHT_LIGHT_SSD1306_SSD1306_FONTS_H
#include <stdint.h>
#ifndef __SSD1306_FONTS_H__
#define __SSD1306_FONTS_H__

#include "ssd1306_conf.h"
#endif

typedef struct {
    const uint8_t FontWidth;    /*!< Font width in pixels */
    uint8_t FontHeight;   /*!< Font height in pixels */
    const uint16_t *data; /*!< Pointer to data font data array */
} FontDef;

#ifdef SSD1306_INCLUDE_FONT_6x8
#endif
#ifdef SSD1306_INCLUDE_FONT_7x10
#endif
#ifdef SSD1306_INCLUDE_FONT_11x18
#endif
#ifdef SSD1306_INCLUDE_FONT_16x26
#endif
#endif // __SSD1306_FONTS_H__
