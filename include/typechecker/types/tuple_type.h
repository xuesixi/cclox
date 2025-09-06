//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_TUPLE_TYPE_H
#define CCLOX_TUPLE_TYPE_H

#include "typechecker/lox_type.h"

class TupleType : public LoxType {
public:
    TupleType(std::vector<TypePtr> &&t) : tuple(t) {
        static_assert(!std::is_abstract_v<TupleType>);
        type_enum = LoxTypeEnum::Tuple;
    }

    bool accept(TypePtr other) const override;

private:
    std::string to_no_parenthesis_string() override;

    std::vector<TypePtr> tuple;
};

#endif //CCLOX_TUPLE_TYPE_H