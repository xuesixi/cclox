//
// Created by Yue Xue  on 8/9/25.
//

#ifndef LOXFUNCTION_H
#define LOXFUNCTION_H
#include "chunk.h"
#include "object.h"

enum class FunctionType {
    Function,
    Main,
};

class LoxFunction: public LoxObject {

public:
    friend class LoxClosure;

    ~LoxFunction() override {
        auto old = runtime.allocated_size.fetch_sub(memory_size);
        if (Flag::show_heap) {
            std::cout << fmt::format("[-] heap: {:^6} -> {:^6}; {}\n", old, old - memory_size, LoxFunction::to_string());
        }
    }
    void clear_reference() override {};

    bool operator==(const LoxObject &other) const override {
        return this == &other;
    }

    std::string to_string() const override {
        if (name == "<main>") {
            return "<proto: main>";
        }
        return fmt::format("<proto: {}>", name);
    }

    Chunk &get_chunk() {
        return chunk;
    }

    void set_name(const std::string &new_name) {
        name = new_name;
    }

    void incre_arity() {
        arity_ ++;
    }
    int arity() {
        return arity_;
    }

private:
    Chunk chunk;
    std::string name;
    int arity_ = 0 ;
    size_t memory_size = 0;

    size_t compute_size() override {
        if (memory_size == 0) {
            memory_size = sizeof(LoxFunction) + chunk.estimate_memory_size() + name.capacity();
            return memory_size;
        } else {
            return memory_size;
        }
    }
};

#endif //LOXFUNCTION_H
