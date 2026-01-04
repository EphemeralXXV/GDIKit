#include <algorithm>

#include "Widget.h"

// Constructor
Widget::Widget() :
    parent(nullptr), rect({0, 0, 0, 0}),
    x(0), y(0), width(0), height(0),
    preferredWidth(0), preferredHeight(0),
    displayed(true), effectiveDisplayed(true),
    visible(true), enabled(true), hovered(false),
    pressed(false), mouseDownInside(false),
    ignoreMouseEvents(false), clipChildren(false)
{}

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

// Size setters
void Widget::SetRect(int l, int t, int r, int b) {
    // Sets the relative rect
    rect = {l, t, r, b};
    UpdateConvenienceGeometry();
    UpdateInternalLayout();
}
void Widget::SetPos(int x, int y) {
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    rect = {x, y, x + w, y + h};
    UpdateConvenienceGeometry();
}
void Widget::SetSize(int w, int h) {
    rect = {rect.left, rect.top, rect.left + w, rect.top + h};
    UpdateConvenienceGeometry();
    UpdateInternalLayout();
}
void Widget::SetPosSize(int x, int y, int w, int h) {
    rect = { x, y, x + w, y + h };
    UpdateConvenienceGeometry();
    UpdateInternalLayout();
}
void Widget::SetPreferredSize(int w, int h) {
    preferredWidth = w;
    preferredHeight = h;
    UpdateInternalLayout();
}

// Helper functions reacting to geometry changes
void Widget::UpdateConvenienceGeometry() {
    // Updates convenience geometry vars on internal geometry changes
    x = rect.left;
    y = rect.top;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}
void Widget::UpdateInternalLayout() {
    // Updates automatic layouts on geometry changes
    // default: no-op. Derived widgets may override to update children elements.
}
void Widget::OnInternalLayoutUpdated() {}

// Get final internal geometry computed from preferred size
int Widget::GetLayoutWidth() const {
    return preferredWidth > 0 ? preferredWidth : width;
}
int Widget::GetLayoutHeight() const {
    return preferredHeight > 0 ? preferredHeight : height;
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
    RECT clip = AbsRect();

    // Calculate true hit area if clipped by any of the ancestors
    const Widget* ancestor = parent;
    while(ancestor) {
        if(ancestor->clipChildren) {
            RECT parentRect = ancestor->AbsRect();
            IntersectRect(&clip, &clip, &parentRect);
        }
        ancestor = ancestor->parent;
    }

    return PtInRect(&clip, p);
}

// Mouse listeners
void Widget::AddMouseListener(std::function<void(const MouseEvent&)> callback) {
    mouseListeners.push_back(callback);
}
void Widget::RemoveMouseListener(const std::function<void(const MouseEvent&)>& callback) {
    // Erase any listener that compares equal to the given callback
    mouseListeners.erase(
        std::remove_if(mouseListeners.begin(), mouseListeners.end(),
            [&](const std::function<void(const MouseEvent&)>& stored) {
                // std::function doesn't have operator==, so compare targets
                return stored.target_type() == callback.target_type() &&
                    stored.target<void(const MouseEvent&)>() == callback.target<void(const MouseEvent&)>();
            }),
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
        listener(e);
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

// --- Rendering ------------------------------------------------------
void Widget::InitRender(HDC hdc) {
    if(!effectiveDisplayed || !visible) return;
    
    // Render must not call SaveDC/RestoreDC again - it's taken care of here
    int saved = SaveDC(hdc);
    Render(hdc);
    RestoreDC(hdc, saved);
}

// --- Other ----------------------------------------------------------
void Widget::ResetTransientStates() {
    hovered = false;
    pressed = false;
    mouseDownInside = false;
}