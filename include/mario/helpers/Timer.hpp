#pragma once

namespace mario {
    //  Delta time, fixed step support.
    class Timer {
    public:
        void reset();

        float delta_seconds() const;
    };

    // Loose coupling for game events.
    class EventBus {
    public:
        void emit(int event_id);

        void subscribe(int event_id);
    };
} // namespace mario
