//
// Created by Yue Xue  on 9/4/25.
//

#ifndef CCLOX_CLASSTYPE_H
#define CCLOX_CLASSTYPE_H

#include "typechecker/lox_type.h"
#include "scanner.h"

class ClassType : public LoxType {
public:
    friend class GlobalNameResolver;
    ClassType(const std::string &class_name);

    bool accept(TypePtr other) const override;

    size_t get_type_id() const;

private:
    std::string to_no_parenthesis_string() override;

    size_t type_id;
};

#endif //CCLOX_CLASSTYPE_H