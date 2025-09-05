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

    /**
     * 判断other是否是自身的子集
     */
    bool accept(TypePtr other) const override {
        // 如果是普通类型，检查是否全部符合
        if (other->type_enum != LoxTypeEnum::Intersection) {
            for (auto & type : intersections) {
                if (type->accept(other) == false) {
                    return false;
                }
            }
            return true;
        }
        // 如果也是交集，则检查other是否包含了自身的所有元素
        auto other_inter = std::static_pointer_cast<IntersectionType>(other);
        for (auto & type : intersections) {
            if (other_inter->accept(type) == false) {
                return false;
            }
        }
        return true;
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
    std::vector<TypePtr> intersections;
};

#endif //CCLOX_INTERSECTION_TYPE_H