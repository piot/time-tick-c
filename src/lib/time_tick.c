/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <inttypes.h>
#include <time-tick/time_tick.h>

void timeTickInit(TimeTick* self, size_t targetDeltaTimeMs, void* ptr, TimeTickFn tickFn, MonotonicTimeMs now, Clog log)
{
    CLOG_ASSERT(targetDeltaTimeMs != 0U, "delta time can not be zero")
    self->tickedUpToMonotonic = now;
    self->lastAdvancedMonotonic = now;
    self->targetDeltaTimeMs = targetDeltaTimeMs;
    self->laggingBehindCount = 0;
    self->timeTickFn = tickFn;
    self->state = TimeTickStateNormal;
    self->ptr = ptr;
    self->log = log;
    self->useQualityChecks = true;
}

void timeTickSetQualityCheckEnabled(TimeTick* self, bool on)
{
    self->useQualityChecks = on;
}

int timeTickUpdate(TimeTick* self, MonotonicTimeMs now)
{
    if (self->state == TimeTickStateFailed) {
        return -2;
    }

    MonotonicTimeMs timeSinceLastAdvance = now - self->lastAdvancedMonotonic;
    if (timeSinceLastAdvance < (5 * (MonotonicTimeMs) self->targetDeltaTimeMs / 10)) {
       return 0;
    }

    MonotonicTimeMs tickTimeAheadDiff = self->tickedUpToMonotonic - now;

    if (tickTimeAheadDiff >= 0) {
        return 0;
    }

    size_t timeDiff = (size_t) (-tickTimeAheadDiff);
    size_t optimalIterationCount = (timeDiff / self->targetDeltaTimeMs) + 1;

    size_t iterationCount = optimalIterationCount;
    const size_t maxIterationCountEachTick = 3;
    if (iterationCount > maxIterationCountEachTick) {
        iterationCount = maxIterationCountEachTick;
    }

    if (iterationCount >= maxIterationCountEachTick && self->useQualityChecks) {
        self->laggingBehindCount++;
        if (self->laggingBehindCount > 30) {
            CLOG_C_SOFT_ERROR(&self->log, "CPU can not catch up. Lagging %" PRId64 " ms. stopping execution",
                              -tickTimeAheadDiff)
            self->state = TimeTickStateFailed;
            return -1;
        }
        if ((self->laggingBehindCount % 20) == 0) {
            CLOG_C_NOTICE(&self->log, "CPU bound. Lagging %" PRId64 " ms. thinking about stopping the time ticker",
                          -tickTimeAheadDiff)
        }
    } else {
        self->laggingBehindCount = 0;
    }

    self->lastAdvancedMonotonic = now;

    for (size_t i = 0; i < iterationCount; i++) {
        int result = self->timeTickFn(self->ptr);
        if (result < 0) {
            self->state = TimeTickStateFailed;
            return result;
        }
    }

    self->tickedUpToMonotonic += self->targetDeltaTimeMs * iterationCount;

    return 0;
}
