//
// Created by idrol on 01/05/2022.
//
#include "parser.h"

size_t lengthUntilNewLine(std::vector<Token> tokens, size_t offset) {
    size_t newLineNum = 0;
    while(tokens[offset+newLineNum].token != EToken::NEWLINE && offset+newLineNum != tokens.size()) newLineNum++;
    return newLineNum;
}

OperatorType parseOperator(Token token) {
    if(token.token != EToken::OPERATOR) {
        fprintf(stderr, "Error: token is not operator\n");
        exit(-1);
    }
    if(token.value == "+") {
        return OperatorType::ADD;
    } else if(token.value == "-") {
        return OperatorType::SUB;
    } else if(token.value == "*") {
        return OperatorType::MUL;
    } else if(token.value == "/") {
        return OperatorType::DIV;
    }
    fprintf(stderr, "%s is an invalid operator\n", token.value.c_str());
    return OperatorType::INVALID;
}

bool hasOperatorPrecedence(char op1, char op2) {
    switch (op1) {
        case '+':
        case '-':
            return !(op2 == '*' || op2 == '/');
        case '*':
        case '/':
            return true;
        default:
            fprintf(stderr, "Unexpected operator %c\n", op1);
            exit(-1);
    }
}

size_t parseEquationTerm(std::shared_ptr<Node> parent, std::vector<Token> tokens, size_t offset) {
    if(tokens[offset].token == EToken::NEWLINE) return offset+1;
    size_t currentOffset = offset;
    size_t expectedTermLength;
    if(tokens[offset].token == EToken::NUMBER) {
        if(tokens[offset+1].token != EToken::OPERATOR || tokens[offset+2].token != EToken::NUMBER) {
            fprintf(stderr, "Unexpected token\n");
            exit(-1);
        }
        expectedTermLength = 3;
    } else if(tokens[offset].token == EToken::OPERATOR) {
        if(tokens[offset+1].token != EToken::NUMBER) {
            fprintf(stderr, "Unexpected token\n");
            exit(-1);
        }
        expectedTermLength = 2;
    } else {
        fprintf(stderr, "Unexpected token\n");
        exit(-1);
    }

    if(tokens[offset+expectedTermLength].token == EToken::OPERATOR) {
        // We have an operator after this term
        char op1 = tokens[offset+expectedTermLength-2].value[0];
        char op2 = tokens[offset+expectedTermLength].value[0];
        bool op1HasPrecedence = hasOperatorPrecedence(op1, op2);
        if(expectedTermLength == 3) {
            if(op1HasPrecedence) {
                auto literal = std::make_shared<NumberNode>(parent);
                literal->type = NodeType::NUMBER;
                literal->value = stoi(tokens[offset].value);
                parent->leafs.push_back(literal);

                auto opNode = std::make_shared<OperatorNode>(parent);
                opNode->type = NodeType::OPERATOR;
                opNode->opType = parseOperator(tokens[offset+1]);
                parent->leafs.push_back(opNode);

                literal = std::make_shared<NumberNode>(parent);
                literal->type = NodeType::NUMBER;
                literal->value = stoi(tokens[offset+2].value);
                parent->leafs.push_back(literal);

                return parseEquationTerm(parent, tokens, offset + 3);
            } else {
                auto literal = std::make_shared<NumberNode>(parent);
                literal->type = NodeType::NUMBER;
                literal->value = stoi(tokens[offset].value);
                parent->leafs.push_back(literal);

                auto opNode = std::make_shared<OperatorNode>(parent);
                opNode->type = NodeType::OPERATOR;
                opNode->opType = parseOperator(tokens[offset+1]);
                parent->leafs.push_back(opNode);

                auto termNode = std::make_shared<Node>(parent);
                termNode->type = NodeType::EXPRESSION;
                parent->leafs.push_back(termNode);

                literal = std::make_shared<NumberNode>(termNode);
                literal->type = NodeType::NUMBER;
                literal->value = stoi(tokens[offset+2].value);
                termNode->leafs.push_back(literal);

                return parseEquationTerm(termNode, tokens, offset + 3);
            }
        } else {
            if(op1HasPrecedence) {
                bool op2HasPrecedence = hasOperatorPrecedence(op2, op1);
                if(op2HasPrecedence) {
                    // op2 has same precedence as op1 just continue
                    auto opNode = std::make_shared<OperatorNode>(parent);
                    opNode->type = NodeType::OPERATOR;
                    opNode->opType = parseOperator(tokens[offset]);
                    parent->leafs.push_back(opNode);

                    auto literal = std::make_shared<NumberNode>(parent);
                    literal->type = NodeType::NUMBER;
                    literal->value = stoi(tokens[offset+1].value);
                    parent->leafs.push_back(literal);

                    return parseEquationTerm(parent, tokens, offset+2);
                } else {
                    // op2 has less precedence than op1
                    // pop the current term node

                    auto opNode = std::make_shared<OperatorNode>(parent);
                    opNode->type = NodeType::OPERATOR;
                    opNode->opType = parseOperator(tokens[offset]);
                    parent->leafs.push_back(opNode);

                    auto literal = std::make_shared<NumberNode>(parent);
                    literal->type = NodeType::NUMBER;
                    literal->value = stoi(tokens[offset+1].value);
                    parent->leafs.push_back(literal);

                    return parseEquationTerm(parent->parent, tokens, offset+2);
                }
            } else {
                auto opNode = std::make_shared<OperatorNode>(parent);
                opNode->type = NodeType::OPERATOR;
                opNode->opType = parseOperator(tokens[offset]);
                parent->leafs.push_back(opNode);

                auto termNode = std::make_shared<Node>(parent);
                termNode->type = NodeType::EXPRESSION;
                parent->leafs.push_back(termNode);

                auto literal = std::make_shared<NumberNode>(termNode);
                literal->type = NodeType::NUMBER;
                literal->value = stoi(tokens[offset+1].value);
                termNode->leafs.push_back(literal);

                return parseEquationTerm(termNode, tokens, offset+2);
            }
        }
    } else {
        // We are at the end of the equation no more recursive and operator precedence
       if(expectedTermLength == 3) {
           // Starts with number
           auto literal = std::make_shared<NumberNode>(parent);
           literal->type = NodeType::NUMBER;
           literal->value = stoi(tokens[offset].value);
           parent->leafs.push_back(literal);

           auto opNode = std::make_shared<OperatorNode>(parent);
           opNode->type = NodeType::OPERATOR;
           opNode->opType = parseOperator(tokens[offset+1]);
           parent->leafs.push_back(opNode);

           literal = std::make_shared<NumberNode>(parent);
           literal->type = NodeType::NUMBER;
           literal->value = stoi(tokens[offset+2].value);
           parent->leafs.push_back(literal);
       } else {
           // Starts with operator
           auto opNode = std::make_shared<OperatorNode>(parent);
           opNode->type = NodeType::OPERATOR;
           opNode->opType = parseOperator(tokens[offset]);
           parent->leafs.push_back(opNode);

           auto literal = std::make_shared<NumberNode>(parent);
           literal->type = NodeType::NUMBER;
           literal->value = stoi(tokens[offset+1].value);
           parent->leafs.push_back(literal);
       }
       return offset+expectedTermLength;
    }
}

size_t parseExpression(std::shared_ptr<Node> parent, std::vector<Token> tokens, size_t offset) {
    switch (tokens[offset].token) {
        case EToken::NUMBER:
            if(tokens[offset+1].token == EToken::OPERATOR) {
                auto expression = std::make_shared<Node>(parent);
                expression->type = NodeType::EXPRESSION;
                parent->leafs.push_back(expression);

                return parseEquationTerm(expression, tokens, offset);
            } else if(tokens[offset+1].token == EToken::NEWLINE) {
                auto literal = std::make_shared<NumberNode>(parent);
                literal->type = NodeType::NUMBER;
                literal->value = stoi(tokens[offset].value);
                parent->leafs.push_back(literal);
                return offset+2;
            } else {
                fprintf(stderr, "Unsupported token processing number\n");
            }
        case EToken::NEWLINE:
            return offset+1;
        default:
            fprintf(stderr, "Unsupported token\n");
            exit(-1);
    }
}

DataType parseDataType(std::string type) {
    if(type == "int") {
        return DataType::INT;
    }

    fprintf(stderr, "Unknown data type %s\n", type.c_str());
    exit(-1);
}

size_t parseAssignment(std::shared_ptr<Node> parent, std::vector<Token> tokens, size_t offset) {
    if(tokens[offset+1].token != EToken::OPERATOR) {
        fprintf(stderr, "Unexpected token\n");
        exit(-1);
    }

    auto assignment = std::make_shared<AssignmentNode>(parent);
    assignment->type = NodeType::ASSIGNMENT;
    parent->leafs.push_back(assignment);

    assignment->name = tokens[offset].value;

    if(tokens[offset+2].token == EToken::NUMBER) {
        if(tokens[offset+2].token != EToken::NEWLINE) {
            // Expression
            return parseExpression(assignment, tokens, offset+2);
        } else {
            // Number
            auto number = std::make_shared<NumberNode>(assignment);
            number->type = NodeType::NUMBER;
            number->value = stoi(tokens[offset+2].value);
            assignment->leafs.push_back(number);
            return offset+3;
        }
    } else {
        fprintf(stderr, "Unexpected token\n");
        exit(-1);
    }
}

size_t parseDeclaration(std::shared_ptr<Node> parent, std::vector<Token> tokens, size_t offset) {
    if(tokens[offset+1].token != EToken::IDENTIFIER) {
        fprintf(stderr, "Identifier expected after data type in variable declaration\n");
        exit(-1);
    }
    auto declaration = std::make_shared<DeclarationNode>(parent);
    declaration->type = NodeType::DECLARATION;
    parent->leafs.push_back(declaration);

    declaration->dataType = parseDataType(tokens[offset].value);
    declaration->name = tokens[offset+1].value;

    auto nextToken = tokens[offset+2];

    if(nextToken.token == EToken::OPERATOR && nextToken.value == "=") {
        // Assignment included
        return parseAssignment(parent, tokens, offset + 1);
    } else if(nextToken.token != EToken::NEWLINE) {
        fprintf(stderr, "Expected newline or = after variable declaration\n");
        exit(-1);
    }
    return offset + 2;
}

size_t parseStatement(std::shared_ptr<Node> parent, std::vector<Token> tokens, size_t offset) {
    auto statement = std::make_shared<Node>(parent);
    statement->type = NodeType::STATEMENT;
    parent->leafs.push_back(statement);

    switch (tokens[offset].token) {
        case EToken::TYPE:
            return parseDeclaration(statement, tokens, offset);
        case EToken::IDENTIFIER:
            return parseAssignment(statement, tokens, offset);
        default:
            fprintf(stderr, "Unsupported token\n");
            exit(-1);
    }
}

std::shared_ptr<Node> parseTokens(std::vector<Token> tokens) {
    auto ast = std::make_shared<Node>(nullptr);
    ast->type = NodeType::PROGRAM;

    size_t offset = 0;
    size_t lastOffset = 0;
    while(offset != tokens.size()) {
        lastOffset = offset;
        if(tokens[offset].token == EToken::NEWLINE) {
            offset++;
        } else {
            offset = parseStatement(ast, tokens, offset);
        }
        if(lastOffset == offset) {
            fprintf(stderr, "Infinite parse loop detected\n");
            exit(-1);
        }
    }

    return ast;
}