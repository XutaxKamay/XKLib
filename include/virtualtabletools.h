
#ifndef VIRTUALTABLETOOLS_H
#define VIRTUALTABLETOOLS_H

#include "types.h"
#include <functional>

namespace XLib
{
    namespace Virtual
    {
        template <typename T = ptr_t>
        /**
         * @brief VTable
         * @param classPtr
         * Gets a virtual table from a class pointer.
         */
        constexpr auto VTable(T classPtr)
        {
            return *view_as<ptr_t**>(classPtr);
        }

        template <safesize_t index, typename T = ptr_t>
        /**
         * @brief Cast_VFunc
         * @param classPtr
         * Gets a virtual function type from a virtual table and an index.
         */
        constexpr inline auto Cast_VFunc(ptr_t classPtr)
        {
            return view_as<T>(VTable(classPtr)[index]);
        }

        template <safesize_t index, typename T = ptr_t>
        /**
         * @brief VFuncPtr
         * @param classPtr
         * Gets a pointer of virtual function pointer from a virtual table
         * and an index.
         */
        constexpr inline auto VFuncPtr(ptr_t classPtr)
        {
            return view_as<T>(&VTable(classPtr)[index]);
        }

        template <safesize_t index,
                  typename TRetType = void,
                  typename... TArgs>
        /**
         * @brief VFunc
         * @param classPtr
         * Gets a virtual function type for calling process from a virtual
         * table and an index.view_as
         */
        constexpr inline auto VFunc(ptr_t classPtr)
        {
#ifdef WINDOWS
            return Cast_VFunc<index,
                              TRetType(__thiscall*)(ptr_t, TArgs...)>(
              classPtr);
#else
            return Cast_VFunc<index, TRetType (*)(ptr_t, TArgs...)>(
              classPtr);
#endif
        }

        template <safesize_t index,
                  typename TRetType = void,
                  typename... TArgs>
        /**
         * @brief CallVFunc
         * @param classPtr
         * @param Args
         * Calls a virtual function from a virtual table and an index.
         */
        constexpr inline auto CallVFunc(ptr_t classPtr, TArgs... Args)
        {
            return VFunc<index, TRetType, TArgs...>(classPtr)(classPtr,
                                                              Args...);
        }
    };

    template <class T>
    class VirtualTable : public T
    {
        using pre_or_post_hook_func_t = std::function<void(ptr_t, ptr_t)>;

      public:
        template <safesize_t index,
                  typename TRetType = void,
                  typename... TArgs>
        constexpr inline auto callVFunc(TArgs... Args)
        {
            return Virtual::CallVFunc<index, TRetType, TArgs...>(this,
                                                                 Args...);
        }

        template <safesize_t index, typename T2 = ptr_t>
        constexpr inline auto VFunc()
        {
            return Virtual::VFunc<index, T2>(this);
        }

        template <safesize_t index, typename T2 = ptr_t>
        constexpr inline auto VFuncPtr()
        {
            return Virtual::VFuncPtr<index, T2>(this);
        }

        template <safesize_t index, typename T2 = ptr_t>
        inline auto hook(T2 newFuncPtr,
                         pre_or_post_hook_func_t before = nullptr,
                         pre_or_post_hook_func_t after  = nullptr)
        {
            auto funcPtr = VFuncPtr<index, T2*>();

            if (funcPtr && *funcPtr && newFuncPtr)
            {
                if (before)
                    before(view_as<ptr_t>(funcPtr),
                           view_as<ptr_t>(newFuncPtr));

                /* __builtin_trap(); */

                *funcPtr = newFuncPtr;

                if (after)
                    after(view_as<ptr_t>(funcPtr),
                          view_as<ptr_t>(newFuncPtr));

                return true;
            }

            return false;
        }

        template <typename T2 = ptr_t>
        constexpr inline auto _vptr()
        {
            return Virtual::VTable<T2>(this);
        }

        auto countVFuncs() -> safesize_t;
    };

    template <class T>
    auto VirtualTable<T>::countVFuncs() -> safesize_t
    {
        auto vtable = _vptr();

        safesize_t count = 0;

        while (vtable[count])
        {
            count++;
        }

        return count;
    }

}

#endif // VIRTUALTABLETOOLS_H
