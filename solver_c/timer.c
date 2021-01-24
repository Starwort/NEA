#include "timer.h"

/* Create a timer with extra_milliseconds on the clock
 */
Time make_timer(int extra_milliseconds) {
    Time now;
    gettimeofday(&now, NULL);
    now.tv_usec += 1000 * extra_milliseconds;
    now.tv_sec += now.tv_usec / 1000000;
    now.tv_usec %= 1000000;
    return now;
}

/* Check if timer has expired
 */
bool expired(Time timer) {
    // timer has not been started; it is not expired
    if (timer.tv_sec == 0) {
        return false;
    }
    Time now;
    gettimeofday(&now, NULL);
    return now.tv_sec > timer.tv_sec
           || (now.tv_sec == timer.tv_sec && now.tv_usec > timer.tv_usec);
}
