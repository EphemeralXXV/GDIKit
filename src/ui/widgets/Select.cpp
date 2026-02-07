#include <windows.h>

#include "Select.h"
#include "Root.h"
#include "Color.h"
#include "Border.h"
#include "FlexLayout.h"
#include "ScopedGDI.h"

Select::Select(std::vector<SelectItemPtr> its) :
    selectedIndex(its.empty() ? -1 : 0)
{
    SetItems(its);
    SetBorder(1, borderColor, BorderSide::All);
    SetPadding(4, 0); // Add some horizontal padding so that the text doesn't touch the border
    AddMouseListener([this](const MouseEvent& e) {
        if(e.type == MouseEventType::Click) {
            if(open) {
                Close();
            }
            else {
                pendingOpen = true;
            }
        }
    });
}

Select::SelectItemPtr Select::GetSelectedItem() const {
    if(selectedIndex < 0 || selectedIndex >= (int)items.size()) {
        return nullptr;
    }
    return items[selectedIndex];
}

void Select::SetItems(std::vector<SelectItemPtr> newItems) {
    items.clear();
    for(size_t i = 0; i < newItems.size(); i++) {
        newItems[i]->SetIndex(i); // Must automatically set index if not set explicitly
        items.push_back(newItems[i]);
    }
    selectedIndex = items.empty() ? -1 : 0;
}

void Select::SetSelectedIndex(int index) {
    if(index < 0 || index >= (int)items.size()) {
        return;
    }

    if(index == selectedIndex) {
        return;
    }

    // Clear previous
    if(selectedIndex >= 0) {
        items[selectedIndex]->SetSelected(false);
    }

    // Mark new
    selectedIndex = index;
    items[selectedIndex]->SetSelected(true);
    
    if(onSelectionChanged) {
        onSelectionChanged(selectedIndex);
    }
}

void Select::SetOnSelectionChanged(std::function<void(int)> cb) {
    onSelectionChanged = std::move(cb);
}

void Select::InitPopup() {
    if(popup) return;
    popup = std::make_shared<Container>();
    
    popup->SetBackgroundColor(Color::FromARGB(230, 30, 30, 30));
    popup->SetBorder(1, borderColor, BorderSide::All);
    auto popupLayout = std::make_unique<VerticalLayout>(0);
    popupLayout->SetAlign(AlignItems::Stretch); // Stretch items to fill the popup width
    popup->SetLayout(std::move(popupLayout));

    // Mark initial selection
    if(selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        items[selectedIndex]->SetSelected(true);
    }
    
    // On each item's select: set selected index and close popup
    for(size_t i = 0; i < items.size(); ++i) {
        SelectItemPtr& it = items[i];
        it->SetSize(0, itemHeight);

        // Preserve the onSelect callback if one was set at initialization
        auto originalCallback = it->onSelect;
        it->SetOnSelect([this, originalCallback, i]() {
            SetSelectedIndex((int)i);
            if(originalCallback) {
                originalCallback();
            }
            Close();
        });

        popup->AddChild(it);
    }
}

void Select::Open() {
    if(!popup) {
        // Lazy initialization
        InitPopup();
    }
    else {
        popup->SetVisible(true);
    }

    std::shared_ptr<Root> root = Root::Get();

    // Dynamic position/size recalculation (Select might change geometry after creation)
    RECT r = EffectiveRect();
    popup->SetPosSize(r.left, r.bottom, r.right - r.left, 0);
    popup->SetAutoHeight(true);
    root->AddChild(popup);

    // Add listener to root, because popup should close when clicking anywhere outside itself
    rootListenerID = root->AddMouseListener([this](const MouseEvent& e) {
        if(!open || e.type != MouseEventType::Down)
            return;

        if(!MouseInRect(e.pos) && !popup->MouseInRect(e.pos)) {
            Close();
        }
    });

    open = true;
}

void Select::Close() {
    if(!open || !popup) return;

    std::shared_ptr<Root> root = Root::Get();
    if(root) {
        root->RemoveChild(popup);

        if(rootListenerID != 0) {
            root->RemoveMouseListener(rootListenerID);
        }
    }

    // Manually clean up SelectItems transient states
    // This is crucial, because popup is a direct child of Root
    // So if any non-root ancestor triggers the reset...
    // SelectItems MUST know about it - and they can (only) learn it from Select
    for(auto& item : items) {
        item->ResetTransientStates();
    }

    popup->SetVisible(false);
    open = false;
}

void Select::Render(HDC hdc) {
    if(pendingOpen) {
        pendingOpen = false;
        Open();
    }

    // --- Background ------------------------------------------------------
    Color bgColor;
    if(!enabled) {
        bgColor = Color::FromRGB(120,120,120);
    }
    else if(pressed) {
        bgColor = pressedColor;
    }
    else if(hovered) {
        bgColor = hoverColor;
    }
    else {
        bgColor = backColor;
    }
    SetBackgroundColor(bgColor);

    RECT innerRect = ComputeInnerRect();

    // --- Text ------------------------------------------------------------
    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    ScopedSelectFont oldFont(hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT));

    RECT textRect = innerRect;
    textRect.right -= 16; // leave space for arrow

    SelectItemPtr selectedItem = GetSelectedItem();
    if(selectedItem) {
        DrawTextW(
            hdc,
            selectedItem->GetText().c_str(),
            -1,
            &textRect,
            DT_SINGLELINE | DT_VCENTER | DT_LEFT
        );

    }

    // --- Arrow -----------------------------------------------------------
    RECT arrowRect = innerRect;
    arrowRect.left = innerRect.right - 16;

    DrawTextW(
        hdc,
        L"▼",
        -1,
        &arrowRect,
        DT_SINGLELINE | DT_VCENTER | DT_CENTER
    );
}

void Select::ResetTransientStates() {
    Widget::ResetTransientStates();
    Close(); // ensures popup removed + listener cleaned up
}