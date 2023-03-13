#pragma once
#include <Windows.h>

template<typename T>
class AdapterWorker
{
private:
    inline static T* m_obj = NULL;
    inline static LRESULT(T::* m_func)(int, WPARAM, LPARAM) = NULL;
    inline static void(T::* m_func2)() = NULL;

public:
    AdapterWorker(T* obj, LRESULT(T::* func)(int, WPARAM, LPARAM))
    {
        m_obj = obj;
        m_func = func;
    }

    AdapterWorker(T* obj, void(T::* func2)())
    {
        m_obj = obj;
        m_func2 = func2;
    }

    static LRESULT HookProc(int code, WPARAM wParam, LPARAM lParam)
    {
        if (!m_obj) return 0;
        return (m_obj->*m_func)(code, wParam, lParam);
    }

    static void HookJob() {
        if (!m_obj) return;
        return (m_obj->*m_func2)();
    }
};


//#pragma once
//#include <Windows.h>
//
//template<typename T, typename R, typename... Args>
//class AdapterWorker
//{
//private:
//    static T* m_obj;
//    static R(T::* m_func)(Args...);
//
//public:
//    AdapterWorker(T* obj, R(T::* func)(Args...))
//    {
//        m_obj = obj;
//        m_func = func;
//    }
//
//    static R Function(Args... args) {
//        if (!m_obj) return R{};
//        return (m_obj->*m_func)(args...);
//    }
//};
//
//template<typename T, typename R, typename... Args>
//T* AdapterWorker<T, R, Args...>::m_obj = nullptr;
//
//template<typename T, typename R, typename... Args>
//R(T::* AdapterWorker<T, R, Args...>::m_func)(Args...) = nullptr;