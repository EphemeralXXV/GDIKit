#include <string>
#include <cmath>
#include <algorithm>

#include "Slider.h"
#include "Color.h"

// --- Slider ------------------------------------------------------------
Slider::Slider(
    const std::wstring& label,
    float minVal = 0.0f,
    float maxVal = 1.0f,
    float step = 0.0f,      // 0 means free movement
    float val = 0.5f
) :
    label(label),
    minValue(minVal),
    maxValue(maxVal),
    step(step),
    value(val),
    showValue(true),
    showLabel(true),
    handleWidth(10),
    sliderOffsetY(0),
    trackColor(Color::FromRGB(100, 100, 100)),
    handleColor(Color::FromRGB(180, 180, 180)),
    hoverColor(Color::FromRGB(220, 220, 220)),
    dragColor(Color::FromRGB(150, 150, 255)),
    isDragging(false)
{}

// Compute handle rect in absolute coordinates
RECT Slider::HandleRect() const {
    // Current handle position as a fraction of the whole slider
    float t = (value - minValue) / (maxValue - minValue);
    int x = AbsX() + int(t * (width - handleWidth));
    int y = AbsY() + sliderOffsetY;
    return RECT{x, y, x + handleWidth, y + preferredHeight};
}

void Slider::ComputeSliderOffsetY(HDC hdc) {
    if(!(showLabel || showValue)) {
        sliderOffsetY = 0;
        return;
    }
    HFONT oldFont = (HFONT)SelectObject(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    int textHeight = tm.tmHeight;
    sliderOffsetY = textHeight + 2; // 2px padding
    SelectObject(hdc, oldFont);
}

void Slider::Render(HDC hdc) {
    if(!visible) return;

    int saved = SaveDC(hdc);

    // Compute text offset if needed
    ComputeSliderOffsetY(hdc);

    // Track
    RECT track = {
        AbsX(),
        AbsY() + sliderOffsetY + preferredHeight/2 - 2,
        AbsX() + width,
        AbsY() + sliderOffsetY + preferredHeight/2 + 2
    };
    HBRUSH br = CreateSolidBrush(trackColor.toCOLORREF());
    FillRect(hdc, &track, br);
    DeleteObject(br);

    // Handle
    RECT hr = HandleRect();
    Color handleCol = handleColor;

    // Determine handle color based on state
    POINT p;
    GetCursorPos(&p);
    if(PtInRect(&hr, p)) { // Handle hovered
        if(parent && parent->MouseInRect(p)) // Only if slider doesn't overflow its container
            handleCol = hoverColor;
    }
    if(isDragging) { // Dragging takes precendence over hovering
        handleCol = dragColor;
    }
    br = CreateSolidBrush(handleCol.toCOLORREF());
    FillRect(hdc, &hr, br);
    DeleteObject(br);

    // Add labels above slider
    HFONT oldFont = (HFONT)SelectObject(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    RECT textRect = { AbsX(), AbsY(), AbsX() + width, AbsY() + sliderOffsetY};

    // Left-aligned label
    DrawTextW(hdc, label.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Right-aligned numeric value
    if(showValue) {
        std::wstring val = std::to_wstring((int)value);
        DrawTextW(hdc, val.c_str(), -1, &textRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
    }
    SelectObject(hdc, oldFont);

    RestoreDC(hdc, saved);
}

void Slider::UpdateValueFromMouse(int mouseX) {
    // Calculate new value
    RECT r = AbsRect();
    int relX = mouseX - r.left - handleWidth / 2;
    float t = (float)relX / (float)(width - handleWidth);
    t = std::clamp(t, 0.0f, 1.0f);
    float newValue = minValue + t * (maxValue - minValue);

    // Snap to step
    if(step > 0.0f)
        newValue = minValue + step * std::round((newValue - minValue) / step);

    // Update value, fire callback
    if(newValue != value) {
        value = newValue;
        if(onValueChanged) onValueChanged(value);
    }
}

void Slider::OnMouseMove(POINT p) {
    if(!enabled) return;

    if(isDragging) UpdateValueFromMouse(p.x);
}

void Slider::OnMouseDown(POINT p) {
    if(!enabled) return;

    // React also to clicks on the track itself
    RECT trackRect = HandleRect();
    trackRect.left = AbsX();
    trackRect.right = AbsX() + width;
    if(!PtInRect(&trackRect, p)) return;

    isDragging = true;
    UpdateValueFromMouse(p.x);
}

void Slider::OnMouseUp(POINT p) {
    if(!enabled) return;

    if(isDragging) {
        UpdateValueFromMouse(p.x);
        isDragging = false;
    }
}