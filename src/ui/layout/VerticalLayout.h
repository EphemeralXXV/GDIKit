#include "Layout.h"

class VerticalLayout : public Layout {
    public:
        VerticalLayout(int spacingY = 0) : spacingY(spacingY) {}

        void Apply() override;

        void SetSpacing(int newSpacingY);  
        int GetSpacing() const { return spacingY; }

    private:
        int spacingY;
};