#pragma once

#include <memory>


namespace Disruptor
{

    class IEventReleaser;

     /// 事件销毁感知
    class IEventReleaseAware
    {
    public:
        virtual ~IEventReleaseAware() = default;

        virtual void setEventReleaser(const std::shared_ptr< IEventReleaser >& eventReleaser) = 0;
    };

} // namespace Disruptor
