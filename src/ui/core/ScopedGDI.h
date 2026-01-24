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
class ScopedOwnedPen {
public:
    ScopedOwnedPen(HDC hdc, int style, int width, COLORREF color) : hdc(hdc) {
        pen = CreatePen(style, width, color);
        oldPen = (HPEN)SelectObject(hdc, pen);
    }

    ~ScopedOwnedPen() {
        if(hdc && oldPen && oldPen != HGDI_ERROR) {
            SelectObject(hdc, oldPen);
        }
        if(pen) {
            DeleteObject(pen);
        }
    }

    HPEN get() const { return pen; }

    ScopedOwnedPen(const ScopedOwnedPen&) = delete;
    ScopedOwnedPen& operator=(const ScopedOwnedPen&) = delete;

private:
    HDC  hdc{};
    HPEN pen{};
    HPEN oldPen{};
};

class ScopedOwnedBrush {
public:
    explicit ScopedOwnedBrush(HDC hdc, COLORREF color) : hdc(hdc) {
        brush = CreateSolidBrush(color);
        oldBrush = (HBRUSH)SelectObject(hdc, brush);
    }

    ~ScopedOwnedBrush() {
        if(hdc && oldBrush && oldBrush != HGDI_ERROR) {
            SelectObject(hdc, oldBrush);
        }
        if(brush) {
            DeleteObject(brush);
        }
    }

    HBRUSH get() const { return brush; }

    ScopedOwnedBrush(const ScopedOwnedBrush&) = delete;
    ScopedOwnedBrush& operator=(const ScopedOwnedBrush&) = delete;

private:
    HDC    hdc{};
    HBRUSH brush{};
    HBRUSH oldBrush{};
};

class ScopedOwnedFont {
public:
    explicit ScopedOwnedFont(HDC hdc, HFONT font) : hdc(hdc), font(font) {
        oldFont = (HFONT)SelectObject(hdc, font);
    }

    ~ScopedOwnedFont() {
        if(hdc && oldFont && oldFont != HGDI_ERROR) {
            SelectObject(hdc, oldFont);
        }
        if(font) {
            DeleteObject(font);
        }
    }

    HFONT get() const { return font; }

    ScopedOwnedFont(const ScopedOwnedFont&) = delete;
    ScopedOwnedFont& operator=(const ScopedOwnedFont&) = delete;

private:
    HDC   hdc{};
    HFONT font{};
    HFONT oldFont{};
};