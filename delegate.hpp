#ifndef DELEGATE_HPP
#define DELEGATE_HPP

#define CB_FORWARD(type, obj) static_cast<type &&>(obj)

namespace CB {
namespace detail {

/// Simple wrapper to object and his member function

template<typename>
class Delegate;

template<typename R, typename... ARGS>
class Delegate<R (ARGS...)> {
public:

    using sig_type = R (ARGS...);
    using this_type = Delegate<sig_type>;
    using func_t = R (*)(void *, ARGS...);

    Delegate()
        : m_obj(0)
        , m_func(0)
    {
        //
    }

    Delegate(const this_type &o)
        : m_obj(o.m_obj)
        , m_func(o.m_func)
    {
        //
    }

    this_type& operator=(const this_type &o)
    {
        m_obj = o.m_obj;
        m_func = o.m_func;
        return *this;
    }

    explicit operator bool()
    {
        return m_func != 0 ? m_obj : 0;
    }

    R operator()(ARGS... args) const {
        return m_func(m_obj, args...);
    }

    R operator()(ARGS... args) const volatile {
        return m_func(m_obj, args...);
    }

private:

    void *m_obj;
    func_t m_func;

    Delegate(void *obj, func_t func)
        : m_obj(obj)
        , m_func(func)
    {
        //
    }


    template<typename _T, typename _R, typename... _ARGS>
    friend struct DelegateWrapper;

    template<typename _T, typename _R, typename... _ARGS>
    friend struct DelegateConstWrapper;

    template<typename _R, typename... _ARGS>
    friend struct DelegateFFWrapper;

};


template<typename T, typename R, typename... ARGS>
struct DelegateWrapper {

    using func_t = R (*)(void *, ARGS...);
    using delegate_t = Delegate<R (ARGS...)>;

    template<R (T::*MFUN)(ARGS...)>
    static delegate_t bind(T *obj)
    {
        struct _ {
            static R wrapper(void *obj, ARGS... args)
            {
                return (static_cast<T *>(obj)->*MFUN)(CB_FORWARD(ARGS, args)...);
            }
        };
        return delegate_t(obj, _::wrapper);
    }

};

template<typename T, typename R, typename... ARGS>
struct DelegateConstWrapper {

    using func_t = R (*)(void *, ARGS...);
    using delegate_t = Delegate<R (ARGS...)>;

    template<R (T::*MFUN)(ARGS...) const >
    static delegate_t bind(const T *const obj)
    {
        struct _ {
            static R wrapper(void *obj, ARGS... args)
            {
                return (static_cast<const T *const>(obj)->*MFUN)(CB_FORWARD(ARGS, args)...);
            }
        };
        return delegate_t(const_cast<T *>(obj), _::wrapper);
    }
};

template<typename R, typename... ARGS>
struct DelegateFFWrapper {

    using func_t = R (*)(void *, ARGS...);
    using delegate_t = Delegate<R (ARGS...)>;

    template<R (*FUNC)(ARGS...)>
    static delegate_t bind()
    {
        struct _ {
            static R wrapper(void *, ARGS... args)
            {
                return FUNC(CB_FORWARD(ARGS, args)...);
            }
        };
        return delegate_t(nullptr, _::wrapper);
    }
};

template<class T, typename R, typename ... ARGS>
DelegateWrapper<T, R, ARGS...> create_delegate(R (T::*)(ARGS...));

template<class T, typename R, typename ... ARGS>
DelegateConstWrapper<T, R, ARGS...> create_delegate(R (T::*)(ARGS...) const);

template<typename R, typename ... ARGS>
DelegateFFWrapper<R, ARGS...> create_delegate(R (*)(ARGS...));

}

using detail::Delegate;

}

#undef CB_FORWARD

#define __CB_DELEGATE_INIT(instance, func) decltype(CB::detail::create_delegate(func))::bind<func>(instance)
#define __CB_DELEGATE_INIT_FF(func) decltype(CB::detail::create_delegate(func))::bind<func>()
#define CB_DELEGATE_INIT __CB_DELEGATE_INIT
#define CB_DELEGATE_FF __CB_DELEGATE_INIT_FF

#endif /* DELEGATE_HPP */
