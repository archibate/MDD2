#pragma once

#include <cstdint>
#include <memory>

/*****************************************************************
 * 数据提供者基类
 *****************************************************************/
namespace Disruptor
{

    template <class T>
    class IDataProvider
    {
    public:
        virtual ~IDataProvider() = default;

        virtual T& operator[](std::int64_t sequence) const = 0;
    };

} // namespace Disruptor
