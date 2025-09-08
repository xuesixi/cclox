//
// Created by Yue Xue  on 9/7/25.
//

#ifndef CCLOX_GLOBAL_NAME_RESOLVER_H
#define CCLOX_GLOBAL_NAME_RESOLVER_H

#include "common.h"
class Token;
class LoxType;
class ClassType;
using TypePtr = std::shared_ptr<LoxType>;

/**
 * 在构建 AST 的时候，在类负责记录出现的全局标识符以及它们对应的类型、全局索引
 */
class GlobalNameResolver {
public:
    /**
     * 代表一个被记录的类
     */
    class ClassRecord {
    public:
        enum class ClassMemberType {
            Field,
            Method
        };

        friend class GlobalNameResolver;

        ClassRecord(const std::string &class_name);

        void check_duplicate_member(const std::string &member_name);

    private:
        size_t class_id;
        std::shared_ptr<ClassType> class_type;
        std::unordered_map<std::string, TypePtr> methods;
        std::unordered_map<std::string, TypePtr> fields;
    };

    /**
     * 获取一个name对应的typeid
     */
    size_t resolve_type_id(const std::string &name);

    /**
     * 根据typeid，读取对应类型的名字
     */
    std::string read_typename_from_id(size_t type_id);

    /**
     * 记录一个新的函数。如果重复，抛出异常
     */
    void record_function(const std::string &name, TypePtr &type);

    /**
     * 记录一个新的类。如果重复，抛出异常
     */
    void record_class(const std::string &class_name);

    /**
     * 记录一个新的类的成员
     */
    void record_class_member(const std::string &class_name, ClassRecord::ClassMemberType member_type,
                                    const std::string &member_name, TypePtr &type);

    /**
     * 寻找一个类的类型，如果没找到该类，抛出异常
     */
    TypePtr find_class(const std::string &name);

    /**
     * 寻找一个类的成员的类型。如果没找到该成员，抛出异常
     */
    TypePtr find_class_member(size_t type_id, const std::string &member_name);

    /**
     * 寻找一个函数
     */
    TypePtr find_function(const std::string &fun_name);

    /**
     * 寻找指定名字对应的类型，如果没找到，返回 Unspecified
     */
    TypePtr find_name_type(const std::string &name);

private:

    std::unordered_map<std::string, size_t> typename_to_id;

    std::vector<std::string> id_to_typename;

    /**
     * 下面两个数据结构包含了在语句解析器建立 AST 的过程中，收集的全局函数和类的名字以及它们对应的类型。
     * 全局函数的类是可以互相调用的（而不必遵循定义的顺序），因为在第一次解析的时候就把它们的名字和类型都记录了。
     */
    std::unordered_map<std::string, TypePtr> global_functions;
    std::unordered_map<std::string, ClassRecord> global_classes;
};

extern GlobalNameResolver name_resolver;
#endif //CCLOX_GLOBAL_NAME_RESOLVER_H