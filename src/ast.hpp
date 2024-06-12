#pragma once

#include <string>
#include <vector>
#include <variant>
#include <cstddef>
#include <utility>
#include <cctype>


struct AstNode final 
{
    std::vector<std::variant<std::string, AstNode>> arguments;
};

using AstArgument = std::variant<std::string, AstNode>;

[[nodiscard]] std::string to_string(const AstNode& ast)
{
    std::stringstream sstream;
    sstream << "(";
    for (std::size_t arg_i = 0; arg_i < ast.arguments.size(); arg_i++)
    {
        if (arg_i != 0)
        {
            sstream << " ";
        }
        auto argument = ast.arguments.at(arg_i);
        if (std::holds_alternative<std::string>(argument))
        {
            sstream << std::get<std::string>(argument);
        }
        else if (std::holds_alternative<AstNode>(argument))
        {
            sstream << to_string(std::get<AstNode>(argument));
        }
    }
    sstream << ")";
    return sstream.str();
}

struct Ast final
{
    std::vector<AstNode> nodes{};

    Ast(std::string_view source)
    {
        const auto get_char =
            [&](std::size_t char_i)
            {
                if (char_i >= source.size())
                {
                    return '\0';
                }
                return source.at(char_i);
            };
        enum class State
        {
            DEFAULT,
            WORD
        };
        auto state = State::DEFAULT;
        std::size_t cur_state_count = 0UZ;
        std::vector<AstNode*> node_stack{};
        for (std::size_t char_i = 0; char_i < source.size(); char_i++)
        {
            switch (state)
            {
                case State::DEFAULT:
                    switch (get_char(char_i))
                    {
                        case '[':
                            if (node_stack.empty())
                            {
                                node_stack.push_back(&this->nodes.emplace_back());
                            }
                            else
                            {
                                node_stack.push_back( 
                                    &std::get<AstNode>(
                                        node_stack.back()->arguments.emplace_back(
                                            std::move(AstNode())
                                        )
                                    )
                                );
                            }
                            break;
                        case ']':
                            if (node_stack.empty())
                            {
                                throw std::runtime_error("invalid closing bracket.");
                            }
                            else
                            {
                                node_stack.pop_back();
                            }
                            break;
                        default:
                            if (!std::isspace(get_char(char_i)))
                            {
                                state = State::WORD;
                                cur_state_count = 1UZ;
                            }
                    }
                    break;
                case State::WORD:
                    if (std::isspace(get_char(char_i)) || get_char(char_i) == '[' || get_char(char_i) == ']')
                    {
                        state = State::DEFAULT;
                        const auto start_i = char_i - cur_state_count;
                        node_stack.back()->arguments.push_back(
                            std::string(
                                source.data() + start_i,
                                cur_state_count
                            )
                        );
                        if (get_char(char_i) == '[' || get_char(char_i) == ']')
                        {
                            char_i--;
                        }
                    }
                    else
                    {
                        cur_state_count++;
                    }
                    break;
            }
        }
    }
};

[[nodiscard]] std::string to_string(const Ast& ast)
{
    std::stringstream sstream;
    for (std::size_t node_i = 0; node_i < ast.nodes.size(); node_i++)
    {
        if (node_i != 0)
        {
            sstream << " ";
        }
        auto& node = ast.nodes.at(node_i);
        sstream << to_string(node);
    }
    return sstream.str();
}
