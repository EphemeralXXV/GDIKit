#pragma once

#include <string>
#include <functional>

#include "Widget.h"
#include "Color.h"

class Checkbox : public Widget {
    public:
        // Constructor
        Checkbox(std::wstring label = L"");

        // State
        bool IsChecked() const { return checked; }
        void SetChecked(bool state);

        // Appearance
        std::wstring GetText() const { return text; }
        void SetText(std::wstring newText) { text = newText; }

        HFONT GetFont() const { return font; }
        void SetFont(HFONT newFont) { font = newFont; }

        Color GetBoxColor()     const { return boxColor; }
        Color GetCheckColor()   const { return checkColor; }
        Color GetHoverColor()   const { return hoverColor; }
        Color GetTextColor()    const { return textColor; }
        void SetBoxColor(Color newColor)    { boxColor = newColor; }
        void SetCheckColor(Color newColor)  { checkColor = newColor; }
        void SetHoverColor(Color newColor)  { hoverColor = newColor; }
        void SetTextColor(Color newColor)   { textColor = newColor; }

        // Rendering
        void Render(HDC hdc) override;

        // Behavior
        void SetOnToggle(std::function<void(bool)> cb);

    private:
        bool checked = false;

        std::wstring text;
        HFONT font = nullptr;
        Color boxColor      = Color::FromARGB(255, 50, 50, 50);
        Color checkColor    = Color::FromARGB(255, 20, 110, 220);
        Color hoverColor    = Color::FromARGB(255, 80, 80, 80);
        Color textColor     = Color::FromRGB(255, 255, 255);
        
        std::function<void(bool)> onToggle; // Called when checkbox is toggled
};