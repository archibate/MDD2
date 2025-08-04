#define _GLIBCXX_USE_CXX11_ABI 0

#include <string>

struct std11string;

struct std98string
{
    std::string str98;

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
    void *ptrs[4];

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

std98string::std98string() = default;
std98string::std98string(std98string const &) = default;
std98string &std98string::operator=(std98string const &) = default;
std98string::~std98string() = default;

std98string::std98string(std11string const &str11)
    : str98(str11.begin(), str11.end())
{}

char const *std98string::begin() const {
    return str98.data();
}

char const *std98string::end() const {
    return str98.data() + str98.size();
}

static_assert(sizeof(std98string) == sizeof(void *));
