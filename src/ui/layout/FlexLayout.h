#include "Layout.h"

enum class FlexDirection {
    Row,
    Column
};

// Direction-specific geomertry
struct AxisGeometry {
    int mainStart;      // Main axis origin (x for Row, y for Column)
    int crossStart;     // Cross axis origin (y for Row, x for Column)
    int mainLength;     // Main axis length (width for Row, height for Column)
    int crossLength;    // Cross axis length (height for Row, width for Column)
};

// Distribution along layout direction
enum class JustifyContent {
    Start,
    Center,
    End,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

// Placement perpendicular to layout direction
enum class AlignItems {
    Start,
    Center,
    End,
    Stretch
};

class FlexLayout : public Layout {
    public:
        // Constructor
        explicit FlexLayout(FlexDirection direction, int spacing = 0) {
            this->direction = direction;
            this->spacing = spacing;
        };

        // Internal updates
        void Apply(const RECT& innerRect) override;

        // Direction
        FlexDirection GetDirection() const { return direction; }
        void SetDirection(FlexDirection newDirection) { direction = newDirection; }

        // Spacing (gaps)
        int GetSpacing() const { return spacing; }
        void SetSpacing(int gap);  

        // Content distribution/alignment
        JustifyContent GetJustify() const { return justify; }
        void SetJustify(JustifyContent newJustify) { justify = newJustify; }

        AlignItems GetAlign() const { return align; }
        void SetAlign(AlignItems newAlign) { align = newAlign; }

    private:
        FlexDirection direction;
        int spacing = 0;

        JustifyContent justify = JustifyContent::Start;
        AlignItems align = AlignItems::Start;

        // Helper functions
        int MainStart(const RECT& r) {
            return direction == FlexDirection::Row ? r.left : r.top;
        }
        int CrossStart(const RECT& r) {
            return direction == FlexDirection::Row ? r.top : r.left;
        }
        int CrossEnd(const RECT& r) {
            return direction == FlexDirection::Row ? r.bottom : r.right;
        }
        int MainLength(int w, int h) {
            return direction == FlexDirection::Row ? w : h;
        }
        int CrossLength(int w, int h) {
            return direction == FlexDirection::Row ? h : w;
        }
        bool MainIsAuto(bool isAutoWidth, bool isAutoHeight) {
            return direction == FlexDirection::Row ? isAutoWidth : isAutoHeight;
        }
        bool CrossIsAuto(bool isAutoWidth, bool isAutoHeight) {
            return direction == FlexDirection::Row ? isAutoHeight : isAutoWidth;
        }

        int ChildMainLength(int preferredWidth, int preferredHeight) {
            return direction == FlexDirection::Row ? preferredWidth : preferredHeight;
        }
        int ChildCrossLength(int preferredWidth, int preferredHeight) {
            return direction == FlexDirection::Row ? preferredHeight : preferredWidth;
        }
        int ChildTotalMarginMain(Spacing margin) {
            return direction == FlexDirection::Row ? margin.left + margin.right : margin.top + margin.bottom;
        }
        int ChildTotalMarginCross(Spacing margin) {
            return direction == FlexDirection::Row ? margin.top + margin.bottom : margin.left + margin.right;
        }
        int ChildMarginMainStart(Spacing margin) {
            return direction == FlexDirection::Row ? margin.left : margin.top;
        }
        int ChildMarginMainEnd(Spacing margin) {
            return direction == FlexDirection::Row ? margin.right : margin.bottom;
        }
        int ChildMarginCrossStart(Spacing margin) {
            return direction == FlexDirection::Row ? margin.top : margin.left;
        }
        int ChildMarginCrossEnd(Spacing margin) {
            return direction == FlexDirection::Row ? margin.bottom : margin.right;
        }
        
        RECT MakeRect(
            int mainStart,
            int crossStart,
            int mainLength,
            int crossLength
        ) {
            if(direction == FlexDirection::Row) {
                return {
                    mainStart,
                    crossStart,
                    mainStart + mainLength,
                    crossStart + crossLength
                };
            }
            else {
                return {
                    crossStart,
                    mainStart,
                    crossStart + crossLength,
                    mainStart + mainLength
                };
            }
        }

};

// Convenience wrappers for lighter client code
class HorizontalLayout : public FlexLayout {
    public:
        HorizontalLayout(int spacing = 0) : FlexLayout(FlexDirection::Row, spacing) {}
};

class VerticalLayout : public FlexLayout {
    public:
        VerticalLayout(int spacing = 0) : FlexLayout(FlexDirection::Column, spacing) {}
};