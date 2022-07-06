#include "tokenizer.h"
#include <cstdio>
#include <istream>
#include <fstream>
#include <unordered_set>

std::unordered_set<std::string> keywords = {
        "if", "then", "else", "true", "false", "end", "return", "do", "break", "global"
};

std::unordered_set<std::string> types = {
        "int", "bool", "string"
};

bool is_separator(char c) {
    switch (c) {
        case '=':
        case '+':
        case '-':
        case '/':
        case '*':
        case '<':
        case '>':
        case '!':
        case '(':
        case ')':
        case ',':
        case ';':
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        default:
            return false;
    }
}

bool is_operator(char c) {
    switch (c) {
        case '=':
        case '+':
        case '-':
        case '/':
        case '*':
        case '<':
        case '>':
        case '!':
        case '(':
        case ')':
            return true;
        default:
            return false;
    }
}

bool is_list_separator(char c) {
    switch (c) {
        case ',':
            return true;
        default:
            return false;
    }
}

bool is_newline(char c) {
    switch (c) {
        case ';':
        case '\n':
            return true;
        default:
            return false;
    }
}

bool is_number(std::string str) {
    auto it = str.begin();
    while(it != str.end() && std::isdigit(*it)) ++it;
    return !str.empty() && it == str.end();
}

bool is_type(std::string token) {
    return types.find(token) != types.end();
}

bool is_keyword(std::string token) {
    return keywords.find(token) != keywords.end();
}

EToken identify_token(std::string token) {
    if(is_number(token)) {
        return EToken::NUMBER;
    } else if(is_type(token)) {
        return EToken::TYPE;
    } else if(is_keyword(token)) {
        return EToken::KEYWORD;
    }
    return EToken::IDENTIFIER;
}

std::string extract_string_token(char* token, size_t len, size_t offset) {
    std::string str;
    for(int y = offset; y < len; y++) {
        if(is_separator(token[y])) break;
        str += token[y];
    }
    return str;
}

std::string extract_operator_token(char* token, size_t len, size_t offset) {
    std::string str;
    char op = token[offset];
    str += op;
    switch (op) {
        case '=':
        case '<':
        case '>':
        case '!':
            if(token[offset+1] == '=') {
                str += token[offset+1];
            }
            break;
        default:
            break;
    }

    return str;
}

size_t skip_comment(char* token, size_t len, size_t offset) {
    size_t commentLen = 0;
    for(int y = offset; y < len; y++) {
        commentLen++;
        if(token[y] == '\n') return commentLen;
    }
    fprintf(stderr, "Error parsing comment no newline found\n");
    exit(-1);
}

void tokenize_separators(char* token, size_t len, std::vector<Token>& tokens) {
    int i = 0;
    while(i < len) {
        auto c = token[i];
        if(c == '\0') return;
        if(is_separator(c)) {
            if(is_operator(c)) {
                if (c == '/' && token[i + 1] == '/') {
                    i += skip_comment(token, len, i);
                    continue;
                }
                auto operatorStr = extract_operator_token(token, len, i);
                i += operatorStr.length();
                tokens.push_back({EToken::OPERATOR, operatorStr});
            } else if(is_list_separator(c)){
                tokens.push_back({EToken::LIST_SEPARATOR, std::string({c})});
                i++;
            } else {
                if(is_newline(c)) {
                    tokens.push_back({EToken::NEWLINE});
                }
                i++;
            }
        } else {
            auto tokenStr = extract_string_token(token, len, i);
            i += tokenStr.length();
            tokens.push_back({identify_token(tokenStr), tokenStr});
        }
    }
}

//void tokenize_separators(char* token, size_t len, std::vector<Token>& tokens) {
//    size_t lastSeparatorPos = 0;
//    int i;
//    for(i = 0; i < len; i++) {
//        if(is_separator(token[i])) {
//            if(token[i] == '\t') {
//                lastSeparatorPos = i + 1;
//                continue;
//            }
//            // Found separator
//            if(i == lastSeparatorPos) {
//                if(token[i] != ' ' && token[i] != '\t') {
//                    //printf("token: %c\n", token[i]);
//                    if(is_evaluator(token, i)) {
//                        std::string_view evaluator{&token[i], 2};
//                        tokens.push_back({EToken::EVALUATOR, std::string(evaluator)});
//                        i++;
//                    } else {
//                        tokens.push_back({EToken::OPERATOR, std::string(1, token[i])});
//                    }
//
//                }
//            } else {
//                char* charToken;
//                charToken = (char*)malloc((i-lastSeparatorPos)+1);
//                memcpy_s(charToken, i-lastSeparatorPos, &token[lastSeparatorPos], i-lastSeparatorPos);
//                charToken[i-lastSeparatorPos] = 0x0;
//                //printf("token: %s\n", charToken);
//                std::string strToken = std::string(charToken);
//                tokens.push_back({identify_token(strToken), strToken});
//                free(charToken);
//
//                if(token[i] != ' ') {
//                    //printf("token: %c\n", token[i]);
//                    if(token[i] == '\n') {
//                        tokens.push_back({EToken::NEWLINE, std::string(1, '\n')});
//                    } else {
//                        if(is_evaluator(token, i)) {
//                            std::string_view evaluator{&token[i], 2};
//                            tokens.push_back({EToken::EVALUATOR, std::string(evaluator)});
//                            i++;
//                        } else {
//                            tokens.push_back({EToken::OPERATOR, std::string(1, token[i])});
//                        }
//                    }
//                }
//            }
//            lastSeparatorPos = i + 1;
//        }
//    }
//
//    if(lastSeparatorPos != i) {
//        char* charToken;
//        charToken = (char*)malloc((i-lastSeparatorPos)+1);
//        memcpy_s(charToken, i-lastSeparatorPos, &token[lastSeparatorPos], i-lastSeparatorPos);
//        charToken[i-lastSeparatorPos] = 0x0;
//        //printf("token: %s\n", charToken);
//        std::string strToken = std::string(charToken);
//        tokens.push_back({identify_token(strToken), strToken});
//        free(charToken);
//        tokens.push_back({EToken::NEWLINE, std::string(1, '\n')});
//        /*if(token[i] != ' ') {
//            //printf("token: %c\n", token[i]);
//            if(token[i] == '\n') {
//
//            } else {
//                tokens.push_back({EToken::OPERATOR, std::string(1, token[i])});
//            }
//        }*/
//    }
//}

std::vector<Token> tokenize(const char* fileName) {
    std::vector<Token> tokens;
    std::fstream file;
    file.open(fileName, std::ios::in);
    if(!file.is_open()) {
        fprintf(stderr, "Could not open file %s\n", fileName);
        return tokens;
    }
    const int strBuffLen = 512;
    char str[strBuffLen];

    while(!file.eof()) {
        file.read(str, strBuffLen);
        tokenize_separators(str, file.gcount(), tokens);
    }

    if(tokens[tokens.size()-1].token != EToken::NEWLINE) {
        tokens.push_back({EToken::NEWLINE});
    }

    return tokens;
}


const char* ETokenAsStr(EToken eToken) {
    switch (eToken) {
        case EToken::UNKNOWN:
            return "Unknown";
        case EToken::TYPE:
            return "Type";
        case EToken::OPERATOR:
            return "Operator";
        case EToken::IDENTIFIER:
            return "Identifier";
        case EToken::NUMBER:
            return "Number";
        case EToken::NEWLINE:
            return "Newline";
    }
}