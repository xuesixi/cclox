//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_TYPECHECKER_H
#define CCLOX_TYPECHECKER_H

#include "typechecker/tokenholder.h"
#include "typechecker/lox_type.h"
#include "common.h"


class TypeParser {
public:
    explicit TypeParser(std::string &&src) {
        tokens = std::make_shared<TokenHolder>(std::move(src));
    }

    explicit TypeParser(std::shared_ptr<TokenHolder> &th) : tokens(th) {
    }

    TypePtr parse_type();

private:
    std::shared_ptr<TokenHolder> tokens;

    TypePtr parse_primary();

    TypePtr parse_union();

    TypePtr parse_intersection();

    TypePtr parse_tuple();

    TypePtr parse_array();

    TypePtr parse_function();
};

#endif //CCLOX_TYPECHECKER_H
