//
// Created by Yue Xue  on 8/16/25.
//

#ifndef LOXCLASS_H
#define LOXCLASS_H
#include "object.h"
#include "cclox_util.h"
#include "objects/loxclosure.h"
#include "value.h"

class LoxClass: public LoxObject {
public:

    explicit LoxClass(const std::string &the_name, uint8_t the_num_fields): name(the_name), num_fields(the_num_fields) {

    }

    size_t compute_size() override {
        if (mem_size == 0) {
            mem_size = sizeof(LoxClass) + estimate_string_map_size(methods);
            return mem_size;
        } else {
            return mem_size;
        }
    }

    void clear_reference() override {

    }

    bool operator==(const LoxObject &other) const override {
        return this == &other;
    }

    [[nodiscard]] std::string to_string() const override {
        return fmt::format("<cls: {}>", name);
    }

    void add_method(std::shared_ptr<LoxClosure> method) {
        if (method->name() == "init") {
            constructor = method;
        }
        methods.insert({method->name(), method});
    }

private:
    std::unordered_map<std::string, std::shared_ptr<LoxClosure>> methods;
    std::shared_ptr<LoxClosure> constructor;
    size_t mem_size = 0;
    std::string name;
    uint8_t num_fields = 0;
};

#endif //LOXCLASS_H
