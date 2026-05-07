#pragma once

#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    exit,
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    let,
    eq,
    plus,
    mult
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

inline std::optional<int> bin_prec(TokenType type) {
    switch (type) {
    case TokenType::plus:
        return 1;
    case TokenType::mult:
        return 2;
    default:
        return {};
    }
}

class Tokenizer {
  public:
    inline explicit Tokenizer(const std::string &src) : m_src(std::move(src)) {}

    inline std::vector<Token> tokenize() {
        std::vector<Token> tokens{};
        std::string buff{};

        while (peek().has_value()) {

            if (std::isalpha(peek().value())) {
                buff.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buff.push_back(consume());
                }
                if (buff == "exit") {
                    tokens.push_back({.type = TokenType::exit});
                    buff.clear();
                    continue;
                } else if (buff == "let") {
                    tokens.push_back({.type = TokenType::let});
                    buff.clear();
                    continue;
                } else {
                    tokens.push_back({.type = TokenType::ident, .value = buff});
                    buff.clear();
                    continue;
                    // std::cerr << "You Cooked?" << std::endl;
                    // std::exit(EXIT_FAILURE);
                }
            } else if (std::isdigit(peek().value())) {
                buff.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buff.push_back(consume());
                }
                tokens.push_back({.type = TokenType::int_lit, .value = buff});
                buff.clear();
                continue;
            } else if (peek().value() == '(') {
                consume();
                tokens.push_back({.type = TokenType::open_paren});
                continue;
            } else if (peek().value() == ')') {
                consume();
                tokens.push_back({.type = TokenType::close_paren});
                continue;
            } else if (peek().value() == '+') {
                consume();
                tokens.push_back({.type = TokenType::plus});
                continue;
            } else if (peek().value() == '*') {
                consume();
                tokens.push_back({.type = TokenType::mult});
                continue;
            } else if (peek().value() == '=') {
                consume();
                tokens.push_back({.type = TokenType::eq});
                continue;
            } else if (peek().value() == ';') {
                consume();
                tokens.push_back({.type = TokenType::semi});
                continue;
            } else if (std::isspace(peek().value())) {
                consume();
                continue;
            }
        }
        m_index = 0;
        return tokens;
    }

  private:
    int m_index = 0;
    const std::string m_src;
    inline std::optional<char> peek(int offset = 0) const {
        if (m_index + offset >= m_src.length()) {
            return {};
        } else {
            return m_src.at(m_index + offset);
        }
    }

    char inline consume() { return m_src.at(m_index++); }
};
