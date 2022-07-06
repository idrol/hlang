//
// Created by idrol on 01/05/2022.
//
#pragma once

#include <string>
#include <vector>

enum class EToken {
    UNKNOWN,
    TYPE, // int bool float
    OPERATOR, // + = ; - * == <= < > >= !=
    KEYWORD,
    IDENTIFIER, // someVarName
    NUMBER, // duh
    LIST_SEPARATOR, // ,
    NEWLINE
};

struct Token {
    EToken token;
    std::string value;
    size_t line;
    size_t column;
    size_t len;
};

std::vector<Token> tokenize(const char* source);

const char* ETokenAsStr(EToken eToken);
