//
// Created by Yue Xue  on 8/17/25.
//

#ifndef LOXNATIVE_H
#define LOXNATIVE_H
#include "object.h"
#include "value.h"
#include "runtime.h"

class VM;

class LoxNative: public LoxObject {
public:
    /**
     * 该函数没有返回值。如果逻辑上有返回值，直接在函数内部置入栈中即可
     * 该函数的调用时，fp位置是LoxNative。该函数会负责弹出它。
     */
    typedef void (*NativeImpl)(VM &vm, size_t fp);

    explicit LoxNative(NativeImpl impl, const std::string &name, int arity): impl_(impl), name_(name), arity_(arity) {

    }

    ~LoxNative() override {
        Runtime::record_free(*this);
    }

    size_t compute_size() override {
        return sizeof(LoxNative) + name_.capacity();
    }

    void clear_reference() override {

    }

    [[nodiscard]] std::string to_string() const override {
        return fmt::format("<native: {}>", name_);
    }

    std::string get_name() const {
        return name_;
    }

    NativeImpl get_impl() const {
        return impl_;
    }

    int get_arity() const {
        return arity_;
    }

    LoxObjectType get_object_type() const override {
        return LoxObjectType::Native;
    }

private:
    const std::string name_;
    const NativeImpl impl_;
    const int arity_;
};

#endif //LOXNATIVE_H
