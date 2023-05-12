/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef TIME_TICK_H
#define TIME_TICK_H

#include <monotonic-time/monotonic_time.h>
#include <stddef.h>
#include <clog/clog.h>

typedef int (*TimeTickFn)(void* self);

typedef struct TimeTick {
    MonotonicTimeMs tickedUpToMonotonic;
    size_t targetDeltaTimeMs;
    void* ptr;
    TimeTickFn timeTickFn;
    Clog log;
} TimeTick;

void timeTickInit(TimeTick* self, size_t targetDeltaTimeMs, void* ptr, TimeTickFn tickFn, MonotonicTimeMs now,
                  Clog log);
int timeTickUpdate(TimeTick* self, MonotonicTimeMs now);

#endif
