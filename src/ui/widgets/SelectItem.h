#include <string>

#include "Widget.h"
#include "Color.h"

class SelectItem : public Widget {
    public:
        friend class Select; // Allow Select to fully manage SelectItem

        SelectItem(std::wstring text, size_t index);

        const std::wstring& GetText() const { return text; }
        size_t GetIndex() const { return index; }

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

        void SetOnSelect(std::function<void(size_t)> cb);

    private:
        std::wstring text;
        size_t index;
        
        bool selected;

        HFONT font;
        Color backColor;
        Color hoverColor;
        Color pressedColor;
        Color selectedColor;
        Color textColor;

        std::function<void(size_t)> onSelect;
};