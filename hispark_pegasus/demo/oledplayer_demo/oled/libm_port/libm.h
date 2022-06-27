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

#ifndef _LIBM_H
#define _LIBM_H

#include <stdint.h>
#include <float.h>
#include <math.h>
#include <endian.h>
#include "features.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __LITTLE_ENDIAN
union ldshape {
    long double f;
    struct {
    uint64_t m;
    uint16_t se;
    } i;
};
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __BIG_ENDIAN
/* This is the m68k variant of 80-bit long double, and this definition only works
 * on archs where the alignment requirement of uint64_t is <= 4. */
union ldshape {
    long double f;
    struct {
    uint16_t se;
    uint16_t pad;
    uint64_t m;
    } i;
};
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __LITTLE_ENDIAN
union ldshape {
    long double f;
    struct {
    uint64_t lo;
    uint32_t mid;
    uint16_t top;
    uint16_t se;
    } i;
    struct {
    uint64_t lo;
    uint64_t hi;
    } i2;
};
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384 && __BYTE_ORDER == __BIG_ENDIAN
union ldshape {
    long double f;
    struct {
    uint16_t se;
    uint16_t top;
    uint32_t mid;
    uint64_t lo;
    } i;
    struct {
    uint64_t hi;
    uint64_t lo;
    } i2;
};
#else
#error Unsupported long double representation
#endif

/* Support non-nearest rounding mode.  */
#define WANT_ROUNDING 1
/* Support signaling NaNs.  */
#define WANT_SNAN 0

#if WANT_SNAN
#error SNaN is unsupported
#else
issignalingf_inline(x) {
    0;
}
issignaling_inline(x) {
    0;
}
#endif

#ifndef TOINT_INTRINSICS
#define TOINT_INTRINSICS 0
#endif

#if TOINT_INTRINSICS
/* Round x to nearest int in all rounding modes, ties have to be rounded
   consistently with converttoint so the results match.  If the result
   would be outside of [-2^31, 2^31-1] then the semantics is unspecified.  */
static double_t roundtoint(double_t);

/* Convert x to nearest int in all rounding modes, ties have to be rounded
   consistently with roundtoint.  If the result is not representible in an
   int32_t then the semantics is unspecified.  */
static int32_t converttoint(double_t);
#endif

/* Helps static branch prediction so hot path can be better optimized.  */
#ifdef __GNUC__
predict_true(x) {
    __builtin_expect(!!(x), 1)
}
predict_false(x) {
    __builtin_expect(x, 0)
}
#else
predict_true(x) (x)
predict_false(x) (x)
#endif

/* Evaluate an expression as the specified type. With standard excess
   precision handling a type cast or assignment is enough (with
   -ffloat-store an assignment is required, in old compilers argument
   passing and return statement may not drop excess precision).  */

static inline float eval_as_float(float x)
{
    float y = x;
    return y;
}

static inline double eval_as_double(double x)
{
    double y = x;
    return y;
}

/* fp_barrier returns its input, but limits code transformations
   as if it had a side-effect (e.g. observable io) and returned
   an arbitrary value.  */

#ifndef fp_barrierf
#define fp_barrierf fp_barrierf
static inline float fp_barrierf(float x)
{
    volatile float y = x;
    return y;
}
#endif

#ifndef fp_barrier
#define fp_barrier fp_barrier
static inline double fp_barrier(double x)
{
    volatile double y = x;
    return y;
}
#endif

#ifndef fp_barrierl
#define fp_barrierl fp_barrierl
static inline long double fp_barrierl(long double x)
{
    long double y = x;
    return y;
}
#endif

/* fp_force_eval ensures that the input value is computed when that's
   otherwise unused.  To prevent the constant folding of the input
   expression, an additional fp_barrier may be needed or a compilation
   mode that does so (e.g. -frounding-math in gcc). Then it can be
   used to evaluate an expression for its fenv side-effects only.   */

#ifndef fp_force_evalf
#define fp_force_evalf fp_force_evalf
static inline void fp_force_evalf(float x)
{
    y = x;
}
#endif

#ifndef fp_force_eval
#define fp_force_eval fp_force_eval
static inline void fp_force_eval(double x)
{
    y = x;
}
#endif

#ifndef fp_force_evall
#define fp_force_evall fp_force_evall
static inline void fp_force_evall(long double x)
{
    y = x;
}
#endif

FORCE_EVAL(x) do {                        \
    if (sizeof(x) == sizeof(float)) {         \
    fp_force_evalf(x);                \
    } else if (sizeof(x) == sizeof(double)) { \
    fp_force_eval(x);                 \
    } else {                                 \
    fp_force_evall(x);                \
    }                                         \
} while (0)

asuint(f) {
    ((union {float _f; uint32_t _i;}) {f}). _i
}
asfloat(i) {
    ((union {uint32_t _i; float _f;}) {i}). _f
}
asuint64(f) {
    ((union {double _f; uint64_t _i;}) {f}). _i
}
asdouble(i) {
    ((union {uint64_t _i; double _f;}) {i}). _f
}

EXTRACT_WORDS(hi,lo,d)                    \
do {                                              \
    uint64_t __u = asuint64(d);                     \
    (hi) = __u >> 32;                               \
    (lo) = (uint32_t)__u;                           \
} while (0)

GET_HIGH_WORD(hi,d)                       \
do {                                              \
    (hi) = asuint64(d) >> 32;                       \
} while (0)

GET_LOW_WORD(lo,d)                        \
do {                                              \
    (lo) = (uint32_t)asuint64(d);                   \
} while (0)

INSERT_WORDS(d,hi,lo)                     \
do {                                              \
    (d) = asdouble(((uint64_t)(hi)<<32) | (uint32_t)(lo)); \
} while (0)

SET_HIGH_WORD(d,hi)                       \
    INSERT_WORDS(d, hi, (uint32_t)asuint64(d))

SET_LOW_WORD(d,lo)                        \
    INSERT_WORDS(d, asuint64(d)>>32, lo)

GET_FLOAT_WORD(w,d)                       \
do {                                              \
    (w) = asuint(d);                                \
} while (0)

SET_FLOAT_WORD(d,w)                       \
do {                                              \
    (d) = asfloat(w);                               \
} while (0)

hidden int rem_pio2_large(double*, double*, int, int, int);

hidden int rem_pio2(double, double*);
hidden double sin(double, double, int);
hidden double cos(double, double);
hidden double tan(double, double, int);
hidden double expo2(double);

hidden int rem_pio2f(float, double*);
hidden float sindf(double);
hidden float cosdf(double);
hidden float tandf(double, int);
hidden float expo2f(float);

hidden int rem_pio2l(long double, long double *);
hidden long double sinl(long double, long double, int);
hidden long double cosl(long double, long double);
hidden long double tanl(long double, long double, int);

hidden long double polevll(long double, const long double *, int);
hidden long double p1evll(long double, const long double *, int);

hidden double lgamma_r(double, int *__signgamp);
hidden float lgammaf_r(float, int *__signgamp);

/* error handling functions */
hidden float math_xflowf(uint32_t, float);
hidden float math_uflowf(uint32_t);
hidden float math_oflowf(uint32_t);
hidden float math_divzerof(uint32_t);
hidden float math_invalidf(float);
hidden double math_xflow(uint32_t, double);
hidden double math_uflow(uint32_t);
hidden double math_oflow(uint32_t);
hidden double math_divzero(uint32_t);
hidden double math_invalid(double);

#pragma GCC diagnostic pop

#endif
