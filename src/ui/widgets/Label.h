#pragma once

#include <string>

#include "Widget.h"
#include "Color.h"

class Label : public Widget {
    public:
        // Constructor
        Label(std::wstring t = L"");

        // Appearance
        std::wstring GetText() const { return text; }
        void SetText(std::wstring newText) { text = newText; }

        HFONT GetFont() const { return font; }
        void SetFont(HFONT newFont) { font = newFont; }

        Color GetTextColor() const { return textColor; }
        void SetTextColor(Color newColor) { textColor = newColor; }

        // Rendering
        RECT ComputeRect(HDC hdc);
        void Render(HDC hdc) override;

    private:
        std::wstring text;
        HFONT font = nullptr;
        Color textColor = Color::FromRGB(255,255,255);
};