#include "HorizontalLayout.h"
#include "Container.h"

void HorizontalLayout::SetSpacing(int newSpacingX) {
    spacingX = newSpacingX;
    Apply();
}

void HorizontalLayout::Apply() {
    if(!container) return;
    
    int x = 0;

    for(auto& child : container->Children()) {
        int w = child->GetLayoutWidth();
        int h = child->GetLayoutHeight();

        child->SetPosSize(x, 0, w, h);

        x += w + spacingX;
    }
}