/*
 * Core.hxx
 *
 * This file is part of the Chip8++ source code.
 * Copyright 2023 Patrick Melo <patrick@patrickmelo.com.br>
 */

#ifndef CHIP8_CORE_H
#define CHIP8_CORE_H

// Only Support C++11 Compilers

#if (__cplusplus < 201103L)
    #error "Your compiler does not support the C++11 standard."
#endif

// C

#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// C++

#include <map>
#include <new>
#include <string>
#include <vector>

// System

extern "C" {
#include <sys/time.h>
#include <unistd.h>
}

// Integer Types

typedef unsigned int uint;
typedef uint8_t      uint8;
typedef uint16_t     uint16;
typedef uint32_t     uint32;
typedef uint64_t     uint64;

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef unsigned long  ulong;
typedef unsigned short ushort;

// Integer Macros

#define UINT8(value)  static_cast<uint8>(value)
#define UINT16(value) static_cast<uint16>(value)

// Integer Union Types

union int16u {
        uint8  i8[2];
        uint16 i16;
};

// String Types

typedef unsigned char uchar;
typedef std::string   string;
typedef char*         cstring;
typedef char const*   charconst;

// Pointer Types

typedef uint8* buffer;
typedef void*  pointer;

// Buffers

#define NewBuffer(bufferSize)     (bufferSize > 0 ? new (std::nothrow) uint8[bufferSize] : NULL)
#define CopyBuffer(in, out, size) memcpy(out, in, size)
#define DeleteBuffer(buffer) \
    if (buffer) {            \
        delete[] buffer;     \
        buffer = NULL;       \
    }

// Debug

#ifdef CHIP8_DEBUG
    #define Debug(tag, message, ...) printf("\033[1;35m[%s] " message "\033[0m\n", tag, ##__VA_ARGS__)
#else
    #define Debug(tag, message, ...)
#endif

#define Info(tag, message, ...)    printf("\033[1;32m[%s] " message "\033[0m\n", tag, ##__VA_ARGS__);
#define Warning(tag, message, ...) printf("\033[1;33m[%s] " message "\033[0m\n", tag, ##__VA_ARGS__);
#define Error(tag, message, ...)   printf("\033[1;31m[%s] " message "\033[0m\n", tag, ##__VA_ARGS__);

#endif    // CHIP8_CORE_H
