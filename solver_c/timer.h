#pragma once
#include "common.h"

#include <sys/time.h>

#define start(timer, time)        \
    if (!timer.tv_sec) {          \
        timer = make_timer(time); \
    }

typedef struct timeval Time;

Time make_timer(int extra_milliseconds);
bool expired(Time timer);
