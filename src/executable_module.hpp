#pragma once

#include <string>
#include "ast.hpp"
#include <llvm/IR/Value.h>
#include <stdexcept>
#include <memory>
#include <utility>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include "llvm/IR/LegacyPassManager.h"
#include <llvm/Support/raw_ostream.h>
#include <sstream>
#include <print>
#include <filesystem>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include "function.hpp"

struct ExecutableModule final
{
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    void parse_type(const AstNode& node);
    llvm::Value* parse_expression(const AstArgument& arg_v);
    llvm::Value* parse_statement(const AstNode& node);
    std::string get_ir();
    void write_obj_file(const std::filesystem::path& path);

    ExecutableModule(const Ast& ast)
    {
        this->context = std::make_unique<llvm::LLVMContext>();
        this->module = std::make_unique<llvm::Module>("test", *this->context.get());
        this->builder = std::make_unique<llvm::IRBuilder<>>(*this->context.get());
        for (const AstNode& node : ast.nodes)
        {
            parse_statement(node);
        }
    }

};

void ExecutableModule::parse_type(const AstNode& node)
{
    // this is stupid and only accepts i32 for now.
    if (node.arguments.size() != 1UZ)
    {
        throw std::runtime_error("invalid type.");
    }
    const auto& word_v = node.arguments.at(0UZ);
    if (!std::holds_alternative<std::string>(word_v))
    {
        throw std::runtime_error("invalid type.");
    }
    const auto& word = std::get<std::string>(word_v);
    if (word != "i32")
    {
        throw std::runtime_error("invalid type.");
    }
}

llvm::Value* ExecutableModule::parse_expression(const AstArgument& arg_v)
{
    if (std::holds_alternative<AstNode>(arg_v))
    {
        return parse_expression(std::get<AstNode>(arg_v));
    }
    else if (std::holds_alternative<std::string>(arg_v))
    {
        const auto& text = std::get<std::string>(arg_v);
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*this->context.get()), llvm::APInt(32, text, 10));
    }
    std::unreachable();
}

llvm::Value* ExecutableModule::parse_statement(const AstNode& node)
{
    const auto& opcode_v = node.arguments.at(0);
    if (std::holds_alternative<AstNode>(opcode_v))
    {
        throw std::runtime_error("first argument of statement must be an opcode.");
    }
    const auto& opcode = std::get<std::string>(opcode_v);
    if (opcode == "return")
    {
        if (node.arguments.size() == 1UZ)
        {
            return this->builder->CreateRetVoid();
        }
        else if (node.arguments.size() == 2UZ)
        {
            return
                this->builder->CreateRet(
                    parse_expression(
                        node.arguments.at(1UZ)
                    )
                );
        }
        else
        {
            throw std::runtime_error("invalid argument count in return statement.");
        }
    }
    else if (opcode == "function")
    {
        const auto& name_v = node.arguments.at(1UZ);
        if (!std::holds_alternative<std::string>(name_v))
        {
            throw std::runtime_error("missing function name.");
        }
        Function function;
        function.name = std::get<std::string>(name_v);
        auto body_i = 0UZ;
        for (auto arg_i = 2UZ; arg_i < node.arguments.size(); arg_i++)
        {
            const auto& arg_v = node.arguments.at(arg_i);
            if (std::holds_alternative<std::string>(arg_v))
            {
                throw std::runtime_error("unexpected word.");
            }
            const auto& arg = std::get<AstNode>(arg_v);
            const auto& arg_opcode_v = arg.arguments.at(0);
            if (!std::holds_alternative<std::string>(arg_opcode_v))
            {
                throw std::runtime_error("first argument of expression must be an opcode.");
            }
            const auto& arg_opcode = std::get<std::string>(arg_opcode_v);
            if (arg_opcode == "arguments")
            {
                // TODO
                throw std::runtime_error("function arguments not supported (yet).");
            }
            else if (arg_opcode == "return_type")
            {
                // TODO check multiple return_type
                if (arg.arguments.size() != 2UZ)
                {
                    throw std::runtime_error("invalid function return expression.");
                }
                const auto& type_v = arg.arguments.at(1UZ);
                if (!std::holds_alternative<AstNode>(type_v))
                {
                    throw std::runtime_error("expected a type in function return expression.");
                }
                const auto& type = std::get<AstNode>(type_v);
                parse_type(type);
            }
            else if (arg_opcode == "body")
            {
                if (body_i != 0UZ)
                {
                    throw std::runtime_error("multiple function body.");
                }
                body_i = arg_i; 
                // further parsing of the body is done bellow.
            }
        }
        if (body_i == 0UZ)
        {
            throw std::runtime_error("missing function body.");
        }
        function.type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*this->context.get()), false); // TODO return stuff other than i32.
        function.function = llvm::Function::Create(function.type, llvm::Function::ExternalLinkage, function.name, this->module.get());
        function.body = llvm::BasicBlock::Create(*this->context, function.name.c_str(), function.function);
        this->builder->SetInsertPoint(function.body);
        const auto& body = std::get<AstNode>(node.arguments.at(body_i));
        for (auto body_arg_i = 1UZ; body_arg_i < body.arguments.size(); body_arg_i++)
        {
            const auto& statement_v = body.arguments.at(body_arg_i);
            if (std::holds_alternative<std::string>(statement_v))
            {
                throw std::runtime_error("invalid word in function body.");
            }
            const auto& statement = std::get<AstNode>(statement_v);
            parse_statement(statement);
        }
    }
    else
    {
        throw std::runtime_error("invalid opcode.");
    }
    std::unreachable();
}

std::string ExecutableModule::get_ir()
{
    std::string result;
    llvm::raw_string_ostream raw_string_ostream(result);
    this->module->print(raw_string_ostream, nullptr);
    return result;
}

void ExecutableModule::write_obj_file(const std::filesystem::path& path)
{
    auto target_triple = llvm::sys::getDefaultTargetTriple();
    this->module->setTargetTriple(target_triple);
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    if (target == nullptr)
    {
        throw std::runtime_error(error);
    }
    const char* cpu = "generic";
    const char* features = "";
    llvm::TargetOptions options;
    auto machine = target->createTargetMachine(target_triple, cpu, features, options, llvm::Reloc::PIC_);
    auto data_layout = machine->createDataLayout();
    this->module->setDataLayout(data_layout);
    this->module->setTargetTriple(target_triple);
    std::error_code error_code;
    llvm::raw_fd_ostream ofile(path.c_str(), error_code, llvm::sys::fs::OF_None);
    if (error_code)
    {
        throw std::runtime_error(error_code.message());
    }
    llvm::legacy::PassManager pass;
    const auto file_type = llvm::CodeGenFileType::ObjectFile;
    if (machine->addPassesToEmitFile(pass, ofile, nullptr, file_type))
    {
        throw std::runtime_error("the target machine can not emit a file of this type.");
    }
    pass.run(*this->module.get());
    ofile.flush();
}