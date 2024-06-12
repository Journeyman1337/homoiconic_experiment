#pragma once
#include <string>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>

struct Function final
{
    std::string name;
    llvm::FunctionType* type;
    llvm::Function* function;
    llvm::BasicBlock* body;
};