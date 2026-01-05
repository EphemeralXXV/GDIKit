// -------------------------------------- //
// -------------------------------------- //
// --- HELPER CLASSES FOR GDI OBJECTS --- //
// -------------------------------------- //
// -------------------------------------- //

#pragma once
#include <windows.h>

// ------------------------------
// Low-level lifetime RAII
// (owns object, does NOT select)
// ------------------------------
class ScopedGdiObject {
public:
    explicit ScopedGdiObject(HGDIOBJ obj) : obj(obj) {}
    ~ScopedGdiObject() {
        if(obj) {
            DeleteObject(obj);
        }
    }

    ScopedGdiObject(const ScopedGdiObject&) = delete;
    ScopedGdiObject& operator=(const ScopedGdiObject&) = delete;

    HGDIOBJ get() const { return obj; }

private:
    HGDIOBJ obj{};
};

// ----------------------------------
// Low-level selection RAII
// (selects + restores, no ownership)
// ----------------------------------
class ScopedSelectPen {
public:
    ScopedSelectPen(HDC hdc, HPEN pen) : hdc(hdc) {
        oldPen = (HPEN)SelectObject(hdc, pen);
    }
    ~ScopedSelectPen() {
        if(hdc && oldPen) {
            SelectObject(hdc, oldPen);
        }
    }

    ScopedSelectPen(const ScopedSelectPen&) = delete;
    ScopedSelectPen& operator=(const ScopedSelectPen&) = delete;

private:
    HDC  hdc{};
    HPEN oldPen{};
};

class ScopedSelectBrush {
public:
    ScopedSelectBrush(HDC hdc, HBRUSH brush) : hdc(hdc) {
        oldBrush = (HBRUSH)SelectObject(hdc, brush);
    }
    ~ScopedSelectBrush() {
        if(hdc && oldBrush) {
            SelectObject(hdc, oldBrush);
        }
    }

    ScopedSelectBrush(const ScopedSelectBrush&) = delete;
    ScopedSelectBrush& operator=(const ScopedSelectBrush&) = delete;

private:
    HDC    hdc{};
    HBRUSH oldBrush{};
};

class ScopedSelectFont {
public:
    ScopedSelectFont(HDC hdc, HFONT font) : hdc(hdc) {
        oldFont = (HFONT)SelectObject(hdc, font);
    }
    ~ScopedSelectFont() {
        if(hdc && oldFont) {
            SelectObject(hdc, oldFont);
        }
    }

    ScopedSelectFont(const ScopedSelectFont&) = delete;
    ScopedSelectFont& operator=(const ScopedSelectFont&) = delete;

private:
    HDC   hdc{};
    HFONT oldFont{};
};

// ------------------------------------
// High-level combined helpers
// (create + select + restore + delete)
// ------------------------------------
class ScopedPen {
public:
    ScopedPen(HDC hdc, int style, int width, COLORREF color) : hdc(hdc) {
        pen = CreatePen(style, width, color);
        oldPen = (HPEN)SelectObject(hdc, pen);
    }

    ~ScopedPen() {
        if(hdc && oldPen) {
            SelectObject(hdc, oldPen);
        }
        if(pen) {
            DeleteObject(pen);
        }
    }

    HPEN get() const { return pen; }

    ScopedPen(const ScopedPen&) = delete;
    ScopedPen& operator=(const ScopedPen&) = delete;

private:
    HDC  hdc{};
    HPEN pen{};
    HPEN oldPen{};
};

class ScopedBrush {
public:
    explicit ScopedBrush(HDC hdc, COLORREF color) : hdc(hdc) {
        brush = CreateSolidBrush(color);
        oldBrush = (HBRUSH)SelectObject(hdc, brush);
    }

    ~ScopedBrush() {
        if(hdc && oldBrush) {
            SelectObject(hdc, oldBrush);
        }
        if(brush) {
            DeleteObject(brush);
        }
    }

    HBRUSH get() const { return brush; }

    ScopedBrush(const ScopedBrush&) = delete;
    ScopedBrush& operator=(const ScopedBrush&) = delete;

private:
    HDC    hdc{};
    HBRUSH brush{};
    HBRUSH oldBrush{};
};

class ScopedFont {
public:
    explicit ScopedFont(HDC hdc, HFONT font) : hdc(hdc), font(font) {
        oldFont = (HFONT)SelectObject(hdc, font);
    }

    ~ScopedFont() {
        if(hdc && oldFont) {
            SelectObject(hdc, oldFont);
        }
        if(font) {
            DeleteObject(font);
        }
    }

    HFONT get() const { return font; }

    ScopedFont(const ScopedFont&) = delete;
    ScopedFont& operator=(const ScopedFont&) = delete;

private:
    HDC   hdc{};
    HFONT font{};
    HFONT oldFont{};
};