#include "Root.h"

std::shared_ptr<Root> Root::instance;

Root::Root(int width, int height) {
    rect = {0, 0, width, height};

    UpdateConvenienceGeometry();
}

std::shared_ptr<Root> Root::Create(int width, int height) {
    auto existing = instance;
    if(existing) {
        throw std::runtime_error("Root already exists!");
    }

    auto root = std::shared_ptr<Root>(new Root(width, height));
    instance = root;
    return root;
}

std::shared_ptr<Root> Root::Get() {
    if(auto inst = instance) {
        return inst;
    }
    throw std::runtime_error("Root not created yet! Call Root::Create() first.");
}