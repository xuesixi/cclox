//
// Created by Yue Xue  on 8/16/25.
//

#ifndef LOXMETHOD_H
#define LOXMETHOD_H
#include "loxclosure.h"
#include "loxinstance.h"
#include "object.h"

class LoxMethod: public LoxObject {
public:

    explicit LoxMethod(const std::shared_ptr<LoxClosure> &cl, const std::shared_ptr<LoxInstance> &rc): closure_(cl), receiver_(rc) {

    }

    ~LoxMethod() override {
        Runtime::record_free(*this);
    }

    size_t compute_size() override {
        return sizeof(LoxMethod);
    }

    void clear_reference() override {

    }

    [[nodiscard]] std::string to_string() const override {
        return fmt::format("<mthd: {}>", closure_->fun_name());
    }

    std::shared_ptr<LoxClosure> & closure() {
        return closure_;
    }

    std::shared_ptr<LoxInstance> &receiver() {
        return receiver_;
    }

    LoxObjectType get_object_type() const override {
        return LoxObjectType::Method;
    }

private:
    std::shared_ptr<LoxClosure> closure_;
    std::shared_ptr<LoxInstance> receiver_;
};

#endif //LOXMETHOD_H
