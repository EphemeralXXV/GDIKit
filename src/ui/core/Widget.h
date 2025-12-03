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
        bool hovered;
        bool pressed;
        bool mouseDownInside; // tracks if mouse click began within widget

        bool clipChildren;

        // Mouse listeners
        std::vector<std::function<void(const MouseEvent&)>> mouseListeners;

        // --- Geometry -----------------------------------------------------
        // Absolute coordinate getters (relative => absolute)
        int AbsX() const;
        int AbsY() const;
        int AbsRight() const;
        int AbsBottom() const;
        RECT AbsRect() const;

        // Size setters
        void SetRect(int l, int t, int r, int b);       // Sets the relative rect
        void SetPosSize(int x, int y, int w, int h);    // Sets the absolute position
        void SetPreferredSize(int w, int h);

        // Helper functions reacting to geometry changes
        void UpdateConvenienceGeometry();               // Updates convenience geometry vars on internal geometry changes
        virtual void UpdateInternalLayout();            // Updates automatic layouts on geometry changes

        // Get final internal geometry computed from preferred size
        int GetLayoutWidth() const;
        int GetLayoutHeight() const;
        
        // Test if cursor currently over widget
        bool MouseInRect(POINT p) const;

        void AddMouseListener(std::function<void(const MouseEvent&)> callback) {
            mouseListeners.push_back(callback);
        }

        // Public FireMouseEvent wrapper for forwarding mouse events from system or parents
        // Should probably use a friend class in the future?
        void FeedMouseEvent(const MouseEvent& e) {
            switch(e.type) {
                case MouseEventType::Enter:
                case MouseEventType::Leave:
                case MouseEventType::Move:  OnMouseMove(e.pos); break;
                case MouseEventType::Down:  OnMouseDown(e.pos); break;
                case MouseEventType::Click:
                case MouseEventType::Up:    OnMouseUp(e.pos);   break;
            }
        }

        // --- Rendering ---
        virtual void Render(HDC hdc);

    protected:
        // --- Mouse event handlers -----------------------------------------
        void FireMouseEvent(const MouseEvent& e) {
            for(auto& listener : mouseListeners)
                listener(e);
        }

        virtual void OnMouseMove(POINT p);
        virtual void OnMouseDown(POINT p);
        virtual void OnMouseUp(POINT p);
};