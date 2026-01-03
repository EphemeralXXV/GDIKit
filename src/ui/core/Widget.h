#pragma once

#include <vector>
#include <functional>
#include <windows.h>

enum class MouseEventType { Enter, Leave, Move, Down, Up, Click };
enum class MouseButton { None = 0, Left = 1, Right = 2 };
struct MouseEvent {
    MouseEventType type;
    POINT pos;  // relative to widget
    MouseButton button;
};

class Widget {
    public:
        // Constructor & destructor
        Widget();
        virtual ~Widget() {};

        // Ancestors
        Widget* GetParent() const { return parent; }
        void SetParent(Widget* newParent);
        Widget* GetMainContainer() const; // Gets the topmost non-Root container

        // Visual state
        bool IsDisplayed() const { return displayed; }
        void SetDisplayed(bool displayed);

        bool IsVisible() const { return visible; }
        void SetVisible(bool visible);

        bool IsEnabled() const { return enabled; }
        void SetEnabled(bool enabled) { this->enabled = enabled; }

        bool IsClippingChildren() const { return clipChildren; }
        void SetChildrenClipping(bool clipChildren) { this->clipChildren = clipChildren; }

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
        void SetPos(int x, int y);                      // Sets the absolute position
        void SetSize(int w, int h);                     // Sets the size
        void SetPosSize(int x, int y, int w, int h);    // Sets the absolute position and size

        // Updates automatic layouts on geometry changes
        virtual void UpdateInternalLayout(); // INTERNAL USE ONLY!
        virtual void OnInternalLayoutUpdated(); // optional callback

        // --- Display & Visibility -----------------------------------------
        virtual void UpdateEffectiveDisplay();  // Updates actual display state based on ancestors state
                                                // Virtual, because Container should override to propagate further
        
        // Test if cursor currently over widget
        bool MouseInRect(POINT p) const;

        // Mouse listeners
        void AddMouseListener(std::function<void(const MouseEvent&)> callback);
        void RemoveMouseListener(const std::function<void(const MouseEvent&)>& callback);

        // Pre-feeding logic (condition checks, etc.) - template method
        virtual void InitFeedMouseEvent(const MouseEvent& e) final;

        // --- Rendering ---
        virtual void InitRender(HDC hdc) final; // Pre-render logic (condition checks, etc.) - template method

    protected:
        // Pointer to parent widget (container)
        Widget* parent;

        // Bounding rectangle relative to parent
        RECT rect;

        // Widget states
        bool displayed; // a'la CSS display
        bool effectiveDisplayed; // Internal/inherited display state (must be non-public)
        bool visible;   // a'la CSS visible
        bool enabled;
        bool clipChildren;
        bool hovered;
        bool pressed;
        bool mouseDownInside; // tracks if mouse click began within widget

        // --- Geometry -----------------------------------------------------
        // Convenient expressions of rect geometry
        int x, y;                               // Origin (top-left) relative to parent
        int width, height;                      // Internal widget size
        int preferredWidth, preferredHeight;    // Widget size as intended by client code (excludes paddings, margins, labels, etc.)
        void SetPreferredSize(int w, int h);    // Should only be used internally, mainly by layouts

        // Helper functions reacting to geometry changes
        void UpdateConvenienceGeometry();       // Updates convenience geometry vars on internal geometry changes

        // --- Mouse events  ------------------------------------------------
        std::vector<std::function<void(const MouseEvent&)>> mouseListeners;

        void FireMouseEvent(const MouseEvent& e);

        // Public FireMouseEvent wrapper for forwarding mouse events from system or parents
        // Contains actual feeding logic
        virtual void FeedMouseEvent(const MouseEvent& e);

        virtual void OnMouseMove(POINT p);
        virtual void OnMouseDown(POINT p);
        virtual void OnMouseUp(POINT p);

        // --- Other events  ------------------------------------------------
        virtual void OnRemovedFromTree() { ResetTransientStates(); };
        virtual void OnDisplayChanged(bool displayed) { if(!displayed) ResetTransientStates(); };
        virtual void OnVisibilityChanged(bool visible) { if(!visible) ResetTransientStates(); };
        virtual void ResetTransientStates();

        // --- Rendering ---
        virtual void Render(HDC hdc) {}; // Actual render logic
};