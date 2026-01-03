#include <string>
#include <cmath>
#include <algorithm>

#include "Slider.h"
#include "Color.h"

// --- Slider ------------------------------------------------------------
Slider::Slider(
    std::wstring label,
    float minVal = 0.0f,
    float maxVal = 1.0f,
    float step = 0.0f,      // 0 means free movement
    float val = 0.5f
) :
    label(label),
    font(nullptr),
    minValue(minVal),
    maxValue(maxVal),
    step(step),
    value(val),
    showValue(true),
    showLabel(true),
    sliderOffsetY(0),
    handleWidth(10),
    handleHeight(20),
    trackColor(Color::FromRGB(100, 100, 100)),
    handleColor(Color::FromRGB(180, 180, 180)),
    hoverColor(Color::FromRGB(220, 220, 220)),
    dragColor(Color::FromRGB(150, 150, 255)),
    labelColor(Color::FromRGB(255, 255, 255)),
    isDragging(false),
    handleHovered(false)
{
    AddMouseListener([this](const MouseEvent& e) {
        switch(e.type) {
            case MouseEventType::Down: {
                // React also to clicks on the track itself
                // Not using MouseInRect, because the Slider AbsRect...
                // ...doesn't account for the label offset
                RECT trackRect = HandleRect();
                trackRect.left = AbsX();
                trackRect.right = AbsX() + width;
                if(!PtInRect(&trackRect, e.pos)) {
                    break;
                }
                isDragging = true;
                UpdateValueFromMouse(e.pos.x);
                break;
            }

            case MouseEventType::Move: {
                RECT hr = HandleRect();
                handleHovered = PtInRect(&hr, e.pos);

                if(isDragging) {
                    UpdateValueFromMouse(e.pos.x);
                }
                break;
            }

            case MouseEventType::Leave:
                handleHovered = false;
                break;

            case MouseEventType::Up:
                isDragging = false;
                break;
        }
    });
}

// Compute handle rect in absolute coordinates
RECT Slider::HandleRect() const {
    // Current handle position as a fraction of the whole slider
    float t = (value - minValue) / (maxValue - minValue);
    int x = AbsX() + int(t * (width - handleWidth));
    int y = AbsY() + sliderOffsetY;
    return RECT{x, y, x + handleWidth, y + handleHeight};
}

int Slider::ComputeLabelHeight(HDC hdc) {
    if(!(showLabel || showValue)) {
        return 0;
    }
    int labelHeight = 0;
    HFONT oldFont = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    labelHeight = tm.tmHeight + 2; // 2px padding
    SelectObject(hdc, oldFont);

    return labelHeight;
}

void Slider::DrawTrack(HDC hdc) {
    RECT track = {
        AbsX(),
        AbsY() + sliderOffsetY + handleHeight/2 - 2,
        AbsX() + width,
        AbsY() + sliderOffsetY + handleHeight/2 + 2
    };
    HBRUSH br = CreateSolidBrush(trackColor.toCOLORREF());
    FillRect(hdc, &track, br);
    DeleteObject(br);
}

void Slider::DrawHandle(HDC hdc) {
    RECT hr = HandleRect();
    Color handleCol = handleColor;

    // Determine handle color based on state
    if(handleHovered) {
        handleCol = hoverColor;
    }
    if(isDragging) { // Dragging takes precendence over hovering
        handleCol = dragColor;
    }
    HBRUSH br = CreateSolidBrush(handleCol.toCOLORREF());
    FillRect(hdc, &hr, br);
    DeleteObject(br);
}

void Slider::DrawLabels(HDC hdc) {
    if(!(showLabel || showValue)) return;

    HFONT oldFont = (HFONT)SelectObject(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    RECT textRect = { AbsX(), AbsY(), AbsX() + width, AbsY() + sliderOffsetY};
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, labelColor.toCOLORREF());

    // Left-aligned label
    if(showLabel) {
        DrawTextW(hdc, label.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    }

    // Right-aligned numeric value
    if(showValue) {
        std::wstring val = std::to_wstring((int)value);
        DrawTextW(hdc, val.c_str(), -1, &textRect, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
    }
    SelectObject(hdc, oldFont);
}

void Slider::Render(HDC hdc) {
    // Only draw what doesn't overflow
    // (track/handle takes precedence)
    if((showLabel || showValue)) {
        int labelHeight = ComputeLabelHeight(hdc);
        
        // Add offset and draw labels only if labels are going to fit
        if(height >= handleHeight + labelHeight) {
            sliderOffsetY = labelHeight;
            DrawLabels(hdc);
        }
        else {
            OutputDebugStringA("[!] Set height is too small for labels! Skipping drawing...\n");
        }
    }
    if(height >= handleHeight) {
        DrawTrack(hdc);
        DrawHandle(hdc);
    }
    else {
        OutputDebugStringA("[!] Set height is too small! Not drawing anything. Make sure widget height is no smaller than handle height.\n");
    }
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

void Slider::SetOnValueChanged(std::function<void(float)> cb) {
    onValueChanged = cb;
}

void Slider::ResetTransientStates() {
    Widget::ResetTransientStates();
    isDragging = false;
}