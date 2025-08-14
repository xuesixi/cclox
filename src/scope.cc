//
// Created by Yue Xue  on 8/6/25.
//

#include "scope.h"

void Scope::add_local(const Token &token) {
    if (locals.size() == UINT8_MAX) {
        throw ScopeLocalOverflowError(
            fmt::format("scope local overflow. You can have up to {} local variables in a scope", UINT8_MAX));
    }

    // 检查本层级内是否有重名的变量
    for (int i = locals.size() - 1; i >= 0; i--) {
        Local &local = locals.at(i);
        if (local.depth != depth) {
            // 重名检查只在本层级之内发生，一旦到了上一层级，就不用管了。
            break;
        }
        if (local.name == token.get_lexeme()) {
            throw SameNameLocalVariableError(fmt::format("local variables with the same name: {}", local.name));
        }
    }
    locals.push_back({token.get_lexeme(), -1});
}

std::optional<uint8_t> Scope::resolve_local(const Token &token) {
    DEBUG_ASSERT(within<uint8_t>(locals_size()-1), "local size overflow");
    for (uint8_t i = locals.size() - 1; ; i--) {
        Local &local = locals.at(i);
        if (local.name == token.get_lexeme()) {
            if (local.is_initialized()) {
                return i;
            } else {
                throw UsingUninitializedLocalError("accessing a variable during its own initialization");
            }
        }
        if (i == 0) {
            return std::nullopt;
        }
    }
}

std::optional<uint8_t> Scope::resolve_upvalue(const Token &token) {

    // 先判断是否已经被捕获，如果已经被捕获，那么直接返回对应索引即可
    for (uint8_t i = 0; i < upvalues.size(); i++) {
        if (upvalues.at(i).name == token.get_lexeme()) {
            return i;
        }
    }

    // 如果已经是最外层，那么返回nullopt。这也是递归的base case
    if (!outer_) {
        return std::nullopt;
    }

    // 尝试寻找外层的本地变量
    auto found = outer_->resolve_local(token);
    if (found.has_value()) {
        Upvalue new_upvalue {true, found.value(),  token.get_lexeme()};
        upvalues.push_back(std::move(new_upvalue));
        return upvalues.size() - 1;
    }

    // 递归调用，尝试在外层的upvalues中寻找/捕获

    found = outer_->resolve_upvalue(token);
    if (found.has_value()) {
        Upvalue new_upvalue {false, found.value(),  token.get_lexeme()};
        upvalues.push_back(std::move(new_upvalue));
        return upvalues.size() - 1;
    }

    // 如果没有找到，说明没有
    return std::nullopt;
}
