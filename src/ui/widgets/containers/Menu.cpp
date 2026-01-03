#include "Menu.h"
#include "VerticalLayout.h"

Menu::Menu(const std::wstring &t) :
    isCollapsed(false),
    isDragging(false),
    isResizing(false),
    dragOffset({0, 0}),
    resizeOffset({0, 0}),
    resizeHandleSize(10), // 10x10 px square in the bottom-right corner
    title(t),
    titleBarHeight(22)
{
    SetChildrenClipping(true);
    SetBackgroundColor(Color::FromARGB(180, 0, 0, 0));
    InitHeader();
    InitBody();
    SetLayout(std::make_unique<VerticalLayout>());
    AddMouseListener([this](const MouseEvent& e) {
        POINT p = e.pos;

        switch(e.type) {
            case MouseEventType::Move:
                if(isDragging) {
                    SetPos(p.x - dragOffset.x, p.y - dragOffset.y);
                }
                if(isResizing) {
                    int newWidth = p.x - AbsX() + resizeOffset.x;
                    if(newWidth < 0) break; // Negative size is invalid; don't bother continuing

                    int newHeight = p.y - AbsY() + resizeOffset.y;
                    if(newHeight < 0) break;

                    SetSize(newWidth, newHeight);
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
                if(headerContainer->MouseInRect(p) &&
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

// --- Subcontainer initialization --------------------------------------------------
void Menu::InitHeader() {
    headerContainer = std::make_shared<Container>();
    headerContainer->SetBackgroundColor(Color::FromRGB(60, 60, 60));
    headerContainer->SetBorder(Color::FromRGB(20, 20, 20), 1, BorderSide::Bottom);

    titleLabel = std::make_shared<Label>(title);
    titleLabel->SetTextColor(Color::FromRGB(220, 220, 220));

    closeButton = std::make_shared<Button>(L"×");
    closeButton->SetOnClick([&](){
        SetDisplayed(false);
    });

    collapseButton = std::make_shared<Button>(L"▾");
    collapseButton->SetOnClick([&](){
        SetCollapsed(!isCollapsed);
        collapseButton->SetText(isCollapsed ? L"▸" : L"▾");
    });

    headerContainer->AddChild(titleLabel);
    headerContainer->AddChild(collapseButton);
    headerContainer->AddChild(closeButton);

    AddChild(headerContainer);
}
void Menu::InitBody() {
    bodyContainer = std::make_shared<Container>();
    AddChild(bodyContainer);
}

// --- Rendering ---------------------------------------------------------
void Menu::OnInternalLayoutUpdated() {
    // Recalculates layout on every header/body change
    // Geometry is only set here (and not during initialization), because it's a dynamic thing
    if(headerContainer) {
        headerContainer->SetSize(width, titleBarHeight);
        titleLabel->SetPosSize(6, 0, width - 40 - 6, titleBarHeight);
        closeButton->SetPosSize(width - 20, 2, titleBarHeight - 4, titleBarHeight - 4);
        collapseButton->SetPosSize(width - 40, 2, titleBarHeight - 4, titleBarHeight - 4);
    }
    if(bodyContainer) {
        bodyContainer->SetSize(width, height - titleBarHeight);
    }
}

void Menu::SetSize(int w, int h) {
    // Clamp width to 50 and height to title bar height (title bar always visible)
    Container::SetSize(std::max(w, 50), std::max(h, titleBarHeight));
}

void Menu::SetPosSize(int x, int y, int w, int h) {
    Container::SetPosSize(x, y, std::max(w, 50), std::max(h, titleBarHeight));
}

void Menu::SetCollapsed(bool collapsed) {
    isCollapsed = collapsed;
    bodyContainer->SetDisplayed(!collapsed);
    // shrink height to header only
    if(collapsed) {
        // Cache expanded size
        SetPreferredSize(width, height);
        SetSize(width, titleBarHeight);
    }
    else {
        SetSize(width, preferredHeight);
    }
}
void Menu::SetTitle(const std::wstring &newTitle) {
    title = newTitle;
    titleLabel->SetText(newTitle);
}
void Menu::SetShowTitleBar(bool show) { 
    headerContainer->SetDisplayed(show);
    // shrink height to content only
    if(show) {
        SetSize(width, preferredHeight);
    }
    else {
        SetSize(width, preferredHeight - titleBarHeight);
    }
}

void Menu::Render(HDC hdc) {    
    // Clip children to menu bounds if overflow is hidden
    if(clipChildren) {
        RECT absRect = AbsRect();
        IntersectClipRect(hdc, absRect.left, absRect.top, absRect.right, absRect.bottom);
    }

    // Render children in order
    Container::Render(hdc);

    if(!isCollapsed) {
        // Draw resize handle
        RenderResizeHandle(hdc);
    }
}