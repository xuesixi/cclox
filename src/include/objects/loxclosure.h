//
// Created by Yue Xue  on 8/13/25.
//

#ifndef LOXCLOSURE_H
#define LOXCLOSURE_H

#include "loxfunction.h"
#include "captured.h"

class LoxClosure: public LoxObject {
public:

    explicit LoxClosure(const std::shared_ptr<LoxFunction> &the_function): function(the_function) {
    }

    ~LoxClosure() override {
        Runtime::record_free(*this);
    }

    void clear_reference() override {};

    bool operator==(const LoxObject &other) const override {
        return this == &other;
    }

    [[nodiscard]] std::string to_string() const override {
        if (function->name == "<main>") {
            return "<main>";
        }
        return fmt::format("<fn: {}>", function->name);
    }

    std::vector<std::shared_ptr<Captured>> captureds;
    std::shared_ptr<LoxFunction> function;

private:
    size_t compute_size() override {
        return sizeof(LoxClosure) + sizeof(std::shared_ptr<Captured>) * captureds.capacity();
    }
};

#endif //LOXCLOSURE_H
