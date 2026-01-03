#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>

#include "Widget.h"
#include "Container.h"
#include "SelectItem.h"
#include "Color.h"

class Select : public Widget {
    public:
        using SelectItemPtr = std::shared_ptr<SelectItem>;

        // Constructor
        Select(std::vector<SelectItemPtr> items = {});

        // --- Items ------------------------------------------------------------
        void SetItems(std::vector<SelectItemPtr> newItems);
        const std::vector<SelectItemPtr>& GetItems() const { return items; }

        int GetSelectedIndex() const { return selectedIndex; }
        SelectItemPtr GetSelectedItem() const;

        void SetSelectedIndex(int index);

        // --- Appearance -------------------------------------------------------
        void SetItemHeight(int h)   { itemHeight = h; }
        int  GetItemHeight() const  { return itemHeight; }
        int  PopupHeight() const    { return itemHeight * static_cast<int>(items.size()); }

        Color GetBackColor()    const { return backColor; }
        Color GetHoverColor()   const { return hoverColor; }
        Color GetPressedColor() const { return pressedColor; }
        Color GetBorderColor()  const { return borderColor; }
        Color GetTextColor()    const { return textColor; }
        void SetBackColor(Color c)      { backColor = c; }
        void SetHoverColor(Color c)     { hoverColor = c; }
        void SetPressedColor(Color c)   { pressedColor = c; }
        void SetBorderColor(Color c)    { borderColor = c; }
        void SetTextColor(Color c)      { textColor = c; }

        // --- Behavior ---------------------------------------------------------
        void SetOnSelectionChanged(std::function<void(int)> cb);

        // --- Rendering --------------------------------------------------------
        void Render(HDC hdc) override;

    protected:
        // Popup control
        void Open();
        void Close();
        bool IsOpen() const { return open; }

        // Misc.
        void ResetTransientStates() override;

    private:
        // Data
        std::vector<SelectItemPtr> items;
        int selectedIndex;

        // Popup state
        bool open;
        bool pendingOpen;
        int itemHeight;

        // Appearance
        Color backColor;
        Color hoverColor;
        Color pressedColor;
        Color borderColor;
        Color textColor;

        // Popup container (created on demand)
        std::shared_ptr<Container> popup;
        void InitPopup();

        // Event listeners
        std::function<void(int)> onSelectionChanged;
        std::function<void(const MouseEvent&)> rootListener;
};
