//
// Created by idrol on 01/05/2022.
//
#include "parser.h"
#include <unordered_map>
#include <unordered_set>

std::unordered_map<std::string, size_t> operatorPrecedence {
        {"==", 1}, {"!=", 1}, {">=", 1}, {"<=", 1}, {"<", 1}, {">", 1},
        {"+", 2}, {"-", 2},
        {"*", 3}, {"/", 3}
};

enum class IdentifierType {
    INVALID,
    FUNCTION,
    VARIABLE
};

std::unordered_map<std::string, IdentifierType> declaredIdentifiers;

void add_declaration(const std::string& str, IdentifierType type) {
    declaredIdentifiers[str] = type;
}

void token_error(const Token& token) {
    fprintf(stderr, "Unexpected token at line:column %zu:%zu\n", token.line, token.column);
    exit(-1);
}

void assert_token(const Token& token, const EToken& eToken, const std::string& value) {
    if(token.token != eToken || token.value != value) {
        token_error(token);
    }
}

void assert_token_type(const Token& token, const EToken& eToken) {
    if(token.token != eToken) {
        token_error(token);
    }
}

IdentifierType get_identifier_declaration(const std::string& str) {
    auto it = declaredIdentifiers.find(str);
    if(it == declaredIdentifiers.end()) return IdentifierType::INVALID;
    return it->second;
}

void assert_declaration_type(const Token& token, const IdentifierType& type) {
    assert_token_type(token, EToken::IDENTIFIER);
    auto it = declaredIdentifiers.find(token.value);
    if(it != declaredIdentifiers.end()) {
        if(it->second == type) {
            return;
        }
    }
    token_error(token);
}

bool is_declared(const std::string& str) {
    return get_identifier_declaration(str) != IdentifierType::INVALID;
}

size_t lengthUntilNewLine(std::vector<Token> tokens, size_t offset) {
    size_t newLineNum = 0;
    while(tokens[offset+newLineNum].token != EToken::NEWLINE && offset+newLineNum != tokens.size()) newLineNum++;
    return newLineNum;
}

void parserError(const char* error) {
    fprintf(stderr, "Parser error: %s\n", error);
    exit(-1);
}

OperatorType parseOperator(Token token) {
    if(token.token != EToken::OPERATOR) {
        return OperatorType::INVALID;
    }
    if(token.value == "+") {
        return OperatorType::ADD;
    } else if(token.value == "-") {
        return OperatorType::SUB;
    } else if(token.value == "*") {
        return OperatorType::MUL;
    } else if(token.value == "/") {
        return OperatorType::DIV;
    } else if(token.value == "==") {
        return OperatorType::EQUALS;
    } else if(token.value == "!=") {
        return OperatorType::NOT_EQUALS;
    } else if(token.value == "<=") {
        return OperatorType::LESS_EQUALS;
    } else if(token.value == ">=") {
        return OperatorType::LARGER_EQUALS;
    } else if(token.value == "<") {
        return OperatorType::LESS_THAN;
    } else if(token.value == ">") {
        return OperatorType::LARGER_THAN;
    }
    return OperatorType::INVALID;
}

size_t getOperatorPrecedence(std::string op1) {
    auto it1 = operatorPrecedence.find(op1);
    if(it1 == operatorPrecedence.end()) {
        fprintf(stderr, "Unexpected operator %s\n", op1.c_str());
        exit(-1);
    }
    return it1->second;
}

bool hasOperatorPrecedence(std::string op1, std::string op2) {
    auto it1 = operatorPrecedence.find(op1);
    if(it1 == operatorPrecedence.end()) {
        fprintf(stderr, "Unexpected operator %s\n", op1.c_str());
        exit(-1);
    }
    auto it2 = operatorPrecedence.find(op2);
    if(it2 == operatorPrecedence.end()) {
        fprintf(stderr, "Unexpected operator %s\n", op2.c_str());
        exit(-1);
    }

    return it1->second >= it2->second;
}

bool is_valid_expression_operand(EToken operand) {
    switch (operand) {
        case EToken::NUMBER:
        case EToken::IDENTIFIER:
            return true;
        default:
            return false;
    }
}

std::shared_ptr<FunctionCallNode> parseFunctionCall(std::vector<Token> tokens, size_t& offset);

std::shared_ptr<Node> parse_expression_operand(std::vector<Token> tokens, size_t& offset) {
    auto token = tokens[offset];

    if(token.token == EToken::NUMBER) {
        auto literal = std::make_shared<NumberNode>();
        literal->value = stoi(token.value);
        return literal;
    } else if(token.token == EToken::IDENTIFIER) {
        if(!is_declared(token.value)) {
            fprintf(stderr, "%s has not been declared\n", token.value.c_str());
            exit(-1);
        }
        auto type = get_identifier_declaration(token.value);
        if(type == IdentifierType::VARIABLE) {
            auto identifier = std::make_shared<IdentifierNode>();
            identifier->identifier = token.value;
            return identifier;
        } else if(type == IdentifierType::FUNCTION) {
            return parseFunctionCall(tokens, offset);
        } else {
            token_error(token);
        }

    } else {
        fprintf(stderr, "Invalid expression operand\n");
        exit(-1);
    }
    return nullptr;
}

bool is_valid_expression_end_token(const Token& token) {
    switch (token.token) {
        case EToken::NEWLINE:
            return true;
        case EToken::KEYWORD:
            if(token.value == "then" || token.value == "do") return true;
            return false;
        case EToken::LIST_SEPARATOR:
            if(token.value == ",") return true;
            return false;
        case EToken::OPERATOR:
            if(token.value == ")") return true;
            return false;
        default:
            return false;
    }
}

// (2*3)+1
// 2+(3*1)

bool is_operand_token_type(const Token& token) {
    switch (token.token) {
        case EToken::IDENTIFIER:
        case EToken::NUMBER:
            return true;
        default:
            return false;
    }
}

// 2+(3+2)*9
// 2+(3+2+1)*9

//       BinOp
// Number, +, Expression
//              BinOp
//        Number, +, Number
//
// Number, +, BinOp
//        BinOp * Number
//  Number, +, Number
//

// (2+2)

// 2*3+1

// BinOp
// Number * BinOp
//

// 2+2+3
// BinOp = 2+BinOp
// BinOp = 2+3


std::shared_ptr<Node> parsePrefixExpression(std::vector<Token> tokens, size_t& offset);

std::shared_ptr<Node> parseBinaryOp(std::vector<Token> tokens, size_t& offset) {
    std::shared_ptr<Node> leftNode;
    auto leftToken = tokens[offset];

    if(is_valid_expression_operand(leftToken.token)) {
        leftNode = parse_expression_operand(tokens, offset);
        offset++;

        if (tokens[offset].token == EToken::NEWLINE)  {
            offset++;
            return leftNode;
        }
    } else if(tokens[offset].token == EToken::OPERATOR && tokens[offset].value == "(") {
        leftNode = parsePrefixExpression(tokens, offset);
    } else {
        // Expected valid operand token
        token_error(leftToken);
    }

    OperatorType opType = parseOperator(tokens[offset]);
    if(opType == OperatorType::INVALID) {
        return leftNode;
    }
    size_t operatorPrecedence = getOperatorPrecedence(tokens[offset].value);
    offset++;

    auto binOp = std::make_shared<BinaryOperation>();
    binOp->left = leftNode;
    binOp->op = opType;
    binOp->precedence = operatorPrecedence;

    auto rightNode = parseBinaryOp(tokens, offset);
//    if(rightNode->type == NodeType::BINARY_OPERATION) {
//        auto rightBinOp = std::reinterpret_pointer_cast<BinaryOperation>(rightNode);
//        if(binOp->precedence > rightBinOp->precedence) {
//            // Rebalance this operator has higher precedence
//            binOp->right = rightBinOp->left;
//            rightBinOp->left = binOp;
//            return rightBinOp;
//        }
//    }
    binOp->right = rightNode;

    return binOp;
}

std::shared_ptr<BinaryOperation> balanceBinaryOpTree(std::shared_ptr<BinaryOperation> binaryOp) {
    auto root = binaryOp;
    if(root->right->type == NodeType::BINARY_OPERATION) {
        auto rightBinaryOp = std::reinterpret_pointer_cast<BinaryOperation>(root->right);
        if(root->precedence >= rightBinaryOp->precedence) {
            root->right = rightBinaryOp->left;
            rightBinaryOp->left = root;
            root = balanceBinaryOpTree(rightBinaryOp);
        } else {
            root->right = balanceBinaryOpTree(rightBinaryOp);
        }
    }
    return root;
}

std::shared_ptr<Node> balanceBinaryOp(std::shared_ptr<Node> node) {
    if(node->type == NodeType::BINARY_OPERATION) return balanceBinaryOpTree(std::reinterpret_pointer_cast<BinaryOperation>(node));
    return node;
}


std::shared_ptr<Node> parsePrefixExpression(std::vector<Token> tokens, size_t& offset) {
    assert_token(tokens[offset], EToken::OPERATOR, "(");
    offset++;
    auto prefixNode = std::make_shared<PrefixExpression>();
    prefixNode->operation = balanceBinaryOp(parseBinaryOp(tokens, offset));
    assert_token(tokens[offset], EToken::OPERATOR, ")");
    offset++;
    return prefixNode;
}

std::shared_ptr<ExpressionNode> parseExpression(std::vector<Token> tokens, size_t& offset) {
    auto expression = std::make_shared<ExpressionNode>();

    switch (tokens[offset].token) {
        case EToken::IDENTIFIER:
        case EToken::NUMBER:
            expression->operation = balanceBinaryOp(parseBinaryOp(tokens, offset));
            return expression;
        case EToken::OPERATOR:
            expression->operation = parsePrefixExpression(tokens, offset);
            return expression;
        default:
            token_error(tokens[offset]);
            return nullptr;
    }
}

std::vector<std::shared_ptr<ExpressionNode>> parseExpressionList(std::vector<Token> tokens, size_t& offset) {
    std::vector<std::shared_ptr<ExpressionNode>> expressionList;

    while(true) {
        expressionList.push_back(parseExpression(tokens, offset));

        if(tokens[offset].token != EToken::OPERATOR || tokens[offset].value != ",") {
            break;
        }
    }
    return expressionList;
}

DataType parseDataType(std::string type) {
    if(type == "int") {
        return DataType::INT;
    } else if(type == "bool") {
        return DataType::BOOL;
    }

    fprintf(stderr, "Unknown data type %s\n", type.c_str());
    exit(-1);
}

size_t findNextToken(std::vector<Token> tokens, EToken eToken, std::string value, size_t offset, size_t max = 0) {
    size_t endOffset = offset+1;
    while(endOffset < tokens.size()) {
        if(max > 0 && endOffset == max) return SIZE_MAX;
        if(tokens[endOffset].token == eToken && tokens[endOffset].value == value) {
            return endOffset;
        }
        endOffset++;
    }
    return SIZE_MAX;
}

size_t findBlockEnd(std::vector<Token> tokens, size_t offset) {
    size_t activeSubBlocks = 0;
    size_t currentOffset = offset;
    while(currentOffset < tokens.size()) {
        auto token = tokens[currentOffset];
        if(token.token == EToken::KEYWORD) {
            if(token.value == "end") {
                if(activeSubBlocks == 0) {
                    return currentOffset;
                } else {
                    activeSubBlocks--;
                }
            } else if(token.value == "do" || token.value == "then") {
                activeSubBlocks++;
            }
        }
        currentOffset++;
    }
    return SIZE_MAX;
}

std::shared_ptr<AssignmentNode> parseAssignment(std::vector<Token> tokens, size_t& offset) {
    assert_token(tokens[offset+1], EToken::OPERATOR, "=");

    auto assignment = std::make_shared<AssignmentNode>();
    assignment->type = NodeType::ASSIGNMENT;

    if(!is_declared(tokens[offset].value)) {
        fprintf(stderr, "%s has not been declared\n", tokens[offset].value.c_str());
        exit(-1);
    }
    assignment->name = tokens[offset].value;
    offset += 2;
    assignment->expression = parseExpression(tokens, offset);
    return assignment;
}

std::shared_ptr<DeclarationNode> parseVariableDeclaration(std::vector<Token> tokens, size_t& offset, bool isGlobal = false) {
    auto declaration = std::make_shared<DeclarationNode>();
    declaration->type = NodeType::DECLARATION;

    declaration->dataType = parseDataType(tokens[offset].value);
    declaration->name = tokens[offset+1].value;
    declaration->isGlobal = isGlobal;

    add_declaration(declaration->name, IdentifierType::VARIABLE);

    offset += 2;

    auto nextToken = tokens[offset];

    if(nextToken.token == EToken::OPERATOR && nextToken.value == "=") {
        // Assignment included
        offset++;
        declaration->defaultValueExpression = parseExpression(tokens, offset);
    }
    return declaration;
}

std::vector<std::shared_ptr<DeclarationNode>> parseVariableDeclarationList(std::vector<Token> tokens, size_t& offset) {
    std::vector<std::shared_ptr<DeclarationNode>> declarationList;

    while(true) {
        declarationList.push_back(parseVariableDeclaration(tokens, offset));

        if(tokens[offset].token != EToken::OPERATOR || tokens[offset].value != ",") {
            break;
        }
    }

    return declarationList;
}



std::vector<std::shared_ptr<ExpressionNode>> parseParameters(std::vector<Token> tokens, size_t offset) {
    assert_token(tokens[offset], EToken::OPERATOR, "(");
    offset++;

    auto expressionList = parseExpressionList(tokens, offset);

    assert_token(tokens[offset], EToken::OPERATOR, ")");

    return expressionList;
}

std::vector<std::shared_ptr<DeclarationNode>> parseParameterDeclarations(std::vector<Token> tokens, size_t& offset) {
    assert_token(tokens[offset], EToken::OPERATOR, "(");
    offset++;

    if(tokens[offset].token == EToken::OPERATOR && tokens[offset].value == ")") {
        offset++;
        return {};
    }

    auto expressionList = parseVariableDeclarationList(tokens, offset);

    assert_token(tokens[offset], EToken::OPERATOR, ")");
    offset++;

    return expressionList;
}

std::shared_ptr<BlockNode> parseBlock(std::vector<Token> tokens, size_t& offset);

std::shared_ptr<FunctionDeclarationNode> parseFunctionDeclaration(std::vector<Token> tokens, size_t& offset) {
    auto declaration = std::make_shared<FunctionDeclarationNode>();

    declaration->returnType = parseDataType(tokens[offset].value);
    offset++;
    declaration->functionName = tokens[offset].value;
    offset++;

    add_declaration(declaration->functionName, IdentifierType::FUNCTION);

    declaration->paramDeclarations = parseParameterDeclarations(tokens, offset);

    assert_token(tokens[offset], EToken::KEYWORD, "do");
    offset++;
    assert_token_type(tokens[offset], EToken::NEWLINE);
    offset++;

    declaration->functionBlock = parseBlock(tokens, offset);

    assert_token(tokens[offset], EToken::KEYWORD, "end");
    offset++;
    assert_token_type(tokens[offset], EToken::NEWLINE);
    offset++;

    return declaration;
}

std::shared_ptr<StatementNode> parseStatementDeclaration(std::vector<Token> tokens, size_t& offset) {
    bool isGlobal = false;
    if(tokens[offset].token == EToken::KEYWORD && tokens[offset].value == "global") {
        isGlobal = true;
        offset++;
        if(tokens[offset+2].token == EToken::OPERATOR && tokens[offset+2].value == "(") {
            token_error(tokens[offset+2]); // Global invalid for functions
        }
    }

    assert_token_type(tokens[offset], EToken::TYPE);
    assert_token_type(tokens[offset+1], EToken::IDENTIFIER);

    if(tokens[offset+2].token == EToken::OPERATOR) {
        if (tokens[offset + 2].value == "=") {
            return parseVariableDeclaration(tokens, offset, isGlobal);
        } else if (tokens[offset + 2].value == "(") {
            return parseFunctionDeclaration(tokens, offset);
        }
    }
    token_error(tokens[offset+2]);
}

bool is_token_keyword(Token token, std::string keyword) {
    if(token.token != EToken::KEYWORD) return false;
    if(token.value == keyword) return true;
    return false;
}

std::shared_ptr<StatementNode> parseStatement(std::vector<Token> tokens, size_t& offset);

std::shared_ptr<BlockNode> parseBlock(std::vector<Token> tokens, size_t& offset) {
    auto blockNode = std::make_shared<BlockNode>();
    while(true) {
        if(tokens[offset].token == EToken::NEWLINE) {
            offset++;
            continue;
        }
        blockNode->statements.push_back(parseStatement(tokens, offset));

        if(tokens[offset].token == EToken::KEYWORD && (tokens[offset].value == "else" || tokens[offset].value == "end")) break;
    }

    return blockNode;
}

std::shared_ptr<BranchNode> parseBranch(std::vector<Token> tokens, size_t& offset) {
    assert_token(tokens[offset], EToken::KEYWORD, "if");

    auto node = std::make_shared<BranchNode>();
    offset++; // If keyword consumed

    node->expression = parseExpression(tokens, offset);

    assert_token(tokens[offset], EToken::KEYWORD, "then");
    offset++;
    assert_token_type(tokens[offset], EToken::NEWLINE);
    offset++;

    node->trueBlock = parseBlock(tokens, offset);

    if(tokens[offset].token == EToken::KEYWORD && tokens[offset].value == "else") {
        offset++;
        assert_token_type(tokens[offset], EToken::NEWLINE);
        offset++;
        node->falseBlock = parseBlock(tokens, offset);
    }

    assert_token(tokens[offset], EToken::KEYWORD, "end");
    offset++;
    assert_token_type(tokens[offset], EToken::NEWLINE);
    offset++;

    return node;
}

std::shared_ptr<FunctionCallNode> parseFunctionCall(std::vector<Token> tokens, size_t& offset) {
    auto functionCall = std::make_shared<FunctionCallNode>();

    functionCall->functionIdentifier = tokens[offset].value;
    offset++;
    assert_token(tokens[offset], EToken::OPERATOR, "(");
    offset++;
    functionCall->argumentsList = parseExpressionList(tokens, offset);
    assert_token(tokens[offset], EToken::OPERATOR, ")");
    offset++;
    return functionCall;
}

std::shared_ptr<StatementNode> parseStatementIdentifier(std::vector<Token> tokens, size_t& offset) {
    auto nextToken = tokens[offset+1];
    if(nextToken.token == EToken::OPERATOR) {
        if(nextToken.value == "(") {
            // Function call
            assert_declaration_type(tokens[offset], IdentifierType::FUNCTION);
            return parseFunctionCall(tokens, offset);
        } else if(nextToken.value == "=") {
            // Assignment
            assert_declaration_type(tokens[offset], IdentifierType::VARIABLE);
            return parseAssignment(tokens, offset);
        }
    }
    token_error(nextToken);
}

std::shared_ptr<LastStatementNode> parseLastStatement(std::vector<Token> tokens, size_t& offset) {
    auto lastStatement = std::make_shared<LastStatementNode>();
    assert_token_type(tokens[offset], EToken::KEYWORD);
    if(tokens[offset].value == "return") {
        offset++;
        if(tokens[offset].token != EToken::NEWLINE) {
            lastStatement->returnExpr = parseExpression(tokens, offset);
        } else {
            assert_token_type(tokens[offset], EToken::NEWLINE);
            offset++;
        }
        return lastStatement;
    } else if(tokens[offset].value == "break") {
        offset++;
        assert_token_type(tokens[offset], EToken::NEWLINE);
        offset++;
        return lastStatement;
    }
    token_error(tokens[offset]);
    return nullptr;
}

std::shared_ptr<StatementNode> parseStatement(std::vector<Token> tokens, size_t& offset) {
    switch (tokens[offset].token) {
        case EToken::TYPE:
            return parseStatementDeclaration(tokens, offset);
        case EToken::IDENTIFIER:
            return parseStatementIdentifier(tokens, offset);
        case EToken::KEYWORD:
            if(tokens[offset].value == "global") {
                return parseStatementDeclaration(tokens, offset);
            } else if(tokens[offset].value == "if") {
                return parseBranch(tokens, offset);
            } else if(tokens[offset].value == "return" || tokens[offset].value == "break") {
                return parseLastStatement(tokens, offset);
            } else {
                parserError("Unsupported keyword");
            }
        default:
            fprintf(stderr, "Unsupported token\n");
            exit(-1);
    }
}

std::shared_ptr<BlockNode> parseProgramBlock(std::vector<Token> tokens) {
    auto blockNode = std::make_shared<BlockNode>();
    size_t offset = 0;
    while(true) {
        if(tokens[offset].token == EToken::NEWLINE) {
            offset++;
            continue;
        }
        blockNode->statements.push_back(parseStatement(tokens, offset));

        if(offset >= (tokens.size()-1)) break;
    }

    //if(offset != (tokens.size()-1)) parserError("Program block did not consume correct amount of tokens");

    return blockNode;
}

std::shared_ptr<ProgramNode> parseTokens(std::vector<Token> tokens) {
    auto ast = std::make_shared<ProgramNode>();

    auto block = std::make_shared<BlockNode>();
    ast->programBlock = parseProgramBlock(tokens);

    return ast;
}

size_t indent = 0;

void print_indent() {
    for(int i = 0; i < indent; i++) {
        printf(" ");
    }
}

const char* get_type_name(DataType type) {
    switch (type) {
        case DataType::BOOL:
            return "bool";
        case DataType::INT:
            return "int";
        case DataType::FLOAT:
            return "float";
        case DataType::STRING:
            return "string";
        case DataType::VOID:
            return "void";
        default:
            return "unknown";
    }
}

void debugBlock(std::shared_ptr<BlockNode> node);

void debugFunctionDeclaration(std::shared_ptr<FunctionDeclarationNode> node) {
    printf("func_dec return_type='%s' name='%s', params=(TODO)\n", get_type_name(node->returnType), node->functionName.c_str());
    indent++;
    debugBlock(node->functionBlock);
    indent--;
}

void debug_number(std::shared_ptr<NumberNode> node) {
    printf("%i", node->value);
}

void debug_identifier(std::shared_ptr<IdentifierNode> node) {
    printf("%s", node->identifier.c_str());
}

void debug_expression(std::shared_ptr<ExpressionNode> expression);

void debug_prefix_expression(std::shared_ptr<PrefixExpression> expression) {
    printf("(");
    debug_expression(expression);
    printf(")");
}

void debug_binary_op(std::shared_ptr<BinaryOperation> binaryOp);

void debug_binary_operand(std::shared_ptr<Node> node) {
    switch (node->type) {
        case NodeType::NUMBER:
            debug_number(std::reinterpret_pointer_cast<NumberNode>(node));
            break;
        case NodeType::IDENTIFIER:
            debug_identifier(std::reinterpret_pointer_cast<IdentifierNode>(node));
            break;
        case NodeType::BINARY_OPERATION:
            debug_binary_op(std::reinterpret_pointer_cast<BinaryOperation>(node));
            break;
        case NodeType::PREFIX_EXPRESSION:
            debug_prefix_expression(std::reinterpret_pointer_cast<PrefixExpression>(node));
            break;
        default:
            printf("ERROR");
            break;
    }
}

const char* get_operator(OperatorType type) {
    switch (type) {
        case OperatorType::ADD:
            return "+";
        case OperatorType::SUB:
            return "-";
        case OperatorType::DIV:
            return "/";
        case OperatorType::MUL:
            return "*";
        case OperatorType::EQUALS:
            return "==";
        case OperatorType::NOT_EQUALS:
            return "!=";
        case OperatorType::LARGER_THAN:
            return ">";
        case OperatorType::LESS_THAN:
            return "<";
        case OperatorType::LARGER_EQUALS:
            return ">=";
        case OperatorType::LESS_EQUALS:
            return "<=";
        default:
            return "ERROR_OP";
    }
}

void debug_binary_op(std::shared_ptr<BinaryOperation> binaryOp) {
    printf("(");
    debug_binary_operand(binaryOp->left);
    printf("%s", get_operator(binaryOp->op));
    debug_binary_operand(binaryOp->right);
    printf(")");
}

void debug_expression(std::shared_ptr<ExpressionNode> expression) {
    print_indent();
    printf("[expression]");
    debug_binary_operand(expression->operation);
    printf("\n");
}

void debug_vardec(std::shared_ptr<DeclarationNode> node) {
    printf("(var_dec type='%s' name='%s')", get_type_name(node->dataType), node->name.c_str());
    if(node->defaultValueExpression) {
        printf(" {expression_begin}\n");
        indent++;
        debug_expression(node->defaultValueExpression);
        indent--;
        print_indent();
        printf("{end}\n");
    }
}

void debugStatement(std::shared_ptr<StatementNode> statementNode) {
    print_indent();
    printf("[statement] ");
    switch (statementNode->type) {
        case NodeType::ASSIGNMENT:
            printf("assignment\n");
            return;
        case NodeType::FUNCTION_CALL:
            printf("function_call\n");
            return;
        case NodeType::DECLARATION:
            debug_vardec(std::reinterpret_pointer_cast<DeclarationNode>(statementNode));
            return;
        case NodeType::FUNCTION_DECLARATION:
            indent++;
            debugFunctionDeclaration(std::reinterpret_pointer_cast<FunctionDeclarationNode>(statementNode));
            indent--;
            return;
        case NodeType::BRANCH:
            printf("branch\n");
            return;
        case NodeType::LAST_STATEMENT:
            printf("return/break\n");
            return;
        default:
            printf("PARSER BUG!!!\n");
            return;
    }
}

void debugBlock(std::shared_ptr<BlockNode> node) {
    print_indent();
    printf("[block]\n");

    indent++;
    for(auto statement: node->statements) {
        debugStatement(statement);
    }
    indent--;
}

void debugAst(std::shared_ptr<ProgramNode> node) {
    setbuf(stdout, NULL);
    printf("[program]\n");
    indent++;
    debugBlock(node->programBlock);
    indent--;
    printf("[end_program]\n");
}