#include "common.h"

int fsize(FILE* fp) {
    int prev = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    fseek(fp, prev, SEEK_SET);  // go back to where we were
    return sz;
}
int _max(int nargs, ...) {
    va_list args;
    va_start(args, nargs);
    int rv = INT32_MIN;
    int arg;
    for (int _ = 0; _ < nargs; _++) {
        arg = va_arg(args, int);
        if (rv < arg) {
            rv = arg;
        }
    }
    va_end(args);
    return rv;
}
int _min(int nargs, ...) {
    va_list args;
    va_start(args, nargs);
    int rv = INT32_MAX;
    int arg;
    for (int _ = 0; _ < nargs; _++) {
        arg = va_arg(args, int);
        if (rv > arg) {
            rv = arg;
        }
    }
    va_end(args);
    return rv;
}
void* slice(void* arr, int start_idx, int end_idx, size_t elem_size) {
    int new_elem = (end_idx - start_idx);
    void* new_arr = malloc(elem_size * new_elem);
    uint8* arith_arr = (uint8*)arr;
    memcpy(new_arr, arith_arr + (start_idx * elem_size), elem_size * new_elem);
    free(arr);
    return new_arr;
}
int clamp(int num, int min, int max) {
    return (min > num) ? min : (max < num) ? max : num;
}
