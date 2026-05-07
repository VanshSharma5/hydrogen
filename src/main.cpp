#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "generation.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage" << std::endl;
        std::cerr << "hydro <input.hy>" << std::endl;
        return EXIT_FAILURE;
    }
    // std::string filename = argv[1];

    const char *filename = argv[1];

    std::stringstream contents_stream;
    {
        std::fstream input(filename, std::ios::in);
        contents_stream << input.rdbuf();
    }

    std::string contents = contents_stream.str();


    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(std::move(tokens));

    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "Invalid Program" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(prog.value());

    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    std::system("nasm -felf64 out.asm");
    std::system("ld -o out out.o");

    return EXIT_SUCCESS;
}