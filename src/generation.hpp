#pragma once

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <variant>

#include "grammer.hpp"

class Generator {
  public:
    inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {}

    void gen_term(const NodeTerm *term) {
        struct TermVisitor {
            Generator *gen;
            void operator()(const NodeTermIntLit *term_int_lit) const {
                gen->m_output << "    mov rax, "
                              << term_int_lit->int_lit.value.value()
                              << std::endl;
                gen->push("rax");
            }
            void operator()(const NodeTermIdent *term_ident) const {
                if (!gen->m_vars.contains(term_ident->ident.value.value())) {
                    std::cerr << "Undeclared identifier: "
                              << term_ident->ident.value.value() << std::endl;
                    std::exit(EXIT_FAILURE);
                }

                const auto &var =
                    gen->m_vars.at(term_ident->ident.value.value());
                std::stringstream offset;
                offset << "QWORD [rsp + "
                       << (gen->m_stack_loc - var.stack_loc - 1) * 8 << "]";
                gen->push(offset.str());
            }
        };

        TermVisitor visitor({.gen = this});
        std::visit(visitor, term->var);
    }

    void gen_bin_exp(const NodeBinExpr *bin_expr) {
        struct BinExprVisitor {
            Generator *gen;

            void operator()(const NodeBinExprAdd *add) const {
                gen->gen_expr(add->lhs);
                gen->gen_expr(add->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    add rax, rbx" << std::endl;
                gen->push("rax");
            }

            void operator()(const NodeBinExprMulti *multi) const {
                gen->gen_expr(multi->lhs);
                gen->gen_expr(multi->rhs);
                gen->pop("rax");
                gen->pop("rbx");
                gen->m_output << "    mul rax, rbx" << std::endl;
                gen->push("rax");
            }
        };
        BinExprVisitor visitor({.gen = this});
        std::visit(visitor, bin_expr->var);
    }

    void gen_expr(const NodeExpr *expr) {
        struct ExprVisitor {

            Generator *gen;

            void operator()(const NodeTerm *node_term) const {
                gen->gen_term(node_term);
            }

            void operator()(const NodeBinExpr *bin_expr) const {

                gen->gen_bin_exp(bin_expr);
            }
        };

        ExprVisitor visitor({.gen = this});
        std::visit(visitor, expr->var);
    }

    void gen_stmt(const NodeStmt *stmt) {
        struct StmtVisitor {
            Generator *gen;
            void operator()(const NodeStmtExit *stmt_exit) {

                gen->gen_expr(stmt_exit->expr);
                gen->m_output << "    mov rax, 60" << std::endl;
                gen->pop("rdi");
                // gen->m_output << "    pop rdi" << std::endl;
                // gen->m_output << "    mov rdi, " << "" << std::endl; // TODO:
                gen->m_output << "    syscall" << std::endl;
            }
            void operator()(const NodeStmtLet *stmt_let) {
                if (gen->m_vars.contains(stmt_let->ident.value.value())) {
                    std::cerr << "Identifire Already used:"
                              << stmt_let->ident.value.value() << std::endl;
                    std::exit(EXIT_FAILURE);
                }

                gen->m_vars.insert({stmt_let->ident.value.value(),
                                    Var{.stack_loc = gen->m_stack_loc}});
                gen->gen_expr(stmt_let->expr);
                // gen->push("")
            }
        };

        StmtVisitor visitor({.gen = this});
        std::visit(visitor, stmt->var);
    }

    std::string gen_prog() {
        /* Start */
        m_output << "global _start" << std::endl;
        m_output << "_start:" << std::endl;
        /* Generated Code start */

        for (const NodeStmt *stmt : m_prog.stmts) {
            gen_stmt(stmt); // Generated Code
        }

        /* Generated Code start */
        /* Exit Block start */
        m_output << "    mov rax, 60" << std::endl;
        m_output << "    mov rdi, 0" << std::endl;
        m_output << "    syscall" << std::endl;
        /* Exit Block Ends */
        /* End */

        return m_output.str();
    }

  private:
    void push(const std::string &reg) {
        m_output << "    push " << reg << std::endl;
        m_stack_loc++;
    }

    void pop(const std::string &reg) {
        m_output << "    pop " << reg << std::endl;
        m_stack_loc--;
    }

    struct Var {
        size_t stack_loc;
    };

    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_loc = 0;

    std::unordered_map<std::string, Var> m_vars{};
};