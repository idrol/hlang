//
// Created by idrol on 01/05/2022.
//
#pragma once

#include <string>
#include <vector>

enum class EToken {
    UNKNOWN,
    TYPE, // int bool float
    OPERATOR, // + = ; - *
    IDENTIFIER, // someVarName
    NUMBER, // duh
    NEWLINE
};

struct Token {
    EToken token;
    std::string value;
};

std::vector<Token> tokenize(const char* source);

const char* ETokenAsStr(EToken eToken);
