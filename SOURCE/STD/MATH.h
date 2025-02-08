/*+++
    STD\MATH.h - ATOS math functions and constants

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.
---*/
#ifndef ATOSMATH_H
#define ATOSMATH_H
#include <STD/TYPES.h>

#define PI 3.14159265358979323846F
#define E 2.71828182845904523536F

#define DEG_TO_RAD(x) (x * (PI / 180))
#define RAD_TO_DEG(x) (x * (180 / PI))

#define ABS(x) (x < 0 ? -x : x)

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

#define POW(x, y) (x ^ y)
#define POW2(x) (x ^ 2)
#define POW3(x) (x ^ 3)

F80 sqrt(F80 x);
F80 sin(F80 x);
F80 cos(F80 x);
F80 tan(F80 x);
F80 asin(F80 x);
F80 acos(F80 x);
F80 atan(F80 x);
F80 sinh(F80 x);

#endif // ATOSMATH_H