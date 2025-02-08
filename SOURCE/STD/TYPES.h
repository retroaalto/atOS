/*+++
    STD\TYPES.h - ATOS basic types and their limits

    Part of atOS-RT

    Licensed under the MIT License. See LICENSE file in the project root for full license information.
---*/
#ifndef ATOSINT_H
#define ATOSINT_H

#define _64_BIT




/*+++
    Boolean type
---*/

#define BOOLEAN _Bool
#define TRUE 1
#define FALSE 0




/*+++
    Numeric types
---*/

typedef void            U0;
typedef unsigned char   U8;
typedef signed char     I8;
typedef unsigned short  U16;
typedef signed short    I16;
typedef unsigned int    U32;
typedef signed int      I32;

#if defined _64_BIT
typedef unsigned long long  U64;
typedef signed long long    I64;
#else
typedef unsigned int    U64;
typedef signed int      I64;
#endif

typedef float           F32;
typedef double          F64;
typedef long double     F80;

typedef U8              BYTE;
typedef U16             WORD;
typedef U32             DWORD;
typedef U64             QWORD;

typedef I8              CHAR;
typedef I16             SHORT;
typedef I32             LONG;
typedef I64             LONGLONG;

typedef U8              UCHAR;
typedef U16             USHORT;
typedef U32             ULONG;
typedef U64             ULONGLONG;

typedef F32             FLOAT;
typedef F64             DOUBLE;
typedef F80             LONGDOUBLE;

typedef U8              CHAR8;
typedef U16             CHAR16;




/*+++
    Numeric limits
---*/

#define U0_MIN          0
#define U8_MIN          0
#define U16_MIN         0
#define U32_MIN         0
#define U64_MIN         0

#define I8_MIN          -128i8
#define I16_MIN         -32768i16
#define I32_MIN         -2147483648i32
#define I64_MIN         -9223372036854775808i64

#define U0_MAX          0x0
#define U8_MAX          0xff
#define U16_MAX         0xffff
#define U32_MAX         0xffffffff
#define U64_MAX         0xffffffffffffffff

#define I8_MAX          127i8
#define I16_MAX         32767i16
#define I32_MAX         2147483647i32
#define I64_MAX         9223372036854775807i64

#define F32_MIN         1.175494351e-38F
#define F64_MIN         2.2250738585072014e-308
#define F80_MIN         3.36210314311209350626267781732175260e-4932L

#define F32_MAX         3.402823466e+38F
#define F64_MAX         1.7976931348623158e+308
#define F80_MAX         1.18973149535723176508575932662800702e+4932L

/*+++
    Pointer types
---*/

#define NULL            0
#define NULLPTR         (U0*)0

#define LONGPTR         I64
#define ULONGPTR        U64

#define LONGPTR_MIN     I64_MIN
#define LONGPTR_MAX     I64_MAX
#define ULONGPTR_MIN    U64_MIN
#define ULONGPTR_MAX    U64_MAX

#endif // ATOSINT_H