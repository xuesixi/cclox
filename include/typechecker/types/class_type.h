//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_CLASSTYPE_H
#define CCLOX_CLASSTYPE_H

#include "typechecker/lox_type.h"
#include "scanner.h"

class ClassType : public LoxType {
public:
    ClassType(const Token &identifier) {
        // todo: 同名类型冲突
        static_assert(!std::is_abstract_v<ClassType>);
        type_id = resolve_type_id(identifier.get_lexeme());
        type_enum = LoxTypeEnum::Class;
    }

    bool accept(TypePtr other) const override;

    size_t get_type_id() const;

private:
    std::string to_no_parenthesis_string() override;

    size_t type_id;
};

#endif //CCLOX_CLASSTYPE_H