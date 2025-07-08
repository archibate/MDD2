#include "stdafx.h"
#include "TypeInfo.h"

#include <regex>
#include <vector>
#include <algorithm>

#if defined(__GNUC__)
# include <cxxabi.h>
#endif /* __GNUC__ */


namespace Disruptor
{

	std::vector<char> is_any_of(std::string str)
	{
		std::vector<char> res;
		for (auto s : str)
			res.push_back(s);
		return res;
	}

	void split(std::vector<std::string>& result, std::string str, std::vector<char> delimiters)
	{
		result.clear();
		size_t start = 0;
		while (start < str.size())
		{
			//根据多个分割符分割
			auto itRes = str.find(delimiters[0], start);
			for (size_t i = 1; i < delimiters.size(); ++i)
			{
				auto it = str.find(delimiters[i],start);
				if (it < itRes)
					itRes = it;
			}

			if (itRes == std::string::npos)
			{
				result.push_back(str.substr(start, str.size() - start));
				break;
			}
			result.push_back(str.substr(start, itRes - start));
			start = itRes;
			++start;
		}
	}

	std::string replace_all_copy(const std::string &str, std::string fd, std::string rp)
	{
		std::string strResult;

		size_t start = 0;
		size_t jump = fd.size();
		while (start < str.size())
		{
			//根据多个分割符分割
			auto itRes = str.find(fd, start);
			if (itRes == std::string::npos)
			{
				strResult = strResult + str.substr(start, str.size() - start);
				break;
			}
			strResult = strResult + str.substr(start, itRes - start) + rp;
			start = itRes + jump;    
		}
		return strResult;
	}

    TypeInfo::TypeInfo(const std::type_info& typeInfo)
        : m_typeInfo(&typeInfo)
        , m_fullyQualifiedName(dotNetify(demangleTypeName(m_typeInfo->name())))
        , m_name(unqualifyName(m_fullyQualifiedName))
    {
    }

    const std::type_info& TypeInfo::intrinsicTypeInfo() const
    {
        return *m_typeInfo;
    }

    const std::string& TypeInfo::fullyQualifiedName() const
    {
        return m_fullyQualifiedName;
    }

    const std::string& TypeInfo::name() const
    {
        return m_name;
    }

    bool TypeInfo::operator==(const TypeInfo& rhs) const
    {
        return intrinsicTypeInfo() == rhs.intrinsicTypeInfo();
    }

    std::string TypeInfo::dotNetify(const std::string& typeName)
    {
        return replace_all_copy(typeName, "::", ".");
    }

    std::string TypeInfo::unqualifyName(const std::string& fullyQualifiedName)
    {
        if (fullyQualifiedName.empty())
            return std::string();

        std::vector< std::string > nameParts;
        split(nameParts, fullyQualifiedName, is_any_of("."));

        if (nameParts.empty())
            return std::string();

        return nameParts[nameParts.size() - 1];
    }

    std::string TypeInfo::demangleTypeName(const std::string& typeName)
    {
#if defined(__GNUC__)
            int status;

            auto demangledName = abi::__cxa_demangle(typeName.c_str(), 0, 0, &status);
            if (demangledName == nullptr)
                return typeName;

            std::string result = demangledName;
            free(demangledName);
            return result;
#else
        std::string demangled = typeName;
        demangled = std::regex_replace(demangled, std::regex("(const\\s+|\\s+const)"), std::string());
        demangled = std::regex_replace(demangled, std::regex("(volatile\\s+|\\s+volatile)"), std::string());
        demangled = std::regex_replace(demangled, std::regex("(static\\s+|\\s+static)"), std::string());
        demangled = std::regex_replace(demangled, std::regex("(class\\s+|\\s+class)"), std::string());
        demangled = std::regex_replace(demangled, std::regex("(struct\\s+|\\s+struct)"), std::string());
        return demangled;
#endif /* defined(__GNUC__) */
    }

} // namespace Disruptor
