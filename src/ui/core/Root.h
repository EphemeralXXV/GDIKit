#pragma once

#include <stdexcept>

#include "Container.h"

class Root : public Container {
    public:
        // Deleted copy/move constructors to enforce singleton
        Root(const Root&) = delete;
        Root& operator=(const Root&) = delete;
        Root(Root&&) = delete;
        Root& operator=(Root&&) = delete;

        // Factory to create/get the single Root instance
        static std::shared_ptr<Root> Create(int width, int height);

        // Getter for the singleton (throws if not yet created)
        static std::shared_ptr<Root> Get();

        // Root never has a parent
        void SetParent(Widget*) = delete;

    private:
        static std::shared_ptr<Root> instance;
        explicit Root(int width, int height);
};