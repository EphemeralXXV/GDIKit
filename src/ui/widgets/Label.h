#pragma once

#include <string>

#include "Widget.h"
#include "Color.h"

class Label : public Widget {
    public:
        Label(const std::wstring &t = L"");

        std::wstring text;
        HFONT font; // Optional custom font
        Color textColor;

        void Render(HDC hdc) override;
};