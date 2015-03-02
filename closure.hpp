#ifndef CLOSURE_HPP_
#define CLOSURE_HPP_

#define CB_FORWARD(type, obj) static_cast<type &&>(obj)

namespace CB {
namespace detail {

using size_t = decltype(sizeof(void *));
using data_t = unsigned char;
using nullptr_t = decltype(nullptr);

struct __alloc_t {};
extern const __alloc_t __alloc;

}
}

inline void* operator new(CB::detail::size_t,
                          CB::detail::__alloc_t, void *p) noexcept
{
    return p;
}

namespace CB
{
namespace detail
{

struct true_type {
    static const bool value = true;
};

struct false_type {
    static const bool value = false;
};

template<typename, typename>
struct is_same : public false_type {};

template<typename T>
struct is_same<T, T> : public true_type {};

/// Used to conditionnaly switch functions
template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { using type = T; };

template<bool B, class T = void>
using enable_if_t = typename enable_if<B,T>::type;


/// Remove ref or rvalue ref
/// Essential to get to original callable type
template<typename T>
struct remove_reference {
    using type = T;
};

template<typename T>
struct remove_reference<T&> {
    using type = T;
};

template<typename T>
struct remove_reference<T&&> {
    using type = T;
};

template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

template<typename T>
T __declval();

struct check_signature_t {

    template<typename FUNC, typename R, typename... ARGS,
             typename = decltype(static_cast<R>(__declval<FUNC>()(__declval<ARGS>()...)))>
    static true_type __test(nullptr_t);

    template<typename, typename...>
    static false_type __test(...);

};

template<typename FUNC, typename R, typename... ARGS>
struct check_signature {
    static const bool value =
        decltype(check_signature_t::__test<FUNC, R, ARGS...>(nullptr))::value;
};


/// Generated virtual table (to escape RTTI)
struct Vtable {
    void (*destroy)(void *) noexcept;
    void (*copy)(const void *, data_t *);
    void (*move)(void *, data_t *) noexcept;
};

template<typename FUNC>
struct ClosureFactory {

    static void copy(const void *src, data_t *dst)
    {
        new (__alloc, dst) FUNC(*static_cast<const FUNC*>(src));
    }

    static void move(void *src, data_t *dst) noexcept
    {
        new (__alloc, dst) FUNC(CB_FORWARD(FUNC, *static_cast<FUNC*>(src)));
    }

    static void destroy(void *obj) noexcept
    {
        static_cast<FUNC*>(obj)->~FUNC();
    }

};

template<typename FUNC, typename R, typename... ARGS>
R call_wrapper(void *obj, ARGS... args)
{
    return static_cast<FUNC *>(obj)->operator()(CB_FORWARD(ARGS, args)...);
}

template<typename FUNC>
const struct Vtable* get_vtable()
{
    static const Vtable vt = {
        ClosureFactory<FUNC>::destroy,
        ClosureFactory<FUNC>::copy,
        ClosureFactory<FUNC>::move
    };

    return &vt;
}

/// Type-Erasure container for callable with fixed size
template<typename Signature, size_t SIZE>
class StaticClosure;

/// Treat different size for SC as same type
/// Should be just is_same partial specialization
/// but it confuses GCC (4.9.2)
template<typename, typename>
struct is_same_sc : public false_type {};

template<typename sig, size_t S1, size_t S2>
struct is_same_sc<StaticClosure<sig, S1>, StaticClosure<sig, S2>>
    : public true_type {};

template<size_t SIZE, typename R, typename ... ARGS>
class StaticClosure<R(ARGS...), SIZE> {
public:

    using sig_type = R(ARGS...);
    using this_type = StaticClosure<sig_type, SIZE>;

    static const size_t DATA_SIZE = SIZE;

    StaticClosure() noexcept
        : m_function(nullptr)
        , m_vtable(nullptr)
    {
        //
    }

    StaticClosure(nullptr_t) noexcept
        : m_function(nullptr)
        , m_vtable(nullptr)
    {
        //
    }

    template<
        typename FUNCTYPE,
        typename = enable_if_t<!is_same<remove_reference_t<FUNCTYPE>, this_type>::value
                               && !is_same_sc<remove_reference_t<FUNCTYPE>, this_type>::value>
        >
    explicit StaticClosure(FUNCTYPE&& f)
    {

        using FUNC = remove_reference_t<FUNCTYPE>;

        static_assert(check_signature<FUNC, R, ARGS...>::value, "Signature does not match");
        static_assert(!is_same<FUNC, this_type>::value, "Inefficient constructor called");
        static_assert(sizeof(FUNC) <= SIZE && alignof(FUNC) <= SIZE, "Functor size exceeds closure storage");

        new (__alloc, m_object) FUNC(CB_FORWARD(FUNCTYPE, f));

        m_vtable = get_vtable<FUNC>();
        m_function = call_wrapper<FUNC, R, ARGS...>;
    }

    ~StaticClosure()
    {
        if (m_vtable) {
            m_vtable->destroy(m_object);
            m_vtable = nullptr;
        }
    }

    StaticClosure(const this_type &other)
        : m_function(other.m_function)
        , m_vtable(other.m_vtable)
    {
        m_vtable->copy(other.m_object, m_object);
    }

    StaticClosure(this_type &&other) noexcept
        : m_function(other.m_function)
        , m_vtable(other.m_vtable)
    {
        m_vtable->move(other.m_object, this->m_object);
    }

    template<size_t SZ>
    StaticClosure(const StaticClosure<sig_type, SZ> &other)
        : m_function(other.m_function)
        , m_vtable(other.m_vtable)
    {
        static_assert(SZ <= SIZE, "Parent closure is bigger");
        m_vtable->copy(other.m_object, m_object);
    }

    template<size_t SZ>
    StaticClosure(StaticClosure<sig_type, SZ> &&other) noexcept
        : m_function(other.m_function)
        , m_vtable(other.m_vtable)
    {
        static_assert(SZ <= SIZE, "Parent closure is bigger");
        m_vtable->move(other.m_object, this->m_object);
    }

    this_type& operator=(const this_type &other)
    {
        from(CB_FORWARD(decltype(other), other));
        return *this;
    }

    this_type& operator=(this_type &&other)
    {
        from(CB_FORWARD(decltype(other), other));
        return *this;
    }

    template<size_t SZ>
    this_type& operator=(const StaticClosure<sig_type, SZ> &other)
    {
        from(CB_FORWARD(decltype(other), other));
        return *this;
    }

    template<size_t SZ>
    this_type& operator=(StaticClosure<sig_type, SZ> &&other) noexcept
    {
        from(CB_FORWARD(decltype(other), other));
        return *this;
    }

    template <typename FUNCTYPE>
    void assign(FUNCTYPE &&f) {
        using FUNC = remove_reference_t<FUNCTYPE>;

        static_assert(check_signature<FUNC, R, ARGS...>::value, "Signature does not match");
        static_assert(sizeof(FUNC) <= SIZE && alignof(FUNC) <= SIZE, "Callable size exceeds closure storage");

        if (m_vtable)
            m_vtable->destroy(m_object);

        new (__alloc, m_object) FUNC(CB_FORWARD(FUNCTYPE, f));

        m_vtable = get_vtable<FUNC>();
        m_function = call_wrapper<FUNC, R, ARGS...>;
    }

    template<size_t SZ>
    void from(const StaticClosure<sig_type, SZ> &other)
    {
        static_assert(SZ <= SIZE, "Parent closure is bigger");

        if (static_cast<const void *>(this)
            == static_cast<const void *>(&other))
            return;

        if (m_vtable) {
            data_t tmp[SIZE];
            other.m_vtable->copy(other.m_object, tmp);
            m_vtable->destroy(m_object);
            other.m_vtable->move(tmp, m_object);

        } else {
            other.m_vtable->copy(other.m_object, m_object);
        }

        m_vtable = other.m_vtable;
        m_function = other.m_function;
    }

    template<size_t SZ>
    void from(StaticClosure<sig_type, SZ> &&other)
    {
        static_assert(SZ <= SIZE, "Parent closure is bigger");

        if (static_cast<const void *>(this)
            == static_cast<const void *>(&other))
            return;

        if (m_vtable)
            m_vtable->destroy(m_object);

        other.m_vtable->move(other.m_object, m_object);
        m_vtable = other.m_vtable;
        m_function = other.m_function;
    }

    void destroy() {
        if (m_vtable) {
            m_vtable->destroy(m_object);
            m_vtable = nullptr;
        }
    }

    explicit operator bool() const {
        return m_vtable != 0 ? true : false;
    }

    R operator()(ARGS ... args) const {
        return m_function((void*)&m_object, args...);
    }

    R operator()(ARGS ... args) const volatile {
        return m_function((void*)&m_object, args...);
    }

private:
    using func_t = R (*)(void *, ARGS...);

    func_t m_function;
    Vtable const *m_vtable;
    data_t m_object[SIZE];

    template<typename SIG, size_t SZ>
    friend class StaticClosure;
};

}

using detail::StaticClosure;

}

#undef CB_FORWARD

#endif /* CLOSURE_HPP_ */
