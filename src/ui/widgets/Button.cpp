#include <windows.h>
#include <string>

#include "Color.h"
#include "Border.h"
#include "ScopedGDI.h"
#include "Button.h"

Button::Button(std::wstring t) :
    text(t),
    font(nullptr),
    backColor(Color::FromARGB(200,30,30,30)),
    hoverColor(Color::FromARGB(220,50,50,50)),
    pressColor(Color::FromARGB(255,20,110,220)),
    borderColor(Color::FromRGB(0,0,0)),
    textColor(Color::FromRGB(255,255,255))
{
    SetBorder(1, borderColor, BorderSide::All);
    AddMouseListener([this](const MouseEvent& e) {
        if(e.type == MouseEventType::Click) {
            if(onClick) onClick();
        }
    });
}

void Button::Render(HDC hdc) {
    RECT innerRect = ComputeInnerRect();

    // Background
    Color bgColor;
    if(!enabled) bgColor = Color::FromRGB(120,120,120);
    else if(pressed) bgColor = pressColor;
    else if(hovered) bgColor = hoverColor;
    else bgColor = backColor;
    SetBackgroundColor(bgColor);

    // Text
    SetBkMode(hdc, TRANSPARENT);
    ScopedSelectFont selFont(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    ::SetTextColor(hdc, textColor.toCOLORREF());
    DrawTextW(hdc, text.c_str(), (int)text.size(), &innerRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}

void Button::SetOnClick(std::function<void()> cb) {
    onClick = cb;
}