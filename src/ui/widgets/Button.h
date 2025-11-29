#pragma once

#include <string>
#include <functional>

#include "Widget.h"
#include "Color.h"

class Button : public Widget {
    public:
        std::wstring text;
        HFONT font;
        Color backColor;
        Color hoverColor;
        Color pressColor;
        Color borderColor;
        Color textColor;

        std::function<void()> onClick;

        Button(const std::wstring &t);

        void Render(HDC hdc) override;

        void OnMouseDown(POINT p) override;
        void OnMouseUp(POINT p) override;
};