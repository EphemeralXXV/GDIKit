#include <stdint.h>

#include "Color.h"

// Using uint8_t instead of int for optimization
enum class BorderSide : uint8_t {
    None    = 0,
    Top     = 1 << 0,
    Bottom  = 1 << 1,
    Left    = 1 << 2,
    Right   = 1 << 3,
    All     = Top | Bottom | Left | Right
};

inline BorderSide operator|(BorderSide a, BorderSide b) {
    return static_cast<BorderSide>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline bool HasSide(BorderSide sides, BorderSide side) {
    return (static_cast<uint8_t>(sides) & static_cast<uint8_t>(side)) != 0;
}

struct Border {
    int thickness = 0;
    Color color = Color::FromARGB(0,0,0,0);;
    BorderSide sides = BorderSide::All;
};