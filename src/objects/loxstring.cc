//
// Created by Yue Xue  on 9/9/25.
//

#include "objects/loxstring.h"
#include "typechecker/types/primitive_type.h"

std::shared_ptr<LoxType> LoxString::get_type_ptr() const {
    return PrimitiveType::StringType;
}
