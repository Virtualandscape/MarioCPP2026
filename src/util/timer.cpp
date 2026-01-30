#include "mario/util/Timer.hpp"

namespace mario {
    void Timer::reset() {
    }

    float Timer::delta_seconds() const { return 0.0f; }

    void EventBus::emit(int event_id) { (void) event_id; }
    void EventBus::subscribe(int event_id) { (void) event_id; }
} // namespace mario
