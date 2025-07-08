#pragma once

#include <cstdint>


namespace Disruptor
{

    /**
      * Callback interface to be implemented for processing events as they become available in the RingBuffer<T>
        *  当事件在环形缓冲区中可用时，要实现的回调接口
      * \tparam T Type of events for sharing during exchange or parallel coordination of an event
      * T 类型用于在交换或并行协调事件期间共享
      * \remark See BatchEventProcessor<T>.SetExceptionHandler if you want to handle exceptions propagated out of the handler.
      */
    template <class T>
    class IEventHandler
    {
    public:
        virtual ~IEventHandler() = default;

           /// 函数功能：当发布被提交时回到改函数
           /// 参数： data：被提交的数据，sequence：被提交的序列号，endOfBatch：指示这是否是来自RingBuffer的批处理中的最后一个事件
           /// 返回值：无
        virtual void onEvent(T& data, std::int64_t sequence, bool endOfBatch) = 0;
    };

} // namespace Disruptor
