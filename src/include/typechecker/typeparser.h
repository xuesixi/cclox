//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_TYPECHECKER_H
#define CCLOX_TYPECHECKER_H
#include <memory>
#include <vector>
#include <sstream>
#include "../scanner.h"

class LoxType {
public:
    using TypePtr = std::shared_ptr<LoxType>;
    static size_t resolve_type_id(const std::string &name);
    static std::string read_typename_from_id(size_t type_id);
    virtual ~LoxType() {}
    std::string to_string() {
        if (parenthesized) {
            return fmt::format("({})", to_no_parenthesis_string());
        } else {
            return to_no_parenthesis_string();
        }
    }

    bool parenthesized = false;
private:
    static std::unordered_map<std::string, size_t> typename_to_id;
    virtual std::string to_no_parenthesis_string() = 0;
    static std::vector<std::string> id_to_typename;
};

using TypePtr = std::shared_ptr<LoxType>;

class PrimaryType: public LoxType {
public:
    PrimaryType(const Token &identifier) {
        // todo: 同名类型冲突
        type_id = resolve_type_id(identifier.get_lexeme());
    }
    std::string to_no_parenthesis_string() override {
        return read_typename_from_id(type_id);
    }
private:
    size_t type_id;
};

class IntersectionType: public LoxType {
public:
    IntersectionType(std::vector<TypePtr> &&i): intersections(std::move(i)) {}

private:
    std::vector<TypePtr> intersections;

    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        for (size_t i = 0; i < intersections.size(); ++i) {
            ss << intersections.at(i)->to_string();
            if (i != intersections.size() - 1) {
                ss << " & ";
            }
        }
        return ss.str();
    }
};

class UnionType: public LoxType {
public:
    UnionType(std::vector<TypePtr> &&u): unions(std::move(u)) {}

    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        for (size_t i = 0; i < unions.size(); ++i) {
            ss << unions.at(i)->to_string();
            if (i != unions.size() - 1) {
                ss << " | ";
            }
        }
        return ss.str();
    }
private:
    std::vector<TypePtr> unions;
};

class TupleType: public LoxType {
public:
    TupleType(std::vector<TypePtr> &&t): sequence(t) {}

    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        ss << "(";
        for (size_t i = 0; i < sequence.size(); ++i) {
            ss << sequence.at(i)->to_string();
            if (i != sequence.size() - 1) {
                ss << ", ";
            }
        }
        ss << ")";
        return ss.str();
    }

private:
    std::vector<TypePtr> sequence;
};

class ArrayType: public LoxType {
public:
    ArrayType(TypePtr &&ptr): element_type(std::move(ptr)) {}

private:
    std::string to_no_parenthesis_string() override {
        return fmt::format("{}[]", element_type->to_string());
    }

    TypePtr element_type;
};

class FunctionType: public LoxType {
public:
    FunctionType(std::vector<TypePtr> &&p, TypePtr && rt): parameters(std::move(p)), return_type(std::move(rt)) {}
private:
    std::string to_no_parenthesis_string() override {
        std::stringstream ss;
        ss << "(";

        for (size_t i = 0; i < parameters.size(); ++i) {
            ss << parameters.at(i)->to_string();
            if (i != parameters.size() - 1) {
                ss << ", ";
            }
        }
        ss << fmt::format(") -> {}", return_type->to_string());
        return ss.str();
    }
    std::vector<TypePtr> parameters;
    TypePtr return_type;
};


class TypeParser {
public:
    TypeParser(std::string &&src) {
        Scanner scanner{std::move(src)};
        while (scanner.has_more()) {
            tokens.push_back(scanner.scan_token());
        }
    }

    TypePtr parse_type();
    TypePtr parse_primary();
    TypePtr parse_union();
    TypePtr parse_intersection();
    TypePtr parse_tuple();
    TypePtr parse_array();
    TypePtr parse_function();

private:
    Token &last_token() {
        if (next == 0) {
            implementation_error("next is 0, you should not call last_token now");
        }
        return tokens.at(next - 1);
    }
    Token &next_token() {
        if (next >= tokens.size()) {
            throw std::out_of_range("token exhausted");
        }
        return tokens.at(next++);
    }
    bool match(TokenType token_type) {
        if (next >= tokens.size()) {
            return false;
        }
        if (tokens.at(next).type == token_type) {
            next ++;
            return true;
        }
        return false;
    }
    void consume(TokenType token_type, const std::string &message) {
        if (!match(token_type)) {
            error_at(last_token(), message);
        }
    }
    void error_at(const Token &token, const std::string &message) {
        throw ExpectedTokenNotFoundError(fmt::format("line {}: {}", token.line, message));
    }
    std::vector<Token> tokens;
    size_t next = 0;
};

#endif //CCLOX_TYPECHECKER_H