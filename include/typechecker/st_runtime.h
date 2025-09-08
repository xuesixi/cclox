//
// Created by Yue Xue  on 9/7/25.
//

#ifndef CCLOX_ST_RUNTIME_H
#define CCLOX_ST_RUNTIME_H
#include "common.h"
#include "value.h"

class ST_Runtime {
public:
    explicit ST_Runtime() {}

    Value &access_global(uint16_t index) {
        return globals.at(index);
    }

    void define_global(const Value &value) {
        globals.push_back(value);
    }

private:
    std::vector<Value> globals;
};

extern ST_Runtime st_runtime;

#endif //CCLOX_ST_RUNTIME_H