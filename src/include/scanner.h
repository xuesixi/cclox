
#ifndef CCLOX_SCANNER_H
#define CCLOX_SCANNER_H

#include <cstddef>
#include <optional>
#include <string>

#include "error.h"

enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS,
    SEMICOLON, SLASH, STAR, STAR_STAR,
    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL, DASH_GREATER, BAR, AMPERSAND, LEFT_BRACKET, RIGHT_BRACKET,
    // Literals.
    IDENTIFIER, STRING, FLOAT, INTEGER, FMT_STRING,

    // Keywords.
    AND, CLASS, ELSE, FALSE,
    FOR, FUN, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS,
    TRUE, VAR, WHILE,

    // extension
    BREAK, CONTINUE, RECUR, Field, Method, Static, Public, Export, Import, As, Try, Catch, Throw,

    ERROR, END_OF_FILE // EOF被内置宏占用了
};


class Token {
public:
    friend class Scanner;
    friend class Compiler;
    friend class TypeParser;
    friend class ExpressionParser;
    friend class TokenHolder;

    Token(): lexeme(), type(TokenType::ERROR), line(-1) {
    };

    /**
     * token会持有自己的lexeme字符串。在构造时，其string参数需要是临时值，主要是text.substr()
     * */
    Token(std::string &&token_content, TokenType token_type, int token_line)
        : lexeme(std::move(token_content)), type(token_type), line(token_line) {
    };

    std::string to_string() const;

    TokenType get_type() const {
        return type;
    }

    const std::string &get_lexeme() const {
        return lexeme;
    }

private:
    std::string lexeme;
    TokenType type;
    int line;
};

class Scanner {
public:
    Scanner(std::string &&file_content): Scanner(std::move(file_content), 1) {
    };

    /**
     * 可以设置初始行号，会用在格式化字符串中
     */
    Scanner(std::string &&file_content, int start_line): text(std::move(file_content)), start_index(0), next_index(0), curr_line(start_line) {
    };

    /**
     * 返回下一个token。
     * @throws ScannerError 如果出现扫描错误
     */
    Token scan_token();

    bool has_more() {
        return !is_at_end();
    }

    /**
     * Splits a string into alternating regular text and "caught" sections.
     *
     * The top level string surrounded by the opening and closing char are "caught",
     * others are 'regular'. Nested brackets are supported but only top-level pairs
     * are marked as caught.
     *
     * @param src The input string to split
     * @param open Opening delimiter character (default: '{')
     * @param close Closing delimiter character (default: '}')
     * @return Optional vector of tuples containing (is_caught, left_index, right_index)
     *         Returns nullopt if brackets are unmatched
     *
     * Each element in the returned vector is tuple<is_caught, left, right>
     * where left and right are both inclusive indices.
     *
     * Example: split("name{anda}end", '{', '}')
     * Returns: {{false, 0, 3}, {true, 4, 9}, {false, 10, 12}}
     *          "name"         "{anda}"       "end"
     */
    static std::optional<std::vector<std::tuple<bool, size_t, size_t>>> split
    (const std::string &src, char open = '{', char close = '}');

private:
    /* 判断字符是不是字母或者下划线*/
    static bool is_alpha_or_digit_or_underscore(char c) {
        return isalpha(c) || c == '_' || isdigit(c);
    }

    /**
     * 以当前的start和next index为范围，创建指定类型的token
     */
    Token make_token(TokenType type) {
        return {text.substr(start_index, next_index - start_index), type, curr_line};
    }

    /**
     * 用指定的信息创建一个代表错误的token
     */
    Token error_token(const std::string &message) {
        return {std::string(message), TokenType::ERROR, curr_line};
    }

    /**
     * 是否到达文本的末尾。实际上是测试 next_index+n 是否大于等于text.size()
     */
    bool is_at_end(int n = 0) {
        return next_index + n >= text.size();
    }

    /* 返回next_index指向的字符，并自增之 */
    char advance() {
        return text.at(next_index++);
    }

    /**
     * 测试next_index是否是指定的字符，如果已经是末尾，或者不匹配，返回false，否则，自增next_index并返回true
     * */
    bool match(char c) {
        if (is_at_end()) {
            return false;
        }
        if (text.at(next_index) != c) {
            return false;
        } else {
            advance();
            return true;
        }
    }

    /*
     * 当参数为空或者为0时，返回next_index所指的字符。其他参数值则返回相对于之的字符。该函数不会进行边界检查。
     */
    char peek_next(size_t n = 0) {
        return text.at(next_index + n);
    }

    /**
     * 跳过各种空格、tab、换行、注释。该函数内部够正确处理文本末尾（使用之前，无需检查是否是文本末尾。但使用之后仍然需要检查）
     * */
    void skip_whitespace();

    /*
     * 下面这几个具体scan某个类型token的函数都会将next_index移动到当前token结尾的后一个字符
     */

    /**
     * 在起初的左引号已经被消费了之后，调用该函数，返回代表该字符串的token。
     * */
    Token scan_string();

    Token scan_fmt_string();

    /**
     * 在第一个数字被消费后，调用该函数，返回代表该数字的integer或者float。
     * */
    Token scan_number();

    /**
     * 在第一个字母/下划线被消费后，调用该函数，返回该标识符之token。
     * */
    Token scan_identifier();

    /**
     * 判断当前start_index和next_index范围内的标识符的类型（可能是普通标识符，也可能是某个具体的关键字）
     * */
    TokenType get_identifier_type();

    const std::string text;
    size_t start_index; // 当前token的起始位置。当读取一个新的token时，会被赋值为next_index
    size_t next_index; // 下一个扫描的字符位置。
    int curr_line;
};

#endif
