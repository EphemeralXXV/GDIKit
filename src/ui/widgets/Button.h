#pragma once

#include <string>
#include <functional>

#include "Widget.h"
#include "Color.h"

class Button : public Widget {
    public:
        // Constructor
        Button(std::wstring t = L"Button");

        // Appearance
        std::wstring GetText() const { return text; }
        void SetText(std::wstring newText) { text = newText; }

        HFONT GetFont() const { return font; }
        void SetFont(HFONT newFont) { font = newFont; }

        Color GetBackColor()    const { return backColor; }
        Color GetHoverColor()   const { return hoverColor; }
        Color GetPressColor()   const { return pressColor; }
        Color GetBorderColor()  const { return borderColor; }
        Color GetTextColor()    const { return textColor; }
        void SetBackColor(Color newColor)   { backColor = newColor; }
        void SetHoverColor(Color newColor)  { hoverColor = newColor; }
        void SetPressColor(Color newColor)  { pressColor = newColor; }
        void SetBorderColor(Color newColor) { borderColor = newColor; }
        void SetTextColor(Color newColor)   { textColor = newColor; }

        // Rendering
        void Render(HDC hdc) override;

        // Behavior
        void SetOnClick(std::function<void()> cb);

    private:
        std::wstring text;
        HFONT font = nullptr;
        Color backColor     = Color::FromARGB(200,30,30,30);
        Color hoverColor    = Color::FromARGB(220,50,50,50);
        Color pressColor    = Color::FromARGB(255,20,110,220);
        Color borderColor   = Color::FromRGB(0,0,0);
        Color textColor     = Color::FromRGB(255,255,255);

        std::function<void()> onClick;
};