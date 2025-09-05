//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_CLASSTYPE_H
#define CCLOX_CLASSTYPE_H

#include "lox_type.h"
#include "scanner.h"

class ClassType : public LoxType {
public:
    ClassType(const Token &identifier) {
        // todo: 同名类型冲突
        static_assert(!std::is_abstract_v<ClassType>);
        type_id = resolve_type_id(identifier.get_lexeme());
        type_enum = LoxTypeEnum::Class;
    }

    bool accept(TypePtr other) const override {
        if (other->type_enum != LoxTypeEnum::Class) {
            return false;
        }
        std::shared_ptr<ClassType> other_class = std::static_pointer_cast<ClassType>(other);
        return type_id == other_class->type_id;
    }

private:
    std::string to_no_parenthesis_string() override {
        return read_typename_from_id(type_id);
    }
    size_t type_id;
};

#endif //CCLOX_CLASSTYPE_H