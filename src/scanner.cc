#include "scanner.h"
#include <algorithm>
#include <string>

Token Scanner::scan_token() {
    while (true) {
        start_index = next_index;
        skip_whitespace();
        if (is_at_end()) {
            return make_token(TokenType::FILE_END);
        }
        switch (advance()) {
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
        }
    }
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

            } else {
                return;
            }
        }
        default:
            return;
        }
    }
}