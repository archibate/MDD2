#pragma once


namespace Disruptor
{
    /// 事件销毁
    class IEventReleaser
    {
    public:
        virtual ~IEventReleaser() = default;

        virtual void release() = 0;
    };

} // namespace Disruptor
