#include "SelectItem.h"
#include "Color.h"
#include "ScopedGDI.h"

SelectItem::SelectItem(std::wstring t, std::string v) :
    text(t), value(v)
{
    SetPadding(4, 0); // Add some horizontal padding so that the text doesn't touch the border
    AddMouseListener([this](const MouseEvent& e) {
        if(e.type == MouseEventType::Click) {
            if(onSelect) {
                onSelect();
            }
        }
    });
}

void SelectItem::SetOnSelect(std::function<void()> cb) {
    onSelect = std::move(cb);
}

void SelectItem::Render(HDC hdc) {
    RECT innerRect = ComputeInnerRect();

    // Background
    Color bgColor;
    if(pressed) {
        bgColor = pressedColor;
    }
    else if(hovered) {
        bgColor = hoverColor;
    }
    else if(selected) {
        bgColor = selectedColor;
    }
    else {
        bgColor = backColor;
    }
    SetBackgroundColor(bgColor);

    // Text
    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    ScopedSelectFont oldFont(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    DrawTextW(
        hdc,
        text.c_str(),
        -1,
        &innerRect,
        DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS
    );
}