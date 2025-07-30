//
// Created by Yue Xue  on 7/30/25.
//

#ifndef CCLOX_VALUE_H
#define CCLOX_VALUE_H
#include <variant>
#include <memory>

using Value = std::variant<long, double, bool>;

#endif //CCLOX_VALUE_H
