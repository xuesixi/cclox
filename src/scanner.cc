#include "scanner.h"
#include <unordered_map>
#include <fmt/core.h>

#include "error.h"

static std::unordered_map<std::string, TokenType> keyword_map{
    {"and", TokenType::And},
    {"class", TokenType::Class},
    {"else", TokenType::Else},
    {"false", TokenType::False},
    {"for", TokenType::For},
    {"fun", TokenType::Fun},
    {"if", TokenType::If},
    {"nil", TokenType::Nil},
    {"or", TokenType::Or},
    {"print", TokenType::Print},
    {"return", TokenType::Return},
    {"super", TokenType::Super},
    {"this", TokenType::This},
    {"true", TokenType::True},
    {"var", TokenType::Var},
    {"while", TokenType::While},
    {"break", TokenType::Break},
    {"continue", TokenType::Continue},
    {"recur", TokenType::Recur},
    {"field", TokenType::Field},
    {"method", TokenType::Method},
    {"public", TokenType::Public},
    {"static", TokenType::Static},
    {"import", TokenType::Import},
    {"export", TokenType::Export},
    {"as", TokenType::As},
    {"throw", TokenType::Throw},
    {"try", TokenType::Try},
    {"catch", TokenType::Catch},
    {"const", TokenType::Const},
    {"int", TokenType::TypeInt},
    {"float", TokenType::TypeFloat},
    {"bool", TokenType::TypeBool},
    {"any", TokenType::TypeAny},
    {"void", TokenType::TypeVoid},
    {"is", TokenType::Is},

};

TokenType Scanner::get_identifier_type() {
    const std::string identifier = text.substr(start_index, next_index - start_index);
    const auto found = keyword_map.find(identifier);
    if (found != keyword_map.end()) {
        return found->second;
    } else {
        return TokenType::IDENTIFIER;
    }
}

Token Scanner::scan_string() {
    while (!is_at_end() && peek_next() != '"') {
        if (peek_next() == '\n') {
            curr_line++;
        }
        advance();
    }
    if (is_at_end()) {
        throw ScannerError(fmt::format("unterminated string at line {}", curr_line));
    }
    advance();
    return make_token(TokenType::STRING);
}

Token Scanner::scan_fmt_string() {
    while (!is_at_end() && peek_next() != '\'') {
        if (peek_next() == '\n') {
            curr_line++;
        }
        advance();
    }
    if (is_at_end()) {
        throw ScannerError(fmt::format("unterminated format string at line {}", curr_line));
    }
    advance();
    return make_token(TokenType::FMT_STRING);
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
    while (!is_at_end() && is_alpha_or_digit_or_underscore(peek_next())) {
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
    const char c = advance();

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
        case ':': return make_token(TokenType::COLON);
        case '|': return make_token(TokenType::BAR);
        case '&': return make_token(TokenType::AMPERSAND);
        case '[': return make_token(TokenType::LEFT_BRACKET);
        case ']': return make_token(TokenType::RIGHT_BRACKET);
        case '-': {
            if (match('>')) return make_token(TokenType::DASH_GREATER);
            return make_token(TokenType::MINUS);
        }
        case '+': return make_token(TokenType::PLUS);
        case '/': return make_token(TokenType::SLASH);
        case '*': {
            if (match('*')) return make_token(TokenType::STAR_STAR);
            return make_token(TokenType::STAR);
        }
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
        case '\'': {
            return scan_fmt_string();
        }
        case '"': // 处理字符串
            return scan_string();
        default:
            throw ScannerError(fmt::format("unknown character at line {}", curr_line));
    }
}


std::optional<std::vector<std::tuple<bool, size_t, size_t>>> Scanner::split(

#define str_not_empty(left, right)  (( (right) - (left) + 1 ) > 0)

    const std::string &src, char open, char close) {
    std::vector<std::tuple<bool,size_t, size_t>> vec;
    std::vector<size_t> stack;

    // left和right都是inclusive的，这意味着长度 = right - left + 1。所以，空字符串的left > right
    size_t regular_left = 0;
    size_t regular_right = 0;

    for (size_t i = 0; i < src.size(); i ++) {
        const char &c = src.at(i);
        if (c == open) {

            if (stack.empty()) {
                // 这意味着一个普通字符串的结束，以及一个内嵌表达式的开始
                regular_right = i - 1;
                if (str_not_empty(regular_left, regular_right)) {
                    vec.push_back({false, regular_left, regular_right});
                }
            }
            stack.push_back(i);

        } else if (c == close) {
            if (stack.empty()) {
                // 不成对的起始符，这是格式错误
                return std::nullopt;
            }
            auto left = stack.back();
            stack.pop_back();
            if (stack.empty()) {
                // 这意味着一个内嵌表达式的结束
                vec.push_back({true, left, i});
                regular_left = i + 1;
            }
        }
    }

    // 末尾的处理。
    regular_right = src.size() - 1;
    if (str_not_empty(regular_left, regular_right)) {
        vec.push_back({false, regular_left, regular_right});
    }
    if (!stack.empty()) {
        return std::nullopt;
    }
    return vec;

#undef str_not_empty
}

std::string Token::to_string() const {
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
                curr_line++;
                advance();
                break;
            case '/': {
                if (!is_at_end(1) && peek_next(1) == '/') {
                    // 说明遇到了注释，那么一路前进到本行结束

                    while (!is_at_end() && peek_next() != '\n') {
                        // 如果遇到了文本结尾，本循环结束。在下一次的外层循环中因为is_at_end()判断而return
                        // 如果遇到了换行符，本循环结束。在下一次的外层循环中，进入换行符的case
                        advance();
                    }
                    break;
                } else if (!is_at_end(1) && peek_next(1) == '*') {
                    /* */
                    advance();
                    advance();
                    while (true) {
                        if (!is_at_end(1) && peek_next() == '*' && peek_next(1) == '/') {
                            advance();
                            advance();
                            break;
                        }
                        if (is_at_end()) {
                            throw ScannerError(fmt::format("unterminated /* comment at line {}", curr_line));
                            return;
                        }
                        if (peek_next() == '\n') {
                            curr_line ++;
                        }
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
