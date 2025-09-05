//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_TUPLE_TYPE_H
#define CCLOX_TUPLE_TYPE_H

#include "lox_type.h"

class TupleType : public LoxType {
public:
    TupleType(std::vector<TypePtr> &&t) : sequence(t) {
        type_enum = LoxTypeEnum::Tuple;
    }

    std::vector<TypePtr> sequence;

private:
    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        ss << "(";
        for (size_t i = 0; i < sequence.size(); ++i) {
            ss << sequence.at(i)->to_string();
            if (i != sequence.size() - 1) {
                ss << ", ";
            }
        }
        ss << ")";
        return ss.str();
    }
};

#endif //CCLOX_TUPLE_TYPE_H