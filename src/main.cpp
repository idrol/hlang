#include <stdio.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char* argv[]) {
    printf("%i\n", argc);
    if(argc != 3) {
        fprintf(stderr, "Usage mylangc <srcFile> <outputFile>");
        exit(-1);
    }

    std::vector<Token> tokens = tokenize(argv[1]);

    auto ast = parseTokens(tokens);

    debugAst(ast);

    //run_program(ast);

    auto var = get_bool_var("isTrue");
    printf("isTrue: %i\n", var);
}
