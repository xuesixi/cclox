//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_INTERSECTION_TYPE_H
#define CCLOX_INTERSECTION_TYPE_H

#include "lox_type.h"

class IntersectionType : public LoxType {
public:
    IntersectionType(std::vector<TypePtr> &&i) : intersections(std::move(i)) {
        static_assert(!std::is_abstract_v<IntersectionType>);
        type_enum = LoxTypeEnum::Intersection;
    }

    std::vector<TypePtr> intersections;

    bool is_subtype_of(TypePtr other) const override {
        answer_yes_to_any(other);
        auto other_inter = std::static_pointer_cast<IntersectionType>(other);
        // todo
    }

private:
    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        size_t i = 0;
        for (auto & type : intersections) {
            ss << type->to_string();
            if (i != intersections.size() - 1) {
                ss << " & ";
            }
            i ++;
        }
        return ss.str();
    }
    size_t cached_hash = 1;
};

#endif //CCLOX_INTERSECTION_TYPE_H