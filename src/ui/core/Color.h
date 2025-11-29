#pragma once

#include <windows.h>

struct Color {
    BYTE a, r, g, b;
    COLORREF toCOLORREF() const { return RGB(r, g, b); }
    static Color FromRGB(BYTE r, BYTE g, BYTE b) { return {255, r, g, b}; }
    static Color FromARGB(BYTE a, BYTE r, BYTE g, BYTE b) { return {a, r, g, b}; }
};