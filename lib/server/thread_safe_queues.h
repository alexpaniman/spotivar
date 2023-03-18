#pragma once
#include "common_include.h"

namespace sptv
{
    template<typename T>
    class ts_queue
    {
        public:
        ts_queue() = default;
        ts_queue(const ts_queue<T>&) = delete;

        virtual ~ts_queue() { clear(); };

        const T& front()
        {
            std::scoped_lock lock(mxQueue);
            return deqQueue.front();
        };

        const T& back()
        {
            std::scoped_lock lock(mxQueue);
            return deqQueue.back();
        };

        void push_back(const T& item)
        {
            std::scoped_lock lock(mxQueue);
            deqQueue.emplace_back(std::move(item)); //deep copy of an elem

            std::unique_lock<std::mutex> ul(mtx_update);
            cv_blocking.notify_one();
        };

        void push_front(const T& item)
        {
            std::scoped_lock lock(mxQueue);
            deqQueue.emplace_front(std::move(item));

            std::unique_lock<std::mutex> ul(mtx_update);
            cv_blocking.notify_one();
        }; 

        bool empty()
        {
            std::scoped_lock lock(mxQueue);
            return deqQueue.empty();
        };

        size_t count()
        {
            std::scoped_lock lock(mxQueue);
            return deqQueue.size();
        };

        void clear()
        {
            std::scoped_lock lock(mxQueue);
            deqQueue.clear();
        }

        T pop_front()
        {
            std::scoped_lock lock(mxQueue);

            auto elem = std::move(deqQueue.front());
            deqQueue.pop_front();

            return elem;
        }

        T pop_back()
        {
            std::scoped_lock lock(mxQueue);

            auto elem = std::move(deqQueue.front());
            deqQueue.pop_back();

             return elem;
        }

        void wait(){
            while (empty()){
                std::unique_lock<std::mutex> ul(mtx_update);
                cv_blocking.wait(ul);
            }
        }


         
        protected:
        std::mutex mxQueue;
        std::deque<T> deqQueue;

        std::condition_variable cv_blocking;
        std::mutex mtx_update;
    };
}