#include <string>

#include "Label.h"
#include "Color.h"

Label::Label(const std::wstring &t) : 
    text(t),
    font(nullptr),
    textColor(Color::FromRGB(255,255,255))
{}

// Compute text geometry from its contents
void Label::ComputeRect(HDC hdc) {
    SIZE size;
    HFONT old = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &size);
    SelectObject(hdc, old);
    SetSize(size.cx, size.cy);
}

void Label::Render(HDC hdc) {
    if (!visible) return;

    int saved = SaveDC(hdc);

    ComputeRect(hdc);
    RECT r = AbsRect();

    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    HFONT old = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    DrawTextW(hdc, text.c_str(), (int)text.size(), &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    SelectObject(hdc, old);
    RestoreDC(hdc, saved);
}