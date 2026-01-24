#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <windows.h>

#include "Color.h"
#include "Border.h"

enum class MouseEventType { Enter, Leave, Move, Down, Up, Click };
enum class MouseButton { None = 0, Left = 1, Right = 2 };

struct MouseEvent {
    MouseEventType type;
    POINT pos;  // absolute (screen coordinates)
    MouseButton button;
};
struct MouseListener {
    size_t id;
    std::function<void(const MouseEvent&)> callback;
};
struct Spacing {
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;
};
struct DimensionProperties {
    bool isAuto = false;
};

class Layout;
class Widget {
    public:
        // Allow Layout access to select parts of Widget via a dedicated proxy
        friend class LayoutWidgetBridge;

        // Constructor & destructor
        Widget();
        virtual ~Widget() {};

        // Ancestors
        virtual Widget* GetParent() const { return parent; }
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
        RECT ComputeInnerRect() const; // Compute rect - padding - border

        // Absolute coordinate getters (relative => absolute)
        int AbsX() const;
        int AbsY() const;
        int AbsRight() const;
        int AbsBottom() const;
        RECT AbsRect() const;

        // Effective coordinate getters
        int EffectiveX() const;
        int EffectiveY() const;
        int EffectiveRight() const;
        int EffectiveBottom() const;
        RECT EffectiveRect() const;
        int EffectiveWidth() const;
        int EffectiveHeight() const;
        
        // Empty getter, because non-Container Widgets can't have children
        // Ergo, they can't have layouts
        // So Container must implement this method on its own
        // As well as the setter and the field itself
        virtual Layout* GetLayout() const { return nullptr; }

        // Get final internal geometry computed from preferred size
        int GetLayoutWidth() const;
        int GetLayoutHeight() const;

        // Size setters
        void SetRect(int l, int t, int r, int b);       // Sets the relative rect
        void SetPos(int x, int y);                      // Sets the position relative to parent
        void SetSize(int w, int h);                     // Sets the size
        void SetPosSize(int x, int y, int w, int h);    // Sets the relative position and size

        // Automatic geometry updates
        virtual void InvalidateLayout(); // Recompute effective geometry on logical changes
        virtual void UpdateInternalLayout(); // Propagate effective geometry recomputation
        // Optional callback; DO NOT set logical geometry here - it'll create an infinite loop
        virtual void OnInternalLayoutUpdated();
        virtual void UpdateEffectiveGeometry(); // Updates effective geometry
                                                // Virtual, because Container should override to propagate further

        // Spacing and dynamic geometry properties
        const Spacing& GetPadding() const { return padding; }
        void SetPadding(int all);
        void SetPadding(int horizontal, int vertical);
        void SetPadding(int top, int bottom, int left, int right);
        
        const Spacing& GetMargin() const { return margin; }
        void SetMargin(int all);
        void SetMargin(int horizontal, int vertical);
        void SetMargin(int top, int bottom, int left, int right);

        DimensionProperties GetWidthProperties() const { return widthProperties; }
        void SetWidthProperties(DimensionProperties properties);
        DimensionProperties GetHeightProperties() const { return heightProperties; }
        void SetHeightProperties(DimensionProperties properties);
        bool IsAutoWidth() const { return widthProperties.isAuto; }
        void SetAutoWidth(bool isAuto);
        bool IsAutoHeight() const { return heightProperties.isAuto; }
        void SetAutoHeight(bool isAuto);
        bool IsFlexGrow() const { return isFlexGrow; }
        void SetFlexGrow(bool flexGrow) { isFlexGrow = flexGrow; }

        // --- Display & Visibility -----------------------------------------
        virtual void UpdateEffectiveDisplay();  // Updates actual display state based on ancestors state
                                                // Virtual, because Container should override to propagate further
        
        // Test if cursor currently over widget
        bool MouseInRect(POINT p) const;

        // Mouse listeners
        size_t AddMouseListener(std::function<void(const MouseEvent&)> callback); // returns ID
        void RemoveMouseListener(size_t id);

        // Pre-feeding logic (condition checks, etc.) - template method
        virtual bool InitFeedMouseEvent(const MouseEvent& e) final;

        // Ignore mouse events to let them fall through to ancestors
        void SetMouseEventsIgnoring(bool ignore) { ignoreMouseEvents = ignore; }
        bool IsIgnoringMouseEvents() { return ignoreMouseEvents; }

        // --- Appearance ---
        Color GetBackgroundColor() const { return backgroundColor; }
        void SetBackgroundColor(const Color& newColor) { backgroundColor = newColor; }

        Border GetBorder() const { return border; }
        void SetBorder(int thickness, const Color& color, BorderSide sides);        

        // --- Rendering ---
        virtual void InitRender(HDC hdc) final; // Pre-render logic (condition checks, etc.) - template method

    protected:
        // Pointer to parent widget (container)
        Widget* parent = nullptr;

        // Bounding rectangles relative to parent
        RECT rect = {0, 0, 0, 0};   // LOGICAL - as set by client code
        RECT effectiveRect;         // EFFECTIVE - as computed internally and rendered on the screen
                                    // (includes offsets, margins, padding, etc.)
        void SetEffectiveRect(int l, int t, int r, int b);
        // Compute and apply effective geometry from logical geometry + padding, margins, etc.
        void ApplyLogicalGeometry(); 

        // Widget states
        bool displayed = true; // a'la CSS display
        bool effectiveDisplayed = true; // Internal/inherited display state (must be non-public)
        bool visible = true;   // a'la CSS visible
        bool enabled = true;
        bool clipChildren = false;
        bool hovered = false;
        bool pressed = false;
        bool mouseDownInside = false; // tracks if mouse click began within widget
        bool ignoreMouseEvents = false; // a'la pointer-events: none

        // --- Geometry -----------------------------------------------------
        // Convenient expressions of rect geometry
        int x = 0, y = 0;                               // Origin (top-left) relative to parent
        int width = 0, height = 0;                      // Internal widget size
        int preferredWidth = 0, preferredHeight = 0;    // Widget size as intended by client code (excludes paddings, margins, labels, etc.)
        void SetPreferredSize(int w, int h);            // Should only be used internally, mainly by layouts

        // Helper functions reacting to geometry changes
        void UpdateConvenienceGeometry();       // Updates convenience geometry vars on internal geometry changes

        // Spacing and dynamic geometry properties
        Spacing padding;
        Spacing margin;
        DimensionProperties widthProperties;
        DimensionProperties heightProperties;
        bool isFlexGrow = false;

        // --- Mouse events  ------------------------------------------------
        std::vector<MouseListener> mouseListeners;
        size_t nextListenerID = 1; // 0 reserved for special cases (null, invalid, etc.)

        void FireMouseEvent(const MouseEvent& e);

        // Public FireMouseEvent wrapper for forwarding mouse events from system or parents
        // Contains actual feeding logic
        virtual bool FeedMouseEvent(const MouseEvent& e);

        virtual bool OnMouseMove(POINT p);
        virtual bool OnMouseDown(POINT p);
        virtual bool OnMouseUp(POINT p);

        // --- Other events  ------------------------------------------------
        virtual void OnRemovedFromTree() { ResetTransientStates(); };
        virtual void OnDisplayChanged(bool displayed) { if(!displayed) ResetTransientStates(); };
        virtual void OnVisibilityChanged(bool visible) { if(!visible) ResetTransientStates(); };
        virtual void ResetTransientStates();

        // --- Appearance ---
        Border border;
        void DrawBorderEdge(HDC hdc, BorderData borderData, BorderSide side);
        void RenderBorder(HDC hdc);
        
        Color backgroundColor = Color::FromARGB(0, 0, 0, 0);
        void RenderBackground(HDC hdc);

        // --- Rendering ---
        virtual void Render(HDC hdc) {}; // Actual render logic
};