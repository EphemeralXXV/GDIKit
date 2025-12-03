#include <string>

#include "Label.h"
#include "Color.h"

Label::Label(const std::wstring &t) : 
    text(t),
    font(nullptr),
    textColor(Color::FromRGB(255,255,255))
{}

void Label::Render(HDC hdc) {
    if (!visible) return;

    int saved = SaveDC(hdc);
    RECT r = AbsRect();

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor.toCOLORREF());
    HFONT old = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    DrawTextW(hdc, text.c_str(), (int)text.size(), &r, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    SelectObject(hdc, old);
    RestoreDC(hdc, saved);
}