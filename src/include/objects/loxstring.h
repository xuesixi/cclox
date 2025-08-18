//
// Created by Yue Xue  on 8/4/25.
//

#ifndef CCLOX_LOXSTRING_H
#define CCLOX_LOXSTRING_H

#include "object.h"
#include "runtime.h"


class LoxString : public LoxObject {
public:
    explicit LoxString(const std::string &content) : LoxObject(), str(content) {};

    explicit LoxString(std::string &&content) : LoxObject(), str(std::move(content)) {};

    ~LoxString() override {
        Runtime::record_free(*this);
    }

    void clear_reference() override {};

    bool operator==(const LoxObject &other) const override {
        auto s = dynamic_cast<const LoxString*>(&other);
        if (s == nullptr) {
            return false;
        }
        return str == s->str;
    }

    bool operator==(const LoxString &other) const {
        return str == other.str;
    }

    std::string operator+(const LoxString &right) const {
        return str + right.str;
    }

    [[nodiscard]] std::string to_string() const override {
        return str;
    }

    std::string to_visual_string() const override {
        return fmt::format("\"{}\"", str);
    }

    LoxObjectType get_object_type() const override {
        return LoxObjectType::String;
    }

private:
    const std::string str;
    size_t compute_size() override {
        return sizeof(LoxString) + str.capacity();
    }
};

#endif //CCLOX_LOXSTRING_H
