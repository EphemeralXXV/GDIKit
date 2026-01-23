#pragma once

#include <memory>
#include <vector>

#include "Widget.h"
#include "Layout.h"
#include "Color.h"

class Container : public Widget {
    public:
        using WidgetPtr = std::shared_ptr<Widget>;
        
        // Constructor
        Container();

        // Ancestors
        Container* GetParent() const override { return dynamic_cast<Container*>(parent); }

        // Child management
        void AddChild(const WidgetPtr& child);
        void RemoveChild(const WidgetPtr& child);
        void RemoveAllChildren();
        const std::vector<WidgetPtr>& Children() const { return children; }

        // --- Geometry & Layout ---
        RECT ApplyChildMargin(const RECT& inner /*Rect - padding*/, const Widget& child) const;

        // Override Widget's no-op implementation
        Layout* GetLayout() const override { return layout.get(); }
        void SetLayout(std::unique_ptr<Layout> newLayout);
        void UpdateInternalLayout() override;
        void UpdateEffectiveGeometry() override;

        // --- Display & Visibility ---
        void UpdateEffectiveDisplay() override;

        // Rendering
        void Render(HDC hdc) override;

        bool FeedMouseEvent(const MouseEvent& e) override;

    protected:
        std::unique_ptr<Layout> layout;
        std::vector<WidgetPtr> children;
    };