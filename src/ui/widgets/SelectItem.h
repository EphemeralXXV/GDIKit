#pragma once

#include <string>

#include "Widget.h"
#include "Color.h"

class SelectItem : public Widget {
    public:
        friend class Select; // Allow Select to fully manage SelectItem

        SelectItem(std::wstring text, std::string value);

        const std::wstring& GetText() const { return text; }
        void SetText(std::wstring t) { text = t; }

        const std::string& GetValue() const { return value; }
        void SetValue(std::string p) { value = std::move(p); }

        size_t GetIndex() const { return index; }
        void SetIndex(size_t idx) { index = idx; }

        void SetSelected(bool sel) { selected = sel; }
        bool IsSelected() const { return selected; }

        // Appearance
        HFONT GetFont() const { return font; }
        void SetFont(HFONT f) { font = f; }

        Color GetBackColor()        const { return backColor; }
        Color GetHoverColor()       const { return hoverColor; }
        Color GetPressedColor()     const { return pressedColor; }
        Color GetSelectedColor()    const { return selectedColor; }
        Color GetTextColor()        const { return textColor; }
        void SetBackColor(Color newColor)       { backColor = newColor; }
        void SetHoverColor(Color newColor)      { hoverColor = newColor; }
        void SetPressedColor(Color newColor)    { pressedColor = newColor; }
        void SetSelectedColor(Color newColor)   { selectedColor = newColor; }
        void SetTextColor(Color newColor)       { textColor = newColor; }

        void Render(HDC hdc) override;

        void SetOnSelect(std::function<void()> cb);

    private:
        size_t index;
        std::wstring text;
        std::string value; // Internal value, akin to HTML <option> value attribute
        
        bool selected = false;

        HFONT font = nullptr;
        Color backColor     = Color::FromRGB(40, 40, 40);
        Color hoverColor    = Color::FromRGB(60, 60, 60);
        Color pressedColor  = Color::FromRGB(70, 70, 70);
        Color selectedColor = Color::FromRGB(50, 50, 50);
        Color textColor     = Color::FromRGB(255, 255, 255);

        std::function<void()> onSelect;
};