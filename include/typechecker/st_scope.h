//
// Created by Yue Xue  on 9/8/25.
//

#ifndef CCLOX_ST_SCOPE_H
#define CCLOX_ST_SCOPE_H

#include "common.h"
#include "error.h"
#include "scanner.h"

class FunctionType;
class LoxType;
class LoxFunction;
using TypePtr = std::shared_ptr<LoxType>;

class ST_ClassScope {
public:
    friend class Compiler;
    friend class AstCompiler;

    void add_field(const std::string &name) {
        if (member_fields.size() == UINT8_MAX) {
            throw Uint8OperandOverflowError("cannot have more than 256 fields");
        }
        const auto found = resolve_field(name);
        if (found) {
            throw DuplicateNameVariableError("fields with the same name");
        }
        member_fields.push_back(name);
    }

    std::optional<uint8_t> resolve_field(const std::string &name) {
        for (size_t i = 0; i < member_fields.size(); i++) {
            if (member_fields.at(i) == name) {
                return i;
            }
        }
        return std::nullopt;
    }

    void incre_method_count() {
        method_count++;
    }

private:
    std::vector<std::string> member_fields;
    std::vector<std::string> static_fields;
    size_t method_count = 0;
};

/**
 * 一个Scope是一个函数级的作用域。代码块级的作用域体现在scope内部中的depth
 */
class ST_Scope {
public:

    friend class Compiler;
    friend class AstCompiler;

    /**
     * 创建一个新的scope，分配一个新的loxfunction（但还没有调用fix_size），添加第一个本地变量为占地符。
     */
    explicit ST_Scope(const std::shared_ptr<ST_Scope> &outer, const std::shared_ptr<FunctionType> &fun_type);

    /**
     * 进入一个新的层级，自增depth，并返回此时的本地变量的数量
     */
    uint8_t step_into();

    /**
     * 离开一个层级。自减depth，并将本地变量的数量从缩减至clear_to。返回缩减的数量
     */
    [[nodiscard]] uint8_t step_out(uint8_t clear_to);

    size_t locals_size() const {
        return locals.size();
    }

    /**
     * 是否是全局scope（depth==0）
     */
    bool is_global_scope() const {
        return depth == 0;
    }

    /**
     * 初始化最新的那个本地变量
     */
    void initialize() {
        DEBUG_ASSERT(locals.size() > 0, "the size should not be 0");
        locals.back().depth = depth;
    }

    /**
     * 将一个token对应的本地变量添加到scope中。如果同层级中存在同名变量，则抛出异常。
     * 添加后，该本地变量尚未被初始化。
     * 如果locals的元素个数超出了uint8，则抛出异常
     */
    void add_local(const Token &token, TypePtr type);


    /**
     * 在scope中查找对应token对应的同名变量，如果没找到，返回nullopt
     * 如果找到了，但没有被初始化，抛出异常；否则返回其对应的索引
     */
    std::optional<uint8_t> resolve_local(const Token &token);

    /**
     * 在scope中寻找对应token的类型。如果没找到，返回 Unspecified
     */
    TypePtr resolve_local_type(const Token &token);

    /**
     * 在已知给定的token不存在于本层的locals中时，调用该函数。
     * 先检查给定的token是否存在于本层的upvalues中（是否已经被捕获），
     * 如果不存在，则尝试捕获：检查外层的本地变量，如果不存在，递归地对外层调用该函数。
     * 如果由此成功捕获，则将其添加到自己的upvalues中，如果没有找到，返回std::nullopt
     *
     * 这里的递归会产生这样的效果：假如函数A中定义了B，函数B中定义了C。
     * 当C捕获A中的本地变量v时，C不会直接捕获A的，而是先让B捕获A的本地变量，添加到B的upvalues中，
     * 如果C再从B的upvalues中进行捕获。
     *
     * @param token 要检察的标识符
     * @return 索引，或者nullopt
     */
    std::optional<uint8_t> resolve_upvalue(const Token &token);

    /**
     * 调用resolve_upvalue进行捕获或者查询，如果捕获到了/查询到了，返回其类型，否则返回Unspecified
     */
    TypePtr resolve_upvalue_type(const Token &token);

private:
    struct ST_Local {

        ST_Local() {
        }

        ST_Local(const std::string &name, int depth, const TypePtr &type): name(name), depth(depth), type(type) {

        }

        bool is_initialized() const {
            return depth != -1;
        }

        std::string name;
        int depth = -1;
        TypePtr type;
    };

    /**
     * Compile Time Upvalue 编译时upvalue。其运行期对应的结构为Captured
     */
    struct ST_Upvalue {

        ST_Upvalue(bool is_local, uint8_t index, const std::string &name, const TypePtr &type):
        is_local(is_local), index(index), name(name), type(type) {

        }

        bool is_local; // 被捕獲的值是來自於外層的 locals 還是 upvalues
        uint8_t index; // 根据 is_local，可以是在外层的locals中的索引，或者在外层的upvalues中的索引
        std::string name;
        TypePtr type;
    };

    std::vector<ST_Local> locals;
    std::vector<ST_Upvalue> upvalues; // 本层级捕获的外层变量
    int depth = 0; // 深度。包括函数层级和{}层级
    std::shared_ptr<LoxFunction> function_;
    std::shared_ptr<ST_Scope> outer_;
};

#endif //CCLOX_ST_SCOPE_H