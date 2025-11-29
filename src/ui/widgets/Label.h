#pragma once

#include <string>

#include "Widget.h"
#include "Color.h"

class Label : public Widget {
    public:
        std::wstring text;
        HFONT font; // Optional custom font
        Color textColor;

        Label(const std::wstring &t);

        void Render(HDC hdc) override;

        // Labels accept no input
        void OnMouseDown(POINT) override {}
        void OnMouseUp(POINT) override {}
        void OnMouseMove(POINT) override {}
};