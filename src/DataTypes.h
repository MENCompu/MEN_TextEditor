#ifndef TE_DATATYPES_H
#define TE_DATATYPES_H

#include <stdint.h>

#define LocalPersistant static
#define GlobalVariable static
#define Internal static

// TODO(JENH): Change this to compiler specific.
#define Always_Inline __forceinline

#define MAX_VALUE_PTR 0xFFFFFFFFFFFFFFFF

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef u32 ufx32;
typedef u64 ufx64;

typedef i32 b32;

#endif //TE_DATATYPES_H
