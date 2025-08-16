//
// Created by Yue Xue  on 8/16/25.
//

#ifndef LOXINSTANCE_H
#define LOXINSTANCE_H

#include "loxclass.h"

class LoxInstance: public LoxObject {
public:
    size_t compute_size() override;

    void clear_reference() override;

    bool operator==(const LoxObject &other) const override;

    [[nodiscard]] std::string to_string() const override;

private:
    std::vector<Value> fields;
    std::shared_ptr<LoxClass> the_class;
};

#endif //LOXINSTANCE_H
