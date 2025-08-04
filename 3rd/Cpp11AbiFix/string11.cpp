#define _GLIBCXX_USE_CXX11_ABI 1

#include <string>
#include <vector>

extern "C" void string_vec_to_11(std::vector<char> const &vec, std::string &str_11)
{
    str_11.assign(vec.begin(), vec.end());
}

extern "C" void string_11_to_vec(std::string const &str_11, std::vector<char> &vec)
{
    vec.assign(str_11.begin(), str_11.end());
}
