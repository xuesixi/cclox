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
 * 全局函数和类是可以互相调用的（而不必遵循定义的顺序），因为在第一次解析的时候就需要把它们的名字和类型都记录了。
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
    void declare_function(const std::string &name, TypePtr &type);

    /**
     * 记录一个新的类。如果重复，抛出异常
     */
    void declare_class(const std::string &class_name);

    /**
     * 记录一个新的类的成员
     */
    void declare_class_member(const std::string &class_name, ClassRecord::ClassMemberType member_type,
                              const std::string &member_name, TypePtr &type);


    /**
     * 寻找一个类的成员的类型。如果没找到该成员，抛出异常
     */
    TypePtr find_class_member(size_t type_id, const std::string &member_name);

    /**
     * 寻找指定名字对应的类型和全局索引<type, index>，如果没找到，返回 <Unspecified, 0>
     */
    std::pair<TypePtr, uint16_t> resolve_name(const std::string &name);

    // /**
    //  * 寻找一个类的类型，如果没找到该类，抛出异常
    //  */
    // TypePtr find_class(const std::string &name);
    //
    // /**
    //  * 寻找一个函数
    //  */
    // TypePtr find_function(const std::string &fun_name);

private:

    // 这里的 typename 和 id 的互相转化其实意义不大，只是为了减少 TypePtr 中储存整个 string 的内存浪费
    std::unordered_map<std::string, size_t> typename_to_id;

    std::vector<std::string> id_to_typename;

    std::unordered_map<std::string, std::pair<TypePtr, uint16_t> > global_functions;

    std::unordered_map<std::string, std::pair<ClassRecord, uint16_t> > global_classes;
};

extern GlobalNameResolver name_resolver;
#endif //CCLOX_GLOBAL_NAME_RESOLVER_H
