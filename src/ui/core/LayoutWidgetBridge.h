#pragma once

#include "Widget.h"

// Dedicated proxy for accessing non-public parts of Widget from Layout
class LayoutWidgetBridge {
    public:
        friend class Layout;
        
    protected:
        static void SetEffectiveRect(
            Widget& w,
            int l, int t, int r, int b
        ) {
            w.SetEffectiveRect(l, t, r, b);
        }
};