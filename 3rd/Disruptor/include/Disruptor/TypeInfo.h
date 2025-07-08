#pragma once

#include <string>
#include <typeinfo>
#include <vector>

namespace Disruptor
{

	std::vector<char> is_any_of(std::string str);

	void split(std::vector<std::string>& result, std::string str, std::vector<char> delimiters);

	std::string replace_all_copy(const std::string &str, std::string fd, std::string rp);

    struct TypeInfo
    {
        explicit TypeInfo(const std::type_info& typeInfo);

        const std::type_info& intrinsicTypeInfo() const;

        const std::string& fullyQualifiedName() const;
        const std::string& name() const;

        bool operator==(const TypeInfo& rhs) const;

        static std::string dotNetify(const std::string& typeName);
        static std::string unqualifyName(const std::string& fullyQualifiedName);
        static std::string demangleTypeName(const std::string& typeName);

    private:
        const std::type_info* m_typeInfo;
        std::string m_fullyQualifiedName;
        std::string m_name;
    };

namespace Utils
{

    template <class T>
    const TypeInfo& getMetaTypeInfo()
    {
        static TypeInfo result(typeid(T));
        return result;
    }

} // namespace Utils
} // namespace Disruptor


#include <functional>
#include <typeindex>


namespace std
{

    template <>
    struct hash<Disruptor::TypeInfo>
    {
    public:
        size_t operator()(const Disruptor::TypeInfo& value) const
        {
            return hash<type_index>{}(type_index{value.intrinsicTypeInfo()});
        }

        typedef size_t result_type;
        typedef Disruptor::TypeInfo argument_type;  
    };

} // namespace std

