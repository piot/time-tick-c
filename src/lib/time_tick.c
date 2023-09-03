/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <inttypes.h>
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
    }

    size_t iterationCount = 1;

    MonotonicTimeMs tickTimeAheadDiff = self->tickedUpToMonotonic - now;

    MonotonicTimeMs maximumAheadTimeFactorUntilSkipping = (3 * (MonotonicTimeMs) self->targetDeltaTimeMs);
    if (tickTimeAheadDiff >= maximumAheadTimeFactorUntilSkipping) {
        // We have ticked too much into the future. We need to skip this update
        // CLOG_C_NOTICE(&self->log, "timeTickUpdate() has been called too frequently, skipping a tick this update")
        return 0;
    }

    const MonotonicTimeMs maximumBehindTimeFactorUntilTickingExtra = (-2 * (MonotonicTimeMs) self->targetDeltaTimeMs);
    const MonotonicTimeMs maximumBehindTimeFactorUntilTickingMax = (-4 * (MonotonicTimeMs) self->targetDeltaTimeMs);
    if (tickTimeAheadDiff < maximumBehindTimeFactorUntilTickingMax) {
        iterationCount = 3;
        CLOG_C_NOTICE(&self->log,
                      "timeTickUpdate() should be updated more frequently, needs to do %zd ticks this update",
                      iterationCount)
    } else if (tickTimeAheadDiff <= maximumBehindTimeFactorUntilTickingExtra) {
        iterationCount = 2;
    }

    for (size_t i = 0; i < iterationCount; i++) {
        int result = self->timeTickFn(self->ptr);
        if (result < 0) {
            return result;
        }
    }

    self->tickedUpToMonotonic += self->targetDeltaTimeMs * iterationCount;

    return 0;
}
