#include <string>

#include "Checkbox.h"
#include "Color.h"

Checkbox::Checkbox(const std::wstring& label = L"") :
    text(label),
    checked(false),
    boxColor(Color::FromARGB(255, 50, 50, 50)),
    checkColor(Color::FromARGB(255, 20, 110, 220)),
    hoverColor(Color::FromARGB(255, 80, 80, 80)),
    textColor(Color::FromRGB(255, 255, 255))
{}

void Checkbox::Render(HDC hdc) {
    if(!visible) return;
    int saved = SaveDC(hdc);

    RECT r = AbsRect();
    int boxSize = height; // square box same height as widget

    // Draw box background
    HBRUSH br = CreateSolidBrush(hovered ? hoverColor.toCOLORREF() : boxColor.toCOLORREF());
    RECT checkboxRect = RECT{r.left, r.top, r.left + boxSize, r.top + boxSize};
    FillRect(hdc, &checkboxRect, br);
    DeleteObject(br);

    // Draw checkmark if checked
    if(checked) {
        HBRUSH checkBr = CreateSolidBrush(checkColor.toCOLORREF());
        RECT checkRect = {r.left + 4, r.top + 4, r.left + boxSize - 4, r.top + boxSize - 4};
        FillRect(hdc, &checkRect, checkBr);
        DeleteObject(checkBr);
    }

    // Draw label text
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor.toCOLORREF());
    HFONT oldFont = (HFONT)SelectObject(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    RECT textRect = { r.left + boxSize + 4, r.top, r.right, r.bottom };
    DrawTextW(hdc, text.c_str(), (int)text.size(), &textRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
    SelectObject(hdc, oldFont);

    RestoreDC(hdc, saved);
}

void Checkbox::OnMouseDown(POINT p) {
    if(!enabled) return;
    if(MouseInRect(p)) pressed = true;
}

void Checkbox::OnMouseUp(POINT p) {
    if(!enabled) return;

    // Toggle checked only if mouse up while over the checkbox
    if(pressed && MouseInRect(p)) {
        checked = !checked;
        if(onToggle) onToggle(checked); // Fire user-provided callback
    }
    pressed = false;
}