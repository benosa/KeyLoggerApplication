#pragma once
#include <Windows.h>

template<typename T, typename R, typename... Args>
class AdapterWorker
{
private:
    static T* m_obj;
    static R(T::* m_func)(Args...);

public:
    AdapterWorker(T* obj, R(T::* func)(Args...))
    {
        m_obj = obj;
        m_func = func;
    }

    static R Function(Args... args) {
        if (!m_obj) return R{};
        return (m_obj->*m_func)(args...);
    }
};

template<typename T, typename R, typename... Args>
T* AdapterWorker<T, R, Args...>::m_obj = nullptr;

template<typename T, typename R, typename... Args>
R(T::* AdapterWorker<T, R, Args...>::m_func)(Args...) = nullptr;