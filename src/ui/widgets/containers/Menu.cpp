#include "Menu.h"
#include "Button.h"

using WidgetPtr = std::shared_ptr<Widget>;

Menu::Menu(const std::wstring &t) :
    isCollapsed(false),
    isDragging(false),
    isResizing(false),
    dragOffset({0, 0}),
    resizeOffset({0, 0}),
    resizeHandleSize(10), // 10x10 px square in the bottom-right corner
    title(t),
    showTitleBar(true),
    titleBarHeight(22),
    backgroundColor(Color::FromARGB(180, 0, 0, 0)),
    drawBackground(false)
{
    SetChildrenClipping(true);
    InitInternalElements();
    AddMouseListener([this](const MouseEvent& e) {
        POINT p = e.pos;

        switch(e.type) {
            case MouseEventType::Move:
                if(isDragging) {
                    SetPosSize(p.x - dragOffset.x, p.y - dragOffset.y, width, height);
                }
                if(isResizing) {
                    int newWidth = p.x - AbsX() + resizeOffset.x;
                    int newHeight = p.y - AbsY() + resizeOffset.y;
                    int newW = std::max(newWidth, 50);
                    int newH = std::max(newHeight, 50);
                    SetPosSize(x, y, newW, newH);
                    UpdateInternalLayout();
                }
                break;

            case MouseEventType::Down: {
                // Resize handle
                RECT resizeHandleAbsRect = ResizeHandleRect();
                if(PtInRect(&resizeHandleAbsRect, p)) {
                    isResizing = true;
                    resizeOffset.x = AbsRight() - p.x;
                    resizeOffset.y = AbsBottom() - p.y;
                    return;
                }
                // Title bar drag -- navigation buttons take precedence!
                if(titleBar->MouseInRect(p) &&
                    !collapseButton->MouseInRect(p) &&
                    !closeButton->MouseInRect(p)
                ) {
                    isDragging = true;
                    dragOffset.x = p.x - AbsX();
                    dragOffset.y = p.y - AbsY();
                    return;
                }
                break;
            }
            
            case MouseEventType::Up:
                isDragging = false;
                isResizing = false;
                break;
        }
        
        // Header children always get events
        for(auto it = headerChildren.rbegin(); it != headerChildren.rend(); ++it) {
            // Feed the mouse event directly to children
            (*it)->FeedMouseEvent(e);
        }
        
        if(!isCollapsed) {
            for(auto it = bodyChildren.rbegin(); it != bodyChildren.rend(); ++it) {
                // Feed the mouse event directly to children
                (*it)->FeedMouseEvent(e);
            }
        }
    });
}

// Resize handle in the bottom-right of the menu
RECT Menu::ResizeHandleRect() const {
    return RECT{
        AbsX() + width - resizeHandleSize,
        AbsY() + height - resizeHandleSize,
        AbsX() + width,
        AbsY() + height
    };
}
void Menu::RenderResizeHandle(HDC hdc) const { // Classic triangle-like diagonal lines
    const int lineCount = 3;
    const int spacing = 3;
    const int cornerPadding = 2; // Distance from the corner

    HPEN pen = CreatePen(PS_SOLID, 1, RGB(180,180,180));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    // RECT for the diagonal lines (going from top-left to bottom-right)
    // Subtract cornerPadding only in y1 and x1 so as to preserve the hitbox
    int x0 = AbsX() + width - resizeHandleSize;
    int y0 = AbsY() + height - resizeHandleSize;
    int x1 = AbsX() + width - cornerPadding;
    int y1 = AbsY() + height - cornerPadding;

    for(int i = 0; i < lineCount; ++i) {
        int offset = i * spacing;

        // Clamp the lines so they don't go past the corner
        int startX  = std::min(x0 + offset, AbsX() + width);
        int startY  = y1;
        int endX    = x1;
        int endY    = std::min(y0 + offset, AbsY() + height);

        // Have to subtract 1 from Y because of MoveToEx/LineTo shenanigans
        MoveToEx(hdc, startX, startY - 1, nullptr);
        LineTo(hdc, endX, endY - 1);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

// --- Child management --------------------------------------------------
void Menu::AddHeaderChild(const WidgetPtr &child) {
    child->SetParent(this);
    headerChildren.push_back(child);
}
void Menu::AddBodyChild(const WidgetPtr &child) {
    child->SetParent(this);
    bodyChildren.push_back(child);
}
void Menu::RemoveAll() {
    headerChildren.clear();
    bodyChildren.clear();
}

// Create children immediately
void Menu::InitInternalElements() {
    titleBar = std::make_shared<Widget>();
    AddHeaderChild(titleBar);

    closeButton = std::make_shared<Button>(L"×");
    closeButton->SetOnClick([&](){
        SetVisible(false);
    });
    AddHeaderChild(closeButton);

    collapseButton = std::make_shared<Button>(L"▾");
    collapseButton->SetOnClick([&](){
        isCollapsed = !isCollapsed;
        collapseButton->SetText(isCollapsed ? L"▸" : L"▾");
    });
    AddHeaderChild(collapseButton);
}

// Update children geometry dynamically
void Menu::UpdateInternalLayout() {
    titleBar->SetPosSize(0, 0, width, titleBarHeight);
    closeButton->SetPosSize(width - 20, 2, 18, 18);
    collapseButton->SetPosSize(width - 40, 2, 18, 18);

    // Propagate to child widgets if needed
    for(auto& c : bodyChildren)
        c->UpdateInternalLayout();
}

// --- Layout - should move to generic container struct once it exists ---
void Menu::BeginLayout(int startX, int startY) {
    currentLayout.cursorX = startX;
    currentLayout.cursorY = startY + titleBarHeight; // Start in the proper menu area, under the title bar
}

void Menu::EndLayout() {
    // nothing for now; placeholder if we want groups later
}

// Place child widget in vertical layout
void Menu::ApplyLayout(Widget* w) {
    int lWidth = w->GetLayoutWidth();
    int lHeight = w->GetLayoutHeight();
    w->SetPosSize(currentLayout.cursorX, currentLayout.cursorY, lWidth, lHeight);
    currentLayout.cursorY += lHeight + currentLayout.spacingY;
}

// AddBodyChild wrapper for containers with layout -- SoC preservation just in case
void Menu::AddChildWithLayout(const WidgetPtr& child) {
    AddBodyChild(child);
    ApplyLayout(child.get());
}

// --- Rendering ---------------------------------------------------------
void Menu::Render(HDC hdc) {
    if(!visible)
        return;
    
    int saved = SaveDC(hdc);

    // --- Title bar ---
    if(showTitleBar) {
        HBRUSH barBrush = CreateSolidBrush(RGB(60, 60, 60));
        RECT titleBarRect = titleBar->AbsRect();
        FillRect(hdc, &titleBarRect, barBrush);
        DeleteObject(barBrush);

        // Bar border line
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(20,20,20));
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        MoveToEx(hdc, AbsX(), AbsY() + titleBarHeight, NULL);
        LineTo(hdc, AbsX() + width, AbsY() + titleBarHeight);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);

        // Title text
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(220,220,220));
        TextOutW(hdc, titleBar->AbsX() + 6, titleBar->AbsY() + 4, title.c_str(), (int)title.size());
    }

    // --- Draw menu background if expanded ---
    if(drawBackground && !isCollapsed) {
        HBRUSH br = CreateSolidBrush(backgroundColor.toCOLORREF());
        RECT bg{ AbsX(), AbsY() + titleBarHeight, AbsX() + width, AbsY() + height };
        FillRect(hdc, &bg, br);
        DeleteObject(br);
    }

    // Clip children to menu bounds if overflow is hidden
    if(clipChildren) {
        RECT absRect = AbsRect();
        IntersectClipRect(hdc, absRect.left, absRect.top, absRect.right, absRect.bottom);
    }

    for(auto &c : headerChildren) {
        if(c->IsVisible()) c->Render(hdc);
    }
    if(!isCollapsed) {
        // Render children in order (if menu is expanded)
        for(auto &c : bodyChildren) {
            if(c->IsVisible()) c->Render(hdc);
        }
        // Draw resize handle
        RenderResizeHandle(hdc);
    }

    RestoreDC(hdc, saved);
}