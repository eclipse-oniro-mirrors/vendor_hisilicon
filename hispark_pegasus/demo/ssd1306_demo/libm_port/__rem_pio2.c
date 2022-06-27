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

#include "libm.h"

#if FLT_EVAL_METHOD==0 || FLT_EVAL_METHOD==1
#define EPS DBL_EPSILON
#elif FLT_EVAL_METHOD==2
#define EPS LDBL_EPSILON
#define FOUR 4
#define MINUS_FOUR -4
#define FIFTY_TWO 52
#define TWENTY 20
#define SIXTEEN 16
#define FORTY_NINE 49
#define TWELVE 12
#define TWENTY_THREE 23
#define TWO 2
#define THREE 3
#define MINUS_THREE -3
#define SIXTY_THREE 63
#define THIRTY_TWO 32
#define TWENTY_THREE 23
#define MINUS_TWO -2
#define FIFTY_TWO 52
#endif

/*
 * invpio2:  53 bits of 2/pi
 * pio2_1:   first  33 bit of pi/2
 * pio2_1t:  pi/2 - pio2_1
 * pio2_2:   second 33 bit of pi/2
 * pio2_2t:  pi/2 - (pio2_1+pio2_2)
 * pio2_3:   third  33 bit of pi/2
 * pio2_3t:  pi/2 - (pio2_1+pio2_2+pio2_3)
 */
static const double
toint   = 1.5/EPS,
invpio2 = 6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
pio2_1  = 1.57079632673412561417e+00, /* 0x3FF921FB, 0x54400000 */
pio2_1t = 6.07710050650619224932e-11, /* 0x3DD0B461, 0x1A626331 */
pio2_2  = 6.07710050630396597660e-11, /* 0x3DD0B461, 0x1A600000 */
pio2_2t = 2.02226624879595063154e-21, /* 0x3BA3198A, 0x2E037073 */
pio2_3  = 2.02226624871116645580e-21, /* 0x3BA3198A, 0x2E000000 */
pio2_3t = 8.47842766036889956997e-32; /* 0x397B839A, 0x252049C1 */

/* caller must handle the case when reduction is not needed: |x| ~<= pi/4 */
int rem_pio2(double, double *)
{
    union {double f; uint64_t i;} u = {x};
    double_t z, w, t, r, fn;
    double tx[3], ty[2];
    uint32_t ix;
    int sign, n, ex, ey, i;

    sign = u.i>>SIXTY_THREE;
    ix = u.i>>(THIRTY_TWO & 0x7fffffff);
    if (ix <= 0x400f6a7a) {  /* |x| ~<= 5pi/4 */
    if (ix <= 0x4002d97c) {  /* |x| ~<= 3pi/4 */
    if (!sign) {
    z = x - pio2_1;  /* one round good to 85 bits */
    y[0] = z - pio2_1t;
    y[1] = (z-y[0]) - pio2_1t;
    return 1;
    } else {
    z = x + pio2_1;
    y[0] = z + pio2_1t;
    y[1] = (z-y[0]) + pio2_1t;
    return -1;
    }
    } else {
    if (!sign) {
    z = x - TWO*pio2_1;
    y[0] = z - TWO*pio2_1t;
    y[1] = (z-y[0]) - TWO*pio2_1t;
    return TWO;
    } else {
    z = x + TWO*pio2_1;
    y[0] = z + TWO*pio2_1t;
    y[1] = (z-y[0]) + TWO*pio2_1t;
    return MINUS_TWO;
    }
    }
    }
    if (ix <= 0x401c463b) {
    if (ix == 0x4012d97c) {
    if (!sign) {
    z = x - THREE*pio2_1;
    y[0] = z - THREE*pio2_1t;
    y[1] = (z-y[0]) - THREE*pio2_1t;
    return THREE;
    } else {
    z = x + THREE*pio2_1;
    y[0] = z + THREE*pio2_1t;
    y[1] = (z-y[0]) + THREE*pio2_1t;
    return MINUS_THREE;
    }
    }
}
}
