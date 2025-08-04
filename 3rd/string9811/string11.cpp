#define _GLIBCXX_USE_CXX11_ABI 1

#include <string>

struct std11string;

struct std98string
{
    void *dataplus;

    std98string();
    std98string(std98string const &);
    std98string &operator=(std98string const &);
    ~std98string();

    std98string(std11string const &);

    char const *begin() const;
    char const *end() const;
    size_t size() const { return end() - begin(); }
    bool empty() const { return begin() == end(); }
    char const *data() const { return begin(); }
    char const *c_str() const { return begin(); }
};

struct std11string
{
    std::string str11;

    std11string();
    std11string(std11string const &);
    std11string &operator=(std11string const &);
    ~std11string();

    std11string(std98string const &);

    char const *begin() const;
    char const *end() const;
    size_t size() const { return end() - begin(); }
    bool empty() const { return begin() == end(); }
    char const *data() const { return begin(); }
    char const *c_str() const { return begin(); }
};

std11string::std11string() = default;
std11string::std11string(std11string const &) = default;
std11string &std11string::operator=(std11string const &) = default;
std11string::~std11string() = default;

std11string::std11string(std98string const &str98)
    : str11(str98.begin(), str98.end())
{}

char const *std11string::begin() const {
    return str11.data();
}

char const *std11string::end() const {
    return str11.data() + str11.size();
}

static_assert(sizeof(std11string) == 4 * sizeof(void *));
