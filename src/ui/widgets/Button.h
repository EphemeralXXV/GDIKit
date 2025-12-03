#pragma once

#include <string>
#include <functional>

#include "Widget.h"
#include "Color.h"

class Button : public Widget {
    public:
        Button(const std::wstring &t = L"Button");

        std::wstring text;
        HFONT font;
        Color backColor;
        Color hoverColor;
        Color pressColor;
        Color borderColor;
        Color textColor;

        void Render(HDC hdc) override;

        void SetOnClick(std::function<void()> cb);

    private:
        std::function<void()> onClick;
};