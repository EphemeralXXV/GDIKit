#pragma once

#include <string>

#include "Widget.h"
#include "Color.h"

enum class TextAlignH {
    Left,
    Center,
    Right
};

enum class TextAlignV {
    Top,
    Center,
    Bottom
};

class Label : public Widget {
    public:
        // Constructor
        Label(std::wstring t = L"");

        // Appearance
        std::wstring GetText() const { return text; }
        void SetText(std::wstring newText);

        HFONT GetFont() const { return font; }
        void SetFont(HFONT newFont);

        Color GetTextColor() const { return textColor; }
        void SetTextColor(Color newColor) { textColor = newColor; }

        TextAlignH GetHAlign() const { return hAlign; }
        void SetHAlign(TextAlignH newAlign) { hAlign = newAlign; }

        TextAlignV GetVAlign() const { return vAlign; }
        void SetVAlign(TextAlignV newAlign) { vAlign = newAlign; }

        // Rendering
        HDC GetMeasureDC();
        SIZE ComputeTextSize();
        void UpdateInternalLayout() override;
        void Render(HDC hdc) override;

    private:
        std::wstring text;
        HFONT font = nullptr;
        Color textColor = Color::FromRGB(255,255,255);
        TextAlignH hAlign = TextAlignH::Left;
        TextAlignV vAlign = TextAlignV::Top;

        UINT ComputeDrawTextFlags() const;
};