#ifndef COMMON_LOADED
#define COMMON_LOADED
#include "variadicmacros.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned int uint;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef __int128_t int128;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef __uint128_t uint128;
typedef char* string;
typedef char* str;

#define eprintfln(format, ...) fprintf(stderr, format "\n", __VA_ARGS__)
#define eprintln(string) fprintf(stderr, string "\n")
#define eprintf(format, ...) fprintf(stderr, format, __VA_ARGS__)
#define eprint(string) fprintf(stderr, string)
#define fprintfln(file, format, ...) fprintf(file, format "\n", __VA_ARGS__)
#define fprintln(file, string) fprintf(file, string "\n")
#define fprint(file, string) fprintf(file, string)
#define printfln(format, ...) printf(format "\n", __VA_ARGS__)
#define println(format) printf(format "\n")
#define print(format) printf(format)

#define max(...) _max(VA_SIZE(__VA_ARGS__), __VA_ARGS__)
#define min(...) _min(VA_SIZE(__VA_ARGS__), __VA_ARGS__)
#define floor(x) (int)x
#define ceil(x) (int)(x + 0.99999)

#ifdef DEBUG
    #define dprintfln(format, ...) printf(format "\n", __VA_ARGS__)
    #define dprintln(string) printf(string "\n")
    #define dprintf(string, ...) printf(string, __VA_ARGS__)
    #define dprint(string) printf(string)
#else
    #define dprintfln(format, ...)
    #define dprintln(string)
    #define dprintf(string, ...)
    #define dprint(string)
#endif

int fsize(FILE* fp);
int _max(int nargs, ...);
int _min(int nargs, ...);
int clamp(int num, int min, int max);
void* slice(void* arr, int start_idx, int end_idx, size_t elem_size);
#endif