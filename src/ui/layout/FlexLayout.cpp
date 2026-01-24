#include "FlexLayout.h"
#include "Container.h"
#include "LayoutWidgetBridge.h"
#include "string"

void FlexLayout::SetSpacing(int newSpacing) {
    if(spacing == newSpacing) return;
    
    spacing = newSpacing;
    
    if(container) {
        // Recompute effective geometry with new spacing
        container->InvalidateLayout();
    }
}

void FlexLayout::Apply(const RECT& innerRect) {
    if(!container) return;

    auto children = container->Children();

    int containerWidth = innerRect.right - innerRect.left;
    int containerHeight = innerRect.bottom - innerRect.top;

    int containerMainLength = MainLength(
        containerWidth,
        containerHeight
    );

    int containerCrossLength = CrossLength(
        containerWidth,
        containerHeight
    );

    // --- PASS 1: Measure ---
    int totalFixedMainLength = 0;       // Sum of all fixed main lengths + margins
    int totalGrowingItems = 0;          // Count of children that grow

    for(auto& child : children) {
        if(!child) continue;

        const Spacing& m = child->GetMargin();

        int childMainLength = ChildMainLength(
            child->GetLayoutWidth(),
            child->GetLayoutHeight()
        );
        int childMarginMain = ChildTotalMarginMain(m);

        if(child->IsFlexGrow()) {
            totalGrowingItems++;
            totalFixedMainLength += childMarginMain; // margins still count
        }
        else {
            totalFixedMainLength += childMainLength + childMarginMain;
        }
    }

    int totalSpacing = spacing * std::max(0, int(children.size() - 1));
    int remainingLength = std::max(0, containerMainLength - totalFixedMainLength - totalSpacing);
    int remainingGrowingItems = totalGrowingItems;

    // --- PASS 2: Assign positions ---
    int cursor = MainStart(innerRect);
    int effectiveSpacing = spacing; // Set spacing + extra computed justify spacing

    switch(justify) {
        case JustifyContent::Start:
            break;
        case JustifyContent::Center:
            cursor += remainingLength / 2;
            break;
        case JustifyContent::End:
            cursor += remainingLength;
            break;
        case JustifyContent::SpaceBetween:
            if(children.size() > 1)
                effectiveSpacing += remainingLength / (children.size() - 1);
            break;
        case JustifyContent::SpaceAround:
            effectiveSpacing += remainingLength / children.size();
            cursor += effectiveSpacing / 2;
            break;
        case JustifyContent::SpaceEvenly:
            effectiveSpacing += remainingLength / (children.size() + 1);
            cursor += effectiveSpacing;
            break;
    }

    for(size_t i = 0; i < children.size(); i++) {
        auto& child = children[i];
        if(!child) continue;

        const Spacing& m = child->GetMargin();

        int childMainLength = ChildMainLength(
            child->GetLayoutWidth(),
            child->GetLayoutHeight()
        );
        int childCrossLength = ChildCrossLength(
            child->GetLayoutWidth(),
            child->GetLayoutHeight()
        );

        int marginMainStart         = ChildMarginMainStart(m);
        int marginMainEnd           = ChildMarginMainEnd(m);
        int marginCrossStart        = ChildMarginCrossStart(m);
        int marginCrossEnd          = ChildMarginCrossEnd(m);

        // Flex-grow children get proportional width
        if(child->IsFlexGrow() && remainingGrowingItems > 0) {
            childMainLength = remainingLength / remainingGrowingItems;
            remainingGrowingItems--;            // reduce remaining flex units
            remainingLength -= childMainLength; // reduce remaining space
        }

        // Compute final cross axis length
        int finalCrossLength = childCrossLength;

        // Make & set final effective rect for the child
        int mainPos = cursor + marginMainStart;
        int crossPos;
        switch(align) {
            case AlignItems::Start:
                crossPos = CrossStart(innerRect) + marginCrossStart;
                break;
            case AlignItems::Center:
                crossPos = CrossStart(innerRect) + (containerCrossLength - finalCrossLength - marginCrossEnd - marginCrossStart) / 2 + marginCrossStart;
                break;
            case AlignItems::End:
                crossPos = CrossEnd(innerRect) - finalCrossLength - marginCrossEnd;
                break;
            case AlignItems::Stretch:
                crossPos = CrossStart(innerRect) + marginCrossStart;
                finalCrossLength = std::max(0, containerCrossLength - marginCrossStart - marginCrossEnd);
                break;
        }
        RECT r = MakeRect(
            mainPos,
            crossPos,
            childMainLength,
            finalCrossLength
        );
        SetEffectiveRect(*child, r.left, r.top, r.right, r.bottom);
               
        // Advance cursor to the next child
        cursor += childMainLength + marginMainStart + marginMainEnd;
        if(i + 1 < children.size()) {
            // Don't add spacing after last child
            cursor += effectiveSpacing;
        }
    }

    if(
        // Auto stretch container to content if main length is 0 (auto)
        MainIsAuto(container->IsAutoWidth(), container->IsAutoHeight()) &&
        // But only if at least one child has fixed size
        totalGrowingItems < children.size()
    ) {
        Spacing padding = container->GetPadding();
        Border border = container->GetBorder();
        RECT effectiveRect = container->EffectiveRect();

        if(direction == FlexDirection::Row) {
            SetEffectiveRect(
                *container,
                effectiveRect.left,
                effectiveRect.top,
                cursor + padding.right + border.right.thickness,
                effectiveRect.bottom
            );
        }
        else {
            SetEffectiveRect(
                *container,
                effectiveRect.left,
                effectiveRect.top,
                effectiveRect.right,
                cursor + padding.bottom + border.bottom.thickness
            );
        }
    }
}