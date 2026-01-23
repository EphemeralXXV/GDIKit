#pragma once

#include <string>
#include <memory>

#include "Widget.h"
#include "Container.h"
#include "Button.h"
#include "Label.h"
#include "Color.h"

class Menu : public Container {
    public:
        using WidgetPtr = std::shared_ptr<Widget>;
        using ContainerPtr = std::shared_ptr<Container>;
        using ButtonPtr = std::shared_ptr<Button>;
        using LabelPtr = std::shared_ptr<Label>;
        
        // Constructor
        Menu(const std::wstring &t = L"Menu");

        // Appearance
        bool IsCollapsed() const { return isCollapsed; }
        void SetCollapsed(bool collapsed);

        // Override defaults to update layout
        void SetSize(int w, int h);
        void SetPosSize(int x, int y, int w, int h);

        void SetTitle(const std::wstring &t);
        std::wstring GetTitle() const { return title; }
        void SetShowTitleBar(bool show);

        // Delegate to body, which is the only mutable part of the menu (for now)
        void SetBodyLayout(std::unique_ptr<Layout> newLayout) {
            if(!bodyContainer) return;
            bodyContainer->SetLayout(std::move(newLayout));
        }
        void SetBodyPadding(int all) {
            if(!bodyContainer) return;
            bodyContainer->SetPadding(all);
        }
        void SetBodyPadding(int horizontal, int vertical) {
            if(!bodyContainer) return;
            bodyContainer->SetPadding(vertical, horizontal);
        }
        void SetBodyPadding(int top, int bottom, int left, int right) {
            if(!bodyContainer) return;
            bodyContainer->SetPadding(top, bottom, left, right);
}
        void SetBodyBackgroundColor(const Color &color) {
            if(!bodyContainer) return;
            bodyContainer->SetBackgroundColor(color);
        }

        // --- Child management --------------------------------------------------
        void AddBodyChild(const WidgetPtr& child) {
            // Delegate to body, which is the only mutable part of the menu (for now)
            bodyContainer->AddChild(child);
        }
        void RemoveAllBodyChildren() {
            bodyContainer->RemoveAllChildren();  // only clear body children
        }

        // --- Rendering ---------------------------------------------------------
        void Render(HDC hdc) override;

    private:
        // Resize handle in the bottom-right of the menu
        RECT ResizeHandleRect() const;
        void RenderResizeHandle(HDC hdc) const;

        // Header (title bar)
        ContainerPtr headerContainer;
        LabelPtr titleLabel;
        ButtonPtr closeButton;
        ButtonPtr collapseButton;

        // Body (main content)
        ContainerPtr bodyContainer;

        // Sub-container initialization
        void InitHeader();
        void InitBody();

        // Window state
        bool isCollapsed;
        bool isDragging;
        bool isResizing;
        POINT dragOffset;
        POINT resizeOffset;
        int resizeHandleSize;

        // Title bar
        std::wstring title;
        int titleBarHeight;
};
