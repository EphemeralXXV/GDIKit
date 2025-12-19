#include <algorithm>

#include "Container.h"

Container::Container() :
    backgroundColor(Color::FromARGB(0, 0, 0, 0))
{
    // default container behavior
}

void Container::AddChild(const WidgetPtr& child) {
    if(!child) return;
    child->SetParent(this);
    children.push_back(child);
    UpdateInternalLayout();
}

void Container::RemoveChild(const WidgetPtr& child) {
    if(!child) return;

    auto it = std::remove(children.begin(), children.end(), child);

    // Only remove the widget if it's actually a child
    if(it != children.end()) {
        child->SetParent(nullptr);
        children.erase(it, children.end());
    }
    
    UpdateInternalLayout();
}

void Container::RemoveAllChildren() {
    for(auto& child : children) {
        child->SetParent(nullptr);
    }
    children.clear();
    UpdateInternalLayout();
}

void Container::SetLayout(std::unique_ptr<Layout> newLayout) {
    if(!newLayout) return;
    layout = std::move(newLayout);
    layout->SetContainer(this);
    UpdateInternalLayout();
}

void Container::UpdateInternalLayout() {
    // First give derived classes a chance to update their own geometry
    Widget::UpdateInternalLayout();

    // Apply layout policy if present
    if(layout) layout->Apply();

    // Call UpdateInternalLayout on children so nested containers propagate
    for(auto& c : children) {
        if(c) c->UpdateInternalLayout();
    }
    // Give derived classes a chance to react
    OnInternalLayoutUpdated();
}

void Container::SetBorder(const Color& color, int thickness, BorderSide sides) {
    border.color = color;
    border.thickness = thickness;
    border.sides = sides;
}

void Container::Render(HDC hdc) {
    if(!visible) return;

    // Background
    if(backgroundColor.a > 0) { // only draw if non-transparent
        HBRUSH br = CreateSolidBrush(backgroundColor.toCOLORREF());
        RECT r = AbsRect();
        FillRect(hdc, &r, br);
        DeleteObject(br);
    }

    // Border
    if(border.thickness > 0) {
        HPEN pen = CreatePen(PS_SOLID, border.thickness, border.color.toCOLORREF());
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        RECT r = AbsRect();

        if(HasSide(border.sides, BorderSide::Top)) {
            MoveToEx(hdc, r.left, r.top, nullptr);
            LineTo(hdc, r.right, r.top);
        }
        if(HasSide(border.sides, BorderSide::Bottom)) {
            MoveToEx(hdc, r.left, r.bottom - border.thickness, nullptr);
            LineTo(hdc, r.right, r.bottom - border.thickness);
        }
        if(HasSide(border.sides, BorderSide::Left)) {
            MoveToEx(hdc, r.left, r.top, nullptr);
            LineTo(hdc, r.left, r.bottom);
        }
        if(HasSide(border.sides, BorderSide::Right)) {
            MoveToEx(hdc, r.right - border.thickness, r.top, nullptr);
            LineTo(hdc, r.right - border.thickness, r.bottom);
        }

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    // Render children in order
    for(auto &c : children) {
        if(c && c->IsVisible()) c->Render(hdc);
    }
}

void Container::FeedMouseEvent(const MouseEvent& e) {
    // First feed the event to children back-to-front (events bubble up)
    for(auto it = children.rbegin(); it != children.rend(); ++it) {
        if(*it && (*it)->IsVisible()) {
            (*it)->FeedMouseEvent(e);
        }
    }

    // Finally give the container itself a chance to handle the event
    Widget::FeedMouseEvent(e);
}