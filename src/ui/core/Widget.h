#pragma once

#include <vector>
#include <functional>
#include <windows.h>

enum class MouseEventType { Enter, Leave, Move, Down, Up, Click };
struct MouseEvent {
    MouseEventType type;
    POINT pos;  // relative to widget
    int button; // left=1, right=2
};

class Widget {
    public:
        // Constructor & destructor
        Widget();
        virtual ~Widget() {};

        Widget* GetParent() const { return parent; }
        void SetParent(Widget* newParent) { parent = newParent; }

        // Visual state
        bool IsVisible() const { return visible; }
        void SetVisible(bool visible) { this->visible = visible; }

        bool IsEnabled() const { return enabled; }
        void SetEnabled(bool enabled) { this->enabled = enabled; }

        bool IsClippingChildren() const { return clipChildren; }
        void SetChildrenClipping(bool clipChildren) { clipChildren = clipChildren; }

        // --- Geometry -----------------------------------------------------
        // Relative geometry read access
        int GetX() const { return x; }
        int GetY() const { return y; }

        int GetWidth() const { return width; }
        int GetHeight() const { return height; }

        int GetPreferredWidth() const { return preferredWidth; }
        int GetPreferredHeight() const { return preferredHeight; }

        RECT GetRect() const { return rect; }

        // Absolute coordinate getters (relative => absolute)
        int AbsX() const;
        int AbsY() const;
        int AbsRight() const;
        int AbsBottom() const;
        RECT AbsRect() const;

        // Get final internal geometry computed from preferred size
        int GetLayoutWidth() const;
        int GetLayoutHeight() const;

        // Size setters
        void SetRect(int l, int t, int r, int b);       // Sets the relative rect
        void SetPosSize(int x, int y, int w, int h);    // Sets the absolute position
        void SetPreferredSize(int w, int h);

        // Updates automatic layouts on geometry changes
        virtual void UpdateInternalLayout(); // INTERNAL USE ONLY!
        
        // Test if cursor currently over widget
        bool MouseInRect(POINT p) const;

        void AddMouseListener(std::function<void(const MouseEvent&)> callback);

        // Public FireMouseEvent wrapper for forwarding mouse events from system or parents
        // Should probably use a friend class in the future?
        void FeedMouseEvent(const MouseEvent& e);

        // --- Rendering ---
        virtual void Render(HDC hdc);

    protected:
        // Pointer to parent widget (container)
        Widget* parent;

        // Bounding rectangle relative to parent
        RECT rect;

        // Convenient expressions of rect geometry
        int x, y;                               // Origin (top-left) relative to parent
        int width, height;                      // Internal widget size
        int preferredWidth, preferredHeight;    // Widget size as intended by client code (excludes paddings, margins, labels, etc.)

        // Widget states
        bool visible;
        bool enabled;
        bool clipChildren;
        bool hovered;
        bool pressed;
        bool mouseDownInside; // tracks if mouse click began within widget

        // --- Geometry -----------------------------------------------------
        // Helper functions reacting to geometry changes
        void UpdateConvenienceGeometry();               // Updates convenience geometry vars on internal geometry changes

        // --- Mouse events  ------------------------------------------------
        std::vector<std::function<void(const MouseEvent&)>> mouseListeners;

        void FireMouseEvent(const MouseEvent& e);

        virtual void OnMouseMove(POINT p);
        virtual void OnMouseDown(POINT p);
        virtual void OnMouseUp(POINT p);
};