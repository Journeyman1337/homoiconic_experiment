#pragma once

#include <llvm/Support/TargetSelect.h>

void initialize_llvm()
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}