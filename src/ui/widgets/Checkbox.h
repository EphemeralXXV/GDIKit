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
        bool checked;

        std::wstring text;
        HFONT font;
        Color boxColor;
        Color checkColor;
        Color hoverColor;
        Color textColor;
        
        std::function<void(bool)> onToggle; // Called when checkbox is toggled
};