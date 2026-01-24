#include <string>

#include "Checkbox.h"
#include "ScopedGDI.h"
#include "Color.h"

Checkbox::Checkbox(std::wstring label) :
    text(label)
{
    AddMouseListener([this](const MouseEvent& e) {
        if(e.type == MouseEventType::Click) {
            SetChecked(!checked);
        }
    });
}

void Checkbox::SetChecked(bool state) {
    if(state == checked) return;
    checked = state;
    if(onToggle) onToggle(checked); // Fire user-provided callback
}

void Checkbox::Render(HDC hdc) {
    RECT r = EffectiveRect();
    int boxSize = EffectiveHeight(); // square box same height as widget

    // Draw box background
    ScopedOwnedBrush br(hdc, hovered ? hoverColor.toCOLORREF() : boxColor.toCOLORREF());
    RECT checkboxRect = RECT{r.left, r.top, r.left + boxSize, r.top + boxSize};
    FillRect(hdc, &checkboxRect, br.get());

    // Draw checkmark if checked
    if(checked) {
        ScopedOwnedBrush checkBr(hdc, checkColor.toCOLORREF());
        RECT checkRect = {r.left + 4, r.top + 4, r.left + boxSize - 4, r.top + boxSize - 4};
        FillRect(hdc, &checkRect, checkBr.get());
    }

    // Draw label text
    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    ScopedSelectFont selFont(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    RECT textRect = { r.left + boxSize + 4, r.top, r.right, r.bottom };
    DrawTextW(hdc, text.c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}

void Checkbox::SetOnToggle(std::function<void(bool)> cb) {
    onToggle = cb;
}