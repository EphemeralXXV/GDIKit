#pragma once

#include <memory>
#include <vector>

#include "Widget.h"
#include "Layout.h"
#include "Color.h"
#include "Border.h"

using WidgetPtr = std::shared_ptr<Widget>;

class Container : public Widget {
public:
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

    // Appearance
    void SetBackgroundColor(const Color& newColor) { backgroundColor = newColor; }
    Color GetBackgroundColor() { return backgroundColor; }

    void SetBorder(const Color& color, int thickness = 1, BorderSide sides = BorderSide::All);
    Border GetBorder() const { return border; }

    // Rendering
    void Render(HDC hdc) override;

    void FeedMouseEvent(const MouseEvent& e);

protected:
    std::vector<WidgetPtr> children;
    std::unique_ptr<Layout> layout;
    Color backgroundColor;
    Border border;
};