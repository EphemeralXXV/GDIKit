#include <algorithm>

#include "Widget.h"
#include "ScopedGDI.h"

// Constructor
Widget::Widget() {}

// Ancestors
void Widget::SetParent(Widget* newParent) {
    if(parent && !newParent) {
        // parent = nullptr -- parent removes this child
        OnRemovedFromTree();
    }
    parent = newParent;
}

Widget* Widget::GetMainContainer() const {
    const Widget* w = this;
    while(w->parent) {
        w = w->parent;
    }
    return const_cast<Widget*>(w);
}

// --- Geometry -----------------------------------------------------
// Absolute coordinate getters (relative => absolute)
int Widget::AbsX() const {
    return parent ? parent->AbsX() + x : x;
}
int Widget::AbsY() const {
    return parent ? parent->AbsY() + y : y;
}
int Widget::AbsRight() const {
    return AbsX() + width;
}
int Widget::AbsBottom() const {
    return AbsY() + height;
}
RECT Widget::AbsRect() const {
    int absX = AbsX();
    int absY = AbsY();
    return RECT{ absX, absY, absX + width, absY + height};
}

// Effective coordinate getters
int Widget::EffectiveX() const {
    return effectiveRect.left;
}
int Widget::EffectiveY() const {
    return effectiveRect.top;
}
int Widget::EffectiveRight() const {
    return effectiveRect.right;
}
int Widget::EffectiveBottom() const {
    return effectiveRect.bottom;
}
RECT Widget::EffectiveRect() const {
    return effectiveRect;
}
int Widget::EffectiveWidth() const {
    return effectiveRect.right - effectiveRect.left;
}
int Widget::EffectiveHeight() const {
    return effectiveRect.bottom - effectiveRect.top;
}

// Size setters
void Widget::SetRect(int l, int t, int r, int b) {
    // Sets the relative rect
    rect = {l, t, r, b};
    UpdateConvenienceGeometry();
    InvalidateLayout();
}
void Widget::SetPos(int x, int y) {
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    rect = {x, y, x + w, y + h};
    UpdateConvenienceGeometry();
    UpdateEffectiveGeometry();  // Don't invalidate - no need for relayout
                                // just update effective geometry of the subtree
}
void Widget::SetSize(int w, int h) {
    rect = {rect.left, rect.top, rect.left + w, rect.top + h};
    UpdateConvenienceGeometry();
    InvalidateLayout();
}
void Widget::SetPosSize(int x, int y, int w, int h) {
    rect = { x, y, x + w, y + h };
    UpdateConvenienceGeometry();
    InvalidateLayout();
}
void Widget::SetPreferredSize(int w, int h) {
    preferredWidth = w;
    preferredHeight = h;
    InvalidateLayout();
}
void Widget::SetLayoutSize(int w, int h) {
    layoutWidth = w;
    layoutHeight = h;
    // Don't invalidate here to avoid infinite loops with layouts
}

RECT Widget::ComputeInnerRect() const {
    RECT r = EffectiveRect();

    r.top    += (padding.top + border.top.thickness);
    r.bottom -= (padding.bottom + border.bottom.thickness);
    r.left   += (padding.left + border.left.thickness);
    r.right  -= (padding.right + border.right.thickness);

    return r;
}

// Helper functions reacting to geometry changes
void Widget::UpdateConvenienceGeometry() {
    // Updates convenience geometry vars on internal geometry changes
    x = rect.left;
    y = rect.top;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}
void Widget::ApplyLogicalGeometry() {
    // Get parent inner rect for border+padding offset from parent edges
    RECT parentInnerRect;
    if(parent) {
        parentInnerRect = parent->ComputeInnerRect(); // derive from parent if present
    }
    else {
        parentInnerRect = {0, 0, 0, 0}; // root or parentless widget
    }

    // Compute effective width/height = logical size - margins
    int effectiveWidth = width - margin.left - margin.right;
    int effectiveHeight = height - margin.top - margin.bottom;

    // Negative dimensions safeguard
    effectiveWidth = std::max(0, effectiveWidth);
    effectiveHeight = std::max(0, effectiveHeight);

    // Compute effective left/top based on anchor
    // (x,y refer to different corners based on anchor; width and height have fixed meaning)
    int effectiveLeft = 0;
    int effectiveTop = 0;

    switch(anchor) {
        case Anchor::TopLeft:
            effectiveLeft = parentInnerRect.left + x + margin.left;
            effectiveTop = parentInnerRect.top + y + margin.top;
            break;

        case Anchor::TopRight:
            effectiveLeft = parentInnerRect.right - (x + width + margin.right);
            effectiveTop = parentInnerRect.top + y + margin.top;
            break;

        case Anchor::BottomLeft:
            effectiveLeft = parentInnerRect.left + x + margin.left;
            effectiveTop = parentInnerRect.bottom - (y + height + margin.bottom);
            break;

        case Anchor::BottomRight:
            effectiveLeft = parentInnerRect.right - (x + width + margin.right);
            effectiveTop = parentInnerRect.bottom - (y + height + margin.bottom);
            break;
    }
    effectiveLeft += parentInnerRect.left;
    effectiveTop += parentInnerRect.top;

    SetEffectiveRect(effectiveLeft, effectiveTop, effectiveLeft + effectiveWidth, effectiveTop + effectiveHeight);
}
void Widget::SetEffectiveRect(int l, int t, int r, int b) {
    effectiveRect = {l, t, r, b};
}
void Widget::InvalidateLayout() {
    if(auto* p = GetParent()) {
        if(p->GetLayout()) {
            // Start reflow at parent if current Container is a layout item
            p->InvalidateLayout();
            return;
        }
    }
    ApplyLogicalGeometry();
    UpdateInternalLayout();
}
void Widget::UpdateInternalLayout() {
    // Updates automatic layouts on geometry changes
    // default: no-op. Derived widgets may override to update children elements.
}
void Widget::OnInternalLayoutUpdated() {}
void Widget::UpdateEffectiveGeometry() {
    if(auto* p = GetParent()) {
        if(p->GetLayout()) {
            return; // Let parent apply geometry with its layout
        }
    }
    ApplyLogicalGeometry();
}
        
// Get preferred size set by client code (default to current size if not set)
int Widget::GetPreferredWidth() const {
    return preferredWidth > 0 ? preferredWidth : width;
}
int Widget::GetPreferredHeight() const {
    return preferredHeight > 0 ? preferredHeight : height;
}

// Spacing & Alignment
void Widget::SetPadding(int all) {
    padding = {all, all, all, all};
    InvalidateLayout();
}
void Widget::SetPadding(int horizontal, int vertical) {
    padding = {vertical, vertical, horizontal, horizontal};
    InvalidateLayout();
}
void Widget::SetPadding(int top, int bottom, int left, int right) {
    padding = {top, bottom, left, right};
    InvalidateLayout();
}

void Widget::SetMargin(int all) {
    margin = {all, all, all, all};
    InvalidateLayout();
}
void Widget::SetMargin(int horizontal, int vertical) {
    margin = {vertical, vertical, horizontal, horizontal};
    InvalidateLayout();
}
void Widget::SetMargin(int top, int bottom, int left, int right) {
    margin = {top, bottom, left, right};
    InvalidateLayout();
}

void Widget::SetWidthProperties(DimensionProperties properties) {
    widthProperties = properties;
    InvalidateLayout();
}
void Widget::SetHeightProperties(DimensionProperties properties) {
    heightProperties = properties;
    InvalidateLayout();
}
void Widget::SetAutoWidth(bool isAuto) {
    widthProperties.isAuto = isAuto;
    InvalidateLayout();
}
void Widget::SetAutoHeight(bool isAuto) {
    heightProperties.isAuto = isAuto;
    InvalidateLayout();
}

// --- Display & Visibility --------------------------------------------
void Widget::SetDisplayed(bool displayed) {
    this->displayed = displayed;
    UpdateEffectiveDisplay();
}

void Widget::UpdateEffectiveDisplay() {
    // Apply actual displayed state as (user-set && inherited)
    bool newEff = displayed &&
        (!parent || parent->effectiveDisplayed);

    if(newEff == effectiveDisplayed) {
        return;
    }

    effectiveDisplayed = newEff;
    OnDisplayChanged(effectiveDisplayed);
}

void Widget::SetVisible(bool visible)  {
    this->visible = visible;
    OnVisibilityChanged(visible);
}

// --- Mouse event handlers -------------------------------------------
// Test if cursor currently over widget
bool Widget::MouseInRect(POINT p) const {
    RECT clip = EffectiveRect();

    // Calculate true hit area if clipped by any of the ancestors
    const Widget* ancestor = parent;
    while(ancestor) {
        if(ancestor->clipChildren) {
            RECT parentRect = ancestor->EffectiveRect();
            IntersectRect(&clip, &clip, &parentRect);
        }
        ancestor = ancestor->parent;
    }

    return PtInRect(&clip, p);
}

// Mouse listeners
size_t Widget::AddMouseListener(std::function<void(const MouseEvent&)> callback) {
    size_t id = nextListenerID++;
    mouseListeners.push_back({id, callback});
    return id;
}
void Widget::RemoveMouseListener(size_t id) {
    mouseListeners.erase(
        std::remove_if(
            mouseListeners.begin(),
            mouseListeners.end(),
            [&](const MouseListener& l) {
                return l.id == id;
            }
        ),
        mouseListeners.end()
    );
}

bool Widget::InitFeedMouseEvent(const MouseEvent& e) {
    if(!effectiveDisplayed || ignoreMouseEvents) return false; // Non-displayed widgets should ignore events
    return FeedMouseEvent(e);
}

bool Widget::FeedMouseEvent(const MouseEvent& e) {
    bool handled = false;
    switch(e.type) {
        case MouseEventType::Enter:
        case MouseEventType::Leave:
        case MouseEventType::Move:  handled = OnMouseMove(e.pos); break;
        case MouseEventType::Down:  handled = OnMouseDown(e.pos); break;
        case MouseEventType::Click:
        case MouseEventType::Up:    handled = OnMouseUp(e.pos); break;
    }
    return handled;
}

void Widget::FireMouseEvent(const MouseEvent& e) {
    for(auto& listener : mouseListeners) {
        listener.callback(e);
    }
}

bool Widget::OnMouseMove(POINT p) {
    bool wasHovered = hovered; // read old state
    hovered = MouseInRect(p);  // read current state

    if(hovered && !wasHovered) {
        FireMouseEvent({MouseEventType::Enter, p, MouseButton::Left});
    }
    if(!hovered && wasHovered) {
        FireMouseEvent({MouseEventType::Leave, p, MouseButton::Left});
    }
    // Fire Move also if pressed - in case of dragging, for example
    if(hovered || pressed) {
        FireMouseEvent({MouseEventType::Move, p, MouseButton::Left});
    }

    // Consume only if actively dragging - other movements may not be exclusive
    // (especially when simultaneously leaving one widget and entering another)
    return pressed;
}
bool Widget::OnMouseDown(POINT p) {
    if(!enabled) return false;

    if(MouseInRect(p)) {
        pressed = true;
        mouseDownInside = true; // track click start
        FireMouseEvent({MouseEventType::Down, p, MouseButton::Left});
        return true;
    }
    return false;
}
bool Widget::OnMouseUp(POINT p) {
    if(!enabled) return false;

    bool handled = pressed;
    if(pressed) {
        // Let MouseUp happen outside widget, e.g. to cancel selection
        FireMouseEvent({MouseEventType::Up, p, MouseButton::Left});
        if(mouseDownInside && MouseInRect(p)) {
            FireMouseEvent({MouseEventType::Click, p, MouseButton::Left});
        }
    }
    pressed = false;
    mouseDownInside = false;

    return handled;
}

// --- Appearance -----------------------------------------------------
void Widget::SetBorder(int thickness, const Color& color, BorderSide sides) {
    if(HasSide(sides, BorderSide::Top))    border.top    = {thickness, color};
    if(HasSide(sides, BorderSide::Right))  border.right  = {thickness, color};
    if(HasSide(sides, BorderSide::Bottom)) border.bottom = {thickness, color};
    if(HasSide(sides, BorderSide::Left))   border.left   = {thickness, color};
}

void Widget::DrawBorderEdge(HDC hdc, BorderData borderData, BorderSide side) {
    if(borderData.thickness <= 0) return;

    RECT r = EffectiveRect();

    // Use precise rects instead of imprecise lines (LineTo)
    RECT br = r;
    switch(side) {
        case BorderSide::Top:
            br.bottom = br.top + borderData.thickness;
            break;
        case BorderSide::Bottom:
            br.top = br.bottom - borderData.thickness;
            break;
        case BorderSide::Left:
            br.right = br.left + borderData.thickness;
            break;
        case BorderSide::Right:
            br.left = br.right - borderData.thickness;
            break;
    }
    ScopedOwnedBrush brush(hdc, borderData.color.toCOLORREF());
    FillRect(hdc, &br, brush.get());
}

void Widget::RenderBorder(HDC hdc) {
    DrawBorderEdge(hdc, border.top, BorderSide::Top);
    DrawBorderEdge(hdc, border.bottom, BorderSide::Bottom);
    DrawBorderEdge(hdc, border.left, BorderSide::Left);
    DrawBorderEdge(hdc, border.right, BorderSide::Right);
}

void Widget::RenderBackground(HDC hdc) {
    if(backgroundColor.a > 0) { // only draw if non-transparent
        ScopedOwnedBrush br(hdc, backgroundColor.toCOLORREF());
        RECT r = EffectiveRect();
        FillRect(hdc, &r, br.get());
    }
}

// --- Rendering ------------------------------------------------------
void Widget::InitRender(HDC hdc) {
    if(!effectiveDisplayed || !visible) return;
    
    // Render must not call SaveDC/RestoreDC again - it's taken care of here
    int saved = SaveDC(hdc);
    RenderBackground(hdc);
    Render(hdc);
    RenderBorder(hdc);
    if(onRender) {
        onRender();
    }
    RestoreDC(hdc, saved);
}

// --- Other ----------------------------------------------------------
void Widget::ResetTransientStates() {
    hovered = false;
    pressed = false;
    mouseDownInside = false;
}