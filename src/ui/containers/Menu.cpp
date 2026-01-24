#include "Menu.h"
#include "FlexLayout.h"
#include "ScopedGDI.h"

Menu::Menu(const std::wstring &t) :
    title(t)
{
    SetChildrenClipping(true);
    SetBackgroundColor(Color::FromARGB(180, 0, 0, 0));
    InitHeader();
    InitBody();
    auto menuLayout = std::make_unique<VerticalLayout>();
    menuLayout->SetAlign(AlignItems::Stretch); // Stretch header and body to menu width
    SetLayout(std::move(menuLayout));
}

// Resize handle in the bottom-right of the menu
RECT Menu::ResizeHandleRect() const {
    return RECT{
        EffectiveRight() - resizeHandleSize,
        EffectiveBottom() - resizeHandleSize,
        EffectiveRight(),
        EffectiveBottom()
    };
}
void Menu::RenderResizeHandle(HDC hdc) const { // Classic triangle-like diagonal lines
    const int lineCount = 3;
    const int spacing = 3;
    const int cornerPadding = 2; // Distance from the corner

    ScopedOwnedPen pen(hdc, PS_SOLID, 1, RGB(180,180,180));

    // RECT for the diagonal lines (going from top-left to bottom-right)
    // Subtract cornerPadding only in y1 and x1 so as to preserve the hitbox
    int x0 = EffectiveRight() - resizeHandleSize;
    int y0 = EffectiveBottom() - resizeHandleSize;
    int x1 = EffectiveRight() - cornerPadding;
    int y1 = EffectiveBottom() - cornerPadding;

    for(int i = 0; i < lineCount; ++i) {
        int offset = i * spacing;

        // Clamp the lines so they don't go past the corner
        int startX  = std::min(x0 + offset, EffectiveRight());
        int startY  = y1;
        int endX    = x1;
        int endY    = std::min(y0 + offset, EffectiveBottom());

        // Have to subtract 1 from Y because of MoveToEx/LineTo shenanigans
        MoveToEx(hdc, startX, startY - 1, nullptr);
        LineTo(hdc, endX, endY - 1);
    }
}

// --- Subcontainer initialization --------------------------------------------------
void Menu::InitHeader() {
    headerContainer = std::make_shared<Container>();
    headerContainer->SetSize(0, titleBarHeight);
    headerContainer->SetBackgroundColor(Color::FromRGB(60, 60, 60));
    headerContainer->SetBorder(1, Color::FromRGB(20, 20, 20), BorderSide::Bottom);
    headerContainer->AddMouseListener([this](const MouseEvent& e){
        if(e.type == MouseEventType::Down && !collapseButton->MouseInRect(e.pos)){
            isDragging = true;
            dragOffset.x = e.pos.x - EffectiveX();
            dragOffset.y = e.pos.y - EffectiveY();
        }
        else if(e.type == MouseEventType::Move && isDragging){
            SetPos(e.pos.x - dragOffset.x, e.pos.y - dragOffset.y);
        }
        else if(e.type == MouseEventType::Up){
            isDragging = false;
        }
    });

    titleLabel = std::make_shared<Label>(title);
    titleLabel->SetSize(0, titleBarHeight);
    titleLabel->SetFlexGrow(true);
    titleLabel->SetMouseEventsIgnoring(true); // Ignore mouse events to allow title bar dragging
    titleLabel->SetTextColor(Color::FromRGB(220, 220, 220));

    closeButton = std::make_shared<Button>(L"×");
    closeButton->SetSize(titleBarHeight - 4, titleBarHeight - 5);
    closeButton->SetOnClick([&](){
        SetDisplayed(false);
    });

    collapseButton = std::make_shared<Button>(L"▾");
    collapseButton->SetSize(titleBarHeight - 4, titleBarHeight - 5);
    collapseButton->SetOnClick([&](){
        SetCollapsed(!isCollapsed);
        collapseButton->SetText(isCollapsed ? L"▸" : L"▾");
    });

    headerContainer->AddChild(titleLabel);
    headerContainer->AddChild(collapseButton);
    headerContainer->AddChild(closeButton);

    headerContainer->SetPadding(0, 0, 6, 2);
    auto headerLayout = std::make_unique<HorizontalLayout>(2);
    headerLayout->SetAlign(AlignItems::Center);
    headerContainer->SetLayout(std::move(headerLayout));

    AddChild(headerContainer);
}
void Menu::InitBody() {
    bodyContainer = std::make_shared<Container>();
    bodyContainer->SetFlexGrow(true); // Fully dependent on Menu size
    bodyContainer->AddMouseListener([this](const MouseEvent& e){
        RECT rh = ResizeHandleRect();
        if(e.type == MouseEventType::Down && PtInRect(&rh, e.pos)){
            isResizing = true;
            resizeOffset.x = EffectiveRight() - e.pos.x;
            resizeOffset.y = EffectiveBottom() - e.pos.y;
        }
        else if(e.type == MouseEventType::Move && isResizing){
            SetSize(e.pos.x - EffectiveX() + resizeOffset.x, e.pos.y - EffectiveY() + resizeOffset.y);
        }
        else if(e.type == MouseEventType::Up){
            isResizing = false;
        }
    });

    AddChild(bodyContainer);
}

// --- Rendering ---------------------------------------------------------
void Menu::SetSize(int w, int h) {
    // Clamp width to 50 and height to title bar height + resize handle size (title bar and resize handle always visible)
    Container::SetSize(std::max(w, 50), std::max(h, titleBarHeight + resizeHandleSize));
}

void Menu::SetPosSize(int x, int y, int w, int h) {
    Container::SetPosSize(x, y, std::max(w, 50), std::max(h, titleBarHeight + resizeHandleSize));
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
        RECT r = EffectiveRect();
        IntersectClipRect(hdc, r.left, r.top, r.right, r.bottom);
    }

    // Render children in order
    Container::Render(hdc);

    if(!isCollapsed) {
        // Draw resize handle
        RenderResizeHandle(hdc);
    }
}
