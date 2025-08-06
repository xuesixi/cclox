//
// Created by Yue Xue  on 8/6/25.
//

#include "scope.h"

void Scope::add_local(const Token &token) {
    if (locals.size() == UINT8_MAX) {
        throw ScopeLocalOverflowError(fmt::format("scope local overflow. You can have up to {} local variables in a scope", UINT8_MAX));
    }

    // 检查本层级内是否有重名的变量
    for (int i = locals.size() - 1; i >= 0; i --) {
        Local &local = locals.at(i);
        if (local.depth != depth) {
            // 重名检查只在本层级之内发生，一旦到了上一层级，就不用管了。
            break;
        }
        if (local.name == token.get_lexeme()) {
            throw SameNameLocalVariableError(fmt::format("local variables with the same name: {}", local.name));
        }
    }
    locals.emplace_back(token);
}

int Scope::resolve_local(const Token &token) {
    for (int i = locals.size() - 1; i >= 0; i --) {
        Local &local = locals.at(i);
        if (local.name == token.get_lexeme()) {
            if (local.isInitialized()) {
                return i;
            } else {
                throw UsingUninitializedLocalError("accessing a variable during its own initialization");
            }
        }
    }
    return -1;
}
