#include <windows.h>
#include <string>

#include "Color.h"
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
    if(!effectiveDisplayed || !visible) return;

    int saved = SaveDC(hdc);

    RECT r = AbsRect();

    // Background
    HBRUSH br;
    if(!enabled) br = CreateSolidBrush(RGB(120,120,120));
    else if(pressed) br = CreateSolidBrush(pressColor.toCOLORREF());
    else if(hovered) br = CreateSolidBrush(hoverColor.toCOLORREF());
    else br = CreateSolidBrush(backColor.toCOLORREF());
    FillRect(hdc, &r, br);
    DeleteObject(br);

    // Border
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor.toCOLORREF());
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);

    // Text
    SetBkMode(hdc, TRANSPARENT);
    HFONT old = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    ::SetTextColor(hdc, textColor.toCOLORREF());
    DrawTextW(hdc, text.c_str(), (int)text.size(), &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
    SelectObject(hdc, old);

    RestoreDC(hdc, saved);
}

void Button::SetOnClick(std::function<void()> cb) {
    onClick = cb;
}