#include <string>
#include <cmath>
#include <algorithm>

#include "Slider.h"
#include "Color.h"
#include "ScopedGDI.h"

// --- Slider ------------------------------------------------------------
Slider::Slider(
    std::wstring label,
    float minVal = 0.0f,
    float maxVal = 1.0f,
    float step = 0.0f,      // 0 means free movement
    float val = 0.5f
) :
    label(label),
    minValue(minVal),
    maxValue(maxVal),
    step(step),
    value(val)
{
    AddMouseListener([this](const MouseEvent& e) {
        switch(e.type) {
            case MouseEventType::Down: {
                // React also to clicks on the track itself
                // Not using MouseInRect, because the Slider AbsRect...
                // ...doesn't account for the label offset
                RECT trackRect = HandleRect();
                trackRect.left = EffectiveX();
                trackRect.right = EffectiveX() + width;
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
    float t = (maxValue > minValue) ?
        // Zero division guard
        ((value - minValue) / (maxValue - minValue)) : 0.0f;
    int x = EffectiveX() + int(t * (width - handleWidth));
    int y = EffectiveY() + sliderOffsetY;
    return RECT{x, y, x + handleWidth, y + handleHeight};
}

int Slider::ComputeLabelHeight(HDC hdc) {
    if(!(showLabel || showValue)) {
        return 0;
    }
    int labelHeight = 0;
    ScopedSelectFont oldFont(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    labelHeight = tm.tmHeight + 2; // 2px padding

    return labelHeight;
}

void Slider::DrawTrack(HDC hdc) {
    RECT track = {
        EffectiveX(),
        EffectiveY() + sliderOffsetY + handleHeight/2 - 2,
        EffectiveX() + width,
        EffectiveY() + sliderOffsetY + handleHeight/2 + 2
    };
    ScopedOwnedBrush br(hdc, trackColor.toCOLORREF());
    FillRect(hdc, &track, br.get());
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
    ScopedOwnedBrush br(hdc, handleCol.toCOLORREF());
    FillRect(hdc, &hr, br.get());
}

void Slider::DrawLabels(HDC hdc) {
    if(!(showLabel || showValue)) return;

    ScopedSelectFont oldFont(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    RECT textRect = { EffectiveX(), EffectiveY(), EffectiveX() + width, EffectiveY() + sliderOffsetY};
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
    RECT r = EffectiveRect();
    int relX = mouseX - r.left - handleWidth / 2;
    float t = (float)relX / (float)(width - handleWidth);
    t = std::clamp(t, 0.0f, 1.0f);
    float newValue = minValue + t * (maxValue - minValue);

    // Snap to step
    if(step > 0.0f) {
        newValue = minValue + step * std::round((newValue - minValue) / step);
    }

    // Update value, fire callback
    if(newValue != value) {
        SetValue(newValue);
    }
}

void Slider::SetOnValueChanged(std::function<void(float)> cb) {
    onValueChanged = cb;
}

void Slider::ResetTransientStates() {
    Widget::ResetTransientStates();
    isDragging = false;
}