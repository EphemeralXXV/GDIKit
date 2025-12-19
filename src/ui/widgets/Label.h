#pragma once

#include <string>

#include "Widget.h"
#include "Color.h"

class Label : public Widget {
    public:
        // Constructor
        Label(const std::wstring &t = L"");

        // Appearance
        std::wstring GetText() const { return text; }
        void SetText(std::wstring newText) { text = newText; }

        HFONT GetFont() const { return font; }
        void SetFont(HFONT newFont) { font = newFont; }

        Color GetTextColor() const { return textColor; }
        void SetTextColor(Color newColor) { textColor = newColor; }

        // Rendering
        void ComputeRect(HDC hdc);
        void Render(HDC hdc) override;

    private:
        std::wstring text;
        HFONT font;
        Color textColor;
};