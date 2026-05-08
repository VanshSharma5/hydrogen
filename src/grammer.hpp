#pragma once

#include "tokenization.hpp"
#include <variant>

struct NodeExpr;

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeTermParen {
    NodeExpr *expr;
};

struct NodeBinExprAdd {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprSub {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprMulti {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv {
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr {
    // NodeBinExprAdd* add;
    std::variant<NodeBinExprAdd *, NodeBinExprMulti *, NodeBinExprDiv *,
                 NodeBinExprSub *>
        var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeTerm *, NodeBinExpr *> var;
};

struct NodeStmtExit {
    NodeExpr *expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr *expr{};
};

struct NodeStmt;

struct NodeStmt {
    std::variant<NodeStmtExit *, NodeStmtLet *> var;
};

struct NodeProg {
    std::vector<NodeStmt *> stmts;
};