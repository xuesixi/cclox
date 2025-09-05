//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_UNION_TYPE_H
#define CCLOX_UNION_TYPE_H

#include "../lox_type.h"

class UnionType : public LoxType {
public:
    UnionType(std::vector<TypePtr> &&u) : unions(std::move(u)) {
        static_assert(!std::is_abstract_v<UnionType>);
        type_enum = LoxTypeEnum::Union;
    }

    /**
     * 本union是否覆盖了other的类型
     */
    bool accept(TypePtr other) const override{
        // 如果other是一个普通类型，那么一一比对
        if (other->type_enum != LoxTypeEnum::Union) {
            for (auto & type : unions) {
                if (type->accept(other)) {
                    return true;
                }
            }
            return false;
        }
        // 如果other也是一个union，则本union必须完全包括其所有元素
        auto other_union = std::static_pointer_cast<UnionType>(other);
        for (auto & type : other_union->unions) {
            if (accept(type) == false) {
                return false;
            }
        }
        return true;
    }

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

    std::vector<TypePtr> unions;
};

#endif //CCLOX_UNION_TYPE_H