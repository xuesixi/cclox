//
// Created by Yue Xue  on 8/13/25.
//

#ifndef LOXCLOSURE_H
#define LOXCLOSURE_H

#include "loxfunction.h"
#include "captured.h"

class LoxClosure: public LoxObject {
public:

    explicit LoxClosure(const std::shared_ptr<LoxFunction> &function): function_(function) {

    }

    ~LoxClosure() override = default;

    void clear_reference() override {};

    bool operator==(const LoxObject &other) const override {
        return this == &other;
    }

    [[nodiscard]] std::string to_string() const override {
        if (function_->name_ == "<main>") {
            return "<main>";
        }
        return fmt::format("<fn: {}>", function_->name_);
    }

    std::vector<std::shared_ptr<Captured>> captureds_;
    std::shared_ptr<LoxFunction> function_;
};

#endif //LOXCLOSURE_H
