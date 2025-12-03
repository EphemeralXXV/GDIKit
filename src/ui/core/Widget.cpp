#include "Widget.h"

// Constructor
Widget::Widget() :
    parent(nullptr), rect({0, 0, 0, 0}),
    x(0), y(0), width(0), height(0),
    preferredWidth(0), preferredHeight(0),
    visible(true), enabled(true),
    hovered(false), pressed(false),
    mouseDownInside(false),
    clipChildren(false)
{}

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
    return RECT{ AbsX(), AbsY(), AbsX() + width, AbsY() + height};
}

// Size setters
void Widget::SetRect(int l, int t, int r, int b) {
    // Sets the relative rect
    rect = {l, t, r, b};
    UpdateConvenienceGeometry();
    UpdateInternalLayout();
}
void Widget::SetPosSize(int x, int y, int w, int h) {
    // Sets the absolute position
    rect = {x, y, x + w, y + h};
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

// Get final internal geometry computed from preferred size
int Widget::GetLayoutWidth() const {
    return preferredWidth > 0 ? preferredWidth : width;
}
int Widget::GetLayoutHeight() const {
    return preferredHeight > 0 ? preferredHeight : height;
}

// --- Mouse event handlers -------------------------------------------
void Widget::OnMouseMove(POINT p) {
    bool wasHovered = hovered; // read old state
    hovered = MouseInRect(p);  // read current state

    if(hovered && !wasHovered) FireMouseEvent({MouseEventType::Enter, p, 1});
    if(!hovered && wasHovered) FireMouseEvent({MouseEventType::Leave, p, 1});
    // Fire Move also if pressed - in case of dragging, for example
    if(hovered || pressed) FireMouseEvent({MouseEventType::Move, p, 1});
}
void Widget::OnMouseDown(POINT p) {
    if(!enabled) return;
    if(MouseInRect(p)) {
        pressed = true;
        mouseDownInside = true; // track click start
        FireMouseEvent({MouseEventType::Down, p, 1});
    }
}
void Widget::OnMouseUp(POINT p) {
    if(!enabled) return;
    if(pressed) {
        // Let MouseUp happen outside widget, e.g. to cancel selection
        FireMouseEvent({MouseEventType::Up, p, 1});
        if(mouseDownInside && MouseInRect(p)) {
            FireMouseEvent({MouseEventType::Click, p, 1});
        }
    }
    pressed = false;
    mouseDownInside = false;
}

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

// --- Rendering ------------------------------------------------------
void Widget::Render(HDC hdc) {}