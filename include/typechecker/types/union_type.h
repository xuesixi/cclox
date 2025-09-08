//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_UNION_TYPE_H
#define CCLOX_UNION_TYPE_H

#include "typechecker/lox_type.h"

class UnionType : public LoxType {
public:
    UnionType(std::vector<TypePtr> &&u) : unions(std::move(u)) {
        static_assert(!std::is_abstract_v<UnionType>);
        type_enum = LoxTypeEnum::Union;
    }

    /**
     * 本union是否覆盖了other的类型
     */
    bool accept(TypePtr other) const override;

    bool contains_nil() const;

private:
    std::string to_no_parenthesis_string() override;

    std::vector<TypePtr> unions;
};

#endif //CCLOX_UNION_TYPE_H