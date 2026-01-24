#include <string>

#include "Label.h"
#include "Color.h"
#include "ScopedGDI.h"

Label::Label(std::wstring t) : 
    text(t)
{}

// Compute text geometry from its contents
RECT Label::ComputeRect(HDC hdc) {
    SIZE size;
    ScopedSelectFont old(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &size);
    return RECT{AbsX(), AbsY(), AbsX() + size.cx, AbsY() + size.cy};
} 

void Label::Render(HDC hdc) {
    // If there is not enough space, skip drawing
    RECT computedRect = ComputeRect(hdc);
    RECT setRect = EffectiveRect();
    if(
        // Height
        (setRect.bottom - setRect.top) < (computedRect.bottom - computedRect.top)
        ||
        // Width
        (setRect.right - setRect.left) < (computedRect.right - computedRect.left)
    ) return;

    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    ScopedSelectFont old(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    DrawTextW(hdc, text.c_str(), -1, &setRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}