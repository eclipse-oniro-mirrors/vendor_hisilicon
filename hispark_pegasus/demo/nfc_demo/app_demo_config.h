/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
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
 * limitations under the License.
 */

#ifndef APP_DEMO_CONFIG_H
#define APP_DEMO_CONFIG_H

#include <hi_i2c.h>

#define CHECK
#define     NFC_I2C_REG_ARRAY_LEN           (32)
#define     NFC_SEND_BUFF                   (3)
#define     C08I_NFC_DEMO_TASK_STAK_SIZE    (1024*10)
#define     NFC_DISPLAY_TASK_STAK_SIZE      (1024)
#define     C08I_NFC_DEMO_TASK_PRIORITY     (25)
#define     C08I_NFC_TASK_PRIORITY          (26)
#define     NFC_TAG_WECHAT
#define     CLEAN_STOP_SIGNAL               (hi_u8(0x00))

#define CLA                                 (1)
#define INS                                 (2)
#define P1                                  (3)
#define P2                                  (4)
#define LC                                  (5)
#define DATA                                (6)

#define C081_NFC_ADDR           0xAE // 7 bit slave device address  1010 111 0/1
#define I2C_WR                  0x00
#define I2C_RD                  0x01
#define C081_NFC_READ_ADDR      0xAF
#define C081NFC_WRITE_ADDR      (C081_NFC_ADDR|I2C_WR)
#define C081NFC_READ_ADDR       (C081_NFC_ADDR|I2C_RD)
#define FM11_E2_USER_ADDR       0x0010
#define FM11_E2_MANUF_ADDR      0x03FF
#define FM11_E2_BLOCK_SIZE      16

#define FM327_FIFO          0xFFF0
#define FIFO_FLUSH_REG      0xFFF1
#define	FIFO_WORDCNT_REG    0xFFF2
#define RF_STATUS_REG       0xFFF3
#define RF_TXEN_REG         0xFFF4
#define RF_BAUD_REG         0xFFF5
#define RF_RATS_REG         0xFFF6
#define MAIN_IRQ_REG        0xFFF7
#define FIFO_IRQ_REG        0xFFF8
#define AUX_IRQ_REG	        0xFFF9
#define MAIN_IRQ_MASK_REG   0xFFFA
#define FIFO_IRQ_MASK_REG   0xFFFB
#define AUX_IRQ_MASK_REG    0xFFFC
#define NFC_CFG_REG         0xFFFD
#define VOUT_CFG_REG        0xFFFE
#define EE_WR_CTRL_REG      0xFFFF

#define MAIN_IRQ            0xFFF7
#define FIFO_IRQ            0xFFF8
#define AUX_IRQ             0xFFF9
#define MAIN_IRQ_MASK       0xFFFA
#define FIFO_IRQ_MASK       0xFFFB
#define AUX_IRQ_MASK        0xFFFC
#define FIFO_FLUSH          0xFFF1
#define	FIFO_WORDCNT        0xFFF2

#define MAIN_IRQ_RF_PWON        0x80
#define MAIN_IRQ_ACTIVE         0x40
#define MAIN_IRQ_RX_START       0x20
#define MAIN_IRQ_RX_DONE        0x10
#define MAIN_IRQ_TX_DONE        0x08
#define MAIN_IRQ_ARBIT          0x04
#define MAIN_IRQ_FIFO           0x02
#define MAIN_IRQ_AUX            0x01
#define FIFO_IRQ_WL             0x08

typedef enum {
    NONE,
    CC_FILE,
    NDEF_FILE
} T4T_FILE;

typedef enum {
    NFC_RECOVERY = 0,
    NFC_CLEAN
} NFCCleanStopSignal;

#define SEND_OLED_FILL_SCRRENC_CMD ((unsigned char)0x0)
#define SEND_CMD_LEN    (8)

#define OLED_X_POSITION_0    (0)
#define OLED_X_POSITION_15   (15)
#define OLED_X_POSITION_16   (16)
#define OLED_X_POSITION_18   (18)
#define OLED_X_POSITION_40   (40)
#define OLED_X_POSITION_48   (48)
#define OLED_X_POSITION_56   (56)
#define OLED_X_POSITION_60   (60)
#define OLED_X_POSITION_81   (81)
#define OLED_X_POSITION_120  (120)

#define OLED_Y_POSITION_0     ((unsigned char)0x0)
#define OLED_Y_POSITION_1     ((unsigned char)0x1)
#define OLED_Y_POSITION_2     ((unsigned char)0x2)
#define OLED_Y_POSITION_3     ((unsigned char)0x3)
#define OLED_Y_POSITION_4     ((unsigned char)0x4)
#define OLED_Y_POSITION_5     ((unsigned char)0x5)
#define OLED_Y_POSITION_6     ((unsigned char)0x6)
#define OLED_Y_POSITION_7     ((unsigned char)0x7)

#define OLED_DISPLAY_STRING_TYPE_1   (1)
#define OLED_DISPLAY_STRING_TYPE_16  (16)

void SetNdefData(void);
unsigned char GetNdefData(unsigned char *ndefFileData, unsigned int ndefLen);
void NfcRead(void);
hi_void OledNfcDisplay(hi_void);
#endif
