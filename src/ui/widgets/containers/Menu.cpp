#include "Menu.h"

using WidgetPtr = std::shared_ptr<Widget>;

Menu::Menu(const std::wstring &t = L"Menu") :
    isCollapsed(false),
    isDragging(false),
    isResizing(false),
    dragOffset({0, 0}),
    resizeOffset({0, 0}),
    resizeHandleSize(10), // 10x10 px square in the bottom-right corner
    title(t),
    showTitleBar(true),
    titleBarHeight(22),
    background(Color::FromARGB(180, 0, 0, 0)),
    drawBackground(false),
    clipChildren(true) // Basically overflow: hidden
{
    InitInternalElements();
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
WidgetPtr titleBar;
WidgetPtr closeButton;
WidgetPtr collapseButton;

void Menu::AddChild(const WidgetPtr &child) {
    child->parent = this;
    children.push_back(child);
}
void Menu::RemoveAll() { children.clear(); }

// Create children immediately
void Menu::InitInternalElements() {
    titleBar = std::make_shared<Widget>();
    AddChild(titleBar);

    closeButton = std::make_shared<Widget>();
    AddChild(closeButton);

    collapseButton = std::make_shared<Widget>();
    AddChild(collapseButton);
}

// Update children geometry dynamically
void Menu::UpdateInternalLayout() {
    titleBar->SetPosSize(0, 0, width, titleBarHeight);
    closeButton->SetPosSize(width - 20, 2, 18, 18);
    collapseButton->SetPosSize(width - 40, 2, 18, 18);

    // Propagate to child widgets if needed
    for(auto& c : children)
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

// AddChild wrapper for containers with layout -- SoC preservation just in case
void Menu::AddChildWithLayout(const WidgetPtr& child) {
    AddChild(child);
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

        // Draw "X"
        RECT closeButtonRect = closeButton->AbsRect();
        DrawTextW(hdc, L"×", 1, &closeButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Draw collapse arrow ▾ or ▸
        WCHAR arrow = isCollapsed ? L'▸' : L'▾';
        RECT collapseButtonRect = collapseButton->AbsRect();
        DrawTextW(hdc, &arrow, 1, &collapseButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // --- Draw menu background if expanded ---
    if(drawBackground && !isCollapsed) {
        HBRUSH br = CreateSolidBrush(background.toCOLORREF());
        RECT bg{ AbsX(), AbsY() + titleBarHeight, AbsX() + width, AbsY() + height };
        FillRect(hdc, &bg, br);
        DeleteObject(br);
    }

    // Draw resize handle
    RenderResizeHandle(hdc);

    // Clip children to menu bounds if overflow is hidden
    if(clipChildren) {
        RECT absRect = AbsRect();
        IntersectClipRect(hdc, absRect.left, absRect.top + titleBarHeight, absRect.right, absRect.bottom);
    }

    // Render children in order (if menu is expanded)
    if(!isCollapsed) {
        for(auto &c : children) {
            if(c->visible) c->Render(hdc);
        }
    }

    RestoreDC(hdc, saved);
}

// --- Event forwarding --------------------------------------------------
void Menu::OnMouseDown(POINT p) {
    if(!visible || !MouseInRect(p)) return;

    // Resizing via bottom-right handle
    RECT resizeHandleRect = ResizeHandleRect();
    if(PtInRect(&resizeHandleRect, p)) {
        isResizing = true;
        resizeOffset.x = AbsRight() - p.x;
        resizeOffset.y = AbsBottom() - p.y;
        return;
    }

    // Dragging via title bar
    RECT titleBarRect = titleBar->AbsRect();
    if(PtInRect(&titleBarRect, p)) {
        RECT closeButtonRect = closeButton->AbsRect();
        if(PtInRect(&closeButtonRect, p)) {
            // Close the window
            visible = false;
            return;
        }

        RECT collapseButtonRect = collapseButton->AbsRect();
        if(PtInRect(&collapseButtonRect, p)) {
            isCollapsed = !isCollapsed;
            return;
        }

        // Start dragging
        isDragging = true;
        dragOffset.x = p.x - AbsX(); // Must use absolute coords for mouse events
        dragOffset.y = p.y - AbsY();
        return;
    }

    // Default: forward to children if visible and not collapsed
    if(!isCollapsed) {
        for (auto it = children.rbegin(); it != children.rend(); ++it)
            (*it)->OnMouseDown(p);
    }
}

void Menu::OnMouseMove(POINT p) {
    if(!visible) return;

    if(isResizing) {
        int newWidth = p.x - AbsX() + resizeOffset.x;
        int newHeight = p.y - AbsY() + resizeOffset.y;

        width = std::max(newWidth, 50);   // minimum width safeguard
        height = std::max(newHeight, 50); // minimum height safeguard
        UpdateInternalLayout();
        return;
    }

    if(isDragging) {
        SetPosSize(p.x - dragOffset.x, p.y - dragOffset.y, width, height);
        return;
    }

    // Forward to children
    if(!isCollapsed) {
        for (auto it = children.rbegin(); it != children.rend(); ++it)
            (*it)->OnMouseMove(p);
    }
}

void Menu::OnMouseUp(POINT p) {
    if(!visible) return;
    isDragging = false;
    isResizing = false;

    // Forward to children
    if(!isCollapsed) {
        for (auto it = children.rbegin(); it != children.rend(); ++it)
            (*it)->OnMouseUp(p);
    }
}