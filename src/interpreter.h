//
// Created by idrol on 05/05/2022.
//
#pragma once
#include "parser.h"

typedef int HInt;
typedef bool HBool;

class Variable {
public:
    Variable() {
        value = nullptr;
    }
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

HInt run_expression(std::shared_ptr<ExpressionNode> node);
void run_statement(std::shared_ptr<StatementNode> node);
void run_program(std::shared_ptr<ProgramNode> node);
Variable* resolve_variable(std::string var);
HInt get_int_var(std::string var);
HBool get_bool_var(std::string var);

void hlang_pushint(HInt num);
HInt hland_popint(HInt num);
void hlang_push(size_t numBytes);
void* hland_pop(size_t numBytes);
void hlang_settop(size_t stackTopInBytes);