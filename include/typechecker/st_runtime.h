//
// Created by Yue Xue  on 9/7/25.
//

#ifndef CCLOX_ST_RUNTIME_H
#define CCLOX_ST_RUNTIME_H
#include "common.h"
#include "error.h"
#include "value.h"

class ST_Runtime {
public:
    explicit ST_Runtime() {}

    /**
     * 访问指定索引所对应的全局值
     */
    Value &access_global(uint16_t index) {
        return globals.at(index);
    }

    /**
     * 申明一个全局标识符。用在 AST 的构建时
     * @return  全局索引
     */
    uint16_t declare_global() {
        if (globals.size() == UINT16_MAX) {
            throw Uint16OperandOverflowError(fmt::format("cannot have more than {} global identifiers", UINT16_MAX));
        }
        globals.push_back(nullptr); // 占位符
        return globals.size() - 1;
    }

    /**
     * 定义一个全局标识符。 用在字节码生成时
     * @param index 之前申明时返回的那个索引
     * @param value 值
     */
    void define_global(uint16_t index, const Value &value) {
        globals.at(index) = value;
    }

    std::vector<Value> & get_globals() {
        return globals;
    }

private:
    std::vector<Value> globals;
};

extern ST_Runtime st_runtime;

#endif //CCLOX_ST_RUNTIME_H