#ifndef UTIL_CALLBACK_HPP
#define UTIL_CALLBACK_HPP

#define UTIL_GET_CALLBACK_FACTORY_BIND_FREE(freeFuncPtr) \
    (callback_detail::GetCallbackFactory(freeFuncPtr).Bind<freeFuncPtr>())
#define BIND_FREE_CB UTIL_GET_CALLBACK_FACTORY_BIND_FREE

#define UTIL_GET_CALLBACK_FACTORY_BIND_MEMBER(memFuncPtr, instancePtr) \
    (callback_detail::GetCallbackFactory(memFuncPtr).Bind<memFuncPtr>(instancePtr))
#define BIND_MEM_CB UTIL_GET_CALLBACK_FACTORY_BIND_MEMBER


namespace callback_detail {

template<typename FuncSignature>
class Callback;

struct NullCallback {};

template<typename R, typename ...Args>
class Callback<R (Args...)>
{
public:
    typedef R ReturnType;

    Callback()                    : func(0), obj(0) {}
    Callback(NullCallback)        : func(0), obj(0) {}
    Callback(const Callback& rhs) : func(rhs.func), obj(rhs.obj) {}
    ~Callback() {}

    Callback& operator=(NullCallback)
        { obj = 0; func = 0; return *this; }
    Callback& operator=(const Callback& rhs)
        { obj = rhs.obj; func = rhs.func; return *this; }

    inline R operator()(Args... arg) const
    {
        return (*func)(obj, arg...);
    }

private:
    typedef const void* Callback::*SafeBoolType;
public:
    inline operator SafeBoolType() const
        { return func != 0 ? &Callback::obj : 0; }
    inline bool operator!() const
        { return func == 0; }

private:
    typedef R (*FuncType)(const void*, Args...);
    Callback(FuncType f, const void* o) : func(f), obj(o) {}

private:
    FuncType func;
    const void* obj;

    template<typename FR, typename ...FA>
    friend class FreeCallbackFactory;

    template<typename FR, typename FT, typename ...FA>
    friend class MemberCallbackFactory;

    template<typename FR, typename FT, typename ...FA>
    friend class ConstMemberCallbackFactory;
};

template<typename R, typename... ARGS>
void operator==(const Callback<R (ARGS...)>&,
                const Callback<R (ARGS...)>&);
template<typename R, typename... ARGS>
void operator!=(const Callback<R (ARGS...)>&,
                const Callback<R (ARGS...)>&);

template<typename R, typename... ARGS>
class FreeCallbackFactory
{
private:
    template<R (*Func)(ARGS...)>
    static R Wrapper(const void*, ARGS... args)
    {
        return (*Func)(args...);
    }

public:
    template<R (*Func)(ARGS...)>
    inline static Callback<R (ARGS...)> Bind()
    {
        return Callback<R (ARGS...)>
            (&FreeCallbackFactory::Wrapper<Func>, 0);
    }
};

template<typename R, typename... ARGS>
inline FreeCallbackFactory<R, ARGS...>
GetCallbackFactory(R (*)(ARGS...))
{
    return FreeCallbackFactory<R, ARGS...>();
}

template<typename R, class T, typename... ARGS>
class MemberCallbackFactory
{
private:
    template<R (T::*Func)(ARGS...)>
    static R Wrapper(const void* o, ARGS... args)
    {
        T* obj = const_cast<T*>(static_cast<const T*>(o));
        return (obj->*Func)(args...);
    }

public:
    template<R (T::*Func)(ARGS...)>
    inline static Callback<R (ARGS...)> Bind(T* o)
    {
        return Callback<R (ARGS...)>
        	(&MemberCallbackFactory::Wrapper<Func>,
            static_cast<const void*>(o));
    }
};

template<typename R, class T, typename... ARGS>
inline MemberCallbackFactory<R, T, ARGS...>
GetCallbackFactory(R (T::*)(ARGS...))
{
    return MemberCallbackFactory<R, T, ARGS...>();
}

template<typename R, class T, typename... ARGS>
class ConstMemberCallbackFactory
{
private:
    template<R (T::*Func)(ARGS...) const>
    static R Wrapper(const void* o, ARGS... args)
    {
        const T* obj = static_cast<const T*>(o);
        return (obj->*Func)(args...);
    }

public:
    template<R (T::*Func)(ARGS...) const>
    inline static Callback<R (ARGS...)> Bind(const T* o)
    {
        return Callback<R (ARGS...)>
            (&ConstMemberCallbackFactory::Wrapper<Func>,
            static_cast<const void*>(o));
    }
};

template<typename R, class T, typename... ARGS>
inline ConstMemberCallbackFactory<R, T, ARGS...>
GetCallbackFactory(R (T::*)(ARGS...) const)
{
    return ConstMemberCallbackFactory<R, T, ARGS...>();
}

}

using callback_detail::Callback;

#endif

