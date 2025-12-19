#pragma once

class Container; // forward

class Layout {
public:
    friend Container;
    
    virtual ~Layout() {}
    // Apply the layout to the container: position children using child's SetPosSize / GetLayoutWidth / GetLayoutHeight
    // To be determined by a specific implementation of the interface
    virtual void Apply() = 0;

    virtual Container* GetContainer() const { return container; }

protected:
    Container* container = nullptr;

    virtual void SetContainer(Container* newContainer) {
        container = newContainer;
    }
};