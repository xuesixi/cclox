//
// Created by Yue Xue  on 8/13/25.
//

#ifndef LOXCLOSURE_H
#define LOXCLOSURE_H

#include "loxfunction.h"
#include "captured.h"
#include "common.h"

class LoxClosure: public LoxObject {
public:

    explicit LoxClosure(const std::shared_ptr<LoxFunction> &function): function_(function) {
    }

    ~LoxClosure() override {
        auto old = runtime.allocated_size.fetch_sub( LoxClosure::get_size());
        if (Flag::show_heap) {
            std::cout << fmt::format("[-] heap: {:^6} -> {:^6}; {}\n", old, old - LoxClosure::get_size(), LoxClosure::to_string());
        }
    }

    void clear_reference() override {};

    bool operator==(const LoxObject &other) const override {
        return this == &other;
    }

    [[nodiscard]] std::string to_string() const override {
        if (function_->name == "<main>") {
            return "<main>";
        }
        return fmt::format("<fn: {}>", function_->name);
    }

    std::vector<std::shared_ptr<Captured>> captureds_;
    std::shared_ptr<LoxFunction> function_;

private:
    size_t get_size() override {
        return sizeof(LoxClosure) + sizeof(std::shared_ptr<Captured>) * captureds_.capacity();
    }
};

#endif //LOXCLOSURE_H
