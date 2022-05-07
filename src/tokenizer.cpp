#include "tokenizer.h"
#include <cstdio>
#include <istream>
#include <fstream>

bool is_separator(char c) {
    switch (c) {
        case '=':
        case '+':
        case '(':
        case ')':
        case '*':
        case '/':
        case '-':
        case ';':
        case ' ':
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

bool is_type(std::string str) {
    if(str == "int") {
        return true;
    }
    return false;
}

EToken identify_token(std::string token) {
    if(is_number(token)) {
        return EToken::NUMBER;
    } else if(is_type(token)) {
        return EToken::TYPE;
    }
    return EToken::IDENTIFIER;
}

void tokenize_separators(char* token, size_t len, std::vector<Token>& tokens) {
    size_t lastSeparatorPos = 0;
    int i;
    for(i = 0; i < len; i++) {
        if(is_separator(token[i])) {
            // Found separator
            if(i == lastSeparatorPos) {
                if(token[i] != ' ') {
                    //printf("token: %c\n", token[i]);
                    tokens.push_back({EToken::OPERATOR, std::string(1, token[i])});
                }
            } else {
                char* charToken;
                charToken = (char*)malloc((i-lastSeparatorPos)+1);
                memcpy_s(charToken, i-lastSeparatorPos, &token[lastSeparatorPos], i-lastSeparatorPos);
                charToken[i-lastSeparatorPos] = 0x0;
                //printf("token: %s\n", charToken);
                std::string strToken = std::string(charToken);
                tokens.push_back({identify_token(strToken), strToken});
                free(charToken);

                if(token[i] != ' ') {
                    //printf("token: %c\n", token[i]);
                    if(token[i] == '\n') {
                        tokens.push_back({EToken::NEWLINE, std::string(1, '\n')});
                    } else {
                        tokens.push_back({EToken::OPERATOR, std::string(1, token[i])});
                    }
                }
            }
            lastSeparatorPos = i + 1;
        }
    }
    if(lastSeparatorPos == 0) {
        //printf("token: %s\n", token);
    } else if(lastSeparatorPos != i) {
        char* charToken;
        charToken = (char*)malloc((i-lastSeparatorPos)+1);
        memcpy_s(charToken, i-lastSeparatorPos, &token[lastSeparatorPos], i-lastSeparatorPos);
        charToken[i-lastSeparatorPos] = 0x0;
        //printf("token: %s\n", charToken);
        std::string strToken = std::string(charToken);
        tokens.push_back({identify_token(strToken), strToken});
        free(charToken);
        tokens.push_back({EToken::NEWLINE, std::string(1, '\n')});
        /*if(token[i] != ' ') {
            //printf("token: %c\n", token[i]);
            if(token[i] == '\n') {

            } else {
                tokens.push_back({EToken::OPERATOR, std::string(1, token[i])});
            }
        }*/
    }
}

std::vector<Token> tokenize(const char* fileName) {
    std::vector<Token> tokens;
    std::fstream file;
    file.open(fileName, std::ios::in);
    if(!file.is_open()) {
        fprintf(stderr, "Could not open file %s\n", fileName);
        return tokens;
    }
    char str[256];

    while(!file.eof()) {
        file.getline(str, 256);
        tokenize_separators(str, strlen(str), tokens);
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