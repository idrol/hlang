//
// Created by idrol on 01/05/2022.
//
#pragma once

#include <utility>
#include <vector>
#include <memory>
#include "tokenizer.h"

enum class NodeType {
    PROGRAM,
    STATEMENT,
    LAST_STATEMENT,
    EXPRESSION,
    PREFIX_EXPRESSION,
    BINARY_OPERATION,
    IDENTIFIER,
    NUMBER,
    DECLARATION,
    FUNCTION_DECLARATION,
    FUNCTION_CALL,
    ASSIGNMENT,
    BLOCK,
    BRANCH
};

enum class OperatorType {
    INVALID,
    ADD,
    MUL,
    DIV,
    SUB,
    EQUALS,
    LESS_EQUALS,
    LARGER_EQUALS,
    LESS_THAN,
    LARGER_THAN,
    NOT_EQUALS
};

enum class DataType {
    VOID,
    BOOL,
    INT,
    FLOAT,
    STRING
};

struct ParamDeclaration {
    DataType type;
    std::string name;
    size_t stackBaseOffset;
};

struct VariableDeclaration {
    DataType type;
    std::string name;
    size_t stackBaseOffset;
};

class Node {
public:
    Node() {};
    NodeType type;
};

class ExpressionNode: public Node {
public:
    ExpressionNode() {
        type = NodeType::EXPRESSION;
    };

    std::shared_ptr<Node> operation;
};

class PrefixExpression: public ExpressionNode {
public:
    PrefixExpression() {
        type = NodeType::PREFIX_EXPRESSION;
    }
};

class BinaryOperation: public Node {
public:
    BinaryOperation() {
        type = NodeType::BINARY_OPERATION;
    }

    std::shared_ptr<Node> left, right;
    OperatorType op;
    size_t precedence;
};

class IdentifierNode: public Node {
public:
    IdentifierNode() {
        type = NodeType::IDENTIFIER;
    };

    std::string identifier;
};

class NumberNode: public Node {
public:
    NumberNode() {
        type = NodeType::NUMBER;
    };

    int value;
};

class StatementNode: public Node {
public:
    StatementNode() {
        type = NodeType::STATEMENT;
    }
};

class LastStatementNode: public StatementNode {
public:
    LastStatementNode() {
        type = NodeType::LAST_STATEMENT;
    }

    std::shared_ptr<ExpressionNode> returnExpr; // Optional
};

class DeclarationNode: public StatementNode {
public:
    DeclarationNode() {
        type = NodeType::DECLARATION;
    };
    bool isGlobal = false;
    DataType dataType;
    std::string name;
    std::shared_ptr<ExpressionNode> defaultValueExpression;
};


class AssignmentNode: public StatementNode {
public:
    AssignmentNode() {
        type = NodeType::ASSIGNMENT;
    };
    std::string name;
    std::shared_ptr<ExpressionNode> expression;
};

class BlockNode: public Node {
public:
    BlockNode() {
        type = NodeType::BLOCK;
    };

    std::vector<std::shared_ptr<StatementNode>> statements;
};

class FunctionCallNode: public StatementNode {
public:
    FunctionCallNode() {
        type = NodeType::FUNCTION_CALL;
    }

    std::string functionIdentifier;
    std::vector<std::shared_ptr<ExpressionNode>> argumentsList;
};

class FunctionDeclarationNode: public StatementNode {
public:
    FunctionDeclarationNode() {
        type = NodeType::FUNCTION_DECLARATION;
    }

    int numParams;
    DataType returnType;
    std::string functionName;
    std::vector<std::shared_ptr<DeclarationNode>> paramDeclarations;
    std::shared_ptr<BlockNode> functionBlock;
};

class BranchNode: public StatementNode {
public:
    BranchNode() {
        type = NodeType::BRANCH;
    };
    std::shared_ptr<ExpressionNode> expression;
    std::shared_ptr<BlockNode> trueBlock;
    std::shared_ptr<BlockNode> falseBlock;
};

class ProgramNode: public Node {
public:
    ProgramNode() {
        type = NodeType::PROGRAM;
    };
    std::shared_ptr<BlockNode> programBlock;
};

std::shared_ptr<ProgramNode> parseTokens(std::vector<Token> tokens);
void debugAst(std::shared_ptr<ProgramNode> node);