//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_TUPLE_TYPE_H
#define CCLOX_TUPLE_TYPE_H

#include "lox_type.h"

class TupleType : public LoxType {
public:
    TupleType(std::vector<TypePtr> &&t) : tuple(t) {
        static_assert(!std::is_abstract_v<TupleType>);
        type_enum = LoxTypeEnum::Tuple;
    }

    bool accept(TypePtr other) const override {
        if (other->type_enum != LoxTypeEnum::Tuple) {
            return false;
        }
        auto other_tuple = std::static_pointer_cast<TupleType>(other);
        if (other_tuple->tuple.size() != tuple.size()) {
            return false;
        }
        for (size_t i = 0; i < tuple.size(); i ++) {
            if (tuple.at(i)->accept(other_tuple->tuple.at(i)) == false) {
                return false;
            }
        }
        return true;
    }

private:
    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        ss << "(";
        for (size_t i = 0; i < tuple.size(); ++i) {
            ss << tuple.at(i)->to_string();
            if (i != tuple.size() - 1) {
                ss << ", ";
            }
        }
        ss << ")";
        return ss.str();
    }

    std::vector<TypePtr> tuple;
};

#endif //CCLOX_TUPLE_TYPE_H