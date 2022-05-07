//
// Created by idrol on 05/05/2022.
//
#pragma once
#include "parser.h"


class Variable {
public:
    Variable(size_t variableSize) {
        value = malloc(variableSize);
    }

    template<typename T> void SetValue(T val) {
        *((T*)value) = val;
    }

    template<typename T> T GetValue() {
        return *(T*)value;
    }
    void* value;
};

size_t run_expression(std::shared_ptr<Node> node);
void run_statement(std::shared_ptr<Node> node);
void run_program(std::shared_ptr<Node> node);
void* get_var(std::string var);