//
// Created by Yue Xue  on 8/16/25.
//

#ifndef LOXINSTANCE_H
#define LOXINSTANCE_H

#include "loxclass.h"

class LoxInstance: public LoxObject {
public:
    explicit LoxInstance(std::shared_ptr<LoxClass> input_class, size_t num_filed):
        the_class(input_class),
        fields(num_filed, nullptr) {
        // 初始化所有字段为nil
    }

    ~LoxInstance() override {
        Runtime::record_free(*this);
    }

    size_t compute_size() override {
        return sizeof(LoxInstance) + sizeof(Value) * fields.capacity();
    }

    void clear_reference() override {

    }

    bool operator==(const LoxObject &other) const override {
        return this == &other;
    }

    [[nodiscard]] std::string to_string() const override {
        return fmt::format("<obj: {}>", the_class->get_name());
    }

    std::shared_ptr<LoxClass> &get_class() {
        return the_class;
    }

    Value &get_field(uint8_t index) {
        return fields.at(index);
    }

private:
    std::vector<Value> fields;
    std::shared_ptr<LoxClass> the_class;
};

#endif //LOXINSTANCE_H
