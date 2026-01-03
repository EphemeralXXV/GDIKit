#include "Layout.h"

class HorizontalLayout : public Layout {
    public:
        HorizontalLayout(int spacingX = 0) : spacingX(spacingX) {}

        void Apply() override;

        void SetSpacing(int newSpacingX); 
        int GetSpacing() const { return spacingX; }

    private:
        int spacingX;
};