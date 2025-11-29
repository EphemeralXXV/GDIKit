#pragma once

#include <string>
#include <functional>

#include "Widget.h"
#include "Color.h"

class Checkbox : public Widget {
    public:
        bool checked;
        std::wstring text;
        
        Color boxColor;
        Color checkColor;
        Color hoverColor;
        Color textColor;

        std::function<void(bool)> onToggle; // Called when checkbox is toggled

        Checkbox(const std::wstring& label);

        void Render(HDC hdc) override;
        void OnMouseDown(POINT p);
        void OnMouseUp(POINT p) override;
};