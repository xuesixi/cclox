//
// Created by Yue Xue  on 8/4/25.
//

#ifndef CCLOX_RUNTIME_H
#define CCLOX_RUNTIME_H
#include <memory>
#include <unordered_map>

class LoxObject;

class Runtime {
public:
    void register_object(long id, const std::shared_ptr<LoxObject> &ptr) {
        weak_pool[id] = std::weak_ptr(ptr);
    }

private:
    std::unordered_map<long, std::weak_ptr<LoxObject>> weak_pool;
};

extern Runtime runtime;

#endif //CCLOX_RUNTIME_H
