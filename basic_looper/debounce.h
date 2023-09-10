#pragma once

class Debouncer {
public:
    /// constructs a new debouncer with a unitless settle time.
    Debouncer(int settle_time) {
        this->settle_time = settle_time;
    };

    /// advances the debouncer by the given dt and registers a new sample.
    void update(unsigned int dt, bool sample) {
        time_stable += dt;

        rising_edge_occurred = false;
        falling_edge_occurred = false;

        // rising or falling edge.
        if (sample != prev_sample) {
            // edge is genuine, switch was stable when transition occurred.
            if (time_stable >= settle_time) {
                current_state = sample;

                if (current_state == true)  rising_edge_occurred = true;
                if (current_state == false) falling_edge_occurred = true;
            }

            time_stable = 0;
        }

        // handle an edge case: if we've been stable for long enough, just use that value
        if (time_stable >= settle_time) current_state = sample;

        prev_sample = sample;
    };

    ///
    bool get() {
        return current_state;
    };

    uint32_t time_in_current_state() {
        return time_stable;
    }

    /// If a rising or falling edge occured during the last update, the corresponding function
    /// among these will return true
    bool rising_edge() {
        return rising_edge_occurred;
    };

    bool falling_edge() {
        return falling_edge_occurred;
    };

private:
    uint32_t time_stable = 0;
    int settle_time;
    bool current_state = false;
    bool prev_sample = false;

    bool rising_edge_occurred = false;
    bool falling_edge_occurred = false;
};
