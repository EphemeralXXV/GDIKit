#include <algorithm>

#include "Container.h"

Container::Container() {
    // default container behavior
}

void Container::AddChild(const WidgetPtr& child) {
    if(!child) return;
    child->SetParent(this);
    children.push_back(child);
    InvalidateLayout();
}

void Container::RemoveChild(const WidgetPtr& child) {
    if(!child) return;

    auto it = std::remove(children.begin(), children.end(), child);

    // Only remove the widget if it's actually a child
    if(it != children.end()) {
        child->SetParent(nullptr);
        children.erase(it, children.end());
    }
    
    InvalidateLayout();
}

void Container::RemoveAllChildren() {
    for(auto& child : children) {
        child->SetParent(nullptr);
    }
    children.clear();
    InvalidateLayout();
}

RECT Container::ApplyChildMargin(const RECT& inner, const Widget& child) const {
    const Spacing& m = child.GetMargin();

    RECT r;
    r.top    = inner.top    + m.top;
    r.bottom = inner.bottom - m.bottom;
    r.left   = inner.left   + m.left;
    r.right  = inner.right  - m.right;

    return r;
}

void Container::SetLayout(std::unique_ptr<Layout> newLayout) {
    if(!newLayout) return;
    layout = std::move(newLayout);
    layout->SetContainer(this);
    InvalidateLayout();
}

void Container::UpdateInternalLayout() {
    // First give derived classes a chance to update their own geometry
    Widget::UpdateInternalLayout();

    // Apply layout policy if present
    if(layout) {
        RECT inner = ComputeInnerRect();
        layout->Apply(inner);
        
        for(auto& child : children) {
            if(!child) continue;

            child->UpdateInternalLayout();
        }
    }
    else {
        // If no layout (absolute positioning), position the children manually
        for(auto& child : children) {
            if(!child) continue;

            child->InvalidateLayout();
        }
    }
    // Give derived classes a chance to react
    OnInternalLayoutUpdated();
}
void Container::UpdateEffectiveGeometry() {
    Widget::UpdateEffectiveGeometry();
    if(layout) {
        RECT inner = ComputeInnerRect();
        layout->Apply(inner);
    }
    for(auto& child : children) {
        if(!child) continue; 
        child->UpdateEffectiveGeometry();
    }
}

void Container::UpdateEffectiveDisplay() {
    Widget::UpdateEffectiveDisplay();

    // Propagate to children
    for(auto& child : children) {
        if(child) {
            child->UpdateEffectiveDisplay();
        }
    }
}

void Container::Render(HDC hdc) {
    // Render children in order
    for(auto &c : children) {
        if(c) c->InitRender(hdc);
    }
}

bool Container::FeedMouseEvent(const MouseEvent& e) {
    bool handled = false;
    Widget* root = GetMainContainer();

    // Move should update hover state for ALL children
    // because Move is not an exclusive event (e.g. might leave one widget and enter another in the event)
    if(e.type == MouseEventType::Move) {
        // First feed the event to children back-to-front (events bubble up)
        for(auto it = children.rbegin(); it != children.rend(); ++it) {
            if(*it) {
                handled = (*it)->InitFeedMouseEvent(e) || handled;
            }
        }
        // Finally give the container itself a chance to handle the event
        bool selfHandled = Widget::FeedMouseEvent(e);
        handled = handled || selfHandled;
    }

    // Other mouse events: stop on first consumer (hit test behavior)
    for(auto it = children.rbegin(); it != children.rend(); ++it) {
        if(*it && (*it)->InitFeedMouseEvent(e)) {
            handled = true;
            break;
        }
    }

    // Container itself, or always root
    if(!handled || this == root) {
        handled = Widget::FeedMouseEvent(e);
    }

    return handled;
}