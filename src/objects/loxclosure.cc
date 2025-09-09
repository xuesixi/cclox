//
// Created by Yue Xue  on 9/9/25.
//

#include "objects/loxclosure.h"
#include "typechecker/types/function_type.h"

void LoxClosure::clear_reference() {
    std::cout << "closure cleared\n";
    captureds.clear();
    function = nullptr;
}

std::string LoxClosure::to_string() const {
    if (function->name == "<main>") {
        return "<main>";
    }
    return fmt::format("<fn: {}>", function->name);
}

void LoxClosure::mark_reference(std::queue<LoxReference> &queue) {
    mark(function, queue);
    for (auto & captured : captureds) {
        mark(captured, queue);
    }
    // todo: 捕获值的gc
}

size_t LoxClosure::compute_size() {
    return sizeof(LoxClosure) + sizeof(std::shared_ptr<Captured>) * captureds.capacity();
}

std::shared_ptr<LoxType> LoxClosure::get_type_ptr() const {
    return function->type;
}
