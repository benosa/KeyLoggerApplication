#pragma once
#include <Windows.h>

template<typename T>
class AdapterWorker
{
private:
    inline static T* m_obj = NULL;
    inline static LRESULT(T::* m_func)(HWND, UINT, WPARAM, LPARAM) = NULL;
    inline static BOOL(T::* m_func3)(HWND, LPARAM) = NULL;
    inline static void(T::* m_func2)() = NULL;

public:
    AdapterWorker(T* obj, LRESULT(T::* func)(HWND, UINT, WPARAM, LPARAM))
    {
        m_obj = obj;
        m_func = func;
    }
    AdapterWorker(T* obj, BOOL(T::* func)(HWND, LPARAM))
    {
        m_obj = obj;
        m_func3 = func;
    }
    AdapterWorker(T* obj, void(T::* func2)())
    {
        m_obj = obj;
        m_func2 = func2;
    }

    static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (!m_obj) return 0;
        return (m_obj->*m_func)(hWnd, message, wParam, lParam);
    }

    static LRESULT EnumWindowsProc(HWND hwnd, LPARAM lParam)
    {
        if (!m_obj) return 0;
        return (m_obj->*m_func3)(hwnd, lParam);
    }

    static void HookJob() {
        if (!m_obj) return;
        return (m_obj->*m_func2)();
    }
};
