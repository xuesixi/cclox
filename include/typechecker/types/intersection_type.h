//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_INTERSECTION_TYPE_H
#define CCLOX_INTERSECTION_TYPE_H

#include "typechecker/lox_type.h"

class IntersectionType : public LoxType {
public:
    IntersectionType(std::vector<TypePtr> &&i) : intersections(std::move(i)) {
        static_assert(!std::is_abstract_v<IntersectionType>);
        type_enum = LoxTypeEnum::Intersection;
    }

    /**
     * 判断other是否是自身的子集
     */
    bool accept(TypePtr other) const override;

private:
    std::string to_no_parenthesis_string() override;

    std::vector<TypePtr> intersections;
};

#endif //CCLOX_INTERSECTION_TYPE_H