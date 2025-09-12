//
// Created by Yue Xue  on 8/9/25.
//

#ifndef LOXFUNCTION_H
#define LOXFUNCTION_H
#include "chunk.h"
#include "object.h"
#include "runtime.h"

class FunctionType;

enum class FunctionTypeEnum {
    None,
    Function,
    Method,
    Main,
    Lambda,
};

class LoxFunction : public LoxObject {
public:
    friend class LoxClosure;

    LoxFunction() = default;

    ~LoxFunction() override {
        Runtime::record_free(*this);
    }

    void clear_reference() override;;

    std::string to_string() const override;

    Chunk &get_chunk() {
        return chunk;
    }

    void set_name(const std::string &new_name) {
        name = new_name;
    }

    void set_type(const std::shared_ptr<FunctionType> &type);

    const std::shared_ptr<FunctionType> & get_type() const {
        return this->type;
    }

    std::shared_ptr<LoxType> get_type_ptr() const override;

    void incre_arity() {
        arity_++;
    }

    int arity() {
        return arity_;
    }

    LoxObjectType get_object_type_enum() const override {
        return LoxObjectType::Function;
    }

    void mark_reference(std::queue<LoxReference> &queue) override;

    size_t compute_size() override;

private:
    Chunk chunk;
    std::string name;
    int arity_ = 0;
    size_t memory_size = 0;
    std::shared_ptr<FunctionType> type;
};

#endif //LOXFUNCTION_H
