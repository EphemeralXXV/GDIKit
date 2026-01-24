#pragma once

#include <string>
#include <algorithm>
#include <functional>

#include "Widget.h"
#include "Color.h"

class Slider : public Widget {
    public:
        // Constructor
        Slider(
            std::wstring label,
            float minVal,
            float maxVal,
            float step,
            float val
        );

        // Appearance
        std::wstring GetLabel() const { return label; }
        void SetLabel(std::wstring l) { label = l; }

        HFONT GetFont() const { return font; }
        void SetFont(HFONT newFont) { font = newFont; }

        float GetValue() const { return value; }
        void SetValue(float newValue) { 
            // Don't allow illegal values
            value = std::clamp(newValue, minValue, maxValue);
            if(onValueChanged) onValueChanged(value);
        }

        float GetMinValue() const { return minValue; }
        void SetMinValue(float newValue) { minValue = newValue; }

        float GetMaxValue() const { return maxValue; }
        void SetMaxValue(float newValue) { maxValue = newValue; }

        float GetStep() const { return step; }
        void SetStep(float newStep) {
            step = std::max(0.0f, newStep);
        }

        void SetShowValue(bool show) { showValue = show; }
        void SetShowLabel(bool show) { showLabel = show; }

        void SetHandleWidth(int w) { handleWidth = w; }
        void SetHandleHeight(int h) { handleHeight = h; }

        // Colors
        Color GetTrackColor()   const { return trackColor; }
        Color GetHandleColor()  const { return handleColor; }
        Color GetHoverColor()   const { return hoverColor; }
        Color GetDragColor()    const { return dragColor; }
        Color GetLabelColor()   const { return labelColor; }
        void SetTrackColor(Color newColor)  { trackColor = newColor; }
        void SetHandleColor(Color newColor) { handleColor = newColor; }
        void SetHoverColor(Color newColor)  { hoverColor = newColor; }
        void SetDragColor(Color newColor)   { dragColor = newColor; }
        void SetLabelColor(Color newColor)  { labelColor = newColor; }

        // Rendering
        RECT HandleRect() const;
        int ComputeLabelHeight(HDC hdc);
        void Render(HDC hdc) override;

        // Behavior
        void UpdateValueFromMouse(int mouseX);
        void SetOnValueChanged(std::function<void(float)> cb);

    protected:
        void ResetTransientStates() override;

    private:
        float minValue;
        float maxValue;
        float step;
        float value;

        int sliderOffsetY = 0;
        int handleWidth = 10;
        int handleHeight = 20;    // Effectively the height of the slider itself (minus the labels)
        bool showValue = true;
        bool showLabel = true;
        bool isDragging = false;
        bool handleHovered = false;

        std::wstring label;
        Color trackColor    = Color::FromRGB(100, 100, 100);
        Color handleColor   = Color::FromRGB(180, 180, 180);
        Color hoverColor    = Color::FromRGB(220, 220, 220);
        Color dragColor     = Color::FromRGB(150, 150, 255);
        Color labelColor    = Color::FromRGB(255, 255, 255);
        HFONT font = nullptr;

        std::function<void(float)> onValueChanged;

        void DrawTrack(HDC hdc);
        void DrawHandle(HDC hdc);
        void DrawLabels(HDC hdc);
};