//
// Created by Yue Xue  on 8/9/25.
//

#ifndef LOXFUNCTION_H
#define LOXFUNCTION_H
#include "chunk.h"
#include "object.h"

enum class FunctionType {
    Function,
    Main,
};

class LoxFunction: public LoxObject {

public:
    friend class LoxClosure;
    ~LoxFunction() override = default;
    void clear_reference() override {};

    bool operator==(const LoxObject &other) const override {
        return this == &other;
    }

    std::string to_string() const override {
        if (name_ == "<main>") {
            return "<main>";
        }
        return fmt::format("<proto: {}>", name_);
    }

    Chunk &get_chunk() {
        return chunk_;
    }

    void set_name(const std::string &name) {
        name_ = name;
    }

    void incre_arity() {
        arity_ ++;
    }
    int arity() {
        return arity_;
    }

private:
    Chunk chunk_;
    std::string name_;
    int arity_ = 0 ;
};

#endif //LOXFUNCTION_H
