#ifndef TE_VECTORS_H
#define TE_VECTORS_H

#include "DataTypes.h"

#define AXIS_HOR   0
#define AXIS_VERT  1

typedef union {
    struct {
	i32 x;
	i32 y;
    };
    struct {
	i32 width;
	i32 height;
    };
    i32 E[2];
} v2_i32; 

typedef union {
    struct {
	u32 x;
	u32 y;
    };
    struct {
	u32 width;
	u32 height;
    };
    u32 E[2];
} v2_u32; 

typedef union {
    struct {
	f32 x;
	f32 y;
    };
    struct {
	f32 width;
	f32 height;
    };
    f32 E[2];
} v2_f32; 

typedef union {
    struct {
	i32 x;
	i32 y;
	i32 z;
    };
    struct {
	i32 red;
	i32 green;
	i32 blue;
    };
    struct {
	i32 width;
	i32 height;
	i32 depth;
    };
    i32 E[3];
} v3_i32; 

typedef union {
    struct {
	f32 x;
	f32 y;
	f32 z;
    };
    struct {
	f32 red;
	f32 green;
	f32 blue;
    };
    struct {
	f32 width;
	f32 height;
	f32 depth;
    };
    f32 E[3];
} v3_f32; 

typedef union {
    struct {
	f32 x;
	f32 y;
	f32 z;
	f32 w;
    };
    struct {
	f32 red;
	f32 green;
	f32 blue;
	f32 alpha;
    };
    struct {
	f32 width;
	f32 height;
	f32 depth;
    };
    v3_f32 xyz;
    f32 E[3];
} v4_f32; 

v2_i32 V2_i32(i32 a, i32 b) {
    v2_i32 result;
    result.x = a;
    result.y = b;
    return result;
}

v2_i32 operator+(v2_i32 a, v2_i32 b) {
    v2_i32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

v2_i32 &operator+=(v2_i32 &_this, v2_i32 a) {
    _this = _this + a;
    return _this;
}

v2_i32 operator-(v2_i32 a) {
    v2_i32 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

v2_i32 operator-(v2_i32 a, v2_i32 b) {
    v2_i32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

v2_i32 &operator-=(v2_i32 &_this, v2_i32 a) {
    _this = _this - a;
    return _this;
}

v2_i32 operator*(v2_i32 a, f32 b) {
    v2_i32 result;
    result.x = (i32)(a.x * b);
    result.y = (i32)(a.y * b);
    return result;
}

v2_i32 operator*(f32 a, v2_i32 b) {
    v2_i32 result;
    result.x = (i32)(a * b.x);
    result.y = (i32)(a * b.y);
    return result;
}

v2_i32 &operator*=(v2_i32 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V2 u32
//

v2_u32 V2_u32(u32 a, u32 b) {
    v2_u32 result;
    result.x = a;
    result.y = b;
    return result;
}

v2_u32 operator+(v2_u32 a, v2_u32 b) {
    v2_u32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

v2_u32 &operator+=(v2_u32 &_this, v2_u32 a) {
    _this = _this + a;
    return _this;
}

v2_i32 operator-(v2_u32 a) {
    v2_i32 result;
    result.x = -(i32)a.x;
    result.y = -(i32)a.y;
    return result;
}

v2_u32 operator-(v2_u32 a, v2_u32 b) {
    v2_u32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

v2_u32 &operator-=(v2_u32 &_this, v2_u32 a) {
    _this = _this - a;
    return _this;
}

v2_u32 operator*(v2_u32 a, f32 b) {
    v2_u32 result;
    result.x = (u32)(a.x * b);
    result.y = (u32)(a.y * b);
    return result;
}

v2_u32 operator*(f32 a, v2_u32 b) {
    v2_u32 result;
    result.x = (u32)(a * b.x);
    result.y = (u32)(a * b.y);
    return result;
}

v2_u32 &operator*=(v2_u32 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V2 f32
//

v2_f32 V2_f32(f32 a, f32 b) {
    v2_f32 result;
    result.x = a;
    result.y = b;
    return result;
}

v2_f32 operator+(v2_f32 a, v2_f32 b) {
    v2_f32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

v2_f32 &operator+=(v2_f32 &_this, v2_f32 a) {
    _this = _this + a;
    return _this;
}

v2_f32 operator-(v2_f32 a) {
    v2_f32 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

v2_f32 operator-(v2_f32 a, v2_f32 b) {
    v2_f32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

v2_f32 &operator-=(v2_f32 &_this, v2_f32 a) {
    _this = _this - a;
    return _this;
}

v2_f32 operator*(v2_f32 a, f32 b) {
    v2_f32 result;
    result.x = a.x * b;
    result.y = a.y * b;
    return result;
}

v2_f32 operator*(f32 a, v2_f32 b) {
    v2_f32 result;
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

v2_f32 &operator*=(v2_f32 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V3
//

v3_f32 V3_f32(f32 a, f32 b, f32 c) {
    v3_f32 result;
    result.x = a;
    result.y = b;
    result.z = c;
    return result;
}

v3_f32 operator+(v3_f32 a, v3_f32 b) {
    v3_f32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

v3_f32 &operator+=(v3_f32 &_this, v3_f32 a) {
    _this = _this + a;
    return _this;
}

v3_f32 operator-(v3_f32 a) {
    v3_f32 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    return result;
}

v3_f32 operator-(v3_f32 a, v3_f32 b) {
    v3_f32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

v3_f32 &operator-=(v3_f32 &_this, v3_f32 a) {
    _this = _this - a;
    return _this;
}

v3_f32 operator*(v3_f32 a, f32 b) {
    v3_f32 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    return result;
}

v3_f32 operator*(f32 a, v3_f32 b) {
    v3_f32 result;
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    return result;
}

v3_f32 &operator*=(v3_f32 &_this, f32 a) {
    _this = _this * a;
    return _this;
}

//
// V4 F32
//

v4_f32 V3_f32(f32 a, f32 b, f32 c, f32 d) {
    v4_f32 result;
    result.x = a;
    result.y = b;
    result.z = c;
    result.w = d;
    return result;
}

v4_f32 operator+(v4_f32 a, v4_f32 b) {
    v4_f32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

v4_f32 &operator+=(v4_f32 &_this, v4_f32 a) {
    _this = _this + a;
    return _this;
}

v4_f32 operator-(v4_f32 a) {
    v4_f32 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    return result;
}

v4_f32 operator-(v4_f32 a, v4_f32 b) {
    v4_f32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

v4_f32 &operator-=(v4_f32 &_this, v4_f32 a) {
    _this = _this - a;
    return _this;
}

v4_f32 operator*(v4_f32 a, f32 b) {
    v4_f32 result;
    result.x = a.x * b;
    result.y = a.y * b;
    result.z = a.z * b;
    result.w = a.w * b;
    return result;
}

v4_f32 operator*(f32 a, v4_f32 b) {
    v4_f32 result;
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    result.w = a * b.w;
    return result;
}

v4_f32 &operator*=(v4_f32 &_this, f32 a) {
    _this = _this * a;
    return _this;
}
// functions

inline v2_f32 HadamardProd(v2_f32 a, v2_f32 b) {
    return {a.x * b.x, a.y * b.y};
}

inline v2_f32 HadamardDiv(v2_f32 a, v2_f32 b) {
    return {a.x / b.x, a.y / b.y};
}

inline v2_u32 HadamardProd(v2_u32 a, v2_u32 b) {
    return {a.x * b.x, a.y * b.y};
}

inline v2_u32 HadamardDiv(v2_u32 a, v2_u32 b) {
    return {a.x / b.x, a.y / b.y};
}

inline v3_f32 HadamardProd(v3_f32 a, v3_f32 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

inline v3_f32 HadamardDiv(v3_f32 a, v3_f32 b) {
    return {a.x / b.x, a.y / b.y, a.z / b.z};
}

inline v4_f32 HadamardProd(v4_f32 a, v4_f32 b) {
    return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

inline v4_f32 HadamardDiv(v4_f32 a, v4_f32 b) {
    return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

inline v2_i32 Toi32(v2_f32 a) {
    v2_i32 result;
    result.x = (i32)a.x;
    result.y = (i32)a.y;
    return result;
}

inline v2_i32 Toi32(v2_u32 a) {
    v2_i32 result;
    result.x = (i32)a.x;
    result.y = (i32)a.y;
    return result;
}

inline v3_i32 Toi32(v3_f32 a) {
    v3_i32 result;
    result.x = (i32)a.x;
    result.y = (i32)a.y;
    result.y = (i32)a.z;
    return result;
}

inline v2_u32 Tou32(v2_i32 a) {
    v2_u32 result;
    result.x = (u32)a.x;
    result.y = (u32)a.y;
    return result;
}

inline v2_u32 Tou32(v2_f32 a) {
    v2_u32 result;
    result.x = (u32)a.x;
    result.y = (u32)a.y;
    return result;
}

inline v2_f32 Tof32(v2_i32 a) {
    v2_f32 result;
    result.x = (f32)a.x;
    result.y = (f32)a.y;
    return result;
}

inline v2_f32 Tof32(v2_u32 a) {
    v2_f32 result;
    result.x = (f32)a.x;
    result.y = (f32)a.y;
    return result;
}

inline v3_f32 Tof32(v3_i32 a) {
    v3_f32 result;
    result.x = (f32)a.x;
    result.y = (f32)a.y;
    result.y = (f32)a.z;
    return result;
}

#endif //TE_VECTORS_H
