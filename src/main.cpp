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

    std::shared_ptr<Node> ast = parseTokens(tokens);

    run_program(ast);

    size_t var = *(size_t*)get_var("test");
    printf("test: %i\n", var);

    var = *(size_t*)get_var("memes");
    printf("memes: %i\n", var);
}
