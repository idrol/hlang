//
// Created by idrol on 05/05/2022.
//
#include "interpreter.h"
#include <unordered_map>
#include <stack>

std::vector<std::unordered_map<std::string, Variable>> variableScopes;

// Valid until next hlang call
Variable* resolve_variable(std::string varName) {
    for(int i = variableScopes.size()-1; i >= 0; i--) {
        auto it = variableScopes[i].find(varName);
        if(it != variableScopes[i].end()) {
            return &it->second;
        }
    }
    return nullptr;
}

int run_op(int num1, OperatorType opType, int num2) {
    switch (opType) {
        default:
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
        case OperatorType::LESS_THAN:
            return num1<num2;
        case OperatorType::LARGER_THAN:
            return num1>num2;
        case OperatorType::LESS_EQUALS:
            return num1<=num2;
        case OperatorType::LARGER_EQUALS:
            return num1>=num2;
        case OperatorType::EQUALS:
            return num1==num2;
        case OperatorType::NOT_EQUALS:
            return num1!=num2;
    }
}

HInt run_binary_operation(std::shared_ptr<BinaryOperation> binaryOp);

HInt run_binary_operand(std::shared_ptr<Node> node) {
    if(node->type == NodeType::NUMBER) {
        return std::reinterpret_pointer_cast<NumberNode>(node)->value;
    } else if(node->type == NodeType::IDENTIFIER) {
        auto identifier = std::reinterpret_pointer_cast<IdentifierNode>(node);
        auto var = resolve_variable(identifier->identifier);
        if(!var) {
            fprintf(stderr, "%s has not been declared\n", identifier->identifier.c_str());
            exit(-1);
        }
        return var->GetValue<HInt>();
    } else if(node->type == NodeType::BINARY_OPERATION) {
        return run_binary_operation(std::reinterpret_pointer_cast<BinaryOperation>(node));
    } else if(node->type == NodeType::PREFIX_EXPRESSION) {
        return run_expression(std::reinterpret_pointer_cast<ExpressionNode>(node));
    }
    // TODO add function call
    fprintf(stderr, "Node type is not supported as expression operand\n");
    exit(-1);
}

HInt run_binary_operation(std::shared_ptr<BinaryOperation> binaryOp) {
    HInt leftValue = run_binary_operand(binaryOp->left);
    HInt rightValue = run_binary_operand(binaryOp->right);

    return run_op(leftValue, binaryOp->op, rightValue);
}

HInt run_expression(std::shared_ptr<ExpressionNode> node) {
    if(node->type != NodeType::EXPRESSION) {
        fprintf(stderr, "Passed node is not an expression!\n");
        exit(-1);
    }

    return run_binary_operand(node->operation);
}


Variable allocDataType(DataType type) {
    switch (type) {
        case DataType::FLOAT:
        case DataType::STRING:
            fprintf(stderr, "Unsupported data type in allocation\n");
            exit(-1);
        case DataType::INT:
            return Variable(sizeof(int));
        case DataType::BOOL:
            return Variable(sizeof(bool));
    }
}

void run_declaration(std::shared_ptr<DeclarationNode> node) {
    auto& scope = variableScopes[variableScopes.size()-1];
    scope[node->name] = allocDataType(node->dataType);
}

void run_assignment(std::shared_ptr<AssignmentNode> node) {
    auto var = resolve_variable(node->name);
    if(!var) {
        fprintf(stderr, "%s has not been declared\n", node->name.c_str());
        exit(-1);
    }
    var->SetValue(run_expression(node->expression));
}

void run_statement(std::shared_ptr<StatementNode> statementNode) {

    if(statementNode->type == NodeType::DECLARATION) {
        run_declaration(std::reinterpret_pointer_cast<DeclarationNode>(statementNode));
    } else if(statementNode->type == NodeType::ASSIGNMENT) {
        run_assignment(std::reinterpret_pointer_cast<AssignmentNode>(statementNode));
    } else {
        fprintf(stderr, "Invalid node found inside of statement node\n");
        exit(-1);
    }
}

void run_block(std::shared_ptr<BlockNode> block);

void run_branch(std::shared_ptr<BranchNode> branch) {
    HInt compareValue = run_expression(branch->expression);
    if(compareValue == 0) {
        run_block(branch->falseBlock);
    } else {
        run_block(branch->trueBlock);
    }
}

void run_block(std::shared_ptr<BlockNode> block) {
    variableScopes.emplace_back();
    for(auto &node: block->statements) {
        if(node->type == NodeType::STATEMENT) {
            run_statement(node);
        } else if(node->type == NodeType::BRANCH) {
            run_branch(std::reinterpret_pointer_cast<BranchNode>(node));
        }
    }
    variableScopes.pop_back();
}

void run_program(std::shared_ptr<ProgramNode> node) {
    variableScopes.emplace_back(); // Setup global scope
    run_block(node->programBlock);
}

HInt get_int_var(std::string varName) {
    auto var = resolve_variable(varName);
    if(var) var->GetValue<HInt>();
    fprintf(stderr, "Variable %s does not exist returning 0\n", varName.c_str());
    return 0;
}

HBool get_bool_var(std::string varName) {
    auto var = resolve_variable(varName);
    if(var) return var->GetValue<HBool>();
    fprintf(stderr, "Variable %s does not exist returning false\n", varName.c_str());
    return false;
}