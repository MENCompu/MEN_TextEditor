#ifndef TE_MATH_H
#define TE_MATH_H

#include "DataTypes.h"

#define KILOBYTES(value) ((value) * 1024)
#define MEGABYTES(value) (KILOBYTES(value) * 1024)
#define GIGABYTES(value) (MEGABYTES(value) * 1024)
#define TERABYTES(value) (GIGABYTES(value) * 1024)

inline i32 TruncateI64ToI32(i64 value) {
    i32 result = value & 0xFFFFFFFF;
    return result;
}

inline i32 TruncateF32ToI32(f32 value) {
    i32 result = (i32)(value);
    return result;
}

inline u32 TruncateF32ToU32(f32 value) {
    u32 result = (u32)(value);
    return result;
}

inline i32 RoundF32ToI32(f32 value) {
    i32 result = (i32)(value + 0.5f);
    return result;
}

inline u32 RoundF32ToU32(f32 value) {
    u32 result = (u32)(value + 0.5f);
    return result;
}

inline i32 CeilF32ToI32(f32 value) {
    i32 result = TruncateF32ToU32(value);
    result++;
    return result;
}

inline u32 CeilF32ToU32(f32 value) {
    u32 result = TruncateF32ToU32(value);
    result++;
    return result;
}

#define GetIntegerDigits(value) IntegerLog10((value))

inline i32 IntegerLog10(i32 value) {
    i32 result = 0;

    while (value >= 10) { 
        value /= 10;
	++result; 
    }

    return result;
}

#endif //TE_MATH_H
