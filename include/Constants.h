// Constants.h
#pragma once

namespace Constants {
    inline constexpr float AMAX = 90;
    inline constexpr float AMIN = -90;
    inline constexpr float EMAX = 89;
    inline constexpr float EMIN = 0;
    inline constexpr float PMAX = 100;
    inline constexpr float PMIN = 0;
    inline constexpr float GRAVITY = -9.81;
    inline constexpr int MAX_DEBRIS = 300;   // cap on total debris pieces alive at once, oldest get dropped past this
    inline constexpr float TARGET_RESPAWN_DELAY = 1.2f;   // seconds the target stays gone after being hit, before reappearing elsewhere
    inline constexpr int DEBRIS_COLOR_VARIATION = 20;
    inline constexpr int MISSES_ALLOWED = 10; // maximum number of misses allowed before game over
}
