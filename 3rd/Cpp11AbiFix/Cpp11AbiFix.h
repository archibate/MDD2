#pragma once

#include <string>
#include <vector>

extern "C" void string_vec_to_98(std::vector<char> const &vec, std::string &str_98);
extern "C" void string_vec_to_11(std::vector<char> const &vec, std::string &str_11);
extern "C" void string_98_to_vec(std::string const &str_98, std::vector<char> &vec);
extern "C" void string_11_to_vec(std::string const &str_11, std::vector<char> &vec);

inline void string_98_to_11(std::string &str)
{
    std::vector<char> vec;
    string_98_to_vec(str, vec);
    string_vec_to_11(vec, str);
}

inline void string_11_to_98(std::string &str)
{
    std::vector<char> vec;
    string_11_to_vec(str, vec);
    string_vec_to_98(vec, str);
}
