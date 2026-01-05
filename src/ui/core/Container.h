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

        // Child management
        void AddChild(const WidgetPtr& child);
        void RemoveChild(const WidgetPtr& child);
        void RemoveAllChildren();
        const std::vector<WidgetPtr>& Children() const { return children; }

        // --- Layout policy ---
        void SetLayout(std::unique_ptr<Layout> newLayout);
        Layout* GetLayout() const { return layout.get(); }
        void UpdateInternalLayout() override;

        // --- Display & Visibility ---
        void UpdateEffectiveDisplay() override;

        // Rendering
        void Render(HDC hdc) override;

        bool FeedMouseEvent(const MouseEvent& e) override;

    protected:
        std::vector<WidgetPtr> children;
        std::unique_ptr<Layout> layout;
    };