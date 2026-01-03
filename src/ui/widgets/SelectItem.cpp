#include "SelectItem.h"
#include "Color.h"

SelectItem::SelectItem(std::wstring t, size_t idx) :
    text(t),
    font(nullptr),
    index(idx),
    selected(false),
    backColor(Color::FromRGB(40, 40, 40)),
    hoverColor(Color::FromRGB(60, 60, 60)),
    pressedColor(Color::FromRGB(70, 70, 70)),
    selectedColor(Color::FromRGB(50, 50, 50)),
    textColor(Color::FromRGB(255, 255, 255))
{
    AddMouseListener([this](const MouseEvent& e) {
        if(e.type == MouseEventType::Click) {
            if(onSelect) {
                onSelect(index);
            }
        }
    });
}

void SelectItem::SetOnSelect(std::function<void(size_t)> cb) {
    onSelect = std::move(cb);
}

void SelectItem::Render(HDC hdc) {
    RECT r = AbsRect();

    // Background
    HBRUSH br;
    
    if(pressed) {
        br = CreateSolidBrush(pressedColor.toCOLORREF());
    }
    else if(hovered) {
        br = CreateSolidBrush(hoverColor.toCOLORREF());
    }
    else if(selected) {
        br = CreateSolidBrush(selectedColor.toCOLORREF());
    }
    else {
        br = CreateSolidBrush(backColor.toCOLORREF());
    }
    FillRect(hdc, &r, br);
    DeleteObject(br);

    // Text
    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    HFONT oldFont = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    DrawTextW(
        hdc,
        text.c_str(),
        -1,
        &r,
        DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS
    );

    SelectObject(hdc, oldFont);
}