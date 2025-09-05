//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_UNION_TYPE_H
#define CCLOX_UNION_TYPE_H

#include "lox_type.h"

class UnionType : public LoxType {
public:
    UnionType(std::vector<TypePtr> &&u) : unions(std::move(u)) {
        static_assert(!std::is_abstract_v<UnionType>);
        type_enum = LoxTypeEnum::Union;
    }

    std::vector<TypePtr> unions;

    bool is_subtype_of(TypePtr other) const override;

private:
    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        size_t i = 0;
        for (auto & type : unions) {
            ss << type->to_string();
            if (i != unions.size() - 1) {
                ss << " | ";
            }
            i ++;
        }
        return ss.str();
    }

    size_t cached_hash = 1;
};

#endif //CCLOX_UNION_TYPE_H