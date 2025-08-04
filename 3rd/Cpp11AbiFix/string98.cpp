#define _GLIBCXX_USE_CXX11_ABI 0

#include <string>
#include <vector>

extern "C" void string_vec_to_98(std::vector<char> const &vec, std::string &str_98)
{
    str_98.assign(vec.begin(), vec.end());
}

extern "C" void string_98_to_vec(std::string const &str_98, std::vector<char> &vec)
{
    vec.assign(str_98.begin(), str_98.end());
}
