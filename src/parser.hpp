#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <ostream>
#include <vector>

#include "arena.hpp"
#include "grammer.hpp"
#include "tokenization.hpp"

class Parser {
  public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4) /* 4 MB */ {
    }

    std::optional<NodeTerm *> parse_term() {
        if (auto int_lit = try_consume(TokenType::int_lit)) {
            // if (peek().has_value() && peek().value().type ==
            // TokenType::int_lit) {
            auto node_term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            node_term_int_lit->int_lit = int_lit.value();
            auto node_term = m_allocator.alloc<NodeTerm>();
            node_term->var = node_term_int_lit;
            return node_term;
        } else if (auto ident = try_consume(TokenType::ident)) {
            // } else if (peek().has_value() &&
            //            peek().value().type == TokenType::ident) {
            auto node_term_ident = m_allocator.alloc<NodeTermIdent>();
            node_term_ident->ident = ident.value();
            auto node_term = m_allocator.alloc<NodeTerm>();
            node_term->var = node_term_ident;
            return node_term;
        } else if (auto ident = try_consume(TokenType::open_paren)) {
            auto expr = parse_expr();
            if(!expr.has_value()) {
                std::cerr << "Expected Expression" << std::endl;
                std::exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "Expected ')'");
            auto term_paren = m_allocator.alloc<NodeTermParen>();
            auto node_term = m_allocator.alloc<NodeTerm>();
            term_paren->expr = expr.value();
            node_term->var = term_paren;
            return node_term;
        } else {
            return {};
        }
    }

    std::optional<NodeExpr *> parse_expr(int min_prec = 0) {
        std::optional<NodeTerm *> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }

        auto expr_lhs = m_allocator.alloc<NodeExpr>();
        expr_lhs->var = term_lhs.value();

        while (true) {
            std::optional<Token> curr_token = peek();
            std::optional<int> prec;
            if (curr_token.has_value()) {
                prec = bin_prec(curr_token->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            } else {
                break;
            }
            Token op = consume();
            int next_min_prec = prec.value() + 1;

            auto expr_rhs = parse_expr(next_min_prec);

            if (!expr_rhs.has_value()) {
                std::cerr << "Unable to Parse Expression" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            auto expr = m_allocator.alloc<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.alloc<NodeExpr>();

            if (op.type == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                expr_lhs2->var = expr_lhs->var;
                add->lhs = expr_lhs2;
                add->rhs = expr_rhs.value();
                expr->var = add;
            } else if (op.type == TokenType::sub) {
                auto sub = m_allocator.alloc<NodeBinExprSub>();
                expr_lhs2->var = expr_lhs->var;
                sub->lhs = expr_lhs2;
                sub->rhs = expr_rhs.value();
                expr->var = sub;
                expr->var = sub;
            } else if (op.type == TokenType::mult) {
                auto multi = m_allocator.alloc<NodeBinExprMulti>();
                expr_lhs2->var = expr_lhs->var;
                multi->lhs = expr_lhs2;
                multi->rhs = expr_rhs.value();
                expr->var = multi;
                expr->var = multi;
            } else if (op.type == TokenType::div) {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                expr_lhs2->var = expr_lhs->var;
                div->lhs = expr_lhs2;
                div->rhs = expr_rhs.value();
                expr->var = div;
                expr->var = div;
            } else {
                assert(false);
            }

            expr_lhs->var = expr;
        }

        return expr_lhs;
    }

    std::optional<NodeStmt *> parse_stmt() {
        if (peek().value().type == TokenType::exit && peek(1).has_value() &&
            peek(1).value().type == TokenType::open_paren) {
            consume();
            consume();
            auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
            if (auto node_expr = parse_expr()) {
                // has value
                stmt_exit->expr = node_expr.value();

            } else {
                std::cerr << "Invalid Expression" << std::endl;
                std::exit(EXIT_FAILURE);
            }

            try_consume(TokenType::close_paren, "Expected ')'");
            try_consume(TokenType::semi, "Expected ';'");

            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_exit;
            return node_stmt;
        } else if (peek().has_value() &&
                   peek().value().type == TokenType::let &&
                   peek(1).has_value() &&
                   peek(1).value().type == TokenType::ident &&
                   peek(2).has_value() &&
                   peek(2).value().type == TokenType::eq) {
            consume(); // "let" => is consume
            auto stmt_let = m_allocator.alloc<NodeStmtLet>();
            stmt_let->ident = consume(); // identifier {variable} =>  is consume
            consume();                   // "=" => is consume
            if (auto expr = parse_expr()) {
                stmt_let->expr = expr.value();
            } else {
                std::cerr << "Invalid Expression" << std::endl;
                std::exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semi, "Expected ';' yaha");
            auto node_stmt = m_allocator.alloc<NodeStmt>();
            node_stmt->var = stmt_let;
            return node_stmt;
        } else {
            return {};
        }
    }

    std::optional<NodeProg> parse_prog() {
        NodeProg prog;
        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(stmt.value());
            } else {
                std::cerr << "Invalid Statement" << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

  private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;

    inline std::optional<Token> try_consume(TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            return {};
        }
    }

    inline Token try_consume(TokenType type, const std::string &err_msg) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        } else {
            std::cerr << err_msg << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> peek(int offset = 0) const {
        if (m_index + offset >= m_tokens.size()) {
            return {};
        } else {
            return m_tokens.at(m_index + offset);
        }
    }

    Token inline consume() { return m_tokens.at(m_index++); }
    ArenaAllocator m_allocator;
};