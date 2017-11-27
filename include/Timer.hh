#pragma once

#include "core.h"

#include <set>
#include <functional>
#include <ctime>

extern "C" {
uint32_t timer_add(uint32_t sec, void * udata, );
};

class TimerItem{
public:
    using OnExpire = std::function<void(void *)>;
    struct Comparer{
        int operator()(TimerItem const & lhs, TimerItem const & rhs) {
            return ((lhs.startTime + lhs.expireTime) - (rhs.startTime + rhs.expireTime));
        }
    };

public:
    TimerItem(uint32_t expireTime, OnExpire && onExpire):
        startTime {(uint32_t) std::time(nullptr)},
        expireTime { expireTime },
        onExpire { std::move(onExpire) }
    { }

    uint32_t startTime;
    uint32_t expireTime;
    OnExpire onExpire;
};

class Timer{
public:
    Timer(Timer const &) = delete;
    Timer(Timer &&) = delete;
    Timer & operator=(Timer const &) = delete;
    Timer && operator=(Timer &&) = delete;

    static Timer & instance() {
        static Timer instance_;
        return instance_;
    }

    uint32_t add(TimerItem && item) {
        items.insert(item);
    }



private:
    Timer() = default;

    std::set<TimerItem, TimerItem::Comparer> items;
};

