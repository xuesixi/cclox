//
// Created by Yue Xue  on 8/6/25.
//

#ifndef CCLOX_SCOPE_H
#define CCLOX_SCOPE_H
#include "common.h"
#include "error.h"
#include "scanner.h"

class Scope {
public:
    /**
     * 进入一个新的层级，自增depth，并返回此时的本地变量的数量
     */
    uint8_t step_into() {
        depth++;
        return locals.size();
    }

    /**
     * 离开一个层级。自减depth，并将本地变量的数量从缩减至clear_to。返回缩减的数量
     */
    uint8_t step_out(uint8_t clear_to) {
        DEBUG_ASSERT(clear_to <= locals.size(), "clear_to should not be greater than locals.size()");
        uint8_t diff = locals.size() - clear_to;
        locals.resize(clear_to);
        depth--;
        return diff;
    }

    size_t locals_size() const {
        return locals.size();
    }

    /**
     * 是否是全局scope（depth==0）
     */
    bool is_global_scope() {
        return depth == 0;
    }

    /**
     * 初始化最新的那个本地变量
     */
    void initialize() {
        DEBUG_ASSERT(locals.size() > 0, "the size should not be 0");
        locals.back().depth = depth;
    }

    /**
     * 将一个token对应的本地变量添加到scope中。如果同层级中存在同名变量，则抛出异常。
     * 添加后，该本地变量尚未被初始化。
     */
    void add_local(const Token &token);

    /**
     * 在scope中查找对应token对应的同名变量，如果没找到，返回-1。
     * 如果找到了，但没有被初始化，抛出异常；否则返回其对应的索引
     */
    int resolve_local(const Token &token);

private:
    struct Local {
        Local(): name(), depth(-1) {
        }

        Local(const Token &token): name(token.get_lexeme()), depth(-1) {
        }

        bool isInitialized() {
            return depth != -1;
        }

        std::string name;
        int depth;
    };

    std::vector<Local> locals;
    int depth = 0;
};
#endif //CCLOX_SCOPE_H
