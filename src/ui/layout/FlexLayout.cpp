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
    int totalGrowingItems = 0;          // Count of children that grow along the main axis
    int maxChildCrossLength = 0;        // Max cross length among children (for container auto-sizing)

    for(auto& child : children) {
        if(!child) continue;

        const Spacing& m = child->GetMargin();

        // Measure child main length + margins
        int childMainLength = ChildMainLength(
            child->GetLayoutWidth(),
            child->GetLayoutHeight()
        );
        int childMarginMain = ChildTotalMarginMain(m);

        // Count flex-grow items and accumulate fixed lengths
        if(child->IsFlexGrow()) {
            totalGrowingItems++;
            totalFixedMainLength += childMarginMain; // margins still count
        }
        else {
            totalFixedMainLength += childMainLength + childMarginMain;
        }

        // Track max cross length
        int childCrossLength = ChildCrossLength(child->GetLayoutWidth(), child->GetLayoutHeight());
        childCrossLength += ChildTotalMarginCross(m);
        maxChildCrossLength = std::max(maxChildCrossLength, childCrossLength);
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

    // --- PASS 3: Adjust container size if auto-sizing ---
    RECT effectiveRect = container->EffectiveRect();
    Spacing padding = container->GetPadding();
    Border border = container->GetBorder();
    RECT finalRect = effectiveRect;

    // Auto stretch container to content if auto sizing
    bool autoSizeMain = MainIsAuto(
        container->IsAutoWidth(),
        container->IsAutoHeight()
    );
    bool autoSizeCross = CrossIsAuto(
        container->IsAutoWidth(),
        container->IsAutoHeight()
    );

    if(autoSizeMain) {
        int mainStartPos = MainStart(innerRect);    // where the main axis starts
        int mainLength = cursor - mainStartPos;     // total length of children + spacing
        if(direction == FlexDirection::Row) {
            finalRect.left = mainStartPos;
            finalRect.right = finalRect.left + mainLength + padding.left + padding.right + border.left.thickness + border.right.thickness;
        }
        else {
            finalRect.top = mainStartPos;
            finalRect.bottom = finalRect.top + mainLength + padding.top + padding.bottom + border.top.thickness + border.bottom.thickness;
        }
    }
    if(autoSizeCross) {
        if(direction == FlexDirection::Row) {
            finalRect.bottom = effectiveRect.top + border.top.thickness + padding.top + maxChildCrossLength + padding.bottom + border.bottom.thickness;
        } 
        else {
            finalRect.right = effectiveRect.left + border.left.thickness + padding.left + maxChildCrossLength + padding.right + border.right.thickness;
        }
    }

    SetEffectiveRect(
        *container,
        finalRect.left,
        finalRect.top,
        finalRect.right,
        finalRect.bottom
    );

    SetLayoutSize(
        *container,
        finalRect.right - finalRect.left,
        finalRect.bottom - finalRect.top
    );
}