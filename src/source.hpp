#pragma once

#include <string>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include <sstream>

std::string get_source(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("file does not exist.");
    }
    std::ifstream ifile(path);
    std::stringstream sstream;
    sstream << ifile.rdbuf();
    return sstream.str();
}