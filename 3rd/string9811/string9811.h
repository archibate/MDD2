#pragma once

#include <string>

struct std11string;

struct std98string
{
private:
    void *dataplus;

public:
    std98string();
    std98string(std98string const &);
    std98string &operator=(std98string const &);
    ~std98string();

    std98string(std11string const &);
#if _GLIBCXX_USE_CXX11_ABI
    std98string(std::string const &str11)
        : std98string(reinterpret_cast<std11string const &>(str11))
    {}
#else
    std98string(std::string const &str98)
        : std98string(reinterpret_cast<std98string const &>(str98))
    {}
#endif
    std98string(const char *str)
        : std98string(std::string(str))
    {}

    char const *begin() const;
    char const *end() const;
    size_t size() const { return end() - begin(); }
    bool empty() const { return begin() == end(); }
    char const *data() const { return begin(); }
    char const *c_str() const { return begin(); }

    operator std::string() const {
        return std::string(begin(), end());
    }
};

struct std11string
{
private:
    void *ptrs[4];

public:
    std11string();
    std11string(std11string const &);
    std11string &operator=(std11string const &);
    ~std11string();

    std11string(std98string const &);
#if _GLIBCXX_USE_CXX11_ABI
    std11string(std::string const &str11)
        : std11string(reinterpret_cast<std11string const &>(str11))
    {}
#else
    std11string(std::string const &str98)
        : std98string(reinterpret_cast<std98string const &>(str98))
    {}
#endif
    std11string(const char *str)
        : std11string(std::string(str))
    {}

    char const *begin() const;
    char const *end() const;
    size_t size() const { return end() - begin(); }
    bool empty() const { return begin() == end(); }
    char const *data() const { return begin(); }
    char const *c_str() const { return begin(); }

    operator std::string() const {
        return std::string(begin(), end());
    }
};
