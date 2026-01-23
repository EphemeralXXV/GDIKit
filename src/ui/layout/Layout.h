#pragma once

#include <windows.h>
#include "LayoutWidgetBridge.h"

class Container; // forward

class Layout {
    public:
        friend Container;
        
        virtual ~Layout() {}
        // Apply the layout to the container: position children using child's SetPosSize / GetLayoutWidth / GetLayoutHeight
        // To be determined by a specific implementation of the interface
        // Callers must pass an inner RECT, which is essentially the container minus padding (true usable area)
        virtual void Apply(const RECT& innerRect) = 0;

        virtual Container* GetContainer() const { return container; }

    protected:
        Container* container = nullptr;

        virtual void SetContainer(Container* newContainer) {
            container = newContainer;
        }

        // Proxy methods for derived layouts to use the LayoutWidgetBridge
        static void SetEffectiveRect(
            Widget& child,
            int l, int t, int r, int b
        ) {
            LayoutWidgetBridge::SetEffectiveRect(child, l, t, r, b);
        }
};