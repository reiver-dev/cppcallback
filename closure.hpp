#ifndef CLOSURE_HPP_
#define CLOSURE_HPP_

#include <type_traits>
#include <cstdint>

#define CB_FORWARD(type, obj) static_cast<type &&>(obj)
#define CB_RR(T) std::remove_reference<T>::type

namespace CB {
namespace detail {

struct Vtable {
    void (*destroy)(void *);
    void (*copy)(const void *, uint8_t *);
    void (*move)(void *, uint8_t *) noexcept;
};


template<typename FUNCTYPE>
struct ClosureFactory {

    typedef typename CB_RR(FUNCTYPE) FUNC;

    static void copy(const void *src, uint8_t *dst) {
        new (dst) FUNC(*(const FUNC*) src);
    }

    static void move(void *src, uint8_t *dst) noexcept {
        new (dst) FUNC(CB_FORWARD(FUNC, (*(FUNC*) src)));
    }

    static void destroy(void *obj) noexcept {
        static_cast<FUNC*>(obj)->~FUNC();
    }

};

template<typename FUNCTYPE, typename R, typename... ARGS>
R call_wrapper(void *obj, ARGS... args) {
    typedef typename CB_RR(FUNCTYPE) FUNC;
    return static_cast<FUNC *>(obj)->operator()(CB_FORWARD(ARGS, args)...);
}


template<typename FUNCTYPE>
const struct Vtable* get_vtable() {
    static const Vtable vt = {
        ClosureFactory<FUNCTYPE>::destroy,
        ClosureFactory<FUNCTYPE>::copy,
        ClosureFactory<FUNCTYPE>::move
    };
    return &vt;
}


template<typename Signature, size_t SIZE>
class StaticClosure;


template<size_t SIZE, typename R, typename ... ARGS>
class StaticClosure<R(ARGS...), SIZE> {
public:

    typedef StaticClosure<R(ARGS...), SIZE> this_type;
    typedef R (sig)(ARGS...);

    static const size_t Size = SIZE;

    StaticClosure() noexcept : m_function(nullptr), m_vtable(nullptr) {
        //
    }

    StaticClosure(std::nullptr_t) noexcept : m_function(nullptr), m_vtable(nullptr) {
        //
    }

    template<typename FUNCTYPE>
    explicit StaticClosure(FUNCTYPE&& f) {
        static_assert(std::alignment_of<decltype(f)>::value <= SIZE, "Functor size exceeds closure storage");
        static_assert(!std::is_same<FUNCTYPE, this_type>::value, "Inefficient constructor called");

        new (m_object) typename CB_RR(FUNCTYPE)(CB_FORWARD(FUNCTYPE, f));
        m_vtable = get_vtable<FUNCTYPE>();
        m_function = call_wrapper<FUNCTYPE, R, ARGS...>;
    }

    ~StaticClosure() {
        if (m_vtable) {
            m_vtable->destroy(m_object);
            m_vtable = nullptr;
        }
    }

    template<size_t SZ>
    StaticClosure(const StaticClosure<sig, SZ> &other) :
            m_function(other.m_function),
            m_vtable(other.m_vtable) {

        static_assert(SZ <= this_type::Size, "Right closure is bigger");
        m_vtable->copy(other.m_object, m_object);

    }

    template<size_t SZ>
    StaticClosure(StaticClosure<sig, SZ> &&other) noexcept :
            m_function(other.m_function), m_vtable(other.m_vtable) {

        static_assert(SZ <= this_type::Size, "Right closure is bigger");
        m_vtable->move(other.m_object, this->m_object);
    }

    template<size_t SZ>
    this_type& operator=(const StaticClosure<sig, SZ> &other) {
        static_assert(SZ <= this_type::Size, "Right closure is bigger");

        if ((uintptr_t)this == (uintptr_t)&other)
            return *this;

        if (m_vtable) {

            uint8_t tmp[SIZE];
            other.m_vtable->copy(other.m_object, tmp);
            m_vtable->destroy(m_object);
            other.m_vtable->move(tmp, m_object);

        } else {

            other.m_vtable->copy(other.m_object, m_object);

        }

        m_vtable = other.m_vtable;
        m_function = other.m_function;

        return *this;
    }

    template<size_t SZ>
    this_type& operator=(StaticClosure<sig, SZ> &&other) noexcept {
        static_assert(SZ <= this_type::Size, "Right closure is bigger");

        if ((uintptr_t)this == (uintptr_t)&other)
            return *this;

        if (m_vtable) {
            m_vtable->destroy(m_object);
        }

        other.m_vtable->move(other.m_object, m_object);

        m_vtable = other.m_vtable;
        m_function = other.m_function;

        return *this;
    }

    template <typename FUNCTYPE>
    void assign(FUNCTYPE &&f) {
        static_assert(sizeof(f) <= SIZE, "Functor size exceeds closure storage");

        if (m_vtable)
            m_vtable->destroy();

        new (m_object) typename ClosureFactory<FUNCTYPE>::FUNC(std::forward<FUNCTYPE>(f));
        m_vtable = get_vtable<FUNCTYPE>();
        m_function = call_wrapper<FUNCTYPE, R, ARGS...>;
    }

    void destroy() {
        m_vtable->destroy(m_object);
        m_vtable = nullptr;
    }

    explicit operator bool() const {
        return m_vtable != 0 ? true : false;
    }

    R operator()(ARGS ... args) const{
        return m_function((void*)&m_object, args...);
    }

private:

    typedef R (*function)(void *, ARGS...);

    function m_function;
    Vtable const *m_vtable;
    uint8_t m_object[SIZE];

    template<typename SIG, size_t SZ>
    friend class StaticClosure;

};

}

using detail::StaticClosure;

}

#endif /* CLOSURE_HPP_ */
