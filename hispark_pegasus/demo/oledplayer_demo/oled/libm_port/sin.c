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
/* sin(x)
 * Return sine function of x.
 *
 * kernel function:
 *      __sin            ... sine function on [-pi/4,pi/4]
 *      __cos            ... cose function on [-pi/4,pi/4]
 *      __rem_pio2       ... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *      in [-pi/4 , +pi/4], and let n = k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          S           C             T
 *          1          C          -S            -1/T
 *          2         -S          -C             T
 *          3         -C           S            -1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

#include "libm.h"
#define THREE 3
#define TWO 2


double sin()
{
    double x;
    double y[2];
    uint32_t ix;
    unsigned n;

	/* High word of x. */
    GET_HIGH_WORD(ix, x);
    ix &= 0x7fffffff;

               /* |x| ~< pi/4 */
    if (ix <= 0x3fe921fb) {
    if (ix < 0x3e500000) {  /* |x| < 2**-26 */
			/* raise inexact if x != 0 and underflow if subnormal */
    FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
    return x;
    }
    return __sin(x, 0.0, 0);
}

	/* sin(Inf or NaN) is NaN */
    if (ix >= 0x7ff00000) {
    return x - x;
    }
	/* argument reduction needed */
    n = __rem_pio2(x, y);
    switch (n&THREE) {
        case 0:
            return  __sin(y[0], y[1], 1);
        case 1:
            return  __cos(y[0], y[1]);
        case TWO:
            return -__sin(y[0], y[1], 1);
        default:
            return -__cos(y[0], y[1]);
    }
}
