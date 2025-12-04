#pragma once

#include <string>
#include <memory>

#include "Widget.h"
#include "Button.h"
#include "Color.h"
#include "LayoutContext.h"

using WidgetPtr = std::shared_ptr<Widget>;
using ButtonPtr = std::shared_ptr<Button>;

class Menu : public Widget {
    public:
        // Constructor
        Menu(const std::wstring &t = L"Menu");

        // Appearance
        bool IsCollapsed() const { return isCollapsed; }
        void SetCollapsed(bool collapsed) { isCollapsed = collapsed; }

        void SetTitle(const std::wstring &t) { title = t; }
        void SetShowTitleBar(bool show) { showTitleBar = show; }

        void SetBackgroundColor(const Color &color) { backgroundColor = color; }
        void SetDrawBackground(bool draw) { drawBackground = draw; }

        // --- Child management --------------------------------------------------        
        void AddHeaderChild(const WidgetPtr &child);
        void AddBodyChild(const WidgetPtr &child);
        void RemoveAll();

        // --- Layout - should move to generic container struct once it exists ---
        void BeginLayout(int startX, int startY);
        void EndLayout();

        // Place child widget in vertical layout
        void ApplyLayout(Widget* w);

        // AddChild wrapper for containers with layout -- SoC preservation just in case
        void AddChildWithLayout(const WidgetPtr& child);

        // --- Rendering ---------------------------------------------------------
        void Render(HDC hdc) override;

    protected:
        // Update children geometry dynamically
        void UpdateInternalLayout() override;

    private:
        LayoutContext currentLayout;

        // Resize handle in the bottom-right of the menu
        RECT ResizeHandleRect() const;
        void RenderResizeHandle(HDC hdc) const;

        // Children
        std::vector<WidgetPtr> headerChildren;  // Title bar elements
        std::vector<WidgetPtr> bodyChildren;    // Main content

        WidgetPtr titleBar;
        ButtonPtr closeButton;
        ButtonPtr collapseButton;
        
        // Create children immediately
        void InitInternalElements();

        // Window state
        bool isCollapsed;
        bool isDragging;
        bool isResizing;
        POINT dragOffset;
        POINT resizeOffset;
        int resizeHandleSize;

        // Title bar
        std::wstring title;
        bool showTitleBar;
        int titleBarHeight;

        // Appearance
        Color backgroundColor;
        bool drawBackground;
};