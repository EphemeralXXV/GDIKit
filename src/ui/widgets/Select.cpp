#include <windows.h>

#include "Select.h"
#include "Root.h"
#include "Color.h"
#include "VerticalLayout.h"
Select::Select(std::vector<SelectItemPtr> its) :
    items(std::move(its)),
    selectedIndex(its.empty() ? -1 : 0),
    open(false),
    pendingOpen(false),
    itemHeight(18),
    backColor(Color::FromRGB(40, 40, 40)),
    hoverColor(Color::FromRGB(60, 60, 60)),
    pressedColor(Color::FromRGB(50, 50, 50)),
    borderColor(Color::FromRGB(20, 20, 20)),
    textColor(Color::FromRGB(255, 255, 255))
{
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
    if(selectedIndex < 0 || selectedIndex >= (int)items.size())
        return nullptr;
    return items[selectedIndex];
}

void Select::SetItems(std::vector<SelectItemPtr> newItems) {
    items = std::move(newItems);
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
    popup = std::make_shared<Container>();
    
    popup->SetBackgroundColor(Color::FromARGB(230, 30, 30, 30));
    popup->SetBorder(borderColor, 1);
    popup->SetLayout(std::make_unique<VerticalLayout>(0));

    // Mark initial selection
    if(selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        items[selectedIndex]->SetSelected(true);
    }
    
    for(size_t i = 0; i < items.size(); ++i) {
        SelectItemPtr& it = items[i];
        it->SetSize(width, itemHeight);

        it->SetOnSelect([this, it, i](int) {
            SetSelectedIndex((int)i);
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

    std::shared_ptr<Root> root = Root::Get();

    // Dynamic position/size recalculation (Select might change geometry after creation)
    RECT r = AbsRect();
    popup->SetPosSize(r.left, r.bottom, r.right - r.left, PopupHeight());
    root->AddChild(popup);

    // Add listener to root, because popup should close when clicking anywhere outside itself
    rootListener = ([this](const MouseEvent& e) {
        if(!open || e.type != MouseEventType::Down)
            return;

        if(!MouseInRect(e.pos) && !popup->MouseInRect(e.pos)) {
            Close();
        }
    });

    root->AddMouseListener(rootListener);

    open = true;
}

void Select::Close() {
    if(!open || !popup) return;

    std::shared_ptr<Root> root = Root::Get();
    if(root) {
        root->RemoveChild(popup);

        if(rootListener) {
            root->RemoveMouseListener(rootListener);
            rootListener = nullptr;
        }
    }

    // Manually clean up SelectItems transient states
    // This is crucial, because popup is a direct child of Root
    // So if any non-root ancestor triggers the reset...
    // SelectItems MUST know about it - and they can (only) learn it from Select
    for(auto& item : items) {
        item->ResetTransientStates();
    }

    popup.reset();
    open = false;
}

void Select::Render(HDC hdc) {
    if(!effectiveDisplayed || !visible) return;

    if(pendingOpen) {
        pendingOpen = false;
        Open();
    }

    int saved = SaveDC(hdc);

    RECT r = AbsRect();

    // --- Background ------------------------------------------------------
    HBRUSH br;
    if(!enabled) {
        br = CreateSolidBrush(RGB(120,120,120));
    }
    else if(pressed) {
        br = CreateSolidBrush(pressedColor.toCOLORREF());
    }
    else if(hovered) {
        br = CreateSolidBrush(hoverColor.toCOLORREF());
    }
    else {
        br = CreateSolidBrush(backColor.toCOLORREF());
    }
    FillRect(hdc, &r, br);
    DeleteObject(br);

    // --- Border ----------------------------------------------------------
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor.toCOLORREF());
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, r.left, r.top, r.right, r.bottom);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);

    // --- Text ------------------------------------------------------------
    SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, textColor.toCOLORREF());
    HFONT oldFont = (HFONT)SelectObject(
        hdc, (HFONT)GetStockObject(DEFAULT_GUI_FONT)
    );

    RECT textRect = r;
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
    RECT arrowRect = r;
    arrowRect.left = r.right - 16;

    DrawTextW(
        hdc,
        L"▼",
        -1,
        &arrowRect,
        DT_SINGLELINE | DT_VCENTER | DT_CENTER
    );

    SelectObject(hdc, oldFont);
    RestoreDC(hdc, saved);
}

void Select::ResetTransientStates() {
    Widget::ResetTransientStates();
    Close(); // ensures popup removed + listener cleaned up
}