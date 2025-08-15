//
// Created by Yue Xue  on 8/4/25.
//

#ifndef CCLOX_RUNTIME_H
#define CCLOX_RUNTIME_H
#include <memory>
#include <vector>

class LoxObject;

class Runtime {
public:
    void register_object(const std::shared_ptr<LoxObject> &ptr) {
        weak_pool.push_back(std::weak_ptr{ptr});
    }

    std::atomic<size_t> allocated_size = 0;

private:
    std::vector<std::weak_ptr<LoxObject>> weak_pool;
};

extern Runtime runtime;

#endif //CCLOX_RUNTIME_H
