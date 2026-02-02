#include <string>

#include "Label.h"
#include "Color.h"
#include "ScopedGDI.h"

Label::Label(std::wstring t) : 
    text(t)
{
    SetChildrenClipping(true);
    SetAutoWidth(true);
    SetAutoHeight(true);
}

// Helper to temporarily get a persistent memory DC for text measuring
HDC Label::GetMeasureDC() {
    static HDC hdc = [] {
        HDC screen = GetDC(nullptr);
        HDC mem = CreateCompatibleDC(screen);
        ReleaseDC(nullptr, screen);
        return mem;
    }();
    return hdc;
}

// Compute text geometry from its contents
SIZE Label::ComputeTextSize() {
    HDC hdc = GetMeasureDC();
    ScopedSelectFont old(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    SIZE size{0, 0};    
    const std::wstring text = GetText();
    if(!text.empty()) {
        GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &size);
    }
    return size;
} 

UINT Label::ComputeDrawTextFlags() const {
    UINT flags = DT_SINGLELINE;

    // Horizontal alignment
    switch(hAlign) {
        case TextAlignH::Left:   flags |= DT_LEFT;   break;
        case TextAlignH::Center: flags |= DT_CENTER; break;
        case TextAlignH::Right:  flags |= DT_RIGHT;  break;
    }
    // Vertical alignment
    switch(vAlign) {
        case TextAlignV::Top:    flags |= DT_TOP;     break;
        case TextAlignV::Center: flags |= DT_VCENTER; break;
        case TextAlignV::Bottom: flags |= DT_BOTTOM;  break;
    }

    return flags;
}

void Label::SetText(std::wstring newText) {
    text = newText;
    InvalidateLayout(); // Text change may affect size (ergo the rect)
}

void Label::SetFont(HFONT newFont) {
    font = newFont;
    InvalidateLayout();
}

// Apply auto size
void Label::UpdateInternalLayout() {
    if(!widthProperties.isAuto && !heightProperties.isAuto) return;

    SIZE textSize = ComputeTextSize();

    int w = width;
    int h = height;

    if(widthProperties.isAuto) {
        w = textSize.cx + padding.left + padding.right;
    }
    if(heightProperties.isAuto) {
        h = textSize.cy + padding.top + padding.bottom;
    }

    SetLayoutSize(w, h);
}

void Label::Render(HDC hdc) {
    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    ScopedSelectFont old(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    DrawTextW(hdc, text.c_str(), -1, &effectiveRect, ComputeDrawTextFlags());
}