//
// Created by Yue Xue  on 9/9/25.
//

#include "objects/loxfunction.h"
#include "typechecker/lox_type.h"
#include "typechecker/types/function_type.h"

void LoxFunction::clear_reference() {
    chunk.constants.clear();
}

std::string LoxFunction::to_string() const {
    if (name == "<main>") {
        return "<proto: main>";
    }
    return fmt::format("<proto: {}>", name);
}

void LoxFunction::set_type(const std::shared_ptr<FunctionType> &type) {
    this->type = type;
}

std::shared_ptr<LoxType> LoxFunction::get_type_ptr() const {
    return type;
}

void LoxFunction::mark_reference(std::queue<LoxReference> &queue) {
    for (auto &constant: chunk.constants) {
        LoxValue::mark_value(constant, queue);
    }
}

size_t LoxFunction::compute_size() {
    if (memory_size == 0) {
        memory_size = sizeof(LoxFunction) + chunk.estimate_memory_size() + name.capacity();
        return memory_size;
    } else {
        return memory_size;
    }
}
