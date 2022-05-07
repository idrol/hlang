//
// Created by idrol on 05/05/2022.
//
#include "interpreter.h"
#include <unordered_map>
#include <stack>

size_t run_op(size_t num1, OperatorType opType, size_t num2) {
    switch (opType) {
        case OperatorType::INVALID:
            fprintf(stderr, "Invalid optype recieved\n");
            exit(-1);
        case OperatorType::ADD:
            return num1+num2;
        case OperatorType::MUL:
            return num1*num2;
        case OperatorType::DIV:
            return num1/num2;
        case OperatorType::SUB:
            return num1-num2;
    }
}

size_t run_expression(std::shared_ptr<Node> node) {
    if(node->type != NodeType::EXPRESSION) {
        fprintf(stderr, "Passed node is not an expression!\n");
        exit(-1);
    }

    size_t num = std::reinterpret_pointer_cast<NumberNode>(node->leafs[0])->value;
    size_t cursor = 1;
    while(cursor < node->leafs.size()) {
        auto opNode = std::reinterpret_pointer_cast<OperatorNode>(node->leafs[cursor]);
        size_t num2;
        if(node->leafs[cursor+1]->type == NodeType::EXPRESSION) {
            num2 = run_expression(node->leafs[cursor+1]);
        } else {
            num2 = std::reinterpret_pointer_cast<NumberNode>(node->leafs[cursor+1])->value;
        }
        num = run_op(num, opNode->opType, num2);
        cursor += 2;
    }

    return num;
}

std::unordered_map<std::string, Variable> globalVariables;
std::stack<std::unordered_map<std::string, Variable>> variableScopes;

Variable allocDataType(DataType type) {
    switch (type) {
        case DataType::BOOL:
        case DataType::FLOAT:
        case DataType::STRING:
            fprintf(stderr, "Unsupported\n");
            exit(-1);
        case DataType::INT:
            return Variable(sizeof(int));
    }
}

void run_declaration(std::shared_ptr<DeclarationNode> node) {
    globalVariables[node->name] = allocDataType(node->dataType);
}

void run_assignment(std::shared_ptr<AssignmentNode> node) {
    auto it = globalVariables.find(node->name);
    if(it == globalVariables.end()) {
        fprintf(stderr, "%s has not been declared\n");
        exit(-1);
    }
    size_t num = 0;
    if(node->leafs[0]->type == NodeType::NUMBER) {
        num = std::reinterpret_pointer_cast<NumberNode>(node->leafs[0])->value;
    } else {
        num = run_expression(node->leafs[0]);
    }

    globalVariables[node->name].SetValue(num);
}

void run_statement(std::shared_ptr<Node> node) {
    if(node->type != NodeType::STATEMENT) {
        fprintf(stderr, "Invalid node passed to run_statement\n");
        exit(-1);
    }
    for(auto& statementPart: node->leafs) {
        if(statementPart->type == NodeType::DECLARATION) {
            run_declaration(std::reinterpret_pointer_cast<DeclarationNode>(statementPart));
        } else if(statementPart->type == NodeType::ASSIGNMENT) {
            run_assignment(std::reinterpret_pointer_cast<AssignmentNode>(statementPart));
        } else {
            fprintf(stderr, "Invalid node found inside of statement node\n");
            exit(-1);
        }
    }
}

void run_program(std::shared_ptr<Node> node) {
    for(auto &statement: node->leafs) {
        run_statement(statement);
    }
}

void* get_var(std::string var) {
    auto it = globalVariables.find(var);
    if(it == globalVariables.end()) {
        return nullptr;
    }
    return globalVariables[var].value;
}