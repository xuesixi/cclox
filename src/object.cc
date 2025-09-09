//
// Created by Yue Xue  on 8/3/25.
//

#include "object.h"

#include "value.h"
#include "objects/loxfunction.h"
#include "typechecker/lox_type.h"
#include "typechecker/types/primitive_type.h"


std::shared_ptr<LoxType> LoxObject::get_type_ptr() const {
    return PrimitiveType::UnspecifiedType;
}


