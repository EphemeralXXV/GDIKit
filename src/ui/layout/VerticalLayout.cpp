#include "VerticalLayout.h"
#include "Container.h"

void VerticalLayout::SetSpacing(int newSpacingY) {
    spacingY = newSpacingY;
    Apply();
}

void VerticalLayout::Apply() {
    if(!container) return;
    
    int y = 0;

    for(auto& child : container->Children()) {
        int w = child->GetLayoutWidth();
        int h = child->GetLayoutHeight();

        child->SetPosSize(0, y, w, h);
        
        y += h + spacingY;
    }
}