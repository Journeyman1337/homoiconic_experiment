#include "source.hpp"
#include "ast.hpp"
#include "executable_module.hpp"
#include "llvm_ext.hpp"
#include <print>

int main()
{
    const auto source = get_source("main.rq");
    auto ast = Ast(source);
    std::print("source text: {}\n", to_string(ast));
    initialize_llvm();
    auto module = ExecutableModule(ast);
    std::print("ir: {}\n", module.get_ir());
    module.write_obj_file("main.obj");
    return 0;
}