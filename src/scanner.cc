#include "scanner.h"
#include <unordered_map>
#include <fmt/core.h>

static std::unordered_map<std::string, TokenType> keyword_map{
        {"and", TokenType::AND},
        {"class", TokenType::CLASS},
        {"else", TokenType::ELSE},
        {"false", TokenType::FALSE},
        {"for", TokenType::FOR},
        {"fun", TokenType::FUN},
        {"if", TokenType::IF},
        {"nil", TokenType::NIL},
        {"or", TokenType::OR},
        {"print", TokenType::PRINT},
        {"return", TokenType::RETURN},
        {"super", TokenType::SUPER},
        {"this", TokenType::THIS},
        {"true", TokenType::TRUE},
        {"var", TokenType::VAR},
        {"while", TokenType::WHILE}
};

TokenType Scanner::get_identifier_type() {
    std::string identifier = text.substr(start_index, next_index - start_index);
    auto found = keyword_map.find(identifier);
    if (found != keyword_map.end()) {
        return found->second;
    } else {
        return TokenType::IDENTIFIER;
    }
}

Token Scanner::scan_string() {
    while (!is_at_end() && peek_next() != '"') {
        if (peek_next() == '\n') {
            curr_line ++;
        }
        advance();
    }
    if (is_at_end()) {
        return error_token("unterminated string");
    }
    advance();
    return make_token(TokenType::STRING);
}

Token Scanner::scan_number() {
    while (!is_at_end() && isdigit(peek_next())) {
        advance();
    }
    if (is_at_end() || peek_next() != '.') {
        return make_token(TokenType::INTEGER);
    }

    advance(); // 能运行到这里，说明下一个字符是小数点，消费之。

    while (!is_at_end() && isdigit(peek_next())) {
        advance();
    }
    return make_token(TokenType::FLOAT);
}

Token Scanner::scan_identifier() {
    while (!is_at_end() && is_alpha_or_underscore(peek_next())) {
       advance();
    }
    return make_token(get_identifier_type());
}

Token Scanner::scan_token() {
    skip_whitespace();
    start_index = next_index;
    if (is_at_end()) {
        return make_token(TokenType::END_OF_FILE);
    }
    char c = advance();

    if (isdigit(c)) {
        // 处理数字
        return scan_number();
    } else if (isalpha(c) || c == '_') {
        // 标识符
        return scan_identifier();
    }

    switch (c) {
        case '(': return make_token(TokenType::LEFT_PAREN);
        case ')': return make_token(TokenType::RIGHT_PAREN);
        case '{': return make_token(TokenType::LEFT_BRACE);
        case '}': return make_token(TokenType::RIGHT_BRACE);
        case ';': return make_token(TokenType::SEMICOLON);
        case ',': return make_token(TokenType::COMMA);
        case '.': return make_token(TokenType::DOT);
        case '-': return make_token(TokenType::MINUS);
        case '+': return make_token(TokenType::PLUS);
        case '/': return make_token(TokenType::SLASH);
        case '*': return make_token(TokenType::STAR);
        case '!': {
            if (match('=')) return make_token(TokenType::BANG_EQUAL);
            else return make_token(TokenType::BANG);
        }
        case '=': {
            if (match('=')) return make_token(TokenType::EQUAL_EQUAL);
            else return make_token(TokenType::EQUAL);
        }
        case '>': {
            if (match('=')) return make_token(TokenType::GREATER_EQUAL);
            else return make_token(TokenType::GREATER);
        }
        case '<': {
            if (match('=')) return make_token(TokenType::LESS_EQUAL);
            else return make_token(TokenType::LESS);
        }
        case '"': // 处理字符串
            return scan_string();
        default:
            return error_token("unknown character");
    }
}

std::string Token::to_string() {
    return fmt::format("line: {}, token: {}", line, lexeme);
}

void Scanner::skip_whitespace() {
    while (true) {

        if (is_at_end()) {
            return;
        }

        switch (peek_next()) {
        case ' ':
        case '\t':
        case '\r':
            advance();
            break;
        case '\n':
            curr_line ++;
            advance();
            break;
        case '/': {
            if (peek_next(1) == '/') {
                // 说明遇到了注释，那么一路前进到本行结束

                while (!is_at_end() && peek_next() != '\n') {
                    // 如果遇到了文本结尾，本循环结束。在下一次的外层循环中因为is_at_end()判断而return
                    // 如果遇到了换行符，本循环结束。在下一次的外层循环中，进入换行符的case
                    advance();
                }
                break;
            } else {
                return;
            }
        }
        default:
            return;
        }
    }
}