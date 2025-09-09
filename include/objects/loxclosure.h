//
// Created by Yue Xue  on 8/13/25.
//

#ifndef LOXCLOSURE_H
#define LOXCLOSURE_H

#include "loxfunction.h"
#include "loxcaptured.h"

class LoxClosure: public LoxObject {
public:

    explicit LoxClosure(const std::shared_ptr<LoxFunction> &the_function): function(the_function) {
    }

    ~LoxClosure() override {
        Runtime::record_free(*this);
    }

    void clear_reference() override;

    [[nodiscard]] std::string to_string() const override;

    std::string fun_name() const {
        return function->name;
    }

    LoxObjectType get_object_type_enum() const override {
        return LoxObjectType::Closure;
    }

    void mark_reference(std::queue<LoxReference> &queue) override;

    std::vector<std::shared_ptr<Captured>> captureds;
    std::shared_ptr<LoxFunction> function;

    size_t compute_size() override;

    std::shared_ptr<LoxType> get_type_ptr() const override;
};

#endif //LOXCLOSURE_H
