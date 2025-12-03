#pragma once

#include <string>
#include <functional>

#include "Widget.h"
#include "Color.h"

class Checkbox : public Widget {
    public:
        Checkbox(const std::wstring& label = L"");

        bool checked;
        std::wstring text;
        
        Color boxColor;
        Color checkColor;
        Color hoverColor;
        Color textColor;

        void Render(HDC hdc) override;

        void SetOnToggle(std::function<void(bool)> cb);

    private:
        std::function<void(bool)> onToggle; // Called when checkbox is toggled
};