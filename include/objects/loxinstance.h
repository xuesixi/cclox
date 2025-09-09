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
        fields.clear();
        the_class = nullptr;
    }

    [[nodiscard]] std::string to_string() const override {
        if (the_class) {
            return fmt::format("<obj: {}>", the_class->get_name());
        } else {
            // 在gc的clear_reference之后，类指针可能为null
            return fmt::format("<obj: unknown>");
        }
    }

    std::shared_ptr<LoxClass> &get_class() {
        return the_class;
    }

    Value &get_field(uint8_t index) {
        return fields.at(index);
    }

    LoxObjectType get_object_type_enum() const override {
        return LoxObjectType::Instance;
    }

    void mark_reference(std::queue<LoxReference> &queue) override {
        for (auto & field : fields) {
            LoxValue::mark_value(field, queue);
        }
        mark(the_class, queue);
    }

private:
    std::vector<Value> fields;
    std::shared_ptr<LoxClass> the_class;
};

#endif //LOXINSTANCE_H
