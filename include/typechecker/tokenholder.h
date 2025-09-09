//
// Created by Yue Xue  on 9/3/25.
//

#ifndef CCLOX_TOKENHOLDER_H
#define CCLOX_TOKENHOLDER_H

#include "common.h"
#include "scanner.h"

/**
 * 该类管理一个token列表。
 * 由于TypeParser, ExpressionParser, StatementParser需要共用同一个token列表来进行解析，它们都使用该类
 */
class TokenHolder {
public:
    explicit TokenHolder(std::string &&src) {
        Scanner scanner{std::move(src)};
        while (scanner.has_more()) {
            tokens.push_back(scanner.scan_token());
        }
    }

    bool is_end() const {
        return next >= tokens.size();
    }

    /**
     * @return 上一次解析的token
     */
    Token &last_token() {
        if (next == 0) {
            IMPL_ERROR("next is 0, you should not call last_token now");
        }
        return tokens.at(next - 1);
    }

    /**
     * 返回next所指的token，并自增next
     * @return 下一个token
     */
    Token &next_token() {
        if (next >= tokens.size()) {
            throw std::out_of_range("token exhausted");
        }
        return tokens.at(next++);
    }

    const Token &peek_next() {
        if (next >= tokens.size()) {
            throw std::out_of_range("token exhausted");
        }
        return tokens.at(next);
    }

    /**
     * 尝试匹配
     * @param token_type 要匹配的类型
     * @return 如果匹配成功，则消费之，返回true。否则返回false
     */
    bool match(TokenType token_type) {
        if (next >= tokens.size()) {
            return false;
        }
        if (tokens.at(next).type == token_type) {
            next++;
            return true;
        }
        return false;
    }

    /**
     * 尝试匹配一系列可能的token
     * @param candidates 可选性
     * @return 如果任意一个匹配成功，则消费之，返回true。否则返回false
     */
    bool match_one_of(std::initializer_list<TokenType> candidates) {
        for (const auto token: candidates) {
            if (match(token)) {
                return true;
            }
        }
        return false;
    }

    /**
     * 要求匹配某个token，某个不匹配则抛出异常。如果不提供任何参数，则是要求以分号结束语句。
     * @param token_type 要匹配的token
     * @param message 如果失败，抛出的异常的错误信息
     * @return 匹配的那个token
     */
    Token &consume(TokenType token_type = TokenType::SEMICOLON, const std::string &message = "expect a semicolon to end the statement") {
        if (!match(token_type)) {
            throw error_at(last_token(), message);
        }
        return last_token();
    }

    /**
     * @param token 这个token出现在不合适的位置
     * @param message 错误消息
     */
    [[nodiscard]] ExpectedTokenNotFoundError error_at(const Token &token, const std::string &message) {
        return ExpectedTokenNotFoundError(fmt::format("line {}: {}", token.line, message));
    }

    /**
     * @param relative_position 错误token的相对位置。相对于next
     * @param message 错误消息
     */
    [[nodiscard]] ExpectedTokenNotFoundError error_at(size_t relative_position, const std::string &message) {
        return error_at(tokens.at(next + relative_position), message);
    }

    /**
     * @return 返回当前的next的值
     */
    size_t save_position() {
        return next;
    }

    /**
     * 将next重置为制定的值
     */
    void rewind(size_t saved_position) {
        next = saved_position;
    }

private:
    size_t next = 0;
    std::vector<Token> tokens;
};

#endif //CCLOX_TOKENHOLDER_H
