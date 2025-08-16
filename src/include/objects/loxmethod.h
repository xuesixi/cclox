//
// Created by Yue Xue  on 8/16/25.
//

#ifndef LOXMETHOD_H
#define LOXMETHOD_H
#include "loxclosure.h"
#include "loxinstance.h"
#include "object.h"

class LoxMethod: LoxObject {
public:
    size_t compute_size() override;

    void clear_reference() override;

    bool operator==(const LoxObject &other) const override;

    [[nodiscard]] std::string to_string() const override;

private:
    std::shared_ptr<LoxClosure> closure_;
    std::shared_ptr<LoxInstance> receiver;
};

#endif //LOXMETHOD_H
