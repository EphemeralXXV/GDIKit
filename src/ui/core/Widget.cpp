#include "Widget.h"

// --- Constructor / Destructor -------------------------------------
Widget::Widget() :
    x(0), y(0), width(0), height(0),
    preferredWidth(0), preferredHeight(0),
    visible(true), enabled(true),
    hovered(false), pressed(false),
    parent(nullptr), rect({0, 0, 0, 0})
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
    hovered = MouseInRect(p);
}
void Widget::OnMouseDown(POINT p) {
    if(!enabled) return;
    if(MouseInRect(p)) pressed = true;
}
void Widget::OnMouseUp(POINT p) {
    if(!enabled) return;
    if(pressed && MouseInRect(p)) {
        // default: no-op. Derived widgets may override to trigger actions.
    }
    pressed = false;
}

// Test if cursor currently over widget
bool Widget::MouseInRect(POINT p) const {
    return p.x >= AbsX() && p.x <= AbsRight() &&
        p.y >= AbsY() && p.y <= AbsBottom();
}

// --- Rendering ------------------------------------------------------
void Widget::Render(HDC hdc) {}