//
// Created by Yue Xue  on 8/16/25.
//

#ifndef LOXCLASS_H
#define LOXCLASS_H
#include "object.h"
#include "cclox_util.h"
#include "stringintern.h"
#include "objects/loxclosure.h"
#include "value.h"

class LoxClass: public LoxObject {
public:

    explicit LoxClass(const std::string &the_name, uint8_t the_num_fields): name(the_name), num_fields(the_num_fields) {

    }

    ~LoxClass() override {
        Runtime::record_free(*this);
    }

    size_t compute_size() override {
        if (mem_size == 0) {
            mem_size = sizeof(LoxClass) + estimate_u16_map_size(methods);
            return mem_size;
        } else {
            return mem_size;
        }
    }

    void clear_reference() override {
        methods.clear();
        constructor_ = nullptr;
    }

    [[nodiscard]] std::string to_string() const override {
        return fmt::format("<cls: {}>", name);
    }

    void add_method(const std::shared_ptr<LoxClosure> &method) {
        if (method->fun_name() == "init") {
            constructor_ = method;
        }
        auto stdid = StringIntern::resolve_string(method->fun_name());
        methods.insert({stdid, method});
    }

    /**
     * 在类中寻找对应名字的方法
     * @throws LoxNameError 如果没找到
     */
    std::shared_ptr<LoxClosure> resolve_method(uint16_t strid) {
        auto found = methods.find(strid);
        if (found == methods.end()) {
            throw LoxNameError(fmt::format("no such method {} in class {}", StringIntern::read_from_id(strid), name));
        }
        return found->second;
    }

    std::string get_name() const {
        return name;
    }

    std::shared_ptr<LoxClosure> &constructor() {
        return constructor_;
    }

    uint8_t get_num_fields() const {
        return num_fields;
    }

    LoxObjectType get_object_type() const override {
        return LoxObjectType::Class;
    }

    void mark_reference(std::queue<LoxReference> &queue) override {
        mark(constructor_, queue);
        for (auto &pair: methods) {
            mark(pair.second, queue);
        }
    }

private:
    std::unordered_map<uint16_t, std::shared_ptr<LoxClosure>> methods;
    std::shared_ptr<LoxClosure> constructor_;
    size_t mem_size = 0;
    std::string name;
    uint8_t num_fields = 0;
};

#endif //LOXCLASS_H
