// Implements the Timer and EventBus utility classes.
// Timer provides timing functionality (currently stubbed), and EventBus provides a simple event system (currently stubbed).

#include "mario/helpers/Timer.hpp"

namespace mario {
    // Resets the timer (stub implementation).
    void Timer::reset() {
    }

    // Returns the elapsed time in seconds since the last reset (stub implementation).
    float Timer::delta_seconds() const { return 0.0f; }

    // Emits an event with the given event ID (stub implementation).
    void EventBus::emit(int event_id) { (void) event_id; }
    // Subscribes to an event with the given event ID (stub implementation).
    void EventBus::subscribe(int event_id) { (void) event_id; }
} // namespace mario
