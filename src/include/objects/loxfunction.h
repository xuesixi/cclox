//
// Created by Yue Xue  on 8/9/25.
//

#ifndef LOXFUNCTION_H
#define LOXFUNCTION_H
#include "chunk.h"
#include "object.h"
#include "runtime.h"

enum class FunctionTypeEnum {
    Function,
    Method,
    Main,
    Lambda,
};

class LoxFunction: public LoxObject {

public:
    friend class LoxClosure;

    ~LoxFunction() override {
        Runtime::record_free(*this);
    }
    void clear_reference() override {
        chunk.constants.clear();
    };

    std::string to_string() const override {
        if (name == "<main>") {
            return "<proto: main>";
        }
        return fmt::format("<proto: {}>", name);
    }

    Chunk &get_chunk() {
        return chunk;
    }

    void set_name(const std::string &new_name) {
        name = new_name;
    }

    void incre_arity() {
        arity_ ++;
    }

    int arity() {
        return arity_;
    }

    LoxObjectType get_object_type() const override {
        return LoxObjectType::Function;
    }

    void mark_reference(std::queue<LoxReference> &queue) override {
        for (auto & constant : chunk.constants) {
            LoxValue::mark_value(constant, queue);
        }
    }

    size_t compute_size() override {
        if (memory_size == 0) {
            memory_size = sizeof(LoxFunction) + chunk.estimate_memory_size() + name.capacity();
            return memory_size;
        } else {
            return memory_size;
        }
    }

private:
    Chunk chunk;
    std::string name;
    int arity_ = 0 ;
    size_t memory_size = 0;
};

#endif //LOXFUNCTION_H
