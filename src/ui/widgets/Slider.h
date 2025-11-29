#pragma once

#include <string>
#include <functional>

#include "Widget.h"
#include "Color.h"

// --- Slider ------------------------------------------------------------
class Slider : public Widget {
    public:
        float minValue;
        float maxValue;
        float step;
        float value;

        std::wstring label;
        bool showValue;
        bool showLabel;

        int handleWidth;

        // Internal offset for drawing text above the track
        int sliderOffsetY;

        Color trackColor;
        Color handleColor;
        Color hoverColor;
        Color dragColor;

        bool isDragging;
        std::function<void(float)> onValueChanged;

        Slider(
            const std::wstring& label, float minVal, float maxVal, float step, float val
        );

        // Compute handle rect in absolute coordinates
        RECT HandleRect() const;

        void ComputeSliderOffsetY(HDC hdc);

        void Render(HDC hdc) override;

        void UpdateValueFromMouse(int mouseX);

        void OnMouseMove(POINT p) override;
        void OnMouseDown(POINT p) override;
        void OnMouseUp(POINT p) override;
};