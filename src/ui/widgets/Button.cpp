#include <windows.h>
#include <string>

#include "Color.h"
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
    AddMouseListener([this](const MouseEvent& e) {
        if(e.type == MouseEventType::Click) {
            if(onClick) onClick();
        }
    });
}

void Button::Render(HDC hdc) {
    RECT r = AbsRect();

    // Background
    COLORREF brushColor;
    if(!enabled) brushColor = RGB(120,120,120);
    else if(pressed) brushColor = pressColor.toCOLORREF();
    else if(hovered) brushColor = hoverColor.toCOLORREF();
    else brushColor = backColor.toCOLORREF();
    ScopedBrush br(hdc, brushColor);
    FillRect(hdc, &r, br.get());

    // Border
    ScopedPen pen(hdc, PS_SOLID, 1, borderColor.toCOLORREF());
    ScopedSelectBrush brush(hdc, (HBRUSH)GetStockObject(NULL_BRUSH));
    Rectangle(hdc, r.left, r.top, r.right, r.bottom);

    // Text
    SetBkMode(hdc, TRANSPARENT);
    ScopedSelectFont selFont(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    ::SetTextColor(hdc, textColor.toCOLORREF());
    DrawTextW(hdc, text.c_str(), (int)text.size(), &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}

void Button::SetOnClick(std::function<void()> cb) {
    onClick = cb;
}