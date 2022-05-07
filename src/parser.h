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
    EXPRESSION,
    OPERATOR,
    NUMBER,
    DECLARATION,
    ASSIGNMENT
};

enum class OperatorType {
    INVALID,
    ADD,
    MUL,
    DIV,
    SUB
};

enum class DataType {
    BOOL,
    INT,
    FLOAT,
    STRING
};

class Node {
public:
    Node(std::shared_ptr<Node> parent): parent(std::move(parent)) {};
    NodeType type;
    std::shared_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> leafs;
};

class NumberNode: public Node {
public:
    NumberNode(std::shared_ptr<Node> parent): Node(std::move(parent)) {};
    size_t value;
};

class OperatorNode: public Node {
public:
    OperatorNode(std::shared_ptr<Node> parent): Node(std::move(parent)) {};
    OperatorType opType;
};

class DeclarationNode: public Node {
public:
    DeclarationNode(std::shared_ptr<Node> parent): Node(std::move(parent)) {};
    DataType dataType;
    std::string name;
};

class AssignmentNode: public Node {
public:
    AssignmentNode(std::shared_ptr<Node> parent): Node(std::move(parent)) {};
    std::string name;
};

std::shared_ptr<Node> parseTokens(std::vector<Token> tokens);