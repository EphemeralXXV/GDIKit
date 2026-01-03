#include <string>

#include "Label.h"
#include "Color.h"

Label::Label(std::wstring t) : 
    text(t),
    font(nullptr),
    textColor(Color::FromRGB(255,255,255))
{}

// Compute text geometry from its contents
RECT Label::ComputeRect(HDC hdc) {
    SIZE size;
    HFONT old = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &size);
    SelectObject(hdc, old);
    return RECT{AbsX(), AbsY(), AbsX() + size.cx, AbsY() + size.cy};
} 

void Label::Render(HDC hdc) {
    // If there is not enough space, skip drawing
    RECT computedRect = ComputeRect(hdc);
    RECT setRect = AbsRect();
    if(
        // Height
        (setRect.bottom - setRect.top) < (computedRect.bottom - computedRect.top)
        ||
        // Width
        (setRect.right - setRect.left) < (computedRect.right - computedRect.left)
    ) return;

    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    HFONT old = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    DrawTextW(hdc, text.c_str(), (int)text.size(), &setRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    SelectObject(hdc, old);
}