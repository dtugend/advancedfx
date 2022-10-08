#ifndef ADVANCEDFX_TYPES_H
#define ADVANCEDFX_TYPES_H


// Platform definitions ////////////////////////////////////////////////////////


#if (_WIN32 || _WIN64) && _WIN64 || __GNUC__ && __x86_64__ || __ppc64__
#define ADVANCEDFX_ENV64
#else
#define ADVANCEDFX_ENV32
#endif

#define ADVANCEDFX_LITTLE_ENDIAN


// Primitive types /////////////////////////////////////////////////////////////


#define ADVANCEDFX_NULLPTR ((void *)0)


//
// AdvancedfxBool

typedef unsigned char AdvancedfxBool_t;

#define ADAVANCEDFX_FALSE 0
#define ADAVANCEDFX_TRUE 1


//
// AdvancedfxUInt8

typedef unsigned char AdvancedfxUInt8_t;


//
// AdvancedfxUInt32

typedef unsigned long AdvancedfxUInt32_t;


//
// AdvancedfxInt32

typedef signed long AdvancedfxInt32_t;


//
// AdvancedfxSize

#ifdef ADVANCEDFX_ENV64
typedef unsigned long long AdvancedfxSize_t;
#else
typedef unsigned int AdvancedfxSize_t;
#endif


//
// AdvancedfxCString

typedef const char* AdvancedfxCString_t;


//
// AdvancedfxFloat

typedef float AdvancedfxFloat_t;


//
// AdvancedfxDouble

typedef double AdvancedfxDobule_t;

#endif
