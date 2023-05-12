/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <time-tick/time_tick.h>

void timeTickInit(TimeTick* self, size_t targetDeltaTimeMs, void* ptr, TimeTickFn tickFn, MonotonicTimeMs now, Clog log)
{
    self->tickedUpToMonotonic = now;
    self->targetDeltaTimeMs = targetDeltaTimeMs;
    self->timeTickFn = tickFn;
    self->ptr = ptr;
    self->log = log;
}

int timeTickUpdate(TimeTick* self, MonotonicTimeMs now)
{
    if (self->targetDeltaTimeMs == 0U) {
        self->tickedUpToMonotonic = now;

        return 0;
    }

    int delta = now - self->tickedUpToMonotonic;

    int iterationCount = delta / self->targetDeltaTimeMs;
    if (iterationCount > 30) {
        // CLOG_C_NOTICE(&self->log, "time is so much in the future, that we can not keep up");
        self->tickedUpToMonotonic = now;
        iterationCount = 1;
    }

    if (iterationCount > 2) {
        CLOG_C_NOTICE(&self->log, "should be updated more frequently, needs to do %d updates", iterationCount)
    }

    if (iterationCount == 0) {
        if ((self->targetDeltaTimeMs - delta) > (int) ((float) self->targetDeltaTimeMs * 0.8f)) {
            iterationCount = 1;
        } else {
            return 0;
        }
    }
    if (iterationCount < 0) {
        return 0;
    }

    if (iterationCount > 4) {
        iterationCount = 4;
    }

    int result = 0;
    for (int i = 0; i < iterationCount; i++) {
        result = self->timeTickFn(self->ptr);
        if (result < 0) {
            return result;
        }
    }

    self->tickedUpToMonotonic += iterationCount * self->targetDeltaTimeMs;

    return 0;
}
